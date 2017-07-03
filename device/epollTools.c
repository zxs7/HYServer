//epollTools.c

#include "epollTools.h"


int epfd;							/* epoll socket fd */

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




/**
 * create epoll
 *
 * return :
 * 0  -->  fail
 * 1  -->  success
 */
int epollCreate()
{
	epfd = epoll_create(EPOLL_LISTEN_MAX);
	return epfd;
}




/**
 * listen epoll event thread
 *
 * if a new message come, choose a thread to handle it
 */
void *listenEpollEvent()
{
	struct epoll_event events[EPOLL_LISTEN_MAX];	/* feedback events */
	int nfds;

	while(1)
	{
		nfds = epoll_wait(epfd, events, EPOLL_LISTEN_MAX, -1);
		int i;
		for(i = 0; i < nfds; i ++)
		{
			/* if there is no thread available at present, wait for a while */
			while(Pstack_empty(&freeRecvThreadStack) == 1)
			{
				printf("there is no thread available!\n");
				usleep(100);
			}

			/* choose a free thread from free threads stack */
			pthread_mutex_lock(&freeRecvThreadStackMutex);
			int availableThreadId = Pstack_top(&freeRecvThreadStack);
			Pstack_pop(&freeRecvThreadStack);
			pthread_mutex_unlock(&freeRecvThreadStackMutex);

			/* copy infomation needed */
			recvThreadParaTable[availableThreadId].sockfd = events[i].data.fd;
			
			/* wake up thread */
			pthread_mutex_unlock(&(recvThreadParaTable[availableThreadId].mutex));		
		}
	}

	close(epfd);
	pthread_exit(NULL); 

}

