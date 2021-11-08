// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string.h>

#include "ezconfig.h"
#include "mcuconfig.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <assert.h>



#include "http_server.h"
#include "httpd_priv.h"
#include "ctrl_sock.h"
#include "ez_iot_log.h"
#include "ez_iot_errno.h"

static const char *TAG = "httpd";

static ez_err_e httpd_accept_conn(struct httpd_data *hd, int listen_fd)
{
    /* If no space is available for new session, close the least recently used one */
    if (hd->config.lru_purge_enable == true)
    {
        if (!httpd_is_sess_available(hd))
        {
            /* Queue asynchronous closure of the least recently used session */
            return httpd_sess_close_lru(hd);
            /* Returning from this allowes the main server thread to process
             * the queued asynchronous control message for closing LRU session.
             * Since connection request hasn't been addressed yet using accept()
             * therefore httpd_accept_conn() will be called again, but this time
             * with space available for one session
             */
        }
    }

    struct sockaddr_in addr_from;
    socklen_t addr_from_len = sizeof(addr_from);
    int new_fd = accept(listen_fd, (struct sockaddr *)&addr_from, &addr_from_len);
    if (new_fd < 0)
    {
        ez_log_w(TAG, LOG_FMT("error in accept (%d)"), errno);
        return ez_errno_fail;
    }
    ez_log_v(TAG, LOG_FMT("newfd = %d"), new_fd);

    struct timeval tv;
    /* Set recv timeout of this fd as per config */
    tv.tv_sec = hd->config.recv_wait_timeout;
    tv.tv_usec = 0;
    setsockopt(new_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

    /* Set send timeout of this fd as per config */
    tv.tv_sec = hd->config.send_wait_timeout;
    tv.tv_usec = 0;
    setsockopt(new_fd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof(tv));

    if (ez_errno_succ != httpd_sess_new(hd, new_fd))
    {
        ez_log_w(TAG, LOG_FMT("session creation failed"));
        close(new_fd);
        return ez_errno_fail;
    }
    ez_log_v(TAG, LOG_FMT("complete"));
    return ez_errno_succ;
}

struct httpd_ctrl_data
{
    enum httpd_ctrl_msg
    {
        HTTPD_CTRL_SHUTDOWN,
        HTTPD_CTRL_WORK,
    } hc_msg;
    httpd_work_fn_t hc_work;
    void *hc_work_arg;
};

ez_err_e httpd_queue_work(httpd_handle_t handle, httpd_work_fn_t work, void *arg)
{
    if (handle == NULL || work == NULL)
    {
        return ez_errno_param_invalid;
    }

    struct httpd_data *hd = (struct httpd_data *)handle;
    struct httpd_ctrl_data msg = {
        .hc_msg = HTTPD_CTRL_WORK,
        .hc_work = work,
        .hc_work_arg = arg,
    };

    int ret = cs_send_to_ctrl_sock(hd->msg_fd, hd->config.ctrl_port, &msg, sizeof(msg));
    if (ret < 0)
    {
        ez_log_w(TAG, LOG_FMT("failed to queue work"));
        return ez_errno_fail;
    }

    return ez_errno_succ;
}

void *httpd_get_global_user_ctx(httpd_handle_t handle)
{
    return ((struct httpd_data *)handle)->config.global_user_ctx;
}

void *httpd_get_global_transport_ctx(httpd_handle_t handle)
{
    return ((struct httpd_data *)handle)->config.global_transport_ctx;
}

static void httpd_close_all_sessions(struct httpd_data *hd)
{
    int fd = -1;
    while ((fd = httpd_sess_iterate(hd, fd)) != -1)
    {
        ez_log_v(TAG, LOG_FMT("cleaning up socket %d"), fd);
        httpd_sess_delete(hd, fd);
        close(fd);
    }
}

static void httpd_process_ctrl_msg(struct httpd_data *hd)
{
    struct httpd_ctrl_data msg;
    int ret = recv(hd->ctrl_fd, &msg, sizeof(msg), 0);
    if (ret <= 0)
    {
        ez_log_w(TAG, LOG_FMT("error in recv (%d)"), errno);
        return;
    }
    if (ret != sizeof(msg))
    {
        ez_log_w(TAG, LOG_FMT("incomplete msg"));
        return;
    }

    switch (msg.hc_msg)
    {
    case HTTPD_CTRL_WORK:
        if (msg.hc_work)
        {
            ez_log_v(TAG, LOG_FMT("work"));
            (*msg.hc_work)(msg.hc_work_arg);
        }
        break;
    case HTTPD_CTRL_SHUTDOWN:
        ez_log_v(TAG, LOG_FMT("shutdown"));
        hd->hd_td.status = THREAD_STOPPING;
        break;
    default:
        break;
    }
}

/* Manage in-coming connection or data requests */
static ez_err_e httpd_server(struct httpd_data *hd)
{
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(hd->listen_fd, &read_set);
    FD_SET(hd->ctrl_fd, &read_set);

    int tmp_max_fd;
    httpd_sess_set_descriptors(hd, &read_set, &tmp_max_fd);
    int maxfd = MAX(hd->listen_fd, tmp_max_fd);
    tmp_max_fd = maxfd;
    maxfd = MAX(hd->ctrl_fd, tmp_max_fd);

    ez_log_v(TAG, LOG_FMT("doing select maxfd+1 = %d"), maxfd + 1);
    int active_cnt = select(maxfd + 1, &read_set, NULL, NULL, NULL);
    if (active_cnt < 0)
    {
        ez_log_e(TAG, LOG_FMT("error in select (%d)"), errno);
        httpd_sess_delete_invalid(hd);
        return ez_errno_succ;
    }

    /* Case0: Do we have a control message? */
    if (FD_ISSET(hd->ctrl_fd, &read_set))
    {
        ez_log_v(TAG, LOG_FMT("processing ctrl message"));
        httpd_process_ctrl_msg(hd);
        if (hd->hd_td.status == THREAD_STOPPING)
        {
            ez_log_v(TAG, LOG_FMT("stopping thread"));
            return ez_errno_fail;
        }
    }

    /* Case1: Do we have any activity on the current data
     * sessions? */
    int fd = -1;
    while ((fd = httpd_sess_iterate(hd, fd)) != -1)
    {
        if (FD_ISSET(fd, &read_set) || (httpd_sess_pending(hd, fd)))
        {
            ez_log_v(TAG, LOG_FMT("processing socket %d"), fd);
            if (httpd_sess_process(hd, fd) != ez_errno_succ)
            {
                ez_log_v(TAG, LOG_FMT("closing socket %d"), fd);
                close(fd);
                /* Delete session and update fd to that
                 * preceding the one being deleted */
                fd = httpd_sess_delete(hd, fd);
            }
        }
    }

    /* Case2: Do we have any incoming connection requests to
     * process? */
    if (FD_ISSET(hd->listen_fd, &read_set))
    {
        ez_log_v(TAG, LOG_FMT("processing listen socket %d"), hd->listen_fd);
        if (httpd_accept_conn(hd, hd->listen_fd) != ez_errno_succ)
        {
            ez_log_w(TAG, LOG_FMT("error accepting new connection"));
        }
    }
    return ez_errno_succ;
}

/* The main HTTPD thread */
static void httpd_thread(void *arg)
{
    int ret;
    struct httpd_data *hd = (struct httpd_data *)arg;
    hd->hd_td.status = THREAD_RUNNING;

    ez_log_v(TAG, LOG_FMT("web server started"));
    while (1)
    {
        ret = httpd_server(hd);
        if (ret != ez_errno_succ)
        {
            break;
        }
    }

    ez_log_v(TAG, LOG_FMT("web server exiting"));
    close(hd->msg_fd);
    cs_free_ctrl_sock(hd->ctrl_fd);
    httpd_close_all_sessions(hd);
    close(hd->listen_fd);
    hd->hd_td.status = THREAD_STOPPED;
    httpd_os_thread_delete(hd->hd_td.handle);
}

static ez_err_e httpd_server_init(struct httpd_data *hd)
{
#ifdef CONFIG_LWIP_IPV6
    int fd = socket(PF_INET6, SOCK_STREAM, 0);
#else
    int fd = socket(PF_INET, SOCK_STREAM, 0);
#endif /* CONFIG_LWIP_IPV6 */
    if (fd < 0)
    {
        ez_log_e(TAG, LOG_FMT("error in socket (%d)"), errno);
        return ez_errno_fail;
    }

#ifdef CONFIG_LWIP_IPV6
    struct in6_addr inaddr_any = IN6ADDR_ANY_INIT;
    struct sockaddr_in6 serv_addr = {
        .sin6_family = PF_INET6,
        .sin6_addr = inaddr_any,
        .sin6_port = htons(hd->config.server_port)};
#else
    struct sockaddr_in serv_addr = {
        .sin_family = PF_INET,
        .sin_addr = {
            .s_addr = htonl(INADDR_ANY)},
        .sin_port = htons(hd->config.server_port)};
#endif /* CONFIG_LWIP_IPV6 */
    int ret = bind(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret < 0)
    {
        ez_log_e(TAG, LOG_FMT("error in bind (%d)"), errno);
        close(fd);
        return ez_errno_fail;
    }

    ret = listen(fd, hd->config.backlog_conn);
    if (ret < 0)
    {
        ez_log_e(TAG, LOG_FMT("error in listen (%d)"), errno);
        close(fd);
        return ez_errno_fail;
    }

    int ctrl_fd = cs_create_ctrl_sock(hd->config.ctrl_port);
    if (ctrl_fd < 0)
    {
        ez_log_e(TAG, LOG_FMT("error in creating ctrl socket (%d)"), errno);
        close(fd);
        return ez_errno_fail;
    }

    int msg_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (msg_fd < 0)
    {
        ez_log_e(TAG, LOG_FMT("error in creating msg socket (%d)"), errno);
        close(fd);
        close(ctrl_fd);
        return ez_errno_fail;
    }

    hd->listen_fd = fd;
    hd->ctrl_fd = ctrl_fd;
    hd->msg_fd = msg_fd;
    return ez_errno_succ;
}

static struct httpd_data *httpd_create(const httpd_config_t *config)
{
    /* Allocate memory for httpd instance data */
    struct httpd_data *hd = calloc(1, sizeof(struct httpd_data));
    if (hd != NULL)
    {   
        hd->hd_calls = calloc(config->max_uri_handlers, sizeof(httpd_uri_t *));
        if (hd->hd_calls == NULL)
        {
            free(hd);
            return NULL;
        }
        hd->hd_sd = calloc(config->max_open_sockets, sizeof(struct sock_db));
        if (hd->hd_sd == NULL)
        {
            free(hd->hd_calls);
            free(hd);
            return NULL;
        }
        struct httpd_req_aux *ra = &hd->hd_req_aux;
        ra->resp_hdrs = calloc(config->max_resp_headers, sizeof(struct resp_hdr));
        if (ra->resp_hdrs == NULL)
        {
            free(hd->hd_sd);
            free(hd->hd_calls);
            free(hd);
            return NULL;
        }
        /* Save the configuration for this instance */
        hd->config = *config;
    }
    else
    {
        ez_log_e(TAG, "mem alloc failed");
    }
    return hd;
}

static void httpd_delete(struct httpd_data *hd)
{
    struct httpd_req_aux *ra = &hd->hd_req_aux;
    /* Free memory of httpd instance data */
    free(ra->resp_hdrs);
    free(hd->hd_sd);

    /* Free registered URI handlers */
    httpd_unregister_all_uri_handlers(hd);
    free(hd->hd_calls);
    free(hd);
}

ez_err_e httpd_start(httpd_handle_t *handle, const httpd_config_t *config)
{
    if (handle == NULL || config == NULL)
    {
        return ez_errno_param_invalid;
    }

    struct httpd_data *hd = httpd_create(config);
    if (hd == NULL)
    {
        /* Failed to allocate memory */
        
        return ez_errno_httpd_alloc_mem;
    }

    if (httpd_server_init(hd) != ez_errno_succ)
    {
        httpd_delete(hd);
        return ez_errno_fail;
    }

    httpd_sess_init(hd);
    if (httpd_os_thread_create(&hd->hd_td.handle, "httpd",
                               hd->config.stack_size,
                               hd->config.task_priority,
                               httpd_thread, hd) != ez_errno_succ)
    {
        /* Failed to launch task */
        httpd_delete(hd);
       
        return ez_errno_httpd_task;
    }

    *handle = (httpd_handle_t *)hd;
    return ez_errno_succ;
}

ez_err_e httpd_stop(httpd_handle_t handle)
{
    struct httpd_data *hd = (struct httpd_data *)handle;
    if (hd == NULL)
    {
        return ez_errno_param_invalid;
    }

    struct httpd_ctrl_data msg;
    memset(&msg, 0, sizeof(msg));
    msg.hc_msg = HTTPD_CTRL_SHUTDOWN;
    cs_send_to_ctrl_sock(hd->msg_fd, hd->config.ctrl_port, &msg, sizeof(msg));

    ez_log_v(TAG, LOG_FMT("sent control msg to stop server"));
    while (hd->hd_td.status != THREAD_STOPPED)
    {
        httpd_os_thread_sleep(100);
    }

    /* Release global user context, if not NULL */
    if (hd->config.global_user_ctx)
    {
        if (hd->config.global_user_ctx_free_fn)
        {
            hd->config.global_user_ctx_free_fn(hd->config.global_user_ctx);
        }
        else
        {
            free(hd->config.global_user_ctx);
        }
        hd->config.global_user_ctx = NULL;
    }

    /* Release global transport context, if not NULL */
    if (hd->config.global_transport_ctx)
    {
        if (hd->config.global_transport_ctx_free_fn)
        {
            hd->config.global_transport_ctx_free_fn(hd->config.global_transport_ctx);
        }
        else
        {
            free(hd->config.global_transport_ctx);
        }
        hd->config.global_transport_ctx = NULL;
    }

    ez_log_v(TAG, LOG_FMT("server stopped"));
    httpd_delete(hd);
    return ez_errno_succ;
}
