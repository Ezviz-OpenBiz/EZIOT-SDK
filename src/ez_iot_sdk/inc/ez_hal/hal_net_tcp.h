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
 * 2019-11-07     XuRongjun    first version
 *******************************************************************************/

#ifndef _HAL_NET_TCP_H_
#define _HAL_NET_TCP_H_

#include <stdint.h>

typedef enum
{
  net_tcp_succ,
  net_tcp_unkown = 5,
  net_tcp_dns = 8,
  net_tcp_connect = 10,
  net_tcp_socket_err = 15,
  net_tcp_socket_timeout = 16,
  net_tcp_socket_closed = 17
} net_tcp_err_t;

/**
  * @brief 创建TCP连接对象
  * 
  * @param[in] nic_name 网卡名称，用于多张网卡场景。无多张网卡或不需要指定网卡，传入NULL。
  * @return void* 连接句柄，NULL表示失败，!NULL表示成功。
  */
void *hal_net_tcp_create(char *nic_name);

/**
  * @brief 网络连接
  * 
  * @param[in] net_work 连接句柄
  * @param[in] svr_name 域名/IP
  * @param[in] svr_port 端口
  * @param[in] timeout 连接超时时间(毫秒)
  * @param[out] real_ip 域名解析得到的IP
  * @return int32_t 详见net_tcp_err_t
  */
int32_t hal_net_tcp_connect(void *net_work, const char *svr_name, int32_t svr_port, int32_t timeout, int8_t real_ip[64]);

/**
 * @brief 接收数据
 * 
 * @param[in] net_work 连接句柄
 * @param[out] read_buf 数据接收缓冲区
 * @param[in] read_buf_maxsize 待接收数据长度
 * @param[in] read_timeout_ms 超时时间
 * @return int32_t 详见net_tcp_err_t
 */
int32_t hal_net_tcp_read(void *net_work, uint8_t *read_buf, int32_t read_buf_maxsize, int32_t read_timeout_ms);

/**
 * @brief 发送数据
 * 
 * @param[in] net_work 连接句柄
 * @param[in] write_buf 数据发送缓冲区
 * @param[in] write_buf_size 待发送数据长度
 * @param[in] send_timeout_ms 超时时间(毫秒)
 * @param[out] real_write_buf_size 已发送数据长度
 * @return int32_t 详见net_tcp_err_t 
 */
int32_t hal_net_tcp_write(void *net_work, uint8_t *write_buf, int32_t write_buf_size, int32_t send_timeout_ms, int32_t *real_write_buf_size);

/**
 * @brief 断开网络连接
 * 
 * @param[in] net_work 连接句柄
 */
void hal_net_tcp_disconnect(void *net_work);

/**
 * @brief 销毁连接对象
 * 
 * @param[in] net_work 连接句柄
 */
void hal_net_tcp_destroy(void *net_work);

#endif