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
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <poll.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

typedef enum
{
    POLL_RECV = POLLIN,
    POLL_SEND = POLLOUT,
    POLL_ALL = POLLIN | POLLOUT
} POLL_TYPE;

typedef struct
{
    int socket_fd;
} linux_net_work;

static in_addr_t ParseHost(const char *host, char real_ip[128]);
static void linuxsocket_setnonblock(int socket_fd);
static net_tcp_err_t linuxsocket_poll(int socket_fd, POLL_TYPE type, int timeout);

void *hal_net_tcp_create(char* nic_name)
{
    linux_net_work *linuxnet_work = NULL;
    const int opt = 1400;
    int ret = 0;
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));

    linuxnet_work = (linux_net_work *)malloc(sizeof(linux_net_work));
    if (linuxnet_work == NULL)
    {
        return NULL;
    }

    linuxnet_work->socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (linuxnet_work->socket_fd == -1)
    {
        free(linuxnet_work);
        return NULL;
    }

    ret = setsockopt(linuxnet_work->socket_fd, IPPROTO_TCP, TCP_MAXSEG, &opt, sizeof(opt));
    if (ret < 0)
    {
        // printf("set socket opt, TCP_MAXSEG error\n");
    }

    if (nic_name && strlen(nic_name) > 0)
    {
        strncpy(ifr.ifr_name, nic_name, sizeof(ifr.ifr_name) - 1);

        ret = setsockopt(linuxnet_work->socket_fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr));
        if (ret < 0)
        {
            // printf("set socket opt, SO_BINDTODEVICE error\n");
        }
    }

    return (void*)linuxnet_work;
}

int32_t hal_net_tcp_connect(void *net_work, const char *svr_name, int32_t svr_port, int32_t timeout_ms, int8_t real_ip[128])
{
    struct sockaddr_in dst_addr;
    struct pollfd pfd;
    int return_value = 0;
    linux_net_work *linuxnet_work = (linux_net_work *)net_work;

    int socket_err = 0;
    socklen_t socklen = sizeof(socket_err);

    if (NULL == linuxnet_work)
    {
        return net_tcp_unkown;
    }

    linuxsocket_setnonblock(linuxnet_work->socket_fd);

    dst_addr.sin_family = AF_INET;
    dst_addr.sin_addr.s_addr = ParseHost(svr_name, real_ip);
    dst_addr.sin_port = htons(svr_port);
    if (INADDR_NONE == dst_addr.sin_addr.s_addr)
        return net_tcp_dns;

    if (connect(linuxnet_work->socket_fd, (const struct sockaddr *)(&dst_addr), sizeof(dst_addr)) == -1 && errno != EINPROGRESS)
    {
        return net_tcp_connect;
    }

    if (timeout_ms <= 0)
    {
        timeout_ms = -1;
    }

    pfd.fd = linuxnet_work->socket_fd;
    pfd.events = POLLOUT | POLLERR | POLLHUP | POLLNVAL;
    return_value = poll(&pfd, 1, timeout_ms);
    if (return_value < 0)
    {
        //socket error
        return net_tcp_socket_err;
    }
    else if (return_value == 0)
    {
        //timeout
        return net_tcp_socket_timeout;
    }
    if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL))
    {
        return net_tcp_socket_err;
    }

    if (getsockopt(linuxnet_work->socket_fd, SOL_SOCKET, SO_ERROR, &socket_err, (socklen_t *)&socklen) == -1)
    {
        return net_tcp_socket_err;
    }
    //check the SO_ERROR state
    if (socket_err != 0)
    {
        errno = socket_err;
        return net_tcp_socket_err;
    }

    return net_tcp_succ;
}

