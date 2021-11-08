/*
 *  dna_sdkconfig.h -- provide dna-system SDK configuration options management.
 *  It is used for SSV6060P platform.
 *
 *  ORIGINAL AUTHOR: Xu Chun (chun.xu@broadlink.com.cn)
 *
 *  Copyright (c) 2016 Broadlink Corporation
 */

#ifndef __DNA_SDKCONFIG_H
#define __DNA_SDKCONFIG_H

/* 
### [Device layer configuration options (middleware/devices)] ###
*/

#define DNA_DEVICE_FRAMEBUFFER_SIZE                      FRAME_SIZE     /* bytes */
#define DNA_DEVICE_POLLING_PERIOD                   		     40     /* ms */
#define DNA_DEVICE_DYNAMIC_MAX_CNT                               1
#define DNA_DEVICE_ASYNC_MODE				                     0
#define DNA_DEVICE_ASYNC_TASK_STACK_SIZE			             4096   /* bytes */
#define DNA_DEVICE_ASYNC_FRAMEBUFFER_DEPTH                       4

/* 
### [Device profile configuration options (middleware/profile)] ###
*/

#define DNA_SPECIAL_CMD_MAX_CNT                                  32
#define DNA_SPECIAL_CMD_MAX_LEN                                  256     /* bytes */
#define DNA_SPECIAL_CMD_BUFF_SIZE                                1024    /* bytes */
#define DNA_KEYVALUE_BUFF_SIZE                                   1024    /* bytes */
#define DNA_KEYVALUE_STRING_LENGTH                               64      /* bytes */

/* 
### [Network management configuration options (middleware/netmgr)] ###
*/
#ifdef CONFIG_ANDLINK_CLOUD
#define NETMGR_TASK_STACK_SIZE                                   4096    /* bytes */
#else
#define NETMGR_TASK_STACK_SIZE                                   4096    /* bytes */
#endif

/* 
### [Clouds configuration options (middleware/clouds)] ###
*/

#define DNA_CLOUD_DYNAMIC_MAX_CNT                                1

/*
[Broadlink cloud]
*/
#define BL_CLOUD_MAIN_TASK_STACK_SIZE                            6144    /* bytes */
#if defined(CONFIG_SECURITY_V2)
#define BL_CLOUD_EVENT_TASK_STACK_SIZE                           (4096+2048)    /* bytes */
#else
#define BL_CLOUD_EVENT_TASK_STACK_SIZE                           4096    /* bytes */
#endif

#define BL_CLOUD_NEW_TIMERTASK_ENABLE                            1

/*
[Joylink cloud]
*/
#define JD_CLOUD_MAIN_TASK_STACK_SIZE                            4096    /* bytes */
#define JD_CLOUD_EVENT_TASK_STACK_SIZE                           1024    /* bytes */

/* 
### [License configuration options] ###
*/

#define LICENSE_CHECK_ENABLE                                     1
#define LICENSE_REMOTE_ACTIVATE                                  1

/* 
### [Peripheral interface configuration options] ###
*/

#define DNA_PLAT_NETWORK_LED_PIN                                 DNA_GPIO_4
#define DNA_PLAT_RESET_PIN                                       DNA_GPIO_5

#define TINYDNS_ENABLE                                          1

#endif

