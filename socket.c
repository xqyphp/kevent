#include "socket.h"
#include "types.h"


status_t
socket_init()
{
#ifdef linux
	//noting to do
	return K_SUCCESS;
#else
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(2, 2);

	// 0 is success
	return WSAStartup(wVersionRequested, &wsaData);
#endif
}

status_t
socket_destory()
{
#ifdef linux
	//noting to do
#else
	return WSACleanup();
#endif
	return K_SUCCESS;
}

errno_t
socket_geterr()
{
#ifdef linux
	return errno;
#else
	return WSAGetLastError();
#endif
}


errno_t
socket_nobblock(socket_t s)
{
#ifdef linux
	//TODO
#else
	unsigned long iMode = 1;
	return ioctlsocket(s, FIONBIO, (unsigned long *)&iMode);//设置成非阻塞模式。
#endif
}
