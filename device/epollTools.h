//epollTools.h

#ifndef _EPOLLTOOLS_H
#define _EPOLLTOOLS_H

#include <sys/epoll.h>
#include <stdio.h>

#include "recvMessage.h"


#define EPOLL_LISTEN_MAX	(100000)		/* maximum of device number epoll can listen */

extern int epfd;							/* epoll socket fd */

extern int addEpollEvent(int epfd, int fd, struct epoll_event *event);

extern int modifyEpollEvent(int epfd, int fd, struct epoll_event *event);

extern int deleteEpollEvent(int epfd, int fd);

extern int epollCreate();

extern void *listenEpollEvent();


#endif


