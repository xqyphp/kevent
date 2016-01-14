//
//  event_linux.c
//  klib
//
//  Created by LiKai on 16/1/8.
//  Copyright ? 2016Äê LiKai. All rights reserved.
//
#ifdef linux

#include "event.h"
#include <sys/epoll.h>
#include <assert.h>

#define BUFFER_LEN 1024

status_t
event_manager_init(event_manager_t* manager,int listenfd,event_callback callback)
{
        manager->epfd = epoll_create(256);
        manager->server_socket = listenfd;
        struct epoll_event ev;
        ev.data.fd = listenfd;
        ev.events = EPOLLIN|EPOLLET;
        epoll_ctl( manager->epfd,EPOLL_CTL_ADD,listenfd,&ev);
        return K_SUCCESS;
}

status_t event_dispatch(event_manager_t* manager)
{
        struct sockaddr serveraddr,clientaddr;
        int connfd,sockfd;
        int i,n,clilen,serlen;
        struct epoll_event ev;
        struct epoll_event events[20];
        char buffer[BUFFER_LEN] = "Hello,Client!";
        while(K_TRUE)
        {
                printf("waiting for client...");
                int nfds = epoll_wait(manager->epfd,events,20,500);
                printf("new client..");
                for(i = 0; i < nfds; i++){
                        if(manager->server_socket == events[i].data.fd)
                        {
                                connfd = accept(manager->server_socket,
                                        &clientaddr,&clilen);
                                assert(connfd >= 0);

                                ev.data.fd = connfd;
                                ev.events = EPOLLET| EPOLLIN;
                                epoll_ctl(manager->epfd,EPOLL_CTL_ADD,connfd,&ev);
                        }else if(events[i].events & EPOLLIN)
                        {
                                printf("epoll ->in");
                                sockfd = events[i].data.fd;
                                if(sockfd < 0)
                                        continue;
                                if( (n = read(sockfd,buffer,BUFFER_LEN)))
                                {
                                        printf("%s\n",buffer);
                                }else if ( n == 0 )
                                {
                                        close(sockfd);
                                        events[i].data.fd = -1;
                                }

                        }else if (events[i].events & EPOLLOUT)
                        {
                                sockfd = events[i].data.fd;
                                write(sockfd,buffer,n);
                        }
                }
        }

}

status_t event_magager_destroy(event_manager_t* manager)
{
        close(manager->epfd);
        close(manager->server_socket);
}

#endif
