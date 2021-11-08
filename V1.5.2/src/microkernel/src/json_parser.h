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
#ifndef H_JSON_PARSER_H_
#define H_JSON_PARSER_H_

#define JSON_PARSER_INTERFACE \
	extern mkernel_internal_error json_parse_devinfo( const char* dev_config_info, dev_basic_info* dev_info); \
	extern mkernel_internal_error json_parse_das_server_info( const char* jsonstring, das_info* das_server_info); \
	extern mkernel_internal_error json_parse_stun_server_info( const char* jsonstring, stun_info* stun_server_info);


#endif