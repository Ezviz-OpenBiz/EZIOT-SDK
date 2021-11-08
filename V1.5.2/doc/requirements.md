# 萤石IoT-SDK接入要求

## 1. 编译环境要求

1. 支持以工具链形式进行交叉编译

2. 支持ubuntu 18.04 TLS x64编译环境

3. 测试build_demo验证可用，见附件 [build_demo.zip](build_demo)




材料提供清单：

- [ ] 交叉编译工具链及安装说明。
- [ ] 依赖软件离线安装包或在线安装方法。（如果有）
- [ ] 验证通过的测试Demo。


## 2. 资源要求

- 支持多任务：预留给SDK最少4个并发运行任务。
- 单品产品：240K ROM 和 40K RAM
- 网关类产品：260K ROM + 400K可擦写存储 和  440K RAM（按100个子设备估算）

## 3. 适配和移植要求

### 3.1 需支持常用的libc接口

printf、memcpy、memset、strcpy、strncpy、sprintf、snprintf、malloc、free、calloc、zalloc、realloc

### 3.2 芯片适配

萤石自研模组，不需要进行适配。

三方合作模组，需进行适配。当前萤石IoT-SDK已经适配的Linux、FreeRTOS、RT-Thread三个系统（后两者需提供lwip源码）。如对接的芯片是单片机或其他操作系统，用户需自行适配以下接口：

#### 3.2.1 网络接口适配

```c
/**
 * @file hal_net_tcp.h
 * @author xurongjun(xurongjun@ezvizlife.com)
 * @brief 
 * @version 0.1
 * @date 2019-11-07
 * 
 * @copyright Copyright (c) 2019
 * 
 */

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
```



#### 3.2.2 信号量接口适配

```C
/**
 * @file hal_semaphore.h
 * @author xurongjun (xurongjun@hikvison.com.cn)
 * @brief 
 * @version 0.1
 * @date 2020-09-08
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef _HAL_THREAD_SEM_H_
#define _HAL_THREAD_SEM_H_

/**
 * @brief 创建信号量对象
 * 
 * @return void* 信号量句柄，NULL表示失败，!NULL表示成功。
 */
void *hal_semaphore_create(void);

/**
 * @brief 销毁信号量对象
 * 
 * @param[in] ptr_sem 信号量句柄
 */
void hal_semaphore_destroy(void *ptr_sem);

/**
 * @brief 等待信号
 * 
 * @param[in] ptr_sem 信号量句柄
 * @return int 0表示成功，-1表示失败。
 */
int hal_semaphore_wait(void *ptr_sem);

/**
 * @brief 等待信号
 * 
 * @param[in] ptr_sem 信号量句柄
 * @param[in] time_ms 超时时间(毫秒)
 * @return int 0表示成功，-1表示失败。
 */
int hal_semaphore_wait_ms(void *ptr_sem, long time_ms);

/**
 * @brief 发送信号
 * 
 * @param ptr_sem 信号量句柄
 * @return int 0表示成功，-1表示失败。
 */
int hal_semaphore_post(void *ptr_sem);

#endif
```



#### 3.2.3 任务相关接口适配

```c
/**
 * @file hal_thread.h
 * @author xurongjun(xurongjun@ezvizlife.com)
 * @brief 
 * @version 0.1
 * @date 2019-11-07
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef _HAL_THREAD_H_
#define _HAL_THREAD_H_
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (*hal_thread_fun_t)(void *user_data);

    /**
     * @brief 创建任务
     * 
     * @param[in] thread_name 任务名称
     * @param[in] thread_fun 任务函数
     * @param[in] stack_size 任务栈大小
     * @param[in] priority 任务优先级(同各平台保持一致，如freertos参考FreeRTOSConfig.h定义)
     * @param[in] user_data 用户参数
     * @return void* 任务对象句柄，NULL表示失败，!NULL表示成功。
     */
    void *hal_thread_create(int8_t *thread_name, hal_thread_fun_t thread_fun, int32_t stack_size, int32_t priority, void *user_data);

    /**
     * @brief 销毁任务对象，等待任务销毁成功后返回，参考pthread_join。
     * 
     * @param[in] handle 任务对象句柄
     * @return int 0表示成功，-1表示失败。
     */
    int hal_thread_destroy(void *handle);

    /**
     * @brief 分离任务并销毁任务对象，参考pthread_detach。
     * 
     * @param[in] handle 任务对象句柄
     * @return int 0表示成功，-1表示失败。
     */
    int hal_thread_detach(void *handle);

    /**
     * @brief 创建互斥量对象
     * 
     * @return void* 互斥量对象句柄，NULL表示失败，!NULL表示成功。
     */
    void *hal_thread_mutex_create();

    /**
     * @brief 销毁互斥量对象
     * 
     * @param[in] ptr_mutex 互斥量对象句柄
     */
    void hal_thread_mutex_destroy(void *ptr_mutex);

    /**
     * @brief 上锁
     * 
     * @param[in] ptr_mutex 互斥量对象句柄
     * @return int 0表示成功，-1表示失败。
     */
    int hal_thread_mutex_lock(void *ptr_mutex);

    /**
     * @brief 解锁
     * 
     * @param[in] ptr_mutex 互斥量对象句柄
     * @return int 0表示成功，-1表示失败。
     */
    int hal_thread_mutex_unlock(void *ptr_mutex);

    /**
     * @brief 任务休眠
     * 
     * @param[in] time_ms 休眠时间(毫秒)
     */
    void hal_thread_sleep(unsigned int time_ms);

#ifdef __cplusplus
}
#endif

#endif
```



