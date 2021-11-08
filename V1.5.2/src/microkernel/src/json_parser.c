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
#include "json_parser.h"
#include "sdk_kernel_def.h"
#include "bscJSON.h"
#include "mkernel_internal_error.h"
#include "ezconfig.h"
#include "mcuconfig.h"

mkernel_internal_error json_parse_sap_devinfo( bscJSON* json_root, dev_basic_info* dev_info)
{
	bscJSON* json_dev_status	 = NULL;
	bscJSON* json_dev_subserial = NULL;
	bscJSON* json_dev_verification_code = NULL;
	bscJSON* json_dev_serial			 = NULL;			
	bscJSON* json_dev_firmwareversion		 = NULL;		
	bscJSON* json_dev_type				 = NULL;	
	bscJSON* json_dev_typedisplay	 = NULL;				
	bscJSON* json_dev_mac		 = NULL;					
	bscJSON* json_dev_nickname		 = NULL;				
	bscJSON* json_dev_firmwareidentificationcode	 = NULL;
	bscJSON* json_dev_oeminfo	= NULL;

	json_dev_status						= bscJSON_GetObjectItem(json_root, "dev_status");
	json_dev_subserial						= bscJSON_GetObjectItem(json_root, "dev_subserial");
	json_dev_verification_code					= bscJSON_GetObjectItem(json_root, "dev_verification_code");
	json_dev_serial						= bscJSON_GetObjectItem(json_root, "dev_serial");
	json_dev_firmwareversion				= bscJSON_GetObjectItem(json_root, "dev_firmwareversion");
	json_dev_type						= bscJSON_GetObjectItem(json_root, "dev_type");
	json_dev_typedisplay					= bscJSON_GetObjectItem(json_root, "dev_typedisplay");
	json_dev_mac							= bscJSON_GetObjectItem(json_root, "dev_mac");
	json_dev_nickname					= bscJSON_GetObjectItem(json_root, "dev_nickname");
	json_dev_firmwareidentificationcode	= bscJSON_GetObjectItem(json_root, "dev_firmwareidentificationcode");
	json_dev_oeminfo					= bscJSON_GetObjectItem(json_root, "dev_oeminfo");

	if (json_dev_status == NULL || json_dev_subserial == NULL || json_dev_serial == NULL || json_dev_verification_code == NULL || json_dev_firmwareversion == NULL || json_dev_type == NULL || \
		json_dev_typedisplay == NULL || json_dev_mac == NULL || json_dev_nickname == NULL || json_dev_firmwareidentificationcode == NULL || json_dev_oeminfo == NULL)
	{
		// ezdev_sdk_kernel_log_error(mkernel_internal_get_error_json, 0, "json_parse_sap loss field");
		return mkernel_internal_get_error_json;
	}

	if (json_dev_status->type != bscJSON_Number || \
		json_dev_subserial->type != bscJSON_String || json_dev_subserial->valuestring == NULL || \
		json_dev_serial->type != bscJSON_String || json_dev_serial->valuestring == NULL || \
		json_dev_verification_code->type != bscJSON_String || json_dev_verification_code->valuestring == NULL || \
		json_dev_firmwareversion->type != bscJSON_String || json_dev_firmwareversion->valuestring == NULL || \
		json_dev_type->type != bscJSON_String || json_dev_type->valuestring == NULL || \
		json_dev_typedisplay->type != bscJSON_String || json_dev_typedisplay->valuestring == NULL || \
		json_dev_mac->type != bscJSON_String || json_dev_mac->valuestring == NULL || \
		json_dev_nickname->type != bscJSON_String || json_dev_nickname->valuestring == NULL || \
		json_dev_firmwareidentificationcode->type != bscJSON_String || json_dev_firmwareidentificationcode->valuestring == NULL || \
		json_dev_oeminfo->type != bscJSON_Number
		)
	{
		// ezdev_sdk_kernel_log_error(mkernel_internal_get_error_json, 0, "json_parse_sap type err");
		return mkernel_internal_get_error_json;
	}

	if( strlen(json_dev_subserial->valuestring) >= ezdev_sdk_devserial_maxlen || \
		strlen(json_dev_serial->valuestring) >= ezdev_sdk_devserial_maxlen || \
		strlen(json_dev_verification_code->valuestring) >= ezdev_sdk_verify_code_maxlen || \
		strlen(json_dev_firmwareversion->valuestring) >= ezdev_sdk_name_len || \
		strlen(json_dev_type->valuestring) >= ezdev_sdk_name_len || \
		strlen(json_dev_typedisplay->valuestring) >= ezdev_sdk_name_len || \
		strlen(json_dev_mac->valuestring) >= ezdev_sdk_name_len || \
		strlen(json_dev_nickname->valuestring) >= ezdev_sdk_name_len || \
		strlen(json_dev_firmwareidentificationcode->valuestring) >= ezdev_sdk_identificationcode_max_len\
		)
	{
		// ezdev_sdk_kernel_log_error(mkernel_internal_get_error_json, 0, "json_parse_sap value range");
		return mkernel_internal_get_error_json;
	}

	dev_info->dev_status = json_dev_status->valueint;
	dev_info->dev_oeminfo = (EZDEV_SDK_UINT32)json_dev_oeminfo->valuedouble;
	
	strncpy(dev_info->dev_subserial, json_dev_subserial->valuestring, strlen(json_dev_subserial->valuestring));
	strncpy(dev_info->dev_verification_code, json_dev_verification_code->valuestring, strlen(json_dev_verification_code->valuestring));
	strncpy(dev_info->dev_serial, json_dev_serial->valuestring, strlen(json_dev_serial->valuestring));
	strncpy(dev_info->dev_firmwareversion, json_dev_firmwareversion->valuestring, strlen(json_dev_firmwareversion->valuestring));
	strncpy(dev_info->dev_type, json_dev_type->valuestring, strlen(json_dev_type->valuestring));
	strncpy(dev_info->dev_typedisplay, json_dev_typedisplay->valuestring, strlen(json_dev_typedisplay->valuestring));
	strncpy(dev_info->dev_mac, json_dev_mac->valuestring, strlen(json_dev_mac->valuestring));
	strncpy(dev_info->dev_nickname, json_dev_nickname->valuestring, strlen(json_dev_nickname->valuestring));
	strncpy(dev_info->dev_firmwareidentificationcode, json_dev_firmwareidentificationcode->valuestring, strlen(json_dev_firmwareidentificationcode->valuestring) - 1);

	return mkernel_internal_succ;
}

