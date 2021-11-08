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
* 
*******************************************************************************/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <unistd.h>
#include <time.h>

#include "ezconfig.h"
#include "mcuconfig.h"

#include "ezdev_sdk_kernel.h"
#include "cjson/bscJSON.h"
#include "hal_time.h"
#include "hal_thread.h"
#include "ez_iot_log.h"
#include "webclient.h"
#include "ez_iot_ota.h"
#include "ez_ota_extern.h"
#include "ez_ota_def.h"
#include "ez_ota.h"
#include "ez_ota_bus.h"
#include "ez_ota_user.h"

#include "mbedtls/md5.h"

#define ez_ota_url_len 270
#define ez_ota_md5_len 32
#define MAX_OTA_FILE 2

static int g_download_status = 0;
 
/**
 * \brief   
 */
typedef struct
{
    char url[270];
    char check_sum[33];
    unsigned int total_size;
    int recv_size;
    int timeout_s;
    int retry_max;
    get_file_cb file_cb;    //数据下载的回调函数，通知应用层去存flash，或者发送到子设备
    notify_cb notify;        // 升级协议中带的数据长度下载完毕后，通知应用层
    void *user_data;
} ez_ota_file_info_t;


ez_ota_file_info_t g_struFiles_info[2]= {0} ;

ez_err_e ez_progress_report(const ota_res_t *pres, const int8_t *pmodule, const int8_t *perr_msg, ota_errcode_e errcode, int8_t status, int16_t progress)
{
    ez_err_e ota_err = ez_errno_succ;
    char *sz_progress = NULL;
    char err_code[16] = {0};
    int32_t  msg_len  = 0;
    unsigned int msg_seq  = 0;
    bscJSON* pJsRoot  = bscJSON_CreateObject();
    ez_log_d(TAG_OTA, "ez_progress_report,status:%d code:%d, process:%d",status, errcode, progress);
    sprintf(err_code,"0x%08x", errcode);
    do
    {
        if (NULL == pJsRoot)
        {
            ez_log_e(TAG_OTA, "ez_progress_report json create err");
            ota_err = ez_errno_ota_json_creat_err;
            break;
        }
        bscJSON_AddNumberToObject(pJsRoot, "status", status);
        bscJSON_AddNumberToObject(pJsRoot, "process", progress);
        bscJSON_AddStringToObject(pJsRoot, "code", err_code);
        if (perr_msg)
        {
            ez_log_d(TAG_OTA, "ez_progress_report, errorMsg:%s", (const char *)perr_msg);
            bscJSON_AddStringToObject(pJsRoot, "errorMsg", (const char *)perr_msg);
        }
        else
        {
            bscJSON_AddStringToObject(pJsRoot, "errorMsg", "success");
        }
        if (pmodule)
        {
            ez_log_d(TAG_OTA, "ez_progress_report, module:%s", (const char *)pmodule);
            bscJSON_AddStringToObject(pJsRoot, "module", (const char *)pmodule);
        }
        else
        {
            bscJSON_AddStringToObject(pJsRoot, "module", "");
        }
        sz_progress = bscJSON_PrintUnformatted(pJsRoot);
        if (NULL == sz_progress)
        {
            ez_log_e(TAG_OTA, "query_packet bscJSON_Print err");
            ota_err = ez_errno_ota_json_format_err;
            break;
        }
        msg_len = strlen(sz_progress);
        ota_err = ez_ota_send_msg_to_platform((unsigned char *)sz_progress, msg_len, pres, "report", "progress", EZ_OTA_REQ, &msg_seq, 0);
        if (ez_errno_succ != ota_err)
        {
            ez_log_e(TAG_OTA, "ez_progress_report failed, status:%d", status);
            break;
        }
    } while (0);

    if (sz_progress)
    {
        free(sz_progress);
    }
    if (pJsRoot)
    {
        bscJSON_Delete(pJsRoot);
    }
    return ota_err;
}

