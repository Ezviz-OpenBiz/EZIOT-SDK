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
* 2021-01-28     xurongjun
*******************************************************************************/

#include <stdlib.h>
#include "fcntl.h"

#include <flashdb.h>
#include <string.h>
#include "ut_config.h"
#include "ez_iot.h"
#include "ez_iot_ota.h"
#include "ez_iot_log.h"
#include "ez_hal/hal_thread.h"
#include "utest.h"

#include "webclient.h"


#define URL "http://resource.ys7cloud.com/group2/M00/00/56/CtwQFmAdNv-AX3d9AAAJjqCE28w95.json"

#define URLSEND "http://10.10.35.12:8080"

#define URL_DUMPSERVER "http://10.220.17.18:8080/uploadDump"
#define URL_DUMPSERVER_TEST "http://172.20.206.211:8080/uploadDump"

#define POST_RESP_BUFSZ                1024
#define POST_HEADER_BUFSZ              1024

#define RAMDOM 11111
#define FORM_DATA "name=\"file...\"; filename=\"file.txt\""
#define FORM_DATA1 "form-data; name=\"authType\""

/* send HTTP POST request by common request interface, it used to receive longer data */
static int webclient_post_comm(const char *uri, const void *post_data, size_t data_len)
{
    struct webclient_session* session = RT_NULL;
    unsigned char *buffer = RT_NULL;
    int index, ret = 0;
    int bytes_read, resp_status;

    buffer = (unsigned char *) web_malloc(POST_RESP_BUFSZ);
    if (buffer == RT_NULL)
    {
        rt_kprintf("no memory for receive response buffer.\n");
        ret = -5;
        goto __exit;
    }

    /* create webclient session and set header response size */
    session = webclient_session_create(POST_HEADER_BUFSZ);
    if (session == RT_NULL)
    {
        ret = -5;
        goto __exit;
    }

    /* build header for upload */
    webclient_header_fields_add(session, "Content-Length: %d\r\n", strlen(post_data));
    webclient_header_fields_add(session, "Content-Type: application/octet-stream\r\n");

    /* send POST request by default header */
    if ((resp_status = webclient_post(session, uri, post_data, data_len)) != 200)
    {
        rt_kprintf("webclient POST request failed, response(%d) error.\n", resp_status);
        ret = -RT_ERROR;
        goto __exit;
    }

    rt_kprintf("webclient post response data: \n");
    do
    {
        bytes_read = webclient_read(session, buffer, POST_RESP_BUFSZ);
        if (bytes_read <= 0)
        {
            break;
        }

        for (index = 0; index < bytes_read; index++)
        {
            rt_kprintf("%c", buffer[index]);
        }
    } while (1);

    rt_kprintf("\n");

__exit:
    if (session)
    {
        webclient_close(session);
    }

    if (buffer)
    {
        web_free(buffer);
    }

    return ret;
}


/**
 * post file to http server.
 *
 * @param URI input server address
 * @param filename post data filename
 * @param form_data  form data
 *
 * @return <0: POST request failed
 *         =0: success
 */
