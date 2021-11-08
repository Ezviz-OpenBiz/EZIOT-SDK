/*******************************************************************************
 * Copyright 漏 2017-2021 Ezviz Inc.
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
 * 2017/3/4       panlong
 *******************************************************************************/
#ifndef H_EZDEV_SDK_KERNE_QUEUEL_H_
#define H_EZDEV_SDK_KERNE_QUEUEL_H_

#include "ezdev_sdk_kernel_struct.h"
#include "mkernel_internal_error.h"
#include "base_typedef.h"
#include "sdk_kernel_def.h"

/**
 *	消息元素
 */
#define QUEUE_DEFINE(MSGTYPE)						\
typedef struct 	tag_queque_element_##MSGTYPE		\
{													\
	ezdev_sdk_kernel_##MSGTYPE* msg;				\
	struct tag_queque_element_##MSGTYPE* next;		\
}queque_element_##MSGTYPE;							\
typedef struct	tag_queque_##MSGTYPE				\
{													\
	EZDEV_SDK_UINT16	maxsize;					\
	EZDEV_SDK_UINT16	size;						\
	queque_element_##MSGTYPE* head;					\
	queque_element_##MSGTYPE* tail;					\
	ezdev_sdk_mutex		lock;						\
}queque_##MSGTYPE;

QUEUE_DEFINE(submsg)
QUEUE_DEFINE(pubmsg_exchange)
QUEUE_DEFINE(inner_cb_notic)

QUEUE_DEFINE(submsg_v3)
QUEUE_DEFINE(pubmsg_exchange_v3)

/**
 *	\brief 队列初始化函数
 */
