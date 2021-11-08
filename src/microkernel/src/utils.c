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
#include <string.h>
#include "utils.h"
#include "mbedtls/rsa.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mkernel_internal_error.h"
#include "ezdev_sdk_kernel_error.h"

#if !defined(BSCOMPTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include BSCOMPTLS_CONFIG_FILE
#endif

#if defined(BSCOMPTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define bscomptls_printf     printf
#endif

#ifndef _REALTEK_RTOS_

int ezRandomGen(unsigned char *buf, unsigned int len)
{
	int ret = 0;
	char *identifier = "ezDevSDK";
	bscomptls_ctr_drbg_context ctr_drbg;
	bscomptls_entropy_context entropy;
	bscomptls_ctr_drbg_init( &ctr_drbg );
	bscomptls_entropy_init( &entropy );
	do 
	{
		if(0 != (ret = bscomptls_ctr_drbg_seed( &ctr_drbg, bscomptls_entropy_func, &entropy, (unsigned char*)identifier, strlen(identifier))))
			break;

		bscomptls_ctr_drbg_set_prediction_resistance( &ctr_drbg, BSCOMPTLS_CTR_DRBG_PR_OFF );

		ret = bscomptls_ctr_drbg_random( &ctr_drbg, buf, len);
	} while (0);

	bscomptls_ctr_drbg_free( &ctr_drbg );
	bscomptls_entropy_free( &entropy );

	return ret;
}

int ezRsaEncrypt(const unsigned char *pIn, int iInLen, unsigned char *pOut, int *iOutLen, const char *pN, const char *pE)
{
    int ret = BSCOMPTLS_EXIT_FAILURE;
	char *identifier = "ezDevSDK";
    bscomptls_rsa_context rsa;
	bscomptls_ctr_drbg_context ctr_drbg;
	bscomptls_entropy_context entropy;
    bscomptls_rsa_init( &rsa, BSCOMPTLS_RSA_PKCS_V15, 0 );
	bscomptls_ctr_drbg_init( &ctr_drbg );
	bscomptls_entropy_init( &entropy );

	do 
	{
		if(0 != bscomptls_ctr_drbg_seed( &ctr_drbg, bscomptls_entropy_func,
			&entropy, (const unsigned char *) identifier,
			strlen( identifier ) ))
		{
			break;
		}

		if (0 != bscomptls_mpi_read_string(&rsa.N, 16, pN) ||
			0 != bscomptls_mpi_read_string(&rsa.E, 16, pE))
		{
			break;
		}

		rsa.len = ( bscomptls_mpi_bitlen( &rsa.N ) + 7 ) >> 3;
		if( *iOutLen < rsa.len )
		{
			///< buf too small
			break;
		}

		if (0 != bscomptls_rsa_pkcs1_encrypt( &rsa, bscomptls_ctr_drbg_random, &ctr_drbg, BSCOMPTLS_RSA_PUBLIC,
			iInLen, pIn, pOut))
		{
			break;
		}
		
		ret = BSCOMPTLS_EXIT_SUCCESS;
		*iOutLen = rsa.len;
	} while (0);

	bscomptls_ctr_drbg_free( &ctr_drbg );
	bscomptls_entropy_free( &entropy );
	bscomptls_rsa_free( &rsa );
	return ret;
}

