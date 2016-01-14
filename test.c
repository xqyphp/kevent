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
	printf("callback->%s",data->data_ptr);
}


int main()
{
	socket_init();

	socket_t serverfd = server_create("127.0.0.1", SERVER_PORT);
	setnonblocking(serverfd);

	server_listen(serverfd);

	event_manager_t event_manager;

	event_manager_init(&event_manager, serverfd, my_event_callback);

	event_dispatch(&event_manager);

	event_magager_destroy(&event_manager);

	socket_destory();

	return 0;
}


#define HELLO_WORLD_SERVER_PORT    6666 
#define LENGTH_OF_LISTEN_QUEUE	   20
#define BUFFER_SIZE                1024
#define FILE_NAME_MAX_SIZE         512

void test()
{
	socket_init();

	//设置一个socket地址结构server_addr,代表服务器internet地址, 端口
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr)); //把一段内存区的内容全部设置为0
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);

	//创建用于internet的流协议(TCP)socket,用server_socket代表服务器socket
	int server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (server_socket < 0)
	{
		printf("Create Socket Failed!");
		exit(1);
	}
	{
		int opt = 1;
		setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	}

	//把socket和socket地址结构联系起来
	if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)))
	{
		printf("Server Bind Port : %d Failed!", HELLO_WORLD_SERVER_PORT);
		exit(1);
	}

	//server_socket用于监听
	if (listen(server_socket, LENGTH_OF_LISTEN_QUEUE))
	{
		printf("Server Listen Failed!");
		exit(1);
	}
	while (1) //服务器端要一直运行
	{
		//定义客户端的socket地址结构client_addr
		struct sockaddr_in client_addr;
		socklen_t length = sizeof(client_addr);

		//接受一个到server_socket代表的socket的一个连接
		//如果没有连接请求,就等待到有连接请求--这是accept函数的特性
		//accept函数返回一个新的socket,这个socket(new_server_socket)用于同连接到的客户的通信
		//new_server_socket代表了服务器和客户端之间的一个通信通道
		//accept函数把连接到的客户端信息填写到客户端的socket地址结构client_addr中
		int new_server_socket = accept(server_socket, (struct sockaddr*)&client_addr, &length);
		if (new_server_socket < 0)
		{
			printf("Server Accept Failed!\n");
			break;
		}

		char buffer[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		length = recv(new_server_socket, buffer, BUFFER_SIZE, 0);
		if (length < 0)
		{
			printf("Server Recieve Data Failed!\n");
			break;
		}
		char file_name[FILE_NAME_MAX_SIZE + 1];
		memset(file_name, 0, FILE_NAME_MAX_SIZE + 1);
		strncpy(file_name, buffer, strlen(buffer)>FILE_NAME_MAX_SIZE ? FILE_NAME_MAX_SIZE : strlen(buffer));
		//        int fp = open(file_name, O_RDONLY);
		//        if( fp < 0 )
		printf("%s\n", file_name);
		FILE * fp = fopen(file_name, "r");
		if (NULL == fp)
		{
			printf("File:\t%s Not Found\n", file_name);
		}
		else
		{
			memset(buffer, 0, BUFFER_SIZE);
			int file_block_length = 0;
			//            while( (file_block_length = read(fp,buffer,BUFFER_SIZE))>0)
			while ((file_block_length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0)
			{
				printf("file_block_length = %d\n", file_block_length);
				//发送buffer中的字符串到new_server_socket,实际是给客户端
				if (send(new_server_socket, buffer, file_block_length, 0) < 0)
				{
					printf("Send File:\t%s Failed\n", file_name);
					break;
				}
				memset(buffer, 0, BUFFER_SIZE);
			}
			//            close(fp);
			fclose(fp);
			printf("File:\t%s Transfer Finished\n", file_name);
		}
		//关闭与客户端的连接
		close(new_server_socket);
	}
	//关闭监听用的socket
	close(server_socket);

	socket_destory();
	return 0;
}