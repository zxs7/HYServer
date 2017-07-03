//epollTools.c

#include "epollTools.h"

/**
 * add fd into epoll with event "event"
 *
 * return :
 * -1  -->  fail
 * 1   -->  success
 */
int addEpollEvent(int epfd, int fd, struct epoll_event *event)
{
	if( epoll_ctl(epfd, EPOLL_CTL_ADD, fd, event) == -1)
		return -1;
	return 1;
}




/**
 * modify fd in epoll with event "event"
 *
 * return :
 * -1  -->  fail
 * 1   -->  success
 */
int modifyEpollEvent(int epfd, int fd, struct epoll_event *event)
{
	if( epoll_ctl (epfd, EPOLL_CTL_MOD, fd, event) == -1)
		return -1;
	return 1;
}




/**
 * delete fd from epoll with event "event"
 *
 * return :
 * -1  -->  fail
 * 1   -->  success
 */
int deleteEpollEvent(int epfd, int fd)
{
	struct epoll_event event;
	if(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event) == -1)
		return -1;
	return 1;
}