int ezRsaDecrypt(const unsigned char *pIn, int iInLen, unsigned char *pOut, int *iOutLen, const char *pP, const char *pQ, 
				 const char *pN, const char *pD, const char *pE)
{
	int ret = BSCOMPTLS_EXIT_FAILURE;
	char *identifier = "ezDevSDK";
	bscomptls_rsa_context rsa;
	bscomptls_ctr_drbg_context ctr_drbg;
	bscomptls_entropy_context entropy;
	bscomptls_rsa_init( &rsa, BSCOMPTLS_RSA_PKCS_V15, 0 );
	bscomptls_ctr_drbg_init( &ctr_drbg );
	bscomptls_entropy_init( &entropy );
	size_t olen = *iOutLen;

	do 
	{
		if(0 != bscomptls_ctr_drbg_seed( &ctr_drbg, bscomptls_entropy_func,
			&entropy, (const unsigned char *) identifier,
			strlen( identifier ) ))
		{
			break;
		}

		if (0 != bscomptls_mpi_read_string(&rsa.N, 16, pN) ||
			0 != bscomptls_mpi_read_string(&rsa.D, 16, pD) ||
			0 != bscomptls_mpi_read_string(&rsa.E, 16, pE) ||
			0 != bscomptls_mpi_read_string(&rsa.P, 16, pP) ||
			0 != bscomptls_mpi_read_string(&rsa.Q, 16, pQ))
		{
			break;
		}

		rsa.len = ( bscomptls_mpi_bitlen( &rsa.N ) + 7 ) >> 3;

		if( iInLen != rsa.len)		///< Invalid RSA signature format
		{
			break;
		}

		if (0 != bscomptls_rsa_pkcs1_decrypt( &rsa, bscomptls_ctr_drbg_random, &ctr_drbg, BSCOMPTLS_RSA_PRIVATE,
				&olen, pIn, pOut, olen))
		{
			break;
		}

		*iOutLen = (int)olen; 
		ret = BSCOMPTLS_EXIT_SUCCESS;
	} while (0);

	bscomptls_ctr_drbg_free( &ctr_drbg );
	bscomptls_entropy_free( &entropy );
	bscomptls_rsa_free( &rsa );
	return ret;
}

#endif // _REALTEK_RTOS_

