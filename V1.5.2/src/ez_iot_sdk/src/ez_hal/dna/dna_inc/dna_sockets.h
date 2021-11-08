/*******************************************************************************
*
*               COPYRIGHT (c) 2015-2016 Broadlink Corporation
*                         All Rights Reserved
*
* The source code contained or described herein and all documents
* related to the source code ("Material") are owned by Broadlink
* Corporation or its licensors.  Title to the Material remains
* with Broadlink Corporation or its suppliers and licensors.
*
* The Material is protected by worldwide copyright and trade secret
* laws and treaty provisions. No part of the Material may be used,
* copied, reproduced, modified, published, uploaded, posted, transmitted,
* distributed, or disclosed in any way except in accordance with the
* applicable license agreement.
*
* No license under any patent, copyright, trade secret or other
* intellectual property right is granted to or conferred upon you by
* disclosure or delivery of the Materials, either expressly, by
* implication, inducement, estoppel, except in accordance with the
* applicable license agreement.
*
* Unless otherwise agreed by Broadlink in writing, you may not remove or
* alter this notice or any other notice embedded in Materials by Broadlink
* or Broadlink's suppliers or licensors in any way.
*
*******************************************************************************/

#ifndef __DNA_SOCKETS_H
#define __DNA_SOCKETS_H

#include "dna_common.h"