#### 3.2.4 时间相关接口适配

```c
/**
 * @file hal_time.h
 * @author xurongjun(xurongjun@ezvizlife.com)
 * @brief 
 * @version 0.1
 * @date 2019-11-07
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#ifndef _HAL_TIME_H_
#define _HAL_TIME_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 创建定时器对象
     * 
     * @return void* 定时器对象句柄，NULL表示失败，!NULL表示成功。
     */
    void *hal_timer_creat();

    /**
     * @brief 判断是否超时
     * 
     * @param[in] timer 定时器对象句柄
     * @param[in] time_ms 超时范围
     * @return uint8_t 1表示超时，0表示未超时
     */
    uint8_t hal_time_isexpired_bydiff(void *timer, uint32_t time_ms);

    /**
     * @brief 判断是否超时
     * 
     * @param[in] timer 定时器对象句柄
     * @return uint8_t 1表示超时，0表示未超时
     */
    uint8_t hal_time_isexpired(void *timer);

    /**
     * @brief 重置定时器
     * 
     * @param[in] timer 定时器对象句柄
     * @param[in] timeout 预设超时时间(毫秒)
     */
    void hal_time_countdown_ms(void *timer, uint32_t timeout);

    /**
     * @brief 重置定时器
     * 
     * @param[in] time 定时器对象句柄
     * @param[in] timeout 预设超时时间(秒)
     */
    void hal_time_countdown(void *timer, uint32_t timeout);

    /**
     * @brief 获取剩余时间
     * 
     * @param[in] time 定时器对象句柄
     * @return uint32_t 剩余时间
     */
    uint32_t hal_time_left_ms(void *timer);

    /**
     * @brief 销毁定时器
     * 
     * @param[in] timer 定时器对象句柄
     */
    void hal_timer_destroy(void *timer);

#ifdef __cplusplus
}
#endif

#endif
```

#### 3.2.5 Linux 参考实现

- 网络接口示例：
```c
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
```

- 信号量接口示例：
```c
#include "hal_semaphore.h"
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <time.h>
#include <errno.h>

typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int num;
} sdk_sem_platform;

void *hal_semaphore_create(void)
{
    sdk_sem_platform *ptr_sem_platform = NULL;
    ptr_sem_platform = (sdk_sem_platform *)malloc(sizeof(sdk_sem_platform));
    if (ptr_sem_platform == NULL)
    {
        return NULL;
    }

    pthread_condattr_t m_condAttr;
    memset(&m_condAttr, 0, sizeof(pthread_condattr_t));
    pthread_condattr_setclock(&m_condAttr, CLOCK_MONOTONIC);

    pthread_mutex_init(&(ptr_sem_platform->mutex), NULL);
    pthread_cond_init(&(ptr_sem_platform->cond), &m_condAttr);
    ptr_sem_platform->num = 0;

    return (void *)ptr_sem_platform;
}

void hal_semaphore_destroy(void *ptr_sem)
{
    sdk_sem_platform *ptr_sem_platform = (sdk_sem_platform *)ptr_sem;
    if (ptr_sem_platform == NULL)
    {
        return;
    }

    pthread_mutex_destroy(&ptr_sem_platform->mutex);
    pthread_cond_destroy(&ptr_sem_platform->cond);
    free(ptr_sem_platform);
}

int hal_semaphore_wait(void *ptr_sem)
{
    sdk_sem_platform *ptr_sem_platform = (sdk_sem_platform *)ptr_sem;
    if (ptr_sem_platform == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&ptr_sem_platform->mutex);
    ptr_sem_platform->num = 0;

    pthread_cond_wait(&ptr_sem_platform->cond, &ptr_sem_platform->mutex);

    ptr_sem_platform->num = 0;
    pthread_mutex_unlock(&ptr_sem_platform->mutex);

    return 0;
}

int hal_semaphore_wait_ms(void *ptr_sem, long time_ms)
{
    int ret = -1;
    struct timespec tv = {0};

    sdk_sem_platform *ptr_sem_platform = (sdk_sem_platform *)ptr_sem;
    if (ptr_sem_platform == NULL)
    {
        return -1;
    }

    if (0 != clock_gettime(CLOCK_MONOTONIC, &tv))
    {
        return -1;
    }

    pthread_mutex_lock(&ptr_sem_platform->mutex);
    ptr_sem_platform->num = 0;

    tv.tv_sec += time_ms / 1000;
    tv.tv_nsec += time_ms % 1000 * 1000000;
    if (ETIMEDOUT != pthread_cond_timedwait(&ptr_sem_platform->cond, &ptr_sem_platform->mutex, &tv))
    {
        ret = 0;
    }

    ptr_sem_platform->num = 0;
    pthread_mutex_unlock(&ptr_sem_platform->mutex);

    return ret;
}

int hal_semaphore_post(void *ptr_sem)
{
    sdk_sem_platform *ptr_sem_platform = (sdk_sem_platform *)ptr_sem;
    if (ptr_sem_platform == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&ptr_sem_platform->mutex);
    ptr_sem_platform->num = 1;
    pthread_cond_signal(&ptr_sem_platform->cond);
    pthread_mutex_unlock(&ptr_sem_platform->mutex);

    return 0;
}
```



