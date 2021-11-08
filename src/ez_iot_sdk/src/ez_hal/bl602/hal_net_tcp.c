/*******************************************************************************
* Copyright © 2017-2021 Ezviz Inc.
*
* All rights reserved. This program and the accompanying materials
* are made available under the terms of the Eclipse Public License v1.0
* and Eclipse Distribution License v1.0 which accompany this distribution.
*
* The Eclipse Public License is available at
*    http://www.eclipse.org/legal/epl-v10.html
* and the Eclipse Distribution License is available at
*   http://www.eclipse.org/org/documents/edl-v10.php.
* 
* Brief:
* 
* 
* Change Logs:
* Date           Author       Notes
* 
*******************************************************************************/
#include "hal_net_tcp.h"
#include <stdlib.h>
#include <string.h>
#include "lwip/netdb.h"
#include "lwip/def.h"
#include "lwip/sockets.h"

typedef struct
{
    int lwip_fd;
} freertos_net_work;

void lwipsocket_setnonblock(int socket_fd)
{
    int flag = lwip_fcntl(socket_fd, F_GETFL, 0);
    if (flag == -1)
    {
        return;
    }

    if (lwip_fcntl(socket_fd, F_SETFL, (flag | O_NONBLOCK)) == -1)
    {
        return;
    }

    return;
}

static in_addr_t ParseHost(const char *host, char szRealIp[128])
{
    if (host == NULL || strlen(host) == 0)
    {
        return htonl(INADDR_ANY);
    }

    char str[128] = {0};
    int ret, herr;
    char buf[1024];
    struct hostent entry, *hp;
    ret = gethostbyname_r(host, &entry, buf, 1024, &hp, &herr);

    if (ret || hp == NULL)
    {
        return INADDR_NONE;
    }

    const char *szParseIp = inet_ntop(entry.h_addrtype, entry.h_addr, str, sizeof(str));
    if (strlen(szParseIp) >= 128)
    {
        strncpy(szRealIp, szParseIp, 128 - 1);
    }
    else
    {
        strncpy(szRealIp, szParseIp, strlen(szParseIp));
    }

    return ((struct in_addr *)(entry.h_addr))->s_addr;
}

void *hal_net_tcp_create(char *nic_name)
{
    freertos_net_work *freertosnet_work = NULL;
    freertosnet_work = (freertos_net_work *)malloc(sizeof(freertos_net_work));

    if (freertosnet_work == NULL)
    {
        goto exit;
    }

    freertosnet_work->lwip_fd = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (freertosnet_work->lwip_fd == -1)
    {
        free(freertosnet_work);
        freertosnet_work = NULL;
        goto exit;
    }

exit:
    return (void *)freertosnet_work;
}

int32_t hal_net_tcp_connect(void *net_work, const char *svr_name, int32_t svr_port, int32_t timeout, int8_t real_ip[64])
{
    int return_value = 0;
    struct sockaddr_in saddr;

    freertos_net_work *freertosnet_work = (freertos_net_work *)net_work;
    if (NULL == freertosnet_work)
    {
        return_value = net_tcp_unkown;
        goto exit;
    }

    //linuxsocket_setnonblock(freertosnet_work->socket_fd);

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = ParseHost(svr_name, (char *)real_ip);
    saddr.sin_port = htons(svr_port);
    int err = connect(freertosnet_work->lwip_fd, (struct sockaddr *)&saddr, sizeof(saddr));
    if (err != 0)
    {
        return_value = net_tcp_connect;
        goto exit;
    }

    return_value = 0;
exit:

    return return_value;
}

int32_t hal_net_tcp_read(void *net_work, uint8_t *read_buf, int32_t read_buf_maxsize, int32_t read_timeout_ms)
{
    int ret = 0;
    int rev_size = 0;
    int rev_total_size = 0;
    struct fd_set read_fd;
    int32_t return_value = 0;

    struct timeval tmval = {read_timeout_ms / 1000, (read_timeout_ms % 1000) * 1000};
    freertos_net_work *freertosnet_work = (freertos_net_work *)net_work;
    if (NULL == freertosnet_work)
    {
        return_value = net_tcp_unkown;
        goto exit;
    }

    do
    {
        FD_ZERO(&read_fd);
        FD_SET(freertosnet_work->lwip_fd, &read_fd);
        ret = select(freertosnet_work->lwip_fd + 1, &read_fd, NULL, NULL, &tmval);
        if (ret < 0)
        {
            return net_tcp_socket_err;
        }
        else if (ret == 0)
        {
            return net_tcp_socket_timeout;
        }

        rev_size = recv(freertosnet_work->lwip_fd, read_buf + rev_total_size, read_buf_maxsize - rev_total_size, 0);
        if (rev_size < 0)
        {
            return net_tcp_socket_err;
        }
        else if (rev_size == 0)
        {
            return net_tcp_socket_closed;
        }

        rev_total_size += rev_size;
    } while (rev_total_size < read_buf_maxsize);

    // os_printf("net_read fd:%d end result:%d rev_total_size:%d , read_buf_maxsize:%d\n", freertosnet_work->lwip_fd, return_value, rev_total_size, read_buf_maxsize);
exit:
    return return_value;
}

int32_t hal_net_tcp_write(void *net_work, uint8_t *write_buf, int32_t write_buf_size, int32_t send_timeout_ms, int32_t *real_write_buf_size)
{
    int ret = 0;
    int send_size = 0;
    int send_total_size = 0;
    int32_t return_value = 0;
    fd_set write_fd;

    struct timeval tmval = {send_timeout_ms / 1000, (send_timeout_ms % 1000) * 1000};
    freertos_net_work *freertosnet_work = (freertos_net_work *)net_work;
    if (NULL == freertosnet_work || NULL == real_write_buf_size)
    {
        return_value = net_tcp_unkown;
        goto exit;
    }

    do
    {
        FD_ZERO(&write_fd);
        FD_SET(freertosnet_work->lwip_fd, &write_fd);
        ret = select(freertosnet_work->lwip_fd + 1, NULL, &write_fd, NULL, &tmval);
        if (ret < 0)
        {
            return net_tcp_socket_err;
        }
        else if (ret == 0)
        {
            return net_tcp_socket_timeout;
        }

        send_size = send(freertosnet_work->lwip_fd, write_buf + send_total_size, write_buf_size - send_total_size, 0);
        if (send_size == -1)
        {
            return net_tcp_socket_err;
        }

        *real_write_buf_size = send_size;
    } while (0);

    // os_printf("net_read fd:%d end result:%d send_total_size:%d , write_buf_size:%d\n", freertosnet_work->lwip_fd, return_value, send_total_size, write_buf_size);
exit:
    return return_value;
}

void hal_net_tcp_disconnect(void *net_work)
{
    freertos_net_work *freertosnet_work = (freertos_net_work *)net_work;
    if (NULL == freertosnet_work)
    {
        return;
    }

    lwip_close(freertosnet_work->lwip_fd);
    freertosnet_work->lwip_fd = -1;
}

void hal_net_tcp_destroy(void *net_work)
{
    freertos_net_work *freertosnet_work = (freertos_net_work *)net_work;
    if (NULL == freertosnet_work)
    {
        return;
    }

    free(freertosnet_work);
}