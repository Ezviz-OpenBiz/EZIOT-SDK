#include "ez_iot_def.h"
#include "ez_protocol.h"
#include "ezxml.h"
#include "ez_iot_ctx.h"
#include "ezdev_sdk_kernel_struct.h"
#include "bscJSON.h"
#include "shadow.h"
#include "ez_iot_log.h"
#include "ezdev_sdk_kernel.h"

#define MSG_TYPE_REQ 0
#define MSG_TYPE_RSP 1

#define CIVIL_RESULT_GENERAL_NO_ERROR 0x00000000     ///< 无错误
#define CIVIL_RESULT_GENERAL_PARSE_FAILED 0x00000003 ///< 报文解析失败
#define CIVIL_RESULT_GENERAL_SYSTEM_ERROR 0x00000004 ///< 系统内部错误

static ezxml_t xml_make_rsp(int32_t result);
static int msg2dev_binding(const char *userid);
static int msg2dev_unbinding();

int32_t cloud2dev_rep_bushandle(void *buf, int len, int rsp_cmd, uint32_t seq, handle_proc_fun proc_func)
{
    ezxml_t rspxml = NULL;
    char *result_str = NULL;
    int32_t result = proc_func(buf, len);

    if (NULL != (rspxml = xml_make_rsp(result)) && NULL != (result_str = ezxml_toxml(rspxml)))
    {
        result = dev2cloud_msg_send((int8_t *)result_str, KERNEL_BASE_DOMAIN_ID, rsp_cmd, (int8_t *)KERNEL_CMD_VER, MSG_TYPE_RSP, seq);
    }

    if (result_str)
        free(result_str);

    if (rspxml)
        ezxml_free(rspxml);

    return result;
}

int32_t cloud2dev_setuserid_req(void *buf, int32_t len)
{
    int32_t result = CIVIL_RESULT_GENERAL_PARSE_FAILED;
    ezxml_t req_xml = NULL;
    ezxml_t userid_xml = NULL;
    char *name = NULL;

    do
    {
        if (NULL == (req_xml = ezxml_parse_str(buf, len)))
        {
            break;
        }

        name = ezxml_name(req_xml);
        if (NULL == name || 0 != strcmp(name, "Request"))
        {
            break;
        }

        if (NULL == (userid_xml = ezxml_child(req_xml, "UserId")))
        {
            break;
        }

        if (0 == strlen(ezxml_txt(userid_xml)))
        {
            result = msg2dev_unbinding();
        }
        else
        {
            result = msg2dev_binding(ezxml_txt(userid_xml));
        }

    } while (0);

    if (req_xml)
        ezxml_free(req_xml);

    return result;
}

int32_t cloud2dev_upgrade_info_rsp(void *buf, int32_t len)
{
    int ret = -1;
    ez_iotsdk_upgrade_file_t upgrade_file = {0};
    ezxml_t root_xml = NULL;
    ezxml_t result_xml = NULL;
    ezxml_t http_xml = NULL;
    ezxml_t fileinfo_xml = NULL;
    ezxml_t interval_xml = NULL;
    char *name = NULL;

    do
    {
        if (NULL == (root_xml = ezxml_parse_str(buf, len)))
        {
            break;
        }

        name = ezxml_name(root_xml);
        if (NULL == name || 0 != strcmp(name, "Response"))
        {
            break;
        }

        result_xml = ezxml_child(root_xml, "Result");
        http_xml = ezxml_child(root_xml, "UpgradeSvrHttp");
        fileinfo_xml = ezxml_child(root_xml, "FileInfo");
        interval_xml = ezxml_child(root_xml, "Interval");

        if (!result_xml || !interval_xml)
        {
            break;
        }

        if (0 != atoi(ezxml_txt(result_xml)))
        {
            ret = atoi(ezxml_txt(result_xml));
            upgrade_file.result = ret;
            ez_iot_ota_set_info(&upgrade_file);
            break;
        }

        strncpy(upgrade_file.http_server.address, ezxml_attr(http_xml, "Address"), 63);
        upgrade_file.http_server.port = atoi(ezxml_attr(http_xml, "Port"));
        strncpy(upgrade_file.http_server.user_name, ezxml_attr(http_xml, "UserName"), 63);
        strncpy(upgrade_file.http_server.password, ezxml_attr(http_xml, "Password"), 63);
        upgrade_file.http_server.max_try_time = atoi(ezxml_attr(http_xml, "MaxTryNum"));

        strncpy(upgrade_file.file_name, ezxml_attr(fileinfo_xml, "Name"), sizeof(upgrade_file.file_name) - 1);
        strncpy(upgrade_file.check_sum, ezxml_attr(fileinfo_xml, "CheckSum"), 63);
        strncpy(upgrade_file.version, ezxml_attr(fileinfo_xml, "Version"), 63);
        upgrade_file.size = atoi(ezxml_attr(fileinfo_xml, "Size"));
        strncpy(upgrade_file.describe, ezxml_attr(fileinfo_xml, "Describe"), 63);

        upgrade_file.interval = atoi(ezxml_txt(interval_xml));

        ez_iot_ota_set_info(&upgrade_file);
        ret = 0;
    } while (0);

    if (root_xml)
        ezxml_free(root_xml);

    return ret;
}

