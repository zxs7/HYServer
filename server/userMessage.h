//userMessage.h

#ifndef _USER_MESSAGE_H
#define _USER_MESSAGE_H


#include "deviceMessage.h"


#define USER_MESSAGE_DEBUG


#define USER_MESSAGE_THREAD_SIZE	(20)
#define USER_MESSAGE_LISTEN_MAX 	(10000)



extern threadInfo userThreadInfoTable[];

extern Pstack freeUserThreadStack;

extern pthread_mutex_t	freeUserThreadStackMutex;

extern const int userListenPort;

extern int userEpollfd;


extern int initUserListenThread();


#endif

