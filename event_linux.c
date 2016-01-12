//
//  event_linux.c
//  klib
//
//  Created by LiKai on 16/1/8.
//  Copyright ? 2016年 LiKai. All rights reserved.
//
#ifdef linux

#include "event.h"

status_t
event_pool_init(event_pool_t* event_pool, socket_t server_socket, socket_event_callback callback)
{
	
}

status_t  event_dispatch(event_pool_t* event_pool)
{

	for (; ; )
	{
		nfds = epoll_wait(epfd, events, 20, 500);
		for (i = 0; i < nfds; ++i)
		{
			if (events[i].data.fd == listenfd) //有新的连接
			{
				connfd = accept(listenfd, (sockaddr *)&clientaddr, &clilen); //accept这个连接
				ev.data.fd = connfd;
				ev.events = EPOLLIN | EPOLLET;
				epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev); //将新的fd添加到epoll的监听队列中
			}
			else if (events[i].events&EPOLLIN) //接收到数据，读socket
			{
				n = read(sockfd, line, MAXLINE)) < 0 //读
				ev.data.ptr = md; //md为自定义类型，添加数据
				ev.events = EPOLLOUT | EPOLLET;
				epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);//修改标识符，等待下一个循环时发送数据，异步处理的精髓
			}
			else if (events[i].events&EPOLLOUT) //有数据待发送，写socket
			{
				struct myepoll_data* md = (myepoll_data*)events[i].data.ptr; //取数据
				sockfd = md->fd;
				send(sockfd, md->ptr, strlen((char*)md->ptr), 0); //发送数据
				ev.data.fd = sockfd;
				ev.events = EPOLLIN | EPOLLET;
				epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev); //修改标识符，等待下一个循环时接收数据
			}
			else
			{
				//其他的处理
			}
		}
	}
}



#endif