#define QUEUE_INIT(MSGTYPE)													\
mkernel_internal_error init_queue_##MSGTYPE(EZDEV_SDK_UINT16 max_size)			\
{																				\
	memset(&g_queue_##MSGTYPE, 0, sizeof(g_queue_##MSGTYPE));					\
	g_queue_##MSGTYPE.maxsize = max_size;										\
	g_queue_##MSGTYPE.lock = ezdev_sdk_kernel_platform_thread_mutex_create();	\
	if (g_queue_##MSGTYPE.lock == NULL)											\
	{																			\
		return mkernel_internal_malloc_error;									\
	}																			\
	return mkernel_internal_succ;												\
}

/**
 *	\brief 队列反初始化函数
 */
#define QUEUE_FINI(MSGTYPE)													\
void fini_queue_##MSGTYPE()													\
{																			\
	queque_element_##MSGTYPE* element = NULL;									\
	ezdev_sdk_kernel_platform_thread_mutex_lock(g_queue_##MSGTYPE.lock);	\
	while(g_queue_##MSGTYPE.head != NULL)										\
	{																			\
		element = g_queue_##MSGTYPE.head;					\
																				\
		g_queue_##MSGTYPE.head = element->next;				\
																				\
		free(element->msg);									\
		element->msg = NULL;									\
		free(element);											\
	}						\
	ezdev_sdk_kernel_platform_thread_mutex_unlock(g_queue_##MSGTYPE.lock);	\
	ezdev_sdk_kernel_platform_thread_mutex_destroy(g_queue_##MSGTYPE.lock);	\
	memset(&g_queue_##MSGTYPE, 0, sizeof(g_queue_##MSGTYPE));	\
}


/**
 *	\brief 透传队列反初始化函数
 */
#define QUEUE_FINI_V3(MSGTYPE)													\
void fini_queue_v3_##MSGTYPE()													\
{																			\
	queque_element_##MSGTYPE* element = NULL;								\
	ezdev_sdk_kernel_platform_thread_mutex_lock(g_queue_raw_##MSGTYPE.lock);	\
	while(g_queue_raw_##MSGTYPE.head != NULL)									\
	{																		\
		element = g_queue_raw_##MSGTYPE.head;					                \
																			\
		g_queue_raw_##MSGTYPE.head = element->next;				                \
																			\
		free(element->msg);									                \
		element->msg = NULL;									            \
		free(element);											            \
	}						                                                \
	ezdev_sdk_kernel_platform_thread_mutex_unlock(g_queue_raw_##MSGTYPE.lock);	\
	ezdev_sdk_kernel_platform_thread_mutex_destroy(g_queue_raw_##MSGTYPE.lock);	\
	memset(&g_queue_raw_##MSGTYPE, 0, sizeof(g_queue_raw_##MSGTYPE));	\
}


/**
 *	\brief 从队列头部取出一个消息
 */
#define QUEUE_POP(MSGTYPE) \
mkernel_internal_error pop_queue_##MSGTYPE(ezdev_sdk_kernel_##MSGTYPE** submsg)			\
{																							\
	queque_element_##MSGTYPE* element = NULL;								\
	ezdev_sdk_kernel_platform_thread_mutex_lock(g_queue_##MSGTYPE.lock);	\
	if (g_queue_##MSGTYPE.head == NULL)										\
	{																						\
		g_queue_##MSGTYPE.tail = NULL;										\
		g_queue_##MSGTYPE.size = 0;													\
		ezdev_sdk_kernel_platform_thread_mutex_unlock(g_queue_##MSGTYPE.lock);	\
		return mkernel_internal_queue_empty;												\
	}																						\
																							\
	*submsg = g_queue_##MSGTYPE.head->msg;								\
	element = g_queue_##MSGTYPE.head;									\
	g_queue_##MSGTYPE.head = g_queue_##MSGTYPE.head->next;				\
																							\
	if (g_queue_##MSGTYPE.head == NULL)												\
	{																						\
		g_queue_##MSGTYPE.tail = NULL;										\
		g_queue_##MSGTYPE.size = 0;													\
	}																						\
	else																					\
	{																						\
		g_queue_##MSGTYPE.size--;													\
	}																						\
	free(element);																		\
	element = NULL;																			\
	ezdev_sdk_kernel_platform_thread_mutex_unlock(g_queue_##MSGTYPE.lock);	\
	return mkernel_internal_succ;															\
}

 /**
 *	\brief 从队列头部取出一个消息,get方法,(消息取到后，队列不变)
 */
#define QUEUE_GET(MSGTYPE) \
mkernel_internal_error get_queue_##MSGTYPE(ezdev_sdk_kernel_##MSGTYPE** submsg)			\
{																					\
	ezdev_sdk_kernel_platform_thread_mutex_lock(g_queue_##MSGTYPE.lock);	\
	if (g_queue_##MSGTYPE.head == NULL)										\
	{																						\
		g_queue_##MSGTYPE.tail = NULL;										\
		g_queue_##MSGTYPE.size = 0;													\
		ezdev_sdk_kernel_platform_thread_mutex_unlock(g_queue_##MSGTYPE.lock);	\
		return mkernel_internal_queue_empty;												\
	}					                                                       \
																	\
	*submsg = g_queue_##MSGTYPE.head->msg;               	  \
	                                                                \
	ezdev_sdk_kernel_platform_thread_mutex_unlock(g_queue_##MSGTYPE.lock);	\
	return mkernel_internal_succ;															\
}
/**
 *	\brief 往队列尾部添加一个消息
 */
#define QUEUE_PUSH(MSGTYPE)																\
mkernel_internal_error push_queue_##MSGTYPE(ezdev_sdk_kernel_##MSGTYPE* submsg)					\
{																							\
	queque_element_##MSGTYPE* element = NULL;												\
	ezdev_sdk_kernel_platform_thread_mutex_lock(g_queue_##MSGTYPE.lock);					\
	ezdev_sdk_kernel_log_trace(0, 0, "%s size:%d, max size:%d", #MSGTYPE, g_queue_##MSGTYPE.size, g_queue_##MSGTYPE.maxsize);\
	if (g_queue_##MSGTYPE.size >= g_queue_##MSGTYPE.maxsize)								\
	{																						\
		ezdev_sdk_kernel_platform_thread_mutex_unlock(g_queue_##MSGTYPE.lock);	\
		return mkernel_internal_queue_full;													\
	}																						\
	element = (queque_element_##MSGTYPE*)malloc(sizeof(queque_element_##MSGTYPE));			\
	if (element == NULL)														\
	{																						\
		ezdev_sdk_kernel_platform_thread_mutex_unlock(g_queue_##MSGTYPE.lock);	\
		return mkernel_internal_malloc_error;												\
	}																						\
	element->msg = submsg;													\
	element->next = NULL;													\
	if (g_queue_##MSGTYPE.tail == NULL || g_queue_##MSGTYPE.head == NULL)							\
	{																						\
		g_queue_##MSGTYPE.head = element;										\
		g_queue_##MSGTYPE.tail = element;										\
	}																						\
	else																					\
	{																						\
		g_queue_##MSGTYPE.tail->next = element;								\
		g_queue_##MSGTYPE.tail = element;										\
	}																						\
	g_queue_##MSGTYPE.size++;																\
	ezdev_sdk_kernel_platform_thread_mutex_unlock(g_queue_##MSGTYPE.lock);	\
	return mkernel_internal_succ;															\
}

/**
 *	\brief 往队列头部添加一个消息
 */
#define QUEUE_PUSH_HEAD(MSGTYPE)			\
mkernel_internal_error push_queue_head_##MSGTYPE(ezdev_sdk_kernel_##MSGTYPE* submsg)		\
{																					\
	queque_element_##MSGTYPE* element = NULL;												\
	ezdev_sdk_kernel_platform_thread_mutex_lock(g_queue_##MSGTYPE.lock);					\
	if (g_queue_##MSGTYPE.size >= g_queue_##MSGTYPE.maxsize)								\
	{																						\
		ezdev_sdk_kernel_platform_thread_mutex_unlock(g_queue_##MSGTYPE.lock);	\
		return mkernel_internal_queue_full;													\
	}																						\
	element = (queque_element_##MSGTYPE*)malloc(sizeof(queque_element_##MSGTYPE));			\
	if (element == NULL)																	\
	{																						\
		ezdev_sdk_kernel_platform_thread_mutex_unlock(g_queue_##MSGTYPE.lock);	\
		return mkernel_internal_malloc_error;												\
	}																						\
	element->msg = submsg;													\
	element->next = NULL;													\
	if (g_queue_##MSGTYPE.tail == NULL || g_queue_##MSGTYPE.head == NULL)					\
	{																						\
		g_queue_##MSGTYPE.head = element;													\
		g_queue_##MSGTYPE.tail = element;										\
	}																						\
	else																					\
	{																						\
		element->next = g_queue_##MSGTYPE.head;												\
		g_queue_##MSGTYPE.head = element;													\
	}																						\
	g_queue_##MSGTYPE.size++;																\
	ezdev_sdk_kernel_platform_thread_mutex_unlock(g_queue_##MSGTYPE.lock);	\
	return mkernel_internal_succ;															\
}

#define EXTERN_QUEUE_FUN(MSGTYPE) \
extern mkernel_internal_error push_queue_##MSGTYPE(ezdev_sdk_kernel_##MSGTYPE* submsg);\
extern mkernel_internal_error push_queue_head_##MSGTYPE(ezdev_sdk_kernel_##MSGTYPE* submsg);\
extern mkernel_internal_error pop_queue_##MSGTYPE(ezdev_sdk_kernel_##MSGTYPE** submsg);\
extern mkernel_internal_error get_queue_##MSGTYPE(ezdev_sdk_kernel_##MSGTYPE** submsg);

#define EXTERN_QUEUE_BASE_FUN	\
extern mkernel_internal_error init_queue(EZDEV_SDK_UINT16 sub_max_size, EZDEV_SDK_UINT16 pub_max_size, EZDEV_SDK_UINT16 inner_max_size);\
extern void fini_queue(void); \
extern void destroy_inner_cb_notic(ezdev_sdk_kernel_inner_cb_notic* ptr_inner_cb_notic);
  

// typedef struct
// {
// 	EZDEV_SDK_UINT16	submsg_maxsize;
// 	EZDEV_SDK_UINT16	submsg_size;
// 	ezdev_sdk_kernel_submsg_queque* submsg_queque_head;
// 	ezdev_sdk_kernel_submsg_queque* submsg_queque_tail;
// 
// 	EZDEV_SDK_UINT16	pubmsg_maxsize;
// 	EZDEV_SDK_UINT16	pubmsg_size;
// 	ezdev_sdk_kernel_pubmsg_queque* pubmsg_queque_head;
// 	ezdev_sdk_kernel_pubmsg_queque* pubmsg_queque_tail;
// }ezdev_sdk_kernel_queue;
// 
// typedef struct
// {
// 	ezdev_sdk_kernel_submsg* msg;
// 	ezdev_sdk_kernel_submsg_queque* next; 
// }ezdev_sdk_kernel_submsg_queque;
// 
// typedef struct
// {
// 	ezdev_sdk_kernel_pubmsg* msg;
// 	ezdev_sdk_kernel_pubmsg_queque* next;
// }ezdev_sdk_kernel_pubmsg_queque;

#endif