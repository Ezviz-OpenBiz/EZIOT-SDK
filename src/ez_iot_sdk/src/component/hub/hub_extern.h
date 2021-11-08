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
 * 2020-08-13    xurongjun     
 *******************************************************************************/

enum
{
    kPu2CenPltHubReportRelationShipReq = 0x00000001,
    kPu2CenPltHubReportRelationShipRsp = 0x00000002,
    kPu2CenPltHubReportOnlineStatusReq = 0x00000003,
    kPu2CenPltHubReportOnlineStatusRsp = 0x00000004,
    kPu2CenPltHubAuthChildDeviceReq = 0x000000010,
    kPu2CenPltHubAuthChildDeviceRsp = 0x000000011,
    kPu2CenPltHubTransferMsgReq = 0x00000007,
    kPu2CenPltHubTransferMsgRsp = 0x00000008,
    kCenPlt2PuAddChildDeviceReq = 0x00004E80,
    kCenPlt2PuAddChildDeviceRsp = 0x00004E81,
    kCenPlt2PuDeleteChildDeviceReq = 0x00004E82,
    kCenPlt2PuDeleteChildDeviceRsp = 0x00004E83,
};

#define EZDEVSDK_HUB_RSP 1
#define EZDEVSDK_HUB_REQ 0

int hub_extern_init();

int hub_extern_finit();

int hub_send_msg_to_platform(const char *msg, int msg_len, int cmd_id, unsigned char msg_response, unsigned int msg_seq);