- 任务接口示例：

```c
#include <hal_thread.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    pthread_mutex_t lock;
} sdk_mutex_platform;

typedef struct thread_handle_platform
{
    pthread_t thread_hd;
    void *thread_arg;
    void (*task_do)(void *user_data);
    char thread_name[16];
} thread_handle;

static void* sdk_thread_fun(void *aArg)
{
    thread_handle *hd = (thread_handle *)aArg;
    if (hd == NULL)
    {
        return NULL;
    }

    prctl(PR_SET_NAME, hd->thread_name);
    hd->task_do(hd->thread_arg);
    return NULL;
}

void *hal_thread_create(int8_t *thread_name, hal_thread_fun_t thread_fun, int32_t stack_size, int32_t priority, void *arg)
{
    thread_handle *handle = (thread_handle *)malloc(sizeof(thread_handle));
    if (handle == NULL)
    {
        return NULL;
    }

    memset(handle, 0, sizeof(thread_handle));
    handle->task_do = thread_fun;
    handle->thread_arg = arg;
    strncpy(handle->thread_name, (char*)thread_name, sizeof(handle->thread_name) - 1);

    if (pthread_create(&handle->thread_hd, NULL, sdk_thread_fun, (void *)handle) != 0)
    {
        free(handle);
        return NULL;
    }

    return handle;
}

int hal_thread_destroy(void *handle)
{
    if (handle == NULL)
    {
        return -1;
    }

    thread_handle *thandle = (thread_handle *)handle;

    if (thandle->thread_hd != 0)
    {
        void *pres = NULL;
        pthread_join(thandle->thread_hd, &pres);
    }

    free(thandle);

    return 0;
}


int hal_thread_detach(void *handle)
{
    if (handle == NULL)
    {
        return -1;
    }

    thread_handle *thandle = (thread_handle *)handle;

    if (thandle->thread_hd != 0)
    {   
        pthread_detach(thandle->thread_hd);
    }

    free(thandle);

    return 0;
}

void *hal_thread_mutex_create()
{
    sdk_mutex_platform *ptr_mutex_platform = NULL;
    ptr_mutex_platform = (sdk_mutex_platform *)malloc(sizeof(sdk_mutex_platform));
    if (ptr_mutex_platform == NULL)
    {
        return NULL;
    }

    pthread_mutex_init(&ptr_mutex_platform->lock, NULL);

    return (void *)ptr_mutex_platform;
}

void hal_thread_mutex_destroy(void *ptr_mutex)
{
    sdk_mutex_platform *ptr_mutex_platform = (sdk_mutex_platform *)ptr_mutex;
    if (ptr_mutex_platform == NULL)
    {
        return;
    }
    pthread_mutex_destroy(&ptr_mutex_platform->lock);
    free(ptr_mutex_platform);
    ptr_mutex_platform = NULL;
}

int hal_thread_mutex_lock(void *ptr_mutex)
{
    sdk_mutex_platform *ptr_mutex_platform = (sdk_mutex_platform *)ptr_mutex;
    if (ptr_mutex_platform == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&ptr_mutex_platform->lock);
    return 0;
}

int hal_thread_mutex_unlock(void *ptr_mutex)
{
    sdk_mutex_platform *ptr_mutex_platform = (sdk_mutex_platform *)ptr_mutex;
    if (ptr_mutex_platform == NULL)
    {
        return -1;
    }

    pthread_mutex_unlock(&ptr_mutex_platform->lock);
    return 0;
}

void hal_thread_sleep(unsigned int time_ms)
{
    usleep((int)(time_ms * 1000));
}
```



