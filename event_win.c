//
//  event_win.c
//  klib
//
//  Created by LiKai on 16/1/8.
//  Copyright ? 2016Äê LiKai. All rights reserved.
//
#ifndef linux

#include "event.h"
#include "socket.h"

typedef struct
{
	OVERLAPPED overlapped;
	WSABUF databuff;
	char buffer[2014];
	int BufferLen;
	int operationType;
}PER_IO_OPERATEION_DATA, *LPPER_IO_OPERATION_DATA, *LPPER_IO_DATA, PER_IO_DATA;


typedef struct
{
	SOCKET socket;
	SOCKADDR_STORAGE ClientAddr;
	event_manager_t*    pool;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;


static DWORD WINAPI ServerWorkThread(LPVOID IpParam);


void
setnonblocking(socket_t sockfd)
{
	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

socket_tserver_create(const char* hostname, int portnumber)
{

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(portnumber);

	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		printf("Create Socket Failed!");
		return -1;
	}

	if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
	{
		printf("Server Bind Port : %d Failed!", portnumber);
		return -1;
	}
	return sockfd;
}
void 
server_listen(socket_t serverfd)
{
	if (listen(serverfd, 20))
	{
		printf("Server Listen Failed!");
		exit(1);
	}
}

status_t
event_manager_init(event_manager_t* event_pool, socket_t server_socket, event_callback callback)
{
	HANDLE completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == completionPort) {  
		return K_ERROR;
	}
	event_pool->completionPort = completionPort;
	event_pool->server_socket = server_socket;
	event_pool->callback = callback;

	return K_SUCCESS;
}

voidevent_magager_destroy(event_manager_t* manager)
{
	
}

status_t  event_dispatch(event_manager_t* event_pool)
{
	SYSTEM_INFO mySysInfo;
	GetSystemInfo(&mySysInfo);

	for (DWORD i = 0; i < (mySysInfo.dwNumberOfProcessors * 2); ++i) {
		HANDLE ThreadHandle = CreateThread(NULL, 0, ServerWorkThread, event_pool->completionPort, 0, NULL);
		if (NULL == ThreadHandle) {
			return K_ERROR;
		}
		CloseHandle(ThreadHandle);
	}

	while (K_TRUE) {
		PER_HANDLE_DATA * PerHandleData = NULL;
		SOCKADDR_IN saRemote;
		int RemoteLen;
		SOCKET acceptSocket;

		RemoteLen = sizeof(saRemote);
		acceptSocket = accept(event_pool->server_socket, (SOCKADDR*)&saRemote, &RemoteLen);
		if (SOCKET_ERROR == acceptSocket) {  
			printf("acceptSocket Error->%d\n", GetLastError());
			return -1;
		}

	
		PerHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));  
		PerHandleData->socket = acceptSocket;
		PerHandleData->pool = event_pool;
		memcpy(&PerHandleData->ClientAddr, &saRemote, RemoteLen);

		CreateIoCompletionPort((HANDLE)(PerHandleData->socket), event_pool->completionPort, (DWORD)PerHandleData, 0);

		LPPER_IO_OPERATION_DATA PerIoData = NULL;
		PerIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATEION_DATA));
		ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED));
		PerIoData->databuff.len = 1024;
		PerIoData->databuff.buf = PerIoData->buffer;
		PerIoData->operationType = 0;    // read  

		DWORD RecvBytes;
		DWORD Flags = 0;
		WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
	}
}

static DWORD WINAPI ServerWorkThread(LPVOID IpParam)
{
	HANDLE CompletionPort = (HANDLE)IpParam;
	DWORD BytesTransferred;
	LPOVERLAPPED IpOverlapped;
	LPPER_HANDLE_DATA PerHandleData = NULL;
	LPPER_IO_DATA PerIoData = NULL;
	DWORD RecvBytes;
	DWORD Flags = 0;
	BOOL bRet = K_FALSE;

	while (K_TRUE) {

		bRet = GetQueuedCompletionStatus(CompletionPort, &BytesTransferred,
			(PULONG_PTR)&PerHandleData, (LPOVERLAPPED*)&IpOverlapped, INFINITE);

		if (bRet == 0) {
			printf("GetQueuedCompletionStatus Error->%d\n", GetLastError());
			return -1;
		}
		PerIoData = (LPPER_IO_DATA)CONTAINING_RECORD(IpOverlapped, PER_IO_DATA, overlapped);


		if (0 == BytesTransferred) {
			closesocket(PerHandleData->socket);
			GlobalFree(PerHandleData);
			GlobalFree(PerIoData);
			continue;
		}


		printf("A Client says->%s\n", PerIoData->databuff.buf);

		socket_event_t data;
		data.clientfd = PerHandleData->socket;
		data.data_len = PerIoData->databuff.len;
		data.data_ptr = PerIoData->databuff.buf;

		if (PerHandleData->pool->callback != K_NULL)
		{
			PerHandleData->pool->callback(&data);
		}

		ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED));
		PerIoData->databuff.len = 1024;
		PerIoData->databuff.buf = PerIoData->buffer;
		PerIoData->operationType = 0;  // read
		WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
	}

}

#endif
