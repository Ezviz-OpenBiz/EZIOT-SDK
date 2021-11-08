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
#include <string.h>
#include <stdlib.h>
#include "dna_inc/dna_sockets.h"

typedef struct 
{
	int socket_fd;
}dna_net_work;

void *hal_net_tcp_create(char* nic_name)
{
    dna_net_work* dna_net_work_p = NULL;
    
	dna_net_work_p = (dna_net_work*)malloc(sizeof(dna_net_work));
	if (dna_net_work_p == NULL)
	{
        goto exit;
	}
    
	dna_net_work_p->socket_fd = dna_socket(DNA_AF_INET, DNA_SOCK_STREAM, 0);
	if (dna_net_work_p->socket_fd == -1)
	{
		free(dna_net_work_p);
		dna_net_work_p = NULL;
		goto exit;
	}
    
exit:
	return (void*)dna_net_work_p;
}

int32_t hal_net_tcp_connect(void *net_work, const char *svr_name, int32_t svr_port, int32_t timeout_ms, int8_t real_ip[128])
{
    int return_value = 0;
	struct dna_hostent   * hostent;
	struct dna_sockaddr_in   saddr = {0};
    
	struct fd_set cnt_fdset;
	struct timeval cnt_timeout;
    char* szParseIp = NULL;

	dna_net_work* dna_net_work_p = (dna_net_work*)net_work;
    
	if (NULL == dna_net_work_p)
	{
		return_value = net_tcp_unkown;
		goto exit;
	}

    hostent = dna_gethostbyname(svr_name);
	if (NULL == hostent)
	{
		return_value = net_tcp_dns;
		goto exit;
	}

	saddr.sin_addr.s_addr = ((struct dna_in_addr *)(hostent->h_addr))->s_addr;    
    saddr.sin_family    = DNA_AF_INET;
    saddr.sin_port      = dna_htons(svr_port);
    
    szParseIp = dna_inet_ntoa((const dna_in_addr_t*)&saddr.sin_addr);
	if (NULL == szParseIp)
	{
		return_value = net_tcp_dns;
		goto exit;
	}
	
	if (strlen(szParseIp) >= 128)
	{
		strncpy(real_ip, szParseIp, 128 - 1);
	}
	else
	{
		strncpy(real_ip, szParseIp, strlen(szParseIp));
	}

    if( dna_connect(dna_net_work_p->socket_fd, (struct dna_sockaddr *)&saddr, sizeof(saddr)) != 0)
    {
        //		 os_printf("unable to connect to remote host %s:%d errno=%d", server_ip, server_port, errno);
        return_value = net_tcp_connect;
        goto exit;
    }
    
    FD_ZERO(&cnt_fdset); 
    FD_SET(dna_net_work_p->socket_fd, &cnt_fdset); 
    
    cnt_timeout.tv_sec = timeout_ms/1000; 
    cnt_timeout.tv_usec = (timeout_ms%1000)*1000;
    
    return_value = dna_select(dna_net_work_p->socket_fd + 1, 0, &cnt_fdset, 0, &cnt_timeout);
    if (return_value <= 0)
    {
        //		 os_printf("select %s:%d returnvalue=%d ,error=%d", server_ip, server_port, return_value, errno);
        return_value = net_tcp_socket_timeout;
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
	int return_value = 0;
	dna_timeval_t tmval = {read_timeout_ms/1000, (read_timeout_ms%1000)*1000};
    dna_net_work* dna_net_work_p = NULL;
	
    if (read_buf == NULL || read_buf_maxsize == 0) {
        goto exit;
    }
    
	dna_net_work_p = (dna_net_work*)net_work;
	if (NULL == dna_net_work_p)
	{
		return_value = net_tcp_unkown;
		goto exit;
	}
    
	do
	{
		FD_ZERO(&read_fd);
		FD_SET(dna_net_work_p->socket_fd, &read_fd);
		ret = dna_select(dna_net_work_p->socket_fd + 1, &read_fd, NULL, NULL, &tmval);
		if (ret < 0)
		{
			//socket error
			return net_tcp_socket_err;
		}
		else if (ret == 0)
		{
			//timeout
			return net_tcp_socket_timeout;
		}

		rev_size = dna_read(dna_net_work_p->socket_fd, read_buf + rev_total_size, read_buf_maxsize - rev_total_size);
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
        
	}while(rev_total_size < read_buf_maxsize);
    
    //	os_printf("net_read fd:%d end result:%d rev_total_size:%d , read_buf_maxsize:%d\n", dna_net_work_p->socket_fd, return_value, rev_total_size, read_buf_maxsize);
exit:
	return return_value;
}

int32_t hal_net_tcp_write(void *net_work, uint8_t *write_buf, int32_t write_buf_size, int32_t send_timeout_ms, int32_t *real_write_buf_size)
{
    int ret = 0;
	int send_size = 0;
	int send_total_size = 0;
	int return_value = 0;
    
	fd_set write_fd;
	struct timeval tmval = {send_timeout_ms/1000, (send_timeout_ms%1000)*1000};
    
	dna_net_work* dna_net_work_p = (dna_net_work*)net_work;
	if (NULL == dna_net_work_p || NULL == real_write_buf_size)
	{
		return_value = net_tcp_unkown;
		goto exit;
	}
	
	do 
	{
		FD_ZERO(&write_fd);
		FD_SET(dna_net_work_p->socket_fd, &write_fd);
        
		ret = dna_select(dna_net_work_p->socket_fd + 1,NULL , &write_fd, NULL, &tmval);
		if (ret < 0)
		{
			//socket error
			return net_tcp_socket_err;
		}
		else if (ret == 0)
		{
			//timeout
			return net_tcp_socket_timeout;
		}
        
		send_size = dna_write(dna_net_work_p->socket_fd, write_buf + send_total_size, write_buf_size - send_total_size);
		if (send_size == -1)
		{
			//socket error
			return net_tcp_socket_err;
		}
        // 		else if (send_size <  write_buf_size - send_total_size)
        // 		{
        // 			//send buf full
        // 			return ezdev_sdk_kernel_net_send_buf_full;
        // 		}
        
		*real_write_buf_size = send_size;

	} while(0);
	
//	os_printf("net_read fd:%d end result:%d send_total_size:%d , write_buf_size:%d\n", dna_net_work_p->socket_fd, return_value, send_total_size, write_buf_size);
exit:
	return return_value;
}

void hal_net_tcp_disconnect(void *net_work)
{
    dna_net_work* dna_net_work_p = (dna_net_work*)net_work;
	if (NULL == dna_net_work_p)
	{
		return;
	}
    
	dna_close(dna_net_work_p->socket_fd);
	dna_net_work_p->socket_fd = -1;
}

void hal_net_tcp_destroy(void *net_work)
{
    dna_net_work* dna_net_work_p = (dna_net_work*)net_work;
	if (NULL == dna_net_work_p)
	{
		return;
	}
	free(dna_net_work_p);
}

static void linuxsocket_setnonblock(int socket_fd)
{
    int flag = dna_fcntl(socket_fd, DNA_F_SETFL, 0);
	if (flag == -1)
	{
		return;
	}
	if (dna_fcntl(socket_fd, DNA_F_SETFL, (flag | DNA_O_NONBLOCK)) == -1)
	{
		return;
	}
	return;
}