static ezxml_t xml_make_rsp(int32_t result)
{
    ezxml_t xml_root = ezxml_new("Response");
    ezxml_t xml_result = NULL;
    char buf[32] = {0};

    if (NULL == xml_root)
        return NULL;

    sprintf(buf, "%d", (int)result);
    xml_result = ezxml_add_child(xml_root, "Result", 0);
    if (NULL == xml_result)
    {
        ezxml_free(xml_root);
        return NULL;
    }

    ezxml_set_txt(xml_result, buf);

    return xml_root;
}

static int msg2dev_binding(const char *userid)
{
    ez_iot_on_binding_t msg = {0};
    ez_log_w(TAG_SDK, "binding");
    strncpy(msg.user_id, userid, sizeof(msg.user_id) - 1);
    ez_iot_set_binding_id((int8_t *)userid);
    ez_iot_clear_query_binding();
    ez_iot_clear_token();

    return ez_iot_get_callbacks()->recv_msg(ez_iot_msg_binding, &msg, sizeof(msg));
}

static int msg2dev_unbinding()
{
    ez_log_w(TAG_SDK, "unbinding");
    ez_iot_clear_query_binding();

    ez_iot_clear_binding_id();
    ez_iot_clear_token();

    return ez_iot_get_callbacks()->recv_msg(ez_iot_msg_unbinding, NULL, 0);
}

int32_t dev2cloud_msg_send(int8_t *buf, int32_t domain_id, int32_t cmd_id, const int8_t *cmd_version, uint8_t msg_response, uint32_t msg_seq)
{
    ezdev_sdk_kernel_pubmsg pubmsg;
    memset(&pubmsg, 0, sizeof(ezdev_sdk_kernel_pubmsg));

    pubmsg.msg_response = msg_response;
    pubmsg.msg_seq = msg_seq;

    pubmsg.msg_body = (unsigned char *)buf;
    pubmsg.msg_body_len = strlen((const char *)buf);

    pubmsg.msg_domain_id = domain_id;
    pubmsg.msg_command_id = cmd_id;

    strncpy(pubmsg.command_ver, (const char *)cmd_version, version_max_len - 1);

    ezdev_sdk_kernel_error sdk_error = ezdev_sdk_kernel_send(&pubmsg);
    if (sdk_error != ezdev_sdk_kernel_succ)
    {
        return -1;
    }

    return 0;
}

int32_t dev2cloud_banding_req(int8_t *token)
{
    int32_t rv = -1;
    bscJSON *root = NULL;
    char *json_str = NULL;

    do
    {
        if (NULL == (root = bscJSON_CreateObject()))
        {
            break;
        }

        bscJSON_AddStringToObject(root, "token", (const char *)token);
        json_str = bscJSON_PrintUnformatted(root);
        if (NULL == json_str)
        {
            break;
        }

        rv = dev2cloud_msg_send((int8_t *)json_str, KERNEL_BASE_DOMAIN_ID, kPu2CenPltBindUserWithTokenReq, (int8_t *)KERNEL_CMD_VER, MSG_TYPE_REQ, 0);
    } while (0);

    if (NULL != json_str)
    {
        free(json_str);
    }

    if (NULL != root)
    {
        bscJSON_Delete(root);
    }

    return rv;
}

int32_t cloud2dev_banding_rsp(void *buf, int32_t len)
{
    int32_t rv = -1;
    bscJSON *root = NULL;
    bscJSON *result = NULL;

    do
    {
        if (NULL == (root = bscJSON_Parse(buf)))
        {
            break;
        }

        if (NULL == (result = bscJSON_GetObjectItem(root, "result")))
        {
            break;
        }

        ez_iot_clear_token();
    } while (0);

    if (NULL != root)
    {
        bscJSON_Delete(root);
    }

    return rv;
}