mkernel_internal_error json_parse_license_devinfo( bscJSON* json_root, dev_basic_info* dev_info)
{
	bscJSON* json_dev_productKey	 = NULL;
	bscJSON* json_dev_deviceName = NULL;
	bscJSON* json_dev_deviceLicense = NULL;			
	bscJSON* json_dev_firmwareversion		 = NULL;						
	bscJSON* json_dev_mac		 = NULL;					
	bscJSON* json_dev_nickname		 = NULL;				

	json_dev_productKey						= bscJSON_GetObjectItem(json_root, "dev_productKey");
	json_dev_deviceName						= bscJSON_GetObjectItem(json_root, "dev_deviceName");
	json_dev_deviceLicense					= bscJSON_GetObjectItem(json_root, "dev_deviceLicense");
	json_dev_firmwareversion				= bscJSON_GetObjectItem(json_root, "dev_firmwareversion");
	json_dev_mac							= bscJSON_GetObjectItem(json_root, "dev_mac");
	json_dev_nickname						= bscJSON_GetObjectItem(json_root, "dev_nickname");

	if (json_dev_productKey == NULL || json_dev_deviceName == NULL || json_dev_deviceLicense == NULL || json_dev_firmwareversion == NULL || json_dev_mac == NULL || \
		json_dev_nickname == NULL )
	{
		// ezdev_sdk_kernel_log_error(mkernel_internal_get_error_json, 0, "json_parse_license_devinfo loss some  field");
		return mkernel_internal_get_error_json;
	}

	if (json_dev_productKey->type != bscJSON_String || \
		json_dev_deviceName->type != bscJSON_String || json_dev_deviceLicense->valuestring == NULL || \
		json_dev_firmwareversion->type != bscJSON_String || json_dev_mac->valuestring == NULL || \
		json_dev_nickname->type != bscJSON_String )
	{
		// ezdev_sdk_kernel_log_error(mkernel_internal_get_error_json, 0, "json_parse_license_devinfo value type is error");
		return mkernel_internal_get_error_json;
	}
	
	if( strlen(json_dev_productKey->valuestring) >= ezdev_sdk_productkey_len || \
		strlen(json_dev_deviceName->valuestring) >= ezdev_sdk_productkey_len || \
		strlen(json_dev_deviceLicense->valuestring) >= ezdev_sdk_verify_code_maxlen || \
		strlen(json_dev_firmwareversion->valuestring) >= ezdev_sdk_name_len || \
		strlen(json_dev_mac->valuestring) >= ezdev_sdk_name_len || \
		strlen(json_dev_nickname->valuestring) >= ezdev_sdk_name_len)
	{
		// ezdev_sdk_kernel_log_error(mkernel_internal_get_error_json, 0, "json_parse_sap_devinfo value is too long");
		return mkernel_internal_get_error_json;
	}

	dev_info->dev_status = 1; //默认值

	sprintf(dev_info->dev_subserial, "%s:%s", json_dev_productKey->valuestring, json_dev_deviceName->valuestring);
	strncpy(dev_info->dev_verification_code, json_dev_deviceLicense->valuestring, strlen(json_dev_deviceLicense->valuestring));
	strncpy(dev_info->dev_serial, dev_info->dev_subserial, strlen(dev_info->dev_subserial));
	strncpy(dev_info->dev_firmwareversion, json_dev_firmwareversion->valuestring, strlen(json_dev_firmwareversion->valuestring));
	
	strncpy(dev_info->dev_type, json_dev_productKey->valuestring, strlen(json_dev_productKey->valuestring));
	strncpy(dev_info->dev_typedisplay, dev_info->dev_type, strlen(dev_info->dev_type));
	
	strncpy(dev_info->dev_mac, json_dev_mac->valuestring, strlen(json_dev_mac->valuestring));
	strncpy(dev_info->dev_nickname, json_dev_nickname->valuestring, strlen(json_dev_nickname->valuestring));

	return mkernel_internal_succ;
}

