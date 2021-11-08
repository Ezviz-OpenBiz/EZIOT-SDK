/**
 * @file ez_protocol.h
 * @author xurongjun (xurongjun@ezvizlife.com)
 * @brief 
 * @version 0.1
 * @date 2019-11-11
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#ifndef _EZ_PROTOCOL_H_
#define _EZ_PROTOCOL_H_
#include <stdint.h>
#include <stdbool.h>
#include "ez_iot_tsl.h"

#define KERNEL_CODE_BASE 0x00010000
#define KERNEL_BASE_DOMAIN_ID 1100
#define KERNEL_BASE_DOMAIN_NAME "domain_base"
#define KERNEL_EXTEND_VER_DEFAULT "v1.0.0"
#define KERNEL_CMD_VER "v1.0.0"
#define hub_module_id 7002
#define hub_module_name "hub"
#define hub_module_version "V2.0.0"

#define KERNEL_SERVICE_DOMAIN_NAME "service"
#define KERNEL_ATTR_DOMAIN_NAME "attribute"
#define KERNEL_EVENT_DOMAIN_NAME "event"

#define KERNEL_OPERATE_BUSI_TYPE "operate"
#define KERNEL_OPERATE_BUSI_REPLY "operate_reply"

#define kCenPlt2PuDomainConfig 0x00000001
#define kPu2CenPltGetUserListReq 0X00003445
#define kPu2CenPltGetUserListRsp 0X00003446
#define kCenPlt2PuSetUserIdReq 0X00004947
#define kCenPlt2PuSetUserIdRsp 0X00004948
#define kPu2CenPltBindUserWithTokenReq 0X00003802
#define kPu2CenPltBindUserWithTokenRsp 0X00003803
#define Pu2CenPltQueryFeatureProfileReq 0x0003870
#define Pu2CenPltQueryFeatureProfileRsp 0x0003871
#define kCenPlt2PuOtaStartUpReq 0x00003850
#define kCenPlt2PuOtaStartUpRsp 0x00003851
#define kPuCenPltOtaReportModuleReq 0x00004F60
#define kPu2CenPltOtaReportModuleRsp 0x00004F61
#define kPu2CenPltOtaQueryPacketReq 0x00004F62
#define kPu2CenPltOtaQueryPacketRsp 0x00004F63
#define kPu2CenPltOtaReportUpgradeProcessReq 0x00004F64
#define kPu2CenPltOtaReportUpgradeProcessRsp 0x00004F65

#define kCenPlt2PuBindUserTouchReq 0x00004F14
#define kCenPlt2PuBindUserTouchRsq 0x00004F15
#define kPu2CenPltReportBindUserTouchWithTokenReq 0x00003808
#define kPu2CenPltReportBindUserTouchWithTokenRsp 0x00003809

#define kCenPlt2PuHubTransferMsgReq 0x00004E86
#define kCenPlt2PuHubTransferMsgRsp 0x00004E87

typedef struct
{
    char address[64];    ///< 升级服务器ip
    unsigned short port; ///< 升级服务器端口
    char reserve[2];     ///< 节对齐
    char user_name[64];  ///< 升级服务器登入用户名
    char password[64];   ///< 升级服务器登入密码
    int max_try_time;    ///< 最大尝试次数
} ez_iotsdk_upgrade_server_t;

typedef struct
{
    ez_iotsdk_upgrade_server_t http_server; ///< http服务器
    char file_name[256];                    ///< 软件包文件名
    char check_sum[64];                     ///< 校验和
    char version[64];                       ///< 版本
    char describe[64];                      ///< 描述信息
    int size;                               ///< 软件包大小，单位Byte
    int interval;                           ///< 上报进度的时间间隔
    int result;                             ///< 升级包查询错误码
    bool is_self;
} ez_iotsdk_upgrade_file_t;

typedef int32_t (*handle_proc_fun)(void *buf, int len);

int32_t cloud2dev_rep_bushandle(void *buf, int len, int rsp_cmd, uint32_t seq, handle_proc_fun proc_func);

int32_t cloud2dev_setuserid_req(void *buf, int32_t len);

int32_t dev2cloud_msg_send(int8_t *buf, int32_t domain_id, int32_t cmd_id, const int8_t *cmd_version, uint8_t msg_response, uint32_t msg_seq);

int32_t cloud2dev_upgrade_info_rsp(void *buf, int32_t len);

int32_t dev2cloud_query_upgrade_info_req(char *dev_subserial, char *dev_type, char *dev_fwversion);

int32_t dev2cloud_updrade_report(int32_t state, int32_t progress, int32_t code);

int32_t dev2cloud_banding_req(int8_t *token);

int32_t cloud2dev_banding_rsp(void *buf, int32_t len);

void cloud2dev_set_ntpinfo_req(void *buf, int32_t len);

int32_t dev2cloud_query_banding_req();

int32_t dev2cloud_query_banding_rsp(void *buf, int32_t len);

int32_t dev2cloud_query_profile_url_req(char *sn, char *type, char *ver, bool need_schema);

int32_t dev2cloud_query_profile_url_rsp(void *buf, int32_t len);

int32_t cloud2dev_contact_bind(void *buf, int len, uint32_t seq);

int32_t dev2cloud_contact_bind_req(int32_t response_code);

#endif