void cloud2dev_set_ntpinfo_req(void *buf, int32_t len)
{
    bscJSON *pRoot = bscJSON_Parse(buf);
    bscJSON *NTP = NULL;
    bscJSON *json = NULL;
    ez_iot_ntp_info_t ntp_info = {0};

    do
    {
        if (NULL == pRoot)
        {
            break;
        }

        NTP = bscJSON_GetObjectItem(pRoot, "NTP");
        if (NULL == NTP)
        {
            break;
        }

        json = bscJSON_GetObjectItem(NTP, "Addr");
        if (NULL != json)
        {
            strncpy(ntp_info.host, json->valuestring, sizeof(ntp_info.host) - 1);
        }

        json = bscJSON_GetObjectItem(NTP, "Port");
        if (NULL != json)
        {
            ntp_info.port = json->valueint;
        }

        json = bscJSON_GetObjectItem(NTP, "Interval");
        if (NULL != json)
        {
            ntp_info.interval = json->valueint / 60;
        }

        json = bscJSON_GetObjectItem(NTP, "Timezone");
        if (NULL != json)
        {
            strncpy(ntp_info.timezone, json->valuestring, sizeof(ntp_info.timezone) - 1);
        }

        ez_iot_get_callbacks()->recv_msg(ez_iot_msg_ntp_info, &ntp_info, sizeof(ntp_info));
    } while (0);

    if (NULL != pRoot)
    {
        bscJSON_Delete(pRoot);
    }
}

int32_t dev2cloud_query_banding_req()
{
    int ret = -1;
    ezxml_t xml_root = ezxml_new("Request");
    ezxml_t xml_devserial = NULL;
    ezxml_t xml_auth = NULL;
    char *req_str = NULL;

    do
    {
        if (NULL == xml_root)
            break;

        xml_devserial = ezxml_add_child(xml_root, "DevSerial", 0);
        xml_auth = ezxml_add_child(xml_root, "Authorization", 0);

        if (!xml_devserial || !xml_auth)
        {
            break;
        }

        ezxml_set_txt(xml_devserial, ezdev_sdk_kernel_getdevinfo_bykey("dev_subserial"));

        req_str = ezxml_toxml(xml_root);
    } while (0);

    if (req_str)
    {
        ret = dev2cloud_msg_send((int8_t *)req_str, KERNEL_BASE_DOMAIN_ID, kPu2CenPltGetUserListReq, (int8_t *)KERNEL_CMD_VER, MSG_TYPE_REQ, 0);
        free(req_str);
    }

    if (xml_root)
        ezxml_free(xml_root);

    return ret;
}

int32_t dev2cloud_query_profile_url_req(char *sn, char *type, char *ver, bool need_schema)
{
    int32_t rv = -1;
    bscJSON *root = NULL;
    char *json_str = NULL;

    do
    {
        if (NULL == (root = bscJSON_CreateObject()))
        {
            break;
        }

        bscJSON_AddStringToObject(root, "devSerial", sn);
        bscJSON_AddStringToObject(root, "pid", type);
        bscJSON_AddStringToObject(root, "version", ver);
        bscJSON_AddStringToObject(root, "profileVersion", "3.0");
        bscJSON_AddBoolToObject(root, "requireSchema", need_schema);

        json_str = bscJSON_PrintUnformatted(root);
        if (NULL == json_str)
        {
            break;
        }

        ez_log_d(TAG_SDK, "profile req msg: %s", json_str);

        rv = dev2cloud_msg_send((int8_t *)json_str, KERNEL_BASE_DOMAIN_ID, Pu2CenPltQueryFeatureProfileReq, (int8_t *)KERNEL_CMD_VER, MSG_TYPE_REQ, 0);
    } while (0);

    if (NULL != json_str)
    {
        free(json_str);
    }

    if (NULL != root)
    {
        bscJSON_Delete(root);
    }

    return rv;
}

#ifdef COMPONENT_TSL_ENABLE
extern void ez_iot_tsl_adapter_profile_url(char *dev_sn, char *url, char *md5, int expire);
#endif

