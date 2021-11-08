/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
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
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include "MQTTWin.h"
#include <ws2tcpip.h>

void TimerInit(Timer* timer)
{
//	timer->end_time = (struct timeval){0, 0};
	timer->end_time = time(NULL);
}

char TimerIsExpired(Timer* timer)
{
// 	struct timeval now, res;
// 	gettimeofday(&now, NULL);
// 	timersub(&timer->end_time, &now, &res);		
// 	return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);

	time_t now_t = time(NULL);
	
	if (now_t >= timer->end_time)
	{
		return (char)1;
	}
	else
	{
		return (char)0;
	}
}


void TimerCountdownMS(Timer* timer, unsigned int timeout)
{
// 	struct timeval now;
// 	struct timeval interval = {timeout / 1000, (timeout % 1000) * 1000};
// 	gettimeofday(&now, NULL);
// 	timeradd(&now, &interval, &timer->end_time);

	timer->end_time = time(NULL);

	timer->end_time += timeout/1000;
}


void TimerCountdown(Timer* timer, unsigned int timeout)
{
// 	struct timeval now;
// 	struct timeval interval = {timeout, 0};
// 	gettimeofday(&now, NULL);
// 	timeradd(&now, &interval, &timer->end_time);
	timer->end_time = time(NULL);

	timer->end_time += timeout;
}


int TimerLeftMS(Timer* timer)
{
// 	struct timeval now, res;
// 	gettimeofday(&now, NULL);
// 	timersub(&timer->end_time, &now, &res);
// 	//printf("left %d ms\n", (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000);
// 	return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;

	time_t now_t = time(NULL);
	if (now_t > timer->end_time)
	{
		return 0;
	}
	else
	{
		return (timer->end_time-now_t)* 1000;
	}
}


int linux_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
 	int bytes = 0;
// 	struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
// 	if (interval.tv_sec < 0 || (interval.tv_sec == 0 && interval.tv_usec <= 0))
// 	{
// 		interval.tv_sec = 0;
// 		interval.tv_usec = 100;
// 	}

//	setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));
	
	int timeout = timeout_ms;

	if (timeout <= 0)
	{
		timeout = 100;
	}
	

	//windows
	setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

	while (bytes < len)
	{
		int rc = recv(n->my_socket, &buffer[bytes], (size_t)(len - bytes), 0);
		if (rc == -1)
		{
// 			if (errno != ENOTCONN && errno != ECONNRESET)
// 			{
				bytes = -1;
				break;
//			}
		}
		else if (rc == 0)
		{
			bytes = -1;
			break;
		}
		
		bytes += rc;
	}
	return bytes;
}


int linux_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	struct timeval tv;
	int	rc = 0;

	tv.tv_sec = 0;  /* 30 Secs Timeout */
	tv.tv_usec = timeout_ms * 1000;  // Not init'ing this can cause strange errors

	//setsockopt(n->my_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,sizeof(struct timeval));
	setsockopt(n->my_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout_ms,sizeof(timeout_ms));

	rc = send(n->my_socket, buffer, len, 0);
	return rc;
}


void NetworkInit(Network* n)
{
	n->my_socket = 0;
	n->mqttread = linux_read;
	n->mqttwrite = linux_write;
}


int NetworkConnect(Network* n, char* addr, int port)
{
	int type = SOCK_STREAM;
	struct sockaddr_in address;
	int rc = -1;
//	sa_family_t family = AF_INET;
	struct addrinfo *result = NULL;
	struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};
	struct addrinfo* res = NULL;

	if ((rc = getaddrinfo(addr, NULL, &hints, &result)) == 0)
	{
		res = result;

		/* prefer ip4 addresses */
		while (res)
		{
			if (res->ai_family == AF_INET)
			{
				result = res;
				break;
			}
			res = res->ai_next;
		}

		if (result->ai_family == AF_INET)
		{
			address.sin_port = htons(port);
			address.sin_family = AF_INET;
			address.sin_addr = ((struct sockaddr_in*)(result->ai_addr))->sin_addr;
		}
		else
			rc = -1;

		freeaddrinfo(result);
	}

	if (rc == 0)
	{
		n->my_socket = socket(AF_INET, type, 0);
		if (n->my_socket != -1)
			rc = connect(n->my_socket, (struct sockaddr*)&address, sizeof(address));
	}

	return rc;
}


void NetworkDisconnect(Network* n)
{
	closesocket(n->my_socket);
	n->my_socket = 0;
}
