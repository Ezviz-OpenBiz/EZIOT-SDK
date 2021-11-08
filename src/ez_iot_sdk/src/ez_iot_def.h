#ifndef _EZ_IOT_DEF_H_
#define _EZ_IOT_DEF_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#if defined(_WIN32) && !defined(__MINGW32__) && \
    (!defined(_MSC_VER) || _MSC_VER < 1600) && !defined(__WINE__)
#include <BaseTsd.h>
    typedef __int8 int8_t;
    typedef unsigned __int8 uint8_t;
    typedef __int16 int16_t;
    typedef unsigned __int16 uint16_t;
    typedef __int32 int32_t;
    typedef unsigned __int32 uint32_t;
    typedef __int64 int64_t;
    typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#include <stdbool.h>
#endif

#include "ezconfig.h"
#include "mcuconfig.h"

#include "ez_iot_ctx.h"
#include "ezkv/ezkv.h"
#include "ezlist/ezlist.h"
#include "s2j/s2j.h"
#include "ez_hal/hal_time.h"
#include "ez_hal/hal_thread.h"
#include "ez_hal/hal_net_tcp.h"
#include "ez_iot_log.h"

#include "bscJSON.h"

#ifdef __FILENAME__
#define FUNC_IN(tag) elog_output(EZ_ELOG_LVL_VERBOSE, tag, __FILENAME__, __FUNCTION__, __LINE__, " in")
#define FUNC_OUT(tag) elog_output(EZ_ELOG_LVL_VERBOSE, tag, __FILENAME__, __FUNCTION__, __LINE__, " out")
#else
#define FUNC_OUT(tag)
#define FUNC_IN(tag)
#endif


#define CHECK_COND_RETURN(cond, errcode) \
    if ((cond))                          \
    {                                    \
		ez_log_e(TAG_SDK, "cond return:0x%x,errcode:0x%x",cond, errcode);    \
        return (errcode);                \
    }

#define CHECK_COND_DONE(cond, errcode) \
    if ((cond))                        \
    {                                  \
		ez_log_e(TAG_SDK, "cond done:0x%x,errcode:0x%x",cond,errcode);    \
        rv = (errcode);                \
        goto done;                     \
    }

#define SAFE_FREE(p) \
    if (p)           \
    {                \
        free(p);     \
        p = NULL;    \
    }

#define CJSON_SAFE_DELETE(p) \
    if (p)                   \
    {                        \
        bscJSON_Delete(p);   \
        p = NULL;            \
    }
#ifdef __cplusplus
}
#endif

#endif