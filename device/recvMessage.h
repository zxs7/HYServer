//recvMessage.h

#ifndef _RECV_MESSAGE_H
#define _RECV_MESSAGE_H


#include <stdlib.h>

#include "heartBeat.h"
#include "socketTools.h"
#include "epollTools.h"
#include "device.h"
#include "userRequest.h"


#define RECV_MESSAGE_DEBUG


#define RECV_THREAD_SIZE	(200)	/* receive message thread size */
#define MESSAGE_HEAD_SIZE	(72)	/* message head size */

/**
 * thread parameter
 */
typedef struct recvThreadPara
{
	int sockfd;						/* socket fd which current thread is using */
	int pthreadId;					/* id of current thread */
	pthread_t PthId;				/* Pid of current thread */
	pthread_mutex_t mutex;			/* clock of current thread */
}recvThreadPara;

extern recvThreadPara recvThreadParaTable[];			/* paramenter table of receive message threads */



/**
 * stack of thread
 */
typedef struct Pstack
{
	int q[RECV_THREAD_SIZE+2];		/* threads id */
	int top;						/* top of stack */
}Pstack; 

extern Pstack freeRecvThreadStack;						/* stack of free receving message threads */

extern pthread_mutex_t	freeRecvThreadStackMutex;		/* clock of stack of free receving message threads */


extern int Pstack_top(Pstack *ps);

extern void Pstack_pop(Pstack *ps);

extern void Pstack_push(Pstack *ps, int k);

extern int Pstack_empty(Pstack *ps);

extern int initRecvThreadPool();


#endif