static int file_download(char *url, unsigned int *total_len, unsigned int readlen, unsigned int *offset,
                         char *check_sum, get_file_cb file_cb, void *user_data, int retry_flg)
{
    int ret = ota_code_download;
    unsigned int need_readlen = 1;
    httpclient_t *h_client = NULL;
    char *recvbuffer = (char *)malloc(readlen);
    int status = 0;
    unsigned int content_len = 0;

	bscomptls_md5_context md5_ctx;			//md5校验 句柄		
	char md5_output[16];				   //md5 校验值			
	char md5_hex[16 * 2 + 1];			  //md5 转成字符串值
	
    ez_log_d(TAG_OTA, "file_download: total_len:%d, readlen:%d, offset:%d", *total_len, readlen, *offset);


	bscomptls_md5_init(&md5_ctx);
	bscomptls_md5_starts(&md5_ctx);

    do
    {
        if (NULL == recvbuffer)
        {
            ez_log_e(TAG_OTA, "malloc recvbuffer err");
            ret = ota_code_genneral;
            goto end;
        }
        h_client = webclient_session_create(1024);
        if (NULL == h_client)
        {
            ez_log_e(TAG_OTA, "webclient_session_create failed");
            ret = ota_code_genneral;
            goto end;
        }
        ez_log_d(TAG_OTA, "url:%s", url);
        if (0 != retry_flg)
        {
            status = webclient_get_position(h_client, url, (int)*offset);
            if (status != 200 && status != 206)
            {
                ez_log_e(TAG_OTA, "webclient_get_position faield,status:%d, offset:%d", status, *offset);
                ret = ota_code_download;
            	goto end;
            }
        }
        else
        {
            status = webclient_get(h_client, url);
            if (status != 200 && status != 206)
            {
                ez_log_e(TAG_OTA, "webclient_get faield,status:%d", *offset);
                ret = ota_code_download;
            	goto end;
            }
        }
        content_len = webclient_content_length_get(h_client);
        if (content_len != *total_len - *offset)
        {
            ez_log_d(TAG_OTA, "content_len not match,content_len:%d,real:%d", content_len, *total_len - *offset);
            ret = ota_code_download;
            goto end;
        }
        ez_log_d(TAG_OTA, "content_len :%d", content_len);
        memset(recvbuffer, 0, readlen);
        while (need_readlen > 0)
        {
            need_readlen = *total_len - *offset;
            readlen = (need_readlen <= readlen) ? need_readlen : readlen;
            if ((unsigned int)webclient_read(h_client, recvbuffer, readlen) != readlen)
            {
                
                ez_log_d(TAG_OTA, "read file err:readlen:%d, need_readlen:%d, offset:%d", readlen, need_readlen, *offset);
                ret = ota_code_download;
            	goto end;
            }
            if (1 == ez_get_exit_status())
            {
				ez_log_e(TAG_OTA, "ota cancel");

                ret = ota_code_cancel;
                goto end;
            }
            need_readlen -= readlen;
            ret = file_cb(*total_len, *offset, recvbuffer, readlen, user_data);

            if (0 != ret)
            {
				ez_log_e(TAG_OTA, "ota  file_cb error,ret=0x%x", ret);

                ret = ota_code_genneral;
                goto end;
            }
            *offset += readlen;
       
            bscomptls_md5_update(&md5_ctx, (const unsigned char *)recvbuffer, readlen);
            
        }
    } while (0);

	bscomptls_md5_finish(&md5_ctx,md5_output);
	ez_log_w(TAG_OTA, "ota total Write len:%d", *total_len);

	do
	{
		bscomptls_hexdump(md5_output, sizeof(md5_output), 0, md5_hex);

		if (0 != strcmp((char *)md5_hex,check_sum))
		{
			ez_log_e(TAG_OTA, "check_sum:%s,%s",md5_hex,check_sum);
			ret = ota_code_sign;
		}
		else
		{
			ez_log_i(TAG_OTA, "check_sum right");
		}
	} while (0);

   
end:
    if (recvbuffer)
    {
        free(recvbuffer);
    }
    if (h_client)
    {
        webclient_close(h_client);
        h_client = NULL;
    }
	ez_log_d(TAG_OTA, "download,ret: 0x%x", ret);

    return ret;
}

