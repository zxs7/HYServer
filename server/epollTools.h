//epollTools.h

#ifndef _EPOLLTOOLS_H
#define _EPOLLTOOLS_H

#include <sys/epoll.h>
#include <stdio.h>

extern int addEpollEvent(int epfd, int fd, struct epoll_event *event);

extern int modifyEpollEvent(int epfd, int fd, struct epoll_event *event);

extern int deleteEpollEvent(int epfd, int fd);


#endif