int ut_webclient_post_file(const char* URI, const char* filename,
        const char* form_data)
{
    size_t length;
    char boundary[60];
    int fd = -1, rc = WEBCLIENT_OK;
    char *header = RT_NULL, *header_ptr;
    unsigned char *buffer = RT_NULL, *buffer_ptr;
    struct webclient_session* session = RT_NULL;
    int resp_data_len = 0;

    fd = open(filename, O_RDONLY, 0);
    if (fd < 0)
    {
        LOG_D("post file failed, open file(%s) error.", filename);
        rc = -WEBCLIENT_FILE_ERROR;
        goto __exit;
    }

    /* get the size of file */
    length = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    buffer = (unsigned char *) web_calloc(1, WEBCLIENT_RESPONSE_BUFSZ);
    if (buffer == RT_NULL)
    {
        LOG_D("post file failed, no memory for response buffer.");
        rc = -WEBCLIENT_NOMEM;
        goto __exit;
    }

    header = (char *) web_calloc(1, WEBCLIENT_HEADER_BUFSZ);
    if (header == RT_NULL)
    {
        LOG_D("post file failed, no memory for header buffer.");
        rc = -WEBCLIENT_NOMEM;
        goto __exit;
    }
    header_ptr = header;

    /* build boundary */
    rt_snprintf(boundary, sizeof(boundary), "--------------------------837727258985537817747805");

    /* build encapsulated mime_multipart information*/
    buffer_ptr = buffer;
    /* first boundary */
    buffer_ptr += rt_snprintf((char*) buffer_ptr,
            WEBCLIENT_RESPONSE_BUFSZ - (buffer_ptr - buffer), "--%s\r\n", boundary);
    buffer_ptr += rt_snprintf((char*) buffer_ptr,
            WEBCLIENT_RESPONSE_BUFSZ - (buffer_ptr - buffer),
            "Content-Disposition: form-data; %s\r\n", form_data);
    buffer_ptr += rt_snprintf((char*) buffer_ptr,
            WEBCLIENT_RESPONSE_BUFSZ - (buffer_ptr - buffer),
            "Content-Type: text/plain\r\n\r\n");
            
    /* calculate content-length */
    length += buffer_ptr - buffer;
    length += rt_strlen(boundary) + 6; /* add the first boundary */

	length += sizeof("Content-Disposition: form-data;name=\"authType\"\r\n");
	length += rt_strlen(boundary) + 50000;

    /* build header for upload */
    header_ptr += rt_snprintf(header_ptr,
            WEBCLIENT_HEADER_BUFSZ - (header_ptr - header),
            "Content-Length: %d\r\n", length);     

    header_ptr += rt_snprintf(header_ptr,
            WEBCLIENT_HEADER_BUFSZ - (header_ptr - header),
            "Content-Type: multipart/form-data; boundary=%s\r\n", boundary);

    session = webclient_session_create(WEBCLIENT_HEADER_BUFSZ);
    if(session == RT_NULL)
    {
        rc = -WEBCLIENT_NOMEM;
        goto __exit;
    }

    rt_strncpy(session->header->buffer, header, rt_strlen(header));
    session->header->length = rt_strlen(session->header->buffer);

    rc = webclient_post(session, URI, NULL, 0);
    if(rc < 0)
    {
        goto __exit;
    }

    /* send mime_multipart */
    webclient_write(session, buffer, buffer_ptr - buffer);

    /* send file data */
    while (1)
    {
        length = read(fd, buffer, WEBCLIENT_RESPONSE_BUFSZ);
        if (length <= 0)
        {
            break;
        }

        webclient_write(session, buffer, length);
    }

    /* send first boundary */
    rt_snprintf((char*) buffer, WEBCLIENT_RESPONSE_BUFSZ, "\r\n--%s\r\n", boundary);   
    webclient_write(session, buffer, rt_strlen(boundary) + 6);

	/* dumpserver 需要第二个mime date */
	rt_snprintf((char*) buffer, WEBCLIENT_RESPONSE_BUFSZ, "Content-Disposition: form-data;name=\"authType\"\r\n");
	webclient_write(session, buffer, rt_strlen(buffer));

	rt_snprintf((char*) buffer, WEBCLIENT_RESPONSE_BUFSZ, "\r\npu\r\n--%s--\r\n",boundary);
	webclient_write(session, buffer, rt_strlen(buffer));



    extern int webclient_handle_response(struct webclient_session *session);
    if( webclient_handle_response(session) != 200)
    {
        rc = -WEBCLIENT_ERROR;
        goto __exit;
    }

    resp_data_len = webclient_content_length_get(session);
    if (resp_data_len > 0)
    {
        int bytes_read = 0;

        rt_memset(buffer, 0x00, WEBCLIENT_RESPONSE_BUFSZ);
        do
        {
            bytes_read = webclient_read(session, buffer,
                resp_data_len < WEBCLIENT_RESPONSE_BUFSZ ? resp_data_len : WEBCLIENT_RESPONSE_BUFSZ);
            if (bytes_read <= 0)
            {
                break;
            }
            resp_data_len -= bytes_read;
        } while(resp_data_len > 0);
    }

__exit:
    if (fd >= 0)
    {
        close(fd);
    }

    if (session != RT_NULL)
    {
        webclient_close(session);
    }

    if (buffer != RT_NULL)
    {
        web_free(buffer);
    }

    if (header != RT_NULL)
    {
        web_free(header);
    }

    return rc;
}

void ut_loggerUp()
{

	char *uri = NULL;

	int rsp_status = 0;
	char buf[1024] = "";
	int already_len = 0;
	char *response = NULL;

	ut_webclient_post_file(URL_DUMPSERVER, "file.txt",
			FORM_DATA);
}

void ut_fileDown()
{
	int rsp_status = 0;
	char *buf = NULL;
	int already_len = 0;

	struct webclient_session *session = NULL;
	session = webclient_session_create(512);
    int read_len = 0;

    do
    {
       	rsp_status = webclient_get(session, URL);
        if (200 != rsp_status)
        {
            ez_log_e(TAG_TSL, "webclient get request failed. http_code: %d", rsp_status);
            break;
        }

        if (0 >= session->content_length)
        {
            ez_log_e(TAG_TSL, "content length illegal: %d", session->content_length);
            break;
        }

        buf = (char *)malloc(session->content_length + 1);
        if (NULL == buf)
        {
            ez_log_e(TAG_TSL, "memory not enough.");
            break;
        }
        memset(*buf, 0, session->content_length + 1);

        int read_len = 0;
        do
        {
            read_len = webclient_read(session, buf + already_len, session->content_length - already_len);
            if (0 >= read_len)
            {
                break;
            }
            if (already_len == session->content_length)
            {
                break;
            }
            already_len += read_len;

        } while (true);       
    } while (0);

	webclient_close(session);

}


UTEST_TC_EXPORT(ut_fileDown, NULL, NULL, DEFAULT_TIMEOUT_S);
UTEST_TC_EXPORT(ut_loggerUp, NULL, NULL, DEFAULT_TIMEOUT_S);


