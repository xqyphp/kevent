//
//  event_win.c
//  klib
//
//  Created by LiKai on 16/1/8.
//  Copyright ? 2016年 LiKai. All rights reserved.
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

/**
* 结构体名称：PER_HANDLE_DATA
* 结构体存储：记录单个套接字的数据，包括了套接字的变量及套接字的对应的客户端的地址。
* 结构体作用：当服务器连接上客户端时，信息存储到该结构体中，知道客户端的地址以便于回访。
**/
typedef struct
{
	SOCKET socket;
	SOCKADDR_STORAGE ClientAddr;
	event_pool_t*    pool;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

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
		bRet = GetQueuedCompletionStatus(CompletionPort, &BytesTransferred, (PULONG_PTR)&PerHandleData, (LPOVERLAPPED*)&IpOverlapped, INFINITE);
		if (bRet == 0) {
			printf("GetQueuedCompletionStatus Error->%d\n", GetLastError());
			return -1;
		}
		PerIoData = (LPPER_IO_DATA)CONTAINING_RECORD(IpOverlapped, PER_IO_DATA, overlapped);

		// 检查在套接字上是否有错误发生
		if (0 == BytesTransferred) {
			closesocket(PerHandleData->socket);
			GlobalFree(PerHandleData);
			GlobalFree(PerIoData);
			continue;
		}

		// 开始数据处理，接收来自客户端的数据
		printf("A Client says->%s\n", PerIoData->databuff.buf);

		socket_event_t data;
		data.client_socket = PerHandleData->socket;
		data.data_len = PerIoData->databuff.len;
		data.data = PerIoData->databuff.buf;

		if(PerHandleData->pool->callback != K_NULL)
		{
			PerHandleData->pool->callback(&data);
		}

		// 为下一个重叠调用建立单I/O操作数据
		ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED)); // 清空内存
		PerIoData->databuff.len = 1024;
		PerIoData->databuff.buf = PerIoData->buffer;
		PerIoData->operationType = 0;  // read
		WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
	}

}

status_t
event_pool_init(event_pool_t* event_pool, socket_t server_socket, socket_event_callback callback)
{
	HANDLE completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == completionPort) {    // 创建IO内核对象失败  
		return K_ERROR;
	}
	event_pool->completionPort = completionPort;
	event_pool->server_socket = server_socket;
	event_pool->callback = callback;

	return K_SUCCESS;
}

status_t  event_push()
{
	
}

status_t  event_dispatch(event_pool_t* event_pool)
{

	// 创建IOCP线程--线程里面创建线程池  
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

		// 接收连接，并分配完成端，这儿可以用AcceptEx()  
		RemoteLen = sizeof(saRemote);
		acceptSocket = accept(event_pool->server_socket, (SOCKADDR*)&saRemote, &RemoteLen);
		if (SOCKET_ERROR == acceptSocket) {   // 接收客户端失败  
			printf("acceptSocket Error->%d\n", GetLastError());
			return -1;
		}

		// 创建用来和套接字关联的单句柄数据信息结构  
		PerHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));  // 在堆中为这个PerHandleData申请指定大小的内存  
		PerHandleData->socket = acceptSocket;
		PerHandleData->pool = event_pool;
		memcpy(&PerHandleData->ClientAddr, &saRemote, RemoteLen);
		//clientGroup.push_back(PerHandleData);       // 将单个客户端数据指针放到客户端组中  

													// 将接受套接字和完成端口关联  
		CreateIoCompletionPort((HANDLE)(PerHandleData->socket), event_pool->completionPort, (DWORD)PerHandleData, 0);


		// 开始在接受套接字上处理I/O使用重叠I/O机制  
		// 在新建的套接字上投递一个或多个异步  
		// WSARecv或WSASend请求，这些I/O请求完成后，工作者线程会为I/O请求提供服务      
		// 单I/O操作数据(I/O重叠)  
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

#endif