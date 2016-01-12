//
//  event.h
//  klib
//
//  Created by LiKai on 16/1/8.
//  Copyright ? 2016Äê LiKai. All rights reserved.
//
#ifndef event_h
#define event_h
#include "types.h"
#include "socket.h"

#pragma comment(lib, "Kernel32.lib")

typedef struct socket_event_s
{
	socket_t client_socket;
	size_t data_len;
	void*  data;
}socket_event_t;

typedef void(*socket_event_callback)(socket_event_t* data);


typedef struct event_pool_s
{
#ifdef linux
	int epoll;
#else
	HANDLE completionPort;
#endif
	socket_t server_socket;
	socket_event_callback callback;
}event_pool_t;

#ifdef linux
#else
#include <WinSock2.h>
#include <Windows.h> 
#endif



status_t
event_pool_init(event_pool_t* event_pool,socket_t server_socket, socket_event_callback callback);

status_t  event_push();

status_t  event_dispatch(event_pool_t* event_pool);

#endif