mkernel_internal_error json_parse_devinfo( const char* dev_config_info, dev_basic_info* dev_info)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	bscJSON* json_root	 = NULL;
	bscJSON* json_dev_auth_mode	 = NULL;
	bscJSON* json_dev_access_mode	 = NULL;

	do 
	{
		json_root = bscJSON_Parse(dev_config_info);
		if (json_root == NULL)
		{
			sdk_error = mkernel_internal_json_parse_error;
			break;
		}
		
		json_dev_auth_mode = bscJSON_GetObjectItem(json_root, "dev_auth_mode");
		if (json_dev_auth_mode == NULL || json_dev_auth_mode->type != bscJSON_Number)
		{
			dev_info->dev_auth_mode = sdk_dev_auth_sap;
		}
		else
		{
			if (json_dev_auth_mode->valueint == sdk_dev_auth_license)	
				dev_info->dev_auth_mode = sdk_dev_auth_license;
			else
				dev_info->dev_auth_mode = sdk_dev_auth_sap;
		}

		json_dev_access_mode = bscJSON_GetObjectItem(json_root, "dev_access_mode");
		if (json_dev_access_mode == NULL || json_dev_access_mode->type != bscJSON_Number)
		{
			dev_info->dev_access_mode = 0;
		}
		else
		{
			dev_info->dev_access_mode = json_dev_access_mode->valueint;
		}

		if(dev_info->dev_auth_mode == sdk_dev_auth_license)
		{
			sdk_error = json_parse_license_devinfo(json_root, dev_info);
		}
		else
		{
			sdk_error = json_parse_sap_devinfo(json_root, dev_info);
		}
	} while (0);
	
	if (json_root != NULL)
	{
		bscJSON_Delete(json_root);
		json_root = NULL;
	}
	return sdk_error;
}

