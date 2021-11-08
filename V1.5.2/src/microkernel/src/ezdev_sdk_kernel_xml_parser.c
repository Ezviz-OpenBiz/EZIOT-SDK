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
#include "ezdev_sdk_kernel_xml_parser.h"
#include "ezxml.h"
#include "mkernel_internal_error.h"
#include "sdk_kernel_def.h"
#include "ezconfig.h"
#include "mcuconfig.h"

mkernel_internal_error cenplt2pusetlbsdomainnamebydasreq_xml_parser(char* xml_buf, unsigned int len, char domain_name[ezdev_sdk_name_len])
{
	ezxml_t	xml_domain_name_node = NULL;
	ezxml_t xml_root = NULL;
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	do 
	{
		//Request
		xml_root = ezxml_parse_str(xml_buf, len);
		if (NULL == xml_root)
		{
			sdk_error = mkernel_internal_xml_parse_error;
			break;
		}
		xml_domain_name_node = ezxml_child(xml_root, "DomainName");
		if ( NULL == xml_domain_name_node )
		{
			sdk_error = mkernel_internal_get_error_xml;
			break;
		}

		if (strlen(ezxml_txt(xml_domain_name_node)) >= ezdev_sdk_name_len)
		{
			sdk_error = mkernel_internal_get_error_xml;
			break;
		}
		
		strncpy(domain_name, ezxml_txt(xml_domain_name_node), strlen(ezxml_txt(xml_domain_name_node)));
	} while (0);

	if (xml_root != NULL)
	{
		ezxml_free(xml_root);
		xml_root = NULL;
	}

	return sdk_error;
}