unsigned int mkiE2ezE(unsigned int mkernel_err)
{
	unsigned int rv = 0;
	
	switch(mkernel_err)
	{
	case mkernel_internal_succ:
		rv = ezdev_sdk_kernel_succ;
		break;
	case mkernel_internal_no_start:
	case mkernel_internal_haven_stop:
	case mkernel_internal_invald_call:
		rv = ezdev_sdk_kernel_invald_call;
		break;
	case mkernel_internal_input_param_invalid:
		rv = ezdev_sdk_kernel_params_invalid;
		break;
	case mkernel_internal_mem_lack:
	case mkernel_internal_call_mqtt_buffer_too_short:
		rv = ezdev_sdk_kernel_buffer_too_small;
		break;
	case mkernel_internal_malloc_error:
		rv = ezdev_sdk_kernel_memory;
		break;
	case mkernel_internal_json_parse_error:
		rv = ezdev_sdk_kernel_json_invalid;
		break;
	case mkernel_internal_get_error_json:
		rv = ezdev_sdk_kernel_json_format;
		break;
	case mkernel_internal_extend_no_find:
		rv = ezdev_sdk_kernel_extend_no_find;
		break;
	case mkernel_internal_extend_full:
		rv = ezdev_sdk_kernel_extend_full;
		break;
	case mkernel_internal_extend_ready:
		rv = ezdev_sdk_kernel_extend_existed;
		break;
	case mkernel_internal_queue_full:
		rv = ezdev_sdk_kernel_queue_full;
		break;
	case mkernel_internal_value_load_err:
		rv = ezdev_sdk_kernel_value_load;
		break;
	case mkernel_internal_value_save_err:
		rv = ezdev_sdk_kernel_value_save;
		break;
	case mkernel_internal_create_sock_error:
	case mkernel_internal_net_poll_err:
	case mkernel_internal_net_getsockopt_error:
	case mkernel_internal_net_poll_event_err:
		rv = ezdev_sdk_kernel_net_create;
		break;
	case mkernel_internal_net_gethostbyname_error:
		rv = ezdev_sdk_kernel_net_dns;
		break;
	case mkernel_internal_net_connect_error:
	case mkernel_internal_net_connect_timeout:
	case mkernel_internal_lbs_connect_error:
	case mkernel_internal_call_mqtt_connect:
		rv = ezdev_sdk_kernel_net_connect;
		break;
	case mkernel_internal_das_need_reconnect:
	case mkernel_internal_net_socket_closed:
	case mkernel_internal_net_socket_err:
		rv = ezdev_sdk_kernel_net_disconnected;
		break;
	case mkernel_internal_net_send_error:
	case mkernel_internal_net_read_error:
	case mkernel_internal_net_read_error_request:
	case mkernel_internal_net_socket_error:
	case mkernel_internal_net_socket_timeout:
	case mkernel_internal_rev_invalid_packet:
	case mkernel_internal_call_mqtt_sub_error:
	case mkernel_internal_call_mqtt_pub_error:
		rv = ezdev_sdk_kernel_net_transmit;
		break;
	case mkernel_internal_platform_lbs_signcheck_error:
		rv = ezdev_sdk_kernel_lbs_authcode_mismatch;
		break;
	case mkernel_internal_platform_lbs_order_error:
	case mkernel_internal_platform_stun_process_invalid:
	case mkernel_internal_platform_das_process_invalid:
		rv = ezdev_sdk_kernel_lbs_invalid_call;
		break;
	case mkernel_internal_platform_invalid_data:
		rv = ezdev_sdk_kernel_lbs_invalid_call;
		break;
	case mkernel_internal_platform_devid_inconformity:
		rv = ezdev_sdk_kernel_lbs_devid_mismatch;
		break;
	case mkernel_internal_platform_masterkey_invalid:
		rv = ezdev_sdk_kernel_lbs_masterkey_mismatch;
		break;
	case mkernel_internal_platform_query_authcode_error:
		rv = ezdev_sdk_kernel_lbs_invalid_dev;
		break;
	case mkernel_internal_platform_stun_sessionkey_inconformity:
		rv = ezdev_sdk_kernel_lbs_sessionkey_mismatch;
		break;
	case mkernel_internal_platform_query_authcode_redis:
		rv = ezdev_sdk_kernel_lbs_server_exception;
		break;
	case mkernel_internal_platform_dec_error:
	case mkernel_internal_platform_enc_error:
		rv = ezdev_sdk_kernel_lbs_server_crypto;
		break;
	case mkernel_internal_platform_getstun_error:
		rv = ezdev_sdk_kernel_lbs_get_data;
		break;
	case mkernel_internal_platform_secretkey_decrypt_fail:
		rv = ezdev_sdk_kernel_secretkey_decrypt_fail;
		break;
	case mkernel_internal_platform_secretkey_overflow_windows:
		rv = ezdev_sdk_kernel_secretkey_overflow_windows;
		break;
	case mkernel_internal_platform_secretkey_no_user:
		rv = ezdev_sdk_kernel_secretkey_no_user;
		break;
	case mkernel_internal_platform_secretkey_serial_not_exist:
		rv = ezdev_sdk_kernel_secretkey_sn_not_exist;
		break;
	case mkernel_internal_platform_secretkey_again:
		rv = ezdev_sdk_kernel_secretkey_again;
		break;
	case mkernel_internal_mqtt_nosupport_protocol_version:
		rv = ezdev_sdk_kernel_das_nosupport_protocol_ver;
		break;
	case mkernel_internal_mqtt_unqualified_client_id:
		rv = ezdev_sdk_kernel_das_client_id_invalid;
		break;
	case mkernel_internal_mqtt_server_unusable:
		rv = ezdev_sdk_kernel_das_server_unusable;
		break;
	case mkernel_internal_mqtt_invalid_username:
		rv = ezdev_sdk_kernel_das_invalid_username;
		break;
	case mkernel_internal_mqtt_unauthorized:
		rv = ezdev_sdk_kernel_das_unauthorized;
		break;
	case mkernel_internal_mqtt_redirect:
		rv = ezdev_sdk_kernel_das_session_invaild;
		break;
	case mkernel_internal_mqtt_blacklist:
		rv = ezdev_sdk_kernel_das_force_dev_risk;
		break;
	case mkernel_internal_force_domain_risk:
		rv = ezdev_sdk_kernel_das_force_domain_risk;
		break;
	case mkernel_internal_force_cmd_risk:
		rv = ezdev_sdk_kernel_das_force_cmd_risk;
		break;
	case mkernel_internal_force_offline:
		rv = ezdev_sdk_kernel_das_force_offline;
		break;
	default:
		rv = ezdev_sdk_kernel_internal;
		break;
	}


	return rv;
}

int get_module_build_date(char* pbuf)
{
	int year = 0;
	int month = 0;
	int day = 0;
	char month_name[4] = {0};
	const char *all_mon_names[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

	sscanf(__DATE__, "%s%d%d", month_name, &day, &year);

	for (month = 0; month < 12; month++)
	{
		if (strcmp(month_name, all_mon_names[month]) == 0)
		{
			break;
		}
	}
	
	month++;
	year -= 2000;
	sprintf(pbuf, " build %02d%02d%02d", year, month, day);
	
	return 0;
}