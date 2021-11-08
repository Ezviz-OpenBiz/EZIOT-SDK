/*
 *  dna_tinydns.h -- provide dna-system TinyDNS Client implementation.
 *
 *  ORIGINAL AUTHOR: Xu Chun (chun.xu@broadlink.com.cn)
 *
 *  Copyright (c) 2016 Broadlink Corporation
 */

#ifndef __DNA_TINYDNS_H
#define __DNA_TINYDNS_H

/* TinyDNS initialization */
int tinydns_init(void);

/*
*  TinyDNS uninitialize.
*/
int tinydns_deinit(void);

/*
*  TinyDNS restart.
*/
int tinydns_restart(void);

/* TinyDNS async mode */
int tinydns_async_gethostbyname(
        const char * hostname,
        unsigned int * addr,
        void (* found)(const char * hostname, unsigned int * ipaddr, void * arg),
        void * arg);

/* TinyDNS sync mode (Not security on mutl-thread) */
dna_hostent_t * tinydns_gethostbyname(const char * name);

#endif

