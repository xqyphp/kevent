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
event_manager_init(event_manager_t* event_pool, socket_t server_socket
                , event_callback callback)
{

        event_pool->server_socket = server_socket;
        event_pool->call_back = callback;
}

status_t
event_dispatch(event_manager_t* event_pool)
{




	for (; ; )
	{
                nfds = epoll_wait(epfd, events, 20, 500);
		for (i = 0; i < nfds; ++i)
		{
			if (events[i].data.fd == listenfd) //
			{
				connfd = accept(listenfd, (sockaddr *)&clientaddr, &clilen); //
				ev.data.fd = connfd;
				ev.events = EPOLLIN | EPOLLET;
				epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev); //
			}
			else if (events[i].events&EPOLLIN) //
			{
				n = read(sockfd, line, MAXLINE);
				ev.data.ptr = md; //
				ev.events = EPOLLOUT | EPOLLET;
				epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);//
			}
                        else if (events[i].events&EPOLLOUT) //
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


			}
		}
	}
}


#endif
