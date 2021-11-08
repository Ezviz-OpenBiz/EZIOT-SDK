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

#ifndef DNA_HTTP_SERVER_H
#define DNA_HTTP_SERVER_H

enum {
    DNA_HTTPD_METHOD_GET = 0,
    DNA_HTTPD_METHOD_POST,
    DNA_HTTPD_METHOD_PUT,
};

typedef int (* httpd_recv_handle_t)(int sd, int method, const char * hostname, const char * path, const char * body, int len);

int dna_httpd_init(unsigned short port);//0: default 80
int dna_httpd_deinit(void);
int dna_httpd_event_register(int (* event_handle)(int sd, int method, const char * hostname, const char * path, const char * body, int len));
int dna_httpd_send(int sd, const unsigned char * buffer, int len);
int dna_httpd_event_unregister(void);

#endif

