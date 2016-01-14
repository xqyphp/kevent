//
//  event_linux.c
//  klib
//
//  Created by LiKai on 16/1/8.
//  Copyright ? 2016Äê LiKai. All rights reserved.
//
#ifdef linux

#include "event.h"


status_t
event_manager_init(event_manager_t* manager,int listenfd)
{
        manager->epfd = epoll_create(256);
        manager->listenfd = listenfd;
        struct epoll_event ev;
        ev.data.fd = listenfd;
        ev.events = EPOLLIN|EPOLLET;
        epoll_ctl( manager->epfd,EPOLL_CTL_ADD,listenfd,&ev);
        return K_SUCCESS;
}

status_t event_dispatch(event_manager_t* manager)
{
        struct sockaddr serveraddr,clientaddr;
        int connfd;
        int i,n,clilen,serlen;
        struct epoll_event ev,events[20];

        int nfds = epoll_wait(manager->listenfd,events,20,500);
        for(i = 0; i < nfds; i++){
                if(manager->listenfd == events[i].data.fd)
                {
                        connfd = accept(manager->listenfd,
                                        &clientaddr,&clilen);
                        assert(connfd >= 0);

                        ev.data.fd = connfd;
                        ev.events = EPOLLET| EPOLLIN;
                        epoll_ctl(manager->epfd,EPOLL_CTL_ADD,connfd,&ev);
                }else if(events[i].events & EPOLLIN)
                {
                        sockfd = events[i].data.fd;
                        if(sockfd < 0)
                                continue;
                        if( (n = read(sockfd))}
                        {
                        }else if ( n == 0 )
                        {
                                close(sockfd);
                                events[i].data.fd = -1;
                        }

                }else if (events[i].events & EPOLLOUT)
                {
                        sockfd = events[i].data.fd;
                        write(sockfd,line,n);
                }
        }
}

void event_magager_destroy(event_manager_t* manager)
{
        close(manager->epfd);
        close(manager->listenfd);
}

#endif