- 时间接口示例：

```c
#include "hal_time.h"
#include <time.h>
#include <stdlib.h>

#define TIMESPEC_THOUSAND 1000
#define TIMESPEC_MILLION 1000000
#define TIMESPEC_BILLION 1000000000

#define Platform_Timespec_Add(a, b, result)              \
    do                                                   \
    {                                                    \
        (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;    \
        (result)->tv_nsec = (a)->tv_nsec + (b)->tv_nsec; \
        if ((result)->tv_nsec >= TIMESPEC_BILLION)       \
        {                                                \
            ++(result)->tv_sec;                          \
            (result)->tv_nsec -= TIMESPEC_BILLION;       \
        }                                                \
    } while (0)

#define Platform_Timespec_Sub(a, b, result)              \
    do                                                   \
    {                                                    \
        (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;    \
        (result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec; \
        if ((result)->tv_nsec < 0)                       \
        {                                                \
            --(result)->tv_sec;                          \
            (result)->tv_nsec += TIMESPEC_BILLION;       \
        }                                                \
    } while (0)

typedef struct
{
    struct timespec time_record;
} linux_time;

void *hal_timer_creat()
{
    linux_time *linuxtime = NULL;
    linuxtime = (linux_time *)malloc(sizeof(linux_time));
    if (linuxtime == NULL)
    {
        return NULL;
    }

    linuxtime->time_record = (struct timespec){0, 0};
    return (void *)linuxtime;
}

uint8_t hal_time_isexpired_bydiff(void *timer, uint32_t time_ms)
{
    struct timespec now, res;
    linux_time *linuxtime = (linux_time *)timer;
    if (linuxtime == NULL)
    {
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &now);
    Platform_Timespec_Sub(&now, &linuxtime->time_record, &res);

    if (res.tv_sec < 0)
    {
        return 0;
    }
    else if (res.tv_sec == 0)
    {
        if ((res.tv_nsec / TIMESPEC_MILLION) > time_ms)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        if ((res.tv_sec * TIMESPEC_THOUSAND + res.tv_nsec / TIMESPEC_MILLION) > time_ms)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

uint8_t hal_time_isexpired(void *timer)
{
    struct timespec now, res;
    linux_time *linuxtime = (linux_time *)timer;
    if (linuxtime == NULL)
    {
        return (char)1;
    }

    clock_gettime(CLOCK_MONOTONIC, &now);
    Platform_Timespec_Sub(&linuxtime->time_record, &now, &res);
    return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_nsec <= 0);
}

void hal_time_countdown_ms(void *timer, uint32_t timeout)
{
    struct timespec now;
    struct timespec interval = {timeout / TIMESPEC_THOUSAND, (timeout % TIMESPEC_THOUSAND) * TIMESPEC_MILLION};
    linux_time *linuxtime = (linux_time *)timer;
    if (linuxtime == NULL)
    {
        return;
    }

    clock_gettime(CLOCK_MONOTONIC, &now);
    Platform_Timespec_Add(&now, &interval, &linuxtime->time_record);
}

void hal_time_countdown(void *timer, uint32_t timeout)
{
    struct timespec now;
    struct timespec interval = {timeout, 0};
    linux_time *linuxtime = (linux_time *)timer;
    if (linuxtime == NULL)
    {
        return;
    }

    clock_gettime(CLOCK_MONOTONIC, &now);
    Platform_Timespec_Add(&now, &interval, &linuxtime->time_record);
}

uint32_t hal_time_left_ms(void *timer)
{
    struct timespec now, res;
    linux_time *linuxtime = (linux_time *)timer;
    if (linuxtime == NULL)
    {
        return 0;
    }

    clock_gettime(CLOCK_MONOTONIC, &now);
    Platform_Timespec_Sub(&linuxtime->time_record, &now, &res);
    return (res.tv_sec < 0) ? 0 : res.tv_sec * TIMESPEC_THOUSAND + res.tv_nsec / TIMESPEC_MILLION;
}

void hal_timer_destroy(void *timer)
{
    linux_time *linuxtime = (linux_time *)timer;
    if (linuxtime == NULL)
    {
        return;
    }

    free(linuxtime);
    linuxtime = NULL;
}
```

