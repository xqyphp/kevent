// TestLib.c : Defines the entry point for the console application.
//

#include "socket.h"
#include "event.h"
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero

#define SERVER_PORT 6666

void my_event_callback(socket_event_t* data)
{
	printf("callback->%s\n",(char*)data->data_ptr);
	const char* sdata = "sdfasssssssssssssssssssssssssfeqf!";
	socket_send(data->clientfd, sdata, strlen(sdata)+1);
	close(data->clientfd);
	data->clientfd = -1;
}


void test_server()
{
	socket_t serverfd = server_create("127.0.0.1", SERVER_PORT);
	setnonblocking(serverfd);

	server_listen(serverfd);

	event_manager_t event_manager;

	event_manager_init(&event_manager, serverfd, my_event_callback);

	event_dispatch(&event_manager);

	event_magager_destroy(&event_manager);
}

void test_client()
{
	socket_t client_socket = client_create("127.0.0.1", SERVER_PORT);

	char buffer[] = "Hello,World!";
	socket_send(client_socket, buffer, sizeof(buffer));
	int len = 0;
	char* recv = socket_recv_all(client_socket, &len);
	if (recv){
		printf(recv);
	}

	close(client_socket);
}

int main()
{
	socket_init();

	test_client();

	socket_destory();

	getchar();
	return 0;
}