extern mkernel_internal_error json_parse_das_server_info( const char* jsonstring, das_info* das_server_info)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	bscJSON * json_item = NULL;
	bscJSON *address_json_item = NULL;
	bscJSON *port_json_item = NULL;
	bscJSON *udpport_json_item = NULL;
	bscJSON *domain_json_item = NULL;
	bscJSON *serverid_json_item = NULL;
	bscJSON *dasinfo_json_item = NULL;
	bscJSON *das_json_item = NULL;

	do 
	{
		json_item = bscJSON_Parse((const char *)jsonstring);
		if (json_item == NULL)
		{
			sdk_error = mkernel_internal_json_parse_error;
			break;
		}

		das_json_item = bscJSON_GetObjectItem(json_item, "Type");
		if (das_json_item == NULL)
		{
			sdk_error = mkernel_internal_json_parse_error;
			break;
		}
		if (das_json_item->type != bscJSON_String || das_json_item->valuestring == NULL)
		{
			sdk_error = mkernel_internal_json_parse_error;
			break;
		}
		if (strcmp(das_json_item->valuestring, "DAS") != 0)
		{
			sdk_error = mkernel_internal_get_error_json;
			break;
		}

		dasinfo_json_item = bscJSON_GetObjectItem(json_item, "DasInfo");
		if (dasinfo_json_item == NULL)
		{
			sdk_error = mkernel_internal_get_error_json;
			break;
		}

		address_json_item = bscJSON_GetObjectItem(dasinfo_json_item, "Address");
		port_json_item = bscJSON_GetObjectItem(dasinfo_json_item, "Port");
		udpport_json_item = bscJSON_GetObjectItem(dasinfo_json_item, "UdpPort");
		domain_json_item = bscJSON_GetObjectItem(dasinfo_json_item, "Domain");
		serverid_json_item = bscJSON_GetObjectItem(dasinfo_json_item, "ServerID");
		if (NULL == serverid_json_item || NULL == port_json_item || NULL == domain_json_item || NULL == serverid_json_item || NULL == udpport_json_item)
		{
			sdk_error = mkernel_internal_get_error_json;
			break;
		}

		if (port_json_item->type != bscJSON_Number || udpport_json_item->type != bscJSON_Number ||\
			serverid_json_item->type != bscJSON_String || serverid_json_item->valuestring == NULL || \
			domain_json_item->type != bscJSON_String || domain_json_item->valuestring == NULL || \
			address_json_item->type != bscJSON_String || address_json_item->valuestring == NULL)
		{
			sdk_error = mkernel_internal_get_error_json;
			break;
		}

		if (strlen(address_json_item->valuestring) >= ezdev_sdk_ip_max_len)
		{
			// ezdev_sdk_kernel_log_debug(mkernel_internal_platform_appoint_error, 0, "parse_crypto_data_rsp_das Address >= 64");
			sdk_error = mkernel_internal_platform_appoint_error;
			break;
		}
		strncpy(das_server_info->das_address, address_json_item->valuestring, strlen(address_json_item->valuestring));

		if (strlen(domain_json_item->valuestring) >= ezdev_sdk_ip_max_len)
		{
			// ezdev_sdk_kernel_log_debug(mkernel_internal_platform_appoint_error, 0, "parse_crypto_data_rsp_das domain >= 64");
			sdk_error = mkernel_internal_platform_appoint_error;
			break;
		}
		strncpy(das_server_info->das_domain, domain_json_item->valuestring, strlen(domain_json_item->valuestring));

		if (strlen(serverid_json_item->valuestring) >= ezdev_sdk_name_len)
		{
			// ezdev_sdk_kernel_log_debug(mkernel_internal_platform_appoint_error, 0, "parse_crypto_data_rsp_das serverid >= 64");
			sdk_error = mkernel_internal_platform_appoint_error;
			break;
		}
		strncpy(das_server_info->das_serverid, serverid_json_item->valuestring, strlen(serverid_json_item->valuestring));

		das_server_info->das_port = port_json_item->valueint;
		das_server_info->das_udp_port = udpport_json_item->valueint;
	} while (0);

	if (NULL != json_item)
	{
		bscJSON_Delete(json_item);
		json_item = NULL;
	}

	return sdk_error;
}