int32_t dev2cloud_query_profile_url_rsp(void *buf, int32_t len)
{
    int32_t rv = -1;
#ifdef COMPONENT_TSL_ENABLE
    bscJSON *root = NULL;
    bscJSON *url = NULL;
    bscJSON *expire = NULL;
    bscJSON *md5 = NULL;
    bscJSON *dev_sn = NULL;

    do
    {
        if (NULL == (root = bscJSON_Parse(buf)))
        {
            break;
        }

        if (NULL == (url = bscJSON_GetObjectItem(root, "url")) || bscJSON_String != url->type)
        {
            break;
        }

        if (NULL == (expire = bscJSON_GetObjectItem(root, "expire")) || bscJSON_Number != expire->type)
        {
            break;
        }

        if (NULL == (md5 = bscJSON_GetObjectItem(root, "md5")) || bscJSON_String != md5->type)
        {
            break;
        }

        if (NULL == (dev_sn = bscJSON_GetObjectItem(root, "devSerial")) || bscJSON_String != dev_sn->type)
        {
            break;
        }

        ez_iot_tsl_adapter_profile_url(dev_sn->valuestring, url->valuestring, md5->valuestring, expire->valueint);
        rv = 0;
    } while (0);

    if (NULL != root)
    {
        bscJSON_Delete(root);
    }
#endif
    return rv;
}

int32_t dev2cloud_query_banding_rsp(void *buf, int32_t len)
{
    int ret = -1;
    ezxml_t root_xml = NULL;
    ezxml_t result_xml = NULL;
    ezxml_t user_xml = NULL;
    char *name = NULL;

    do
    {
        if (NULL == (root_xml = ezxml_parse_str(buf, len)))
        {
            break;
        }

        name = ezxml_name(root_xml);
        if (NULL == name || 0 != strcmp(name, "Response"))
        {
            break;
        }

        result_xml = ezxml_child(root_xml, "Result");
        if (!result_xml)
        {
            break;
        }

        if (0x101c02 == atoi(ezxml_txt(result_xml)))
        {
            ret = msg2dev_unbinding();
            break;
        }

        user_xml = ezxml_child(root_xml, "User");
        if (!user_xml)
        {
            ret = msg2dev_unbinding();
        }
        else
        {
            ret = msg2dev_binding(ezxml_attr(user_xml, "Id"));
        }
    } while (0);

    if (root_xml)
        ezxml_free(root_xml);

    return ret;
}

int32_t cloud2dev_contact_bind(void *buf, int len, uint32_t seq)
{
    int32_t rv = -1;
    bscJSON *root = NULL;
    bscJSON *token = NULL;
    bscJSON *bindPeriod = NULL;
    ez_iot_challenge_t challenge_info = {0};
    char bind_result[32] = {0};

    do
    {
        if (NULL == (root = bscJSON_Parse(buf)))
        {
            break;
        }

        if (NULL == (token = bscJSON_GetObjectItem(root, "token")) || bscJSON_Number != token->type)
        {
            break;
        }

        if (NULL == (bindPeriod = bscJSON_GetObjectItem(root, "bindPeriod")) || bscJSON_Number != bindPeriod->type)
        {
            break;
        }

        challenge_info.challenge_code = token->valueint;
        challenge_info.validity_period = bindPeriod->valueint;
        rv = ez_iot_get_callbacks()->recv_msg(ez_iot_msg_contact_binding, &challenge_info, sizeof(challenge_info));
    } while (0);

    sprintf(bind_result, "{\"result\":%d}", rv);
    rv = dev2cloud_msg_send((int8_t *)bind_result, KERNEL_BASE_DOMAIN_ID, kCenPlt2PuBindUserTouchRsq, (int8_t *)KERNEL_CMD_VER, MSG_TYPE_RSP, seq);

    if (NULL != root)
    {
        bscJSON_Delete(root);
    }

    return rv;
}

int32_t dev2cloud_contact_bind_req(int32_t response_code)
{
    int32_t rv = -1;
    bscJSON *root = NULL;
    char *json_str = NULL;

    do
    {
        if (NULL == (root = bscJSON_CreateObject()))
        {
            break;
        }

        bscJSON_AddNumberToObject(root, "token", response_code);
        json_str = bscJSON_PrintUnformatted(root);
        if (NULL == json_str)
        {
            break;
        }

        rv = dev2cloud_msg_send((int8_t *)json_str, KERNEL_BASE_DOMAIN_ID, kPu2CenPltReportBindUserTouchWithTokenReq, (int8_t *)KERNEL_CMD_VER, MSG_TYPE_REQ, 0);
    } while (0);

    if (NULL != json_str)
    {
        free(json_str);
    }

    if (NULL != root)
    {
        bscJSON_Delete(root);
    }

    return rv;
}