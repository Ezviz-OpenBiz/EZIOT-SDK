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
#ifndef H_EZDEV_SDK_KERNEL_XML_PARSER_H_
#define H_EZDEV_SDK_KERNEL_XML_PARSER_H_

#define EZDEV_SDK_KERNEL_XML_PARSER_INTERFACE	 \
	mkernel_internal_error cenplt2pusetlbsdomainnamebydasreq_xml_parser(char* xml_buf, unsigned int len, char domain_name[ezdev_sdk_name_len]);

#endif