mkernel_internal_error json_parse_stun_server_info( const char* jsonstring, stun_info* stun_server_info)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	bscJSON * json_item = NULL;
	bscJSON *stun_json_item = NULL;
	bscJSON *interval_json_item = NULL;
	bscJSON *stuninfo_json_item = NULL;
	int array_count = 0;
	bscJSON * stun1_json_item = NULL;
	bscJSON * stun2_json_item = NULL;
	bscJSON *stun1_address_json_item = NULL;
	bscJSON *stun1_port_json_item = NULL;
	bscJSON *stun1_domain_json_item = NULL;

	bscJSON *stun2_address_json_item = NULL;
	bscJSON *stun2_port_json_item = NULL;
	bscJSON *stun2_domain_json_item = NULL;

	do 
	{
		json_item = bscJSON_Parse((const char *)jsonstring);
		if (json_item == NULL)
		{
			sdk_error = mkernel_internal_json_parse_error;
			break;
		}

		stun_json_item = bscJSON_GetObjectItem(json_item, "Type");
		if (stun_json_item == NULL)
		{
			sdk_error = mkernel_internal_json_parse_error;
			break;
		}
		if (stun_json_item->type != bscJSON_String || stun_json_item->valuestring == NULL)
		{
			sdk_error = mkernel_internal_json_parse_error;
			break;
		}

		if (strcmp(stun_json_item->valuestring, "STUN") != 0)
		{
			sdk_error = mkernel_internal_get_error_json;
			break;
		}

		interval_json_item = bscJSON_GetObjectItem(json_item, "Interval");
		if (interval_json_item == NULL)
		{
			sdk_error = mkernel_internal_get_error_json;
			break;
		}
		if (interval_json_item->type != bscJSON_Number )
		{
			sdk_error = mkernel_internal_get_error_json;
			break;
		}

		stun_server_info->stun_interval = interval_json_item->valueint;

		stuninfo_json_item = bscJSON_GetObjectItem(json_item, "StunInfo");
		if (stuninfo_json_item == NULL)
		{
			sdk_error = mkernel_internal_get_error_json;
			break;
		}

		array_count = bscJSON_GetArraySize(stuninfo_json_item);
		if (array_count != 2)
		{
			sdk_error = mkernel_internal_get_error_json;
			break;
		}
		stun1_json_item = bscJSON_GetArrayItem(stuninfo_json_item, 0);
		stun2_json_item = bscJSON_GetArrayItem(stuninfo_json_item, 1);
		if (NULL == stun2_json_item || NULL == stun2_json_item)
		{
			sdk_error = mkernel_internal_get_error_json;
			break;
		}

		stun1_address_json_item = bscJSON_GetObjectItem(stun1_json_item, "Address");
		stun1_port_json_item = bscJSON_GetObjectItem(stun1_json_item, "Port");
		stun1_domain_json_item = bscJSON_GetObjectItem(stun1_json_item, "Domain");

		stun2_address_json_item = bscJSON_GetObjectItem(stun2_json_item, "Address");
		stun2_port_json_item = bscJSON_GetObjectItem(stun2_json_item, "Port");
		stun2_domain_json_item = bscJSON_GetObjectItem(stun2_json_item, "Domain");

		if (NULL == stun1_address_json_item || NULL == stun1_port_json_item || NULL == stun1_domain_json_item || NULL == stun2_address_json_item || NULL == stun2_port_json_item || NULL == stun2_domain_json_item)
		{
			sdk_error = mkernel_internal_get_error_json;
			break;
		}

		if (stun1_port_json_item->type != bscJSON_Number || \
			stun1_address_json_item->type != bscJSON_String || NULL == stun1_address_json_item->valuestring || \
			stun1_domain_json_item->type != bscJSON_String || NULL == stun1_domain_json_item->valuestring || \
			stun2_port_json_item->type != bscJSON_Number || \
			stun2_address_json_item->type != bscJSON_String || NULL == stun2_address_json_item->valuestring || \
			stun2_domain_json_item->type != bscJSON_String || NULL == stun2_domain_json_item->valuestring)
		{
			sdk_error = mkernel_internal_get_error_json;
			break;
		}

		if (strlen(stun1_address_json_item->valuestring) >= ezdev_sdk_ip_max_len)
		{
			// ezdev_sdk_kernel_log_debug(mkernel_internal_platform_appoint_error, 0, "parse_crypto_data_rsp_stun stun1 address >= 64");
			sdk_error = mkernel_internal_platform_appoint_error;
			break;
		}
		strncpy(stun_server_info->stun1_address, stun1_address_json_item->valuestring, strlen(stun1_address_json_item->valuestring));

		if (strlen(stun1_domain_json_item->valuestring) >= ezdev_sdk_ip_max_len)
		{
			// ezdev_sdk_kernel_log_debug(mkernel_internal_platform_appoint_error, 0, "parse_crypto_data_rsp_stun stun1 domain >= 64");
			sdk_error = mkernel_internal_platform_appoint_error;
			break;
		}
		strncpy(stun_server_info->stun1_domain, stun1_domain_json_item->valuestring, strlen(stun1_domain_json_item->valuestring));

		stun_server_info->stun1_port = stun1_port_json_item->valueint;

		if (strlen(stun2_address_json_item->valuestring) >= ezdev_sdk_ip_max_len)
		{
			sdk_error = mkernel_internal_platform_appoint_error;
			// ezdev_sdk_kernel_log_debug(mkernel_internal_platform_appoint_error, 0, "parse_crypto_data_rsp_stun stun2 address >= 64");
			break;
		}
		strncpy(stun_server_info->stun2_address, stun2_address_json_item->valuestring, strlen(stun2_address_json_item->valuestring));

		if (strlen(stun2_domain_json_item->valuestring) >= ezdev_sdk_ip_max_len)
		{
			sdk_error = mkernel_internal_platform_appoint_error;
			// ezdev_sdk_kernel_log_debug(mkernel_internal_platform_appoint_error, 0, "parse_crypto_data_rsp_stun stun2 domain >= 64");
			break;
		}
		strncpy(stun_server_info->stun2_domain, stun2_domain_json_item->valuestring, strlen(stun2_domain_json_item->valuestring));

		stun_server_info->stun2_port = stun2_port_json_item->valueint;

	} while (0);

	if (NULL != json_item)
	{
		bscJSON_Delete(json_item);
		json_item = NULL;
	}

	return sdk_error;
}