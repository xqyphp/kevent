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
	socket_t clientfd;
	size_t data_len;
	void*  data_ptr;
}socket_event_t;

typedef void(*event_callback)(socket_event_t* data);


typedef struct event_manager_s
{
#ifdef linux
	int epoll;
#else
	HANDLE completionPort;
#endif
	socket_t server_socket;
	event_callback callback;
}event_manager_t;

#ifdef linux
#else
#include <WinSock2.h>
#include <Windows.h>
#endif

void 
setnonblocking(socket_t sockfd);

int server_create(const char* hostname, int portnumber);
void server_listen(socket_t serverfd);

status_t
event_manager_init(event_manager_t* event_manager,socket_t serverfd, event_callback callback);
status_t  event_dispatch(event_manager_t* event_manager);
void event_magager_destroy(event_manager_t* manager);
#endif

