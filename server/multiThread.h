//multiThread.h

#ifndef _MULTITHREAD_H
#define _MULTITHREAD_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_THREAD_SIZE		(1000)

/**
 * thread parameter
 */
typedef struct threadInfo
{
	int sockfd;						/* socket fd  */
	int index;						/* id of current thread */
	pthread_t threadId;				/* Pid of current thread */
	pthread_mutex_t threadMutex;	/* clock of current thread */

}threadInfo;




/**
 * thread stack
 */
typedef struct Pstack
{
	int q[MAX_THREAD_SIZE];			/* thread queue */
	int top;						/* top thread */

}Pstack;


extern int Pstack_top(Pstack *ps);

extern void Pstack_pop(Pstack *ps);

extern void Pstack_push(Pstack *ps, int k);

extern int Pstack_empty(Pstack *ps);



#endif