static void ota_file_download_thread(void *arg)
{
    int ret = -1;
    int retry_times = 0;
    unsigned int offset = 0;
    int max_try_time = 0;
    int retry_flag = 0;

	for(int file_index=0;file_index<MAX_OTA_FILE;file_index++)
	{
		offset = 0; //下载第二个文件时，offset清0

		//hal_thread_sleep(1500); //http 大量占用网络缓冲区，先休眠下 

		ez_ota_file_info_t *file_info = &g_struFiles_info[file_index];
		if(0 == file_info->total_size )
		{	
			continue;
		}
		max_try_time = file_info->retry_max;
		unsigned int total_len = file_info->total_size;
		if(retry_times > max_try_time)
		{
			 ez_log_e(TAG_OTA, "network error,break the upgrade process");
			 break;
		}
		
	    do
	    {
	        ret = file_download(file_info->url, &total_len, file_info->recv_size, &offset, file_info->check_sum,
	                            file_info->file_cb, file_info->user_data, retry_flag);
	        if (ota_code_download == ret)
	        {
	            if (++retry_times > max_try_time)
	            {
	                ez_log_e(TAG_OTA, "download error,has retry %d times",retry_times);
	                break;
	            }
	            if (1 == ez_get_exit_status())
	            {
					ez_log_e(TAG_OTA, "application has cancel the upgrade");
	                break;
	            }
	            ez_log_w(TAG_OTA, "get ota file fialed retry_times:%d", retry_times);
	            retry_flag = 1;
	        }

	    } while (ota_code_download == ret);	    

		if (0 != ret)
		{
			ez_log_e(TAG_OTA, "http_upgrade_download return err,ret:%d, retry_times:%d", ret, retry_times);
			file_info->notify(result_failed, file_info->user_data);
		}
		else
		{
			file_info->notify(result_suc, file_info->user_data);
		}
	}
    g_download_status = 0;

    return;
}

ez_err_e ez_ota_file_download(ota_download_info_t *input_info, get_file_cb file_cb, notify_cb notify, void *user_data)
{
    ez_err_e err = ez_errno_succ;


    ez_ota_file_info_t *file_info = NULL;


    char thread_name[32] = {0};
    void *handle = NULL;
    ez_log_d(TAG_OTA, "ezdev_ota_download start!");
    if (0 != g_download_status)
    {
		if(0 == g_struFiles_info[1].total_size) //第二个升级文件没有被使用过
		{

			file_info = &g_struFiles_info[1];			
	        file_info->file_cb = file_cb;
	        file_info->notify = notify;
	        file_info->recv_size = input_info->block_size;
	        file_info->timeout_s = input_info->timeout_s;
	        file_info->retry_max = input_info->retry_max;
	        file_info->total_size = input_info->total_size;
	        file_info->user_data = user_data;

	        strncpy(file_info->url, (char *)input_info->url, sizeof(file_info->url) - 1);
	        strncpy(file_info->check_sum, (char *)input_info->degist, sizeof(file_info->check_sum) - 1);

	        return ez_errno_succ;
        }
        else
        {

			return ez_errno_ota_download_already;
        }
        
    }

    do
    {
		/**/
		if(0 == g_struFiles_info[0].total_size)    //第一个升级文件没有被使用过
		{
		
			file_info = &g_struFiles_info[0];			
	        file_info->file_cb = file_cb;
	        file_info->notify = notify;
	        file_info->recv_size = input_info->block_size;
	        file_info->timeout_s = input_info->timeout_s;
	        file_info->retry_max = input_info->retry_max;
	        file_info->total_size = input_info->total_size;
	        file_info->user_data = user_data;

	        strncpy(file_info->url, (char *)input_info->url, sizeof(file_info->url) - 1);
	        strncpy(file_info->check_sum, (char *)input_info->degist, sizeof(file_info->check_sum) - 1);
			
	        strncpy(thread_name, "ez_ota_download", sizeof(thread_name) - 1);
       
	        handle = hal_thread_create((int8_t *)thread_name, ota_file_download_thread, 1024 * 6, 1, (void *)file_info);
	        if (NULL == handle)
	        {
	            ez_log_e(TAG_OTA, "ota_download_tread create error");
	            err = ez_errno_ota_memory;
	            break;
	        }
	        
	        hal_thread_detach(handle);
	        handle = NULL;
		}
		else
		{

		}

        g_download_status = 1;

    } while (0);

    if (ez_errno_succ != err)
    {
		#if 0
        if (file_info)
        {
            free(file_info);
            file_info = NULL;
        }
        #endif
    }

    return err;
}