#ifdef __cplusplus
    extern "C" {
#endif

#define DNA_AF_INET                                 2
#if defined(__mt7688__) || defined(__mt7628__)
#define DNA_SOCK_STREAM                             2
#define DNA_SOCK_DGRAM                              1
#else
#define DNA_SOCK_STREAM                             1
#define DNA_SOCK_DGRAM                              2
#endif
#define DNA_IPPROTO_TCP                             6
#define DNA_IPPROTO_UDP                             17
#define DNA_IPADDR_NONE                             (0xffffffffUL)
#define DNA_IPADDR_LOOPBACK                         (0x7f000001UL)
#define DNA_IPADDR_BROADCAST                        (0xffffffffUL)

#if defined(__mt7688__) || defined(__mt7628__)
#define DNA_O_NONBLOCK                              0x80 /* nonblocking I/O */
#else
#define DNA_O_NONBLOCK                              1 /* nonblocking I/O */
#endif
#define DNA_F_GETFL                                 3
#define DNA_F_SETFL                                 4

#define DNA_INADDR_ANY                               ((unsigned int)0x00000000UL)
#define DNA_IPPROTO_IP                              0
#if defined(__mt7688__) || defined(__mt7628__)
#define DNA_IP_ADD_MEMBERSHIP                       35
#else
#define DNA_IP_ADD_MEMBERSHIP                       3
#endif
/*
 * Option flags per-socket.
 */
#define DNA_SO_DEBUG                                0x0001 /* turn on debugging info recording */
#if defined(__mt7688__) || defined(__mt7628__)
#define DNA_SO_ACCEPTCONN                           0x1009 /* socket has had listen() */
#else
#define DNA_SO_ACCEPTCONN                           0x0002 /* socket has had listen() */
#endif
#define DNA_SO_REUSEADDR                            0x0004 /* Allow local address reuse */
#define DNA_SO_KEEPALIVE                            0x0008 /* keep connections alive */
#define DNA_SO_DONTROUTE                            0x0010 /* just use interface addresses */
#define DNA_SO_BROADCAST                            0x0020 /* permit to send and to receive broadcast messages */
#if !defined(__mt7688__) && !defined(__mt7628__)    /* mtk7688 not defined */
#define DNA_SO_USELOOPBACK                          0x0040 /* bypass hardware when possible */
#endif
#define DNA_SO_LINGER                               0x0080 /* linger on close if data present */
#define DNA_SO_OOBINLINE                            0x0100 /* leave received OOB data in line */
#define DNA_SO_REUSEPORT                            0x0200 /* allow local address & port reuse */
#if defined(__mt7688__) || defined(__mt7628__)
#define DNA_SO_BINDTODEVICE                         25	   /* Bind to device */
#else
#define DNA_SO_BINDTODEVICE                         0x0400 /* Bind to device */
#endif

#define DNA_SO_DONTLINGER                           ((int)(~DNA_SO_LINGER))

/*
 * Additional options, not kept in so_options.
 */
#define DNA_SO_SNDBUF                               0x1001 /* send buffer size */
#define DNA_SO_RCVBUF                               0x1002 /* receive buffer size */
#define DNA_SO_SNDLOWAT                             0x1003 /* send low-water mark */
#define DNA_SO_RCVLOWAT                             0x1004 /* receive low-water mark */
#define DNA_SO_SNDTIMEO                             0x1005 /* send timeout */
#define DNA_SO_RCVTIMEO                             0x1006 /* receive timeout */
#define DNA_SO_ERROR                                0x1007 /* get error status and clear */
#define DNA_SO_TYPE                                 0x1008 /* get socket type */
#if !defined(__mt7688__) && !defined(__mt7628__)/* mtk7688 not defined. */
#define DNA_SO_CONTIMEO                             0x1009 /* connect timeout */
#endif
#if defined(__mt7688__) || defined(__mt7628__)
#define DNA_SO_NO_CHECK                             11	   /* don't create UDP checksum */
#else
#define DNA_SO_NO_CHECK                             0x100a /* don't create UDP checksum */
#endif

#if defined(__mt7688__) || defined(__mt7628__)
#define DNA_SOL_SOCKET                              0xffff /* options for socket level */
#else
#define DNA_SOL_SOCKET                              0xfff  /* options for socket level */
#endif

enum {
    DNA_DNS_MAIN_SERVER              =  0,
    DNA_DNS_BACKUP_SERVER,
};

#define DNA_DNS_OK                                  0      /* No error */
#define DNA_DNS_INPROGRESS                         -5      /* Operation in progress    */

/*
 * Select uses bit masks of file descriptors in longs.
 * These macros manipulate such bit fields (the filesystem macros use chars).
 * FD_SETSIZE may be defined by the user, but the default here
 * should be >= NOFILE (param.h).
 */
#ifndef FD_SET
#ifndef FD_SETSIZE
#define  FD_SETSIZE      64
#endif
#define FD_SET(n, p)  ((p)->fd_bits[(n)/8] |=  (1 << ((n) & 7)))
#define FD_CLR(n, p)  ((p)->fd_bits[(n)/8] &= ~(1 << ((n) & 7)))
#define FD_ISSET(n, p) ((p)->fd_bits[(n)/8] &   (1 << ((n) & 7)))
#define FD_ZERO(p)    memset((void *)(p), 0, sizeof(*(p)))

typedef struct fd_set {
	unsigned char fd_bits[(FD_SETSIZE + 7) / 8];
} fd_set;
#endif /* FD_SET */

typedef int dna_socklen_t;

typedef struct dna_sockaddr {
#if defined (__mt7688__) || defined (__linux__) || defined (__mt7628__)
    unsigned short sa_family;
#else
    unsigned char sa_len;
    unsigned char sa_family;
#endif
    unsigned char sa_data[14];
} dna_sockaddr_t;

typedef struct dna_in_addr {
    unsigned int s_addr;
} dna_in_addr_t;

typedef struct dna_sockaddr_in {
#if defined(__mt7688__) || defined(__linux__) || defined(__mt7628__)
    unsigned short sin_family;
#else
    unsigned char sin_len;
    unsigned char sin_family;
#endif
    unsigned short sin_port;
    dna_in_addr_t  sin_addr;
    char sin_zero[8];
} dna_sockaddr_in_t;

typedef struct dna_timeval {
    long tv_sec;         /* seconds */
    long tv_usec;        /* and microseconds */
} dna_timeval_t;

typedef struct dna_hostent {
    char  *h_name;      /* Official name of the host. */
    char **h_aliases;   /* A pointer to an array of pointers to alternative host names,
                           terminated by a null pointer. */
    int    h_addrtype;  /* Address type. */
    int    h_length;    /* The length, in bytes, of the address. */
    char **h_addr_list; /* A pointer to an array of pointers to network addresses (in
                           network byte order) for the host, terminated by a null pointer. */
#define h_addr h_addr_list[0] /* for backward compatibility */
} dna_hostent_t;

/*
 * Structure used for manipulating linger option.
 */
typedef struct dna_linger {
   int l_onoff;                /* option on/off */
   int l_linger;               /* linger time in seconds */
} dna_linger_t;

typedef struct dna_ip_mreq {
    dna_in_addr_t imr_multiaddr; /* IP multicast address of group */
    dna_in_addr_t imr_interface; /* local IP address of interface */
} dna_ip_mreq_t;

/** @defgroup dna_socket API: Please reference standard BSD Socket interface.
  * @{
  */

unsigned short dna_htons(unsigned short n);

unsigned short dna_ntohs(unsigned short n);

unsigned int dna_htonl(unsigned int n);

unsigned int dna_ntohl(unsigned int n);

unsigned int dna_inet_addr(const char * cp);

char* dna_inet_ntoa(const dna_in_addr_t * addr);

int dna_inet_aton(const char * cp, dna_in_addr_t * addr);

/* Please refer to dna_errno.h */
int dna_errno(void);

int dna_socket(int domain, int type, int protocol);

int dna_close(int sd);

void dna_bzero(void * dest, unsigned int len);

int dna_bind(int sd, const dna_sockaddr_t * addr, dna_socklen_t addrlen);

int dna_listen(int sd, int backlog);

int dna_connect(int sd, dna_sockaddr_t * addr, dna_socklen_t addrlen);

int dna_accept(int sd, dna_sockaddr_t * addr, dna_socklen_t * addrlen);

int dna_sendto(int sd, const void * buff, int len, unsigned int flags, const dna_sockaddr_t * to, dna_socklen_t tolen);

int dna_write(int sd, const void * buff, int len);

int dna_send( int sd, const void *buf, int len, int flags);

int dna_recvfrom(int sd, void * buff, int len, unsigned int flags, dna_sockaddr_t * from, dna_socklen_t * fromlen);

int dna_read(int sd, void * buff, int len);

int dna_recv( int sd, void *buf, int len, int flags);

int dna_select(int maxsd, fd_set * readset, fd_set * writeset, fd_set * exceptset, dna_timeval_t * timeout);

int dna_fcntl(int sd, int cmd, int val);

int dna_setsockopt(int sd, int level, int optname, const void * optval, dna_socklen_t optlen);

int dna_getsockopt(int sd, int level, int optname, void * optval, dna_socklen_t *optlen);

/* Sync mode */
dna_hostent_t * dna_gethostbyname(const char *name);

/* Async mode 
* Success: DNA_DNS_OK or DNA_DNS_INPROGRESS
*/
int dna_dns_gethostbyname(
        const char *hostname,
        unsigned int * addr,
        void (* found)(const char * hostname, unsigned int * ipaddr, void * arg),
        void * arg);

/* Get DNS server (main and backup) address
*  index: please refer to DNA_DNS_MAIN_SERVER/DNA_DNS_BACKUP_SERVER.
*  Return server address (network byte order)
*/
unsigned int dna_dns_server_addr(int index);

#ifdef __cplusplus
}
#endif

#endif