int32_t hal_net_tcp_read(void *net_work, uint8_t *read_buf, int32_t read_buf_maxsize, int32_t read_timeout_ms)
{
    int rev_size = 0;
    int rev_total_size = 0;
    net_tcp_err_t return_value = 0;

    linux_net_work *linuxnet_work = (linux_net_work *)net_work;
    if (NULL == linuxnet_work)
    {
        return net_tcp_unkown;
    }

    do
    {
        return_value = linuxsocket_poll(linuxnet_work->socket_fd, POLL_RECV, read_timeout_ms);
        if (return_value != net_tcp_succ)
        {
            //socket error  or  socket close
            return return_value;
        }

        rev_size = recv(linuxnet_work->socket_fd, read_buf + rev_total_size, read_buf_maxsize - rev_total_size, 0);
        if (rev_size < 0)
        {
            //socket error
            return net_tcp_socket_err;
        }
        else if (rev_size == 0)
        {
            // socket close
            return net_tcp_socket_closed;
        }
        rev_total_size += rev_size;

    } while (rev_total_size < read_buf_maxsize);

    return net_tcp_succ;
}

int32_t hal_net_tcp_write(void *net_work, uint8_t *write_buf, int32_t write_buf_size, int32_t send_timeout_ms, int32_t *real_write_buf_size)
{
    int send_size = 0;
    int send_total_size = 0;

    net_tcp_err_t return_value = 0;

    linux_net_work *linuxnet_work = (linux_net_work *)net_work;
    if (NULL == linuxnet_work || NULL == real_write_buf_size)
    {
        return net_tcp_unkown;
    }

    do
    {
        return_value = linuxsocket_poll(linuxnet_work->socket_fd, POLL_SEND, send_timeout_ms);
        if (return_value != net_tcp_succ)
        {
            //socket error  or  socket close
            return return_value;
        }

        send_size = send(linuxnet_work->socket_fd, write_buf + send_total_size, write_buf_size - send_total_size, 0);
        if (send_size == -1)
        {
            return net_tcp_socket_err;
        }

        *real_write_buf_size = send_size;

    } while (0);

    return net_tcp_succ;
}

void hal_net_tcp_disconnect(void *net_work)
{
    linux_net_work *linuxnet_work = (linux_net_work *)net_work;
    if (NULL == linuxnet_work)
    {
        return;
    }
    close(linuxnet_work->socket_fd);
    linuxnet_work->socket_fd = 0;
}

void hal_net_tcp_destroy(void *net_work)
{
    linux_net_work *linuxnet_work = (linux_net_work *)net_work;
    if (NULL == linuxnet_work)
    {
        return;
    }
    free(linuxnet_work);
}

static in_addr_t ParseHost(const char *host, char real_ip[128])
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
        strncpy(real_ip, szParseIp, 128 - 1);
    }
    else
    {
        strncpy(real_ip, szParseIp, strlen(szParseIp));
    }

    return ((struct in_addr *)(entry.h_addr))->s_addr;
}

static void linuxsocket_setnonblock(int socket_fd)
{
    int flag = fcntl(socket_fd, F_GETFL);
    if (flag == -1)
    {
        return;
    }
    if (fcntl(socket_fd, F_SETFL, (flag | O_NONBLOCK)) == -1)
    {
        return;
    }
    return;
}

static net_tcp_err_t linuxsocket_poll(int socket_fd, POLL_TYPE type, int timeout)
{
    struct pollfd poll_fd;
    int nfds = 0;

    poll_fd.fd = socket_fd;
    poll_fd.events = type;
    poll_fd.revents = 0;

    if (socket_fd < 0)
    {
        return net_tcp_unkown;
    }

    nfds = poll(&poll_fd, 1, timeout);
    if (nfds < 0)
    {
        //ez_log_d(shadow,"poll error, errno %d\n", errno);
        if (errno == EINTR)
        {
            return net_tcp_succ;
        }
        else
        {
            return net_tcp_socket_err;
        }
    }
    else if (nfds > 0)
    {
        if (poll_fd.revents & type)
        { // prior to check error
            return net_tcp_succ;
        }
        else if (poll_fd.revents & (POLLNVAL | POLLERR | POLLHUP))
        {
            return net_tcp_socket_err;
        }
        else
        {
            return net_tcp_socket_err;
        }
    }
    else
    {
        // timeout
        return net_tcp_socket_timeout;
    }
}