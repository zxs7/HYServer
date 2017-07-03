//thread.c

#include "recvMessage.h"


Pstack freeRecvThreadStack;					/* stack of free receving message threads */

pthread_mutex_t	freeRecvThreadStackMutex;	/* clock of stack of free receving message threads */


/**
 * return value of top item 
 */ 
int Pstack_top(Pstack *ps)
{
	return ps->q[ps->top - 1];
}

/**
 * delete top item
 */
void Pstack_pop(Pstack *ps)
{
	ps->top--;
}

/**
 * push a new item with value k to the top
 */
void Pstack_push(Pstack *ps, int k)
{
	ps->q[ps->top] = k;
	ps->top++;
}

/**
 * judge weather the stack is empty
 */
int Pstack_empty(Pstack *ps)
{
	if(ps->top == 0)
	return 1;
	return 0;
}




/* parameter table of receive message threads */
recvThreadPara recvThreadParaTable[RECV_THREAD_SIZE];			

/**
 * message format：
 *
 * 1. format：
 *       #heartBeat#     deviceId       password
 *    |<-  11byte  ->|<-  34byte  ->|<-  27byte  ->|
 *    receive this message means ：server accepted device's connect request, and send register infomation
 *
 * 2. format：
 *       #heartBeat#            111....111
 *    |<-  11byte  ->|<-          61byte         ->|
 *    receive this message means ：server received device's heartbeat message successfully
 *
 * 3. format：
 *       #USmessage#    userSockfd    followMSGlen      followMSG
 *    |<-  11byte  ->|<-  34byte  ->|<-  27byte  ->|<-    Xbyte   ->|
 *    receive this message means ：server transmitted message from user
 *
 */
void *recvMessage(int *id)
{
	char messageHead[MESSAGE_HEAD_SIZE];		/* buffer of message head */
	int threadId = *id;							/* id of current thread */
	int sockfd;									/* socket fd current thread is using */
	int ret;
	int i;
	struct epoll_event ev;						/* epoll event */

	/* detach from parent thread */
	pthread_detach(pthread_self());


wait_unlock:

	/* unlock thread */
	pthread_mutex_lock( &(recvThreadParaTable[threadId].mutex) );

	/* get socket fd */
	sockfd = recvThreadParaTable[threadId].sockfd;	

	/* remove current sockfd from epoll */
	deleteEpollEvent(epfd, sockfd);


read_message:

	/* read message head */
	memset(messageHead, '\0', sizeof(messageHead));
	ret = readNonBlocking_n(sockfd, messageHead, MESSAGE_HEAD_SIZE);

	/* if reading message head failed, close socket and start a new device */
	if(ret != 1)
	{
		close(sockfd);
		resetDevice(deviceTable, sockfd);
		createANewDevice();
		goto set_free;
	}

	/* if it's a heartbeat message */
	if(strncmp(messageHead, "#heartBeat#", MESSAGE_HEAD) == 0)
	{
		/* check out it's a feedback message or a register message */
		int messageType;
		messageType = heartBeatMessageType(messageHead);

		/* if it's a feedback message, and says success */
		if(messageType == 1)
		{
			deviceTable[sockfd].lastHeartBeatTime = ustime();

#ifdef RECV_MESSAGE_DEBUG
			printf("=== Received feedback message of socket : %d (succeed)!\n", sockfd);
#endif
		}
		/* if it's a feedback message, and says fail */
		else if(messageType == 2)
		{
			deviceTable[sockfd].lastHeartBeatTime = ustime();

#ifdef RECV_MESSAGE_DEBUG
			printf("=== Received feedback message of socket : %d (failed)!\n", sockfd);
#endif
		}
		/* if it's a register message */
		else
		{
			char deviceId[DEVICEID_SIZE+1];
			char password[PASSWORD_SIZE+1];
			analysisHeartBeatMSG(messageHead, deviceId, password);
			updateDevice(deviceTable, sockfd, deviceId, password);

#ifdef RECV_MESSAGE_DEBUG
			printf("=== Received register message of socket %d :\n", sockfd);
			printf("=== deviceId : %s\n", deviceId);
			printf("=== password : %s\n", password);
#endif
		}
		
	}
	/* if it's a message from user */
	else if(strncmp(messageHead, "#USmessage#", MESSAGE_HEAD) == 0)
	{
		int userSockfd;
		int messageLen;
		analysisUserMessage(messageHead, &userSockfd, &messageLen);

#ifdef RECV_MESSAGE_DEBUG
		printf("=== Received user message from socket %d, message length : %d\n", userSockfd, messageLen);
#endif

		/* read message body */
		char *message = (char *)malloc(sizeof(char) * messageLen);
		ret = readNonBlocking_n(sockfd, message, messageLen);
		if(ret != 1)
		{
			close(sockfd);
			resetDevice(deviceTable, sockfd);
			createANewDevice();
			goto set_free;
		}

		/* handle with the message, and return the result(include message head) */
		int retMessageLen;
		char *retMessage;
		retMessage = handleUserRequest(userSockfd, sockfd, message, messageLen, &retMessageLen);
		free(message);

		/* send the return message to server */
		ret = writeNonBlocking_n(sockfd, retMessage, retMessageLen);
		if(ret != 1)
		{
			close(sockfd);
			free(retMessage);
			resetDevice(deviceTable, sockfd);
			createANewDevice();
			goto set_free;
		}

		/* to avoid memory leak */
		free(retMessage);
	}
	/* if it's neither, close fd, and create a new one */
	else
	{
		printf("*** Error: Received bad message header!\n");

#ifdef RECV_MESSAGE_DEBUG
		printf("*** Message content : %s\n", messageHead);
#endif
		close(sockfd);
		resetDevice(deviceTable, sockfd);
		createANewDevice();
		goto set_free;
	}


	/* check sockfd status */
	ret = socketHasData(sockfd);
	/* if sockfd has data to read, goto read message step */
	if(ret == 1)
		goto read_message;
	/* if socket closed or exception occured, closed it */
	else if(ret == 0 || ret == -2)
	{
		close(sockfd);
		resetDevice(deviceTable, sockfd);
		createANewDevice();
		goto set_free;
	}


	/* put current sockfd into epoll again */
	ev.data.fd = sockfd;
	ev.events = EPOLLIN|EPOLLET;
	addEpollEvent(epfd, sockfd, &ev);


set_free:	

	/* push current thread into free stack */
	pthread_mutex_lock(&freeRecvThreadStackMutex);
	Pstack_push(&freeRecvThreadStack, threadId);
	pthread_mutex_unlock(&freeRecvThreadStackMutex);
	
	goto wait_unlock;
 

thread_exit:
	pthread_exit(NULL); 
}




/**
 * init receiving message thread pool
 */
int initRecvThreadPool()
{
	/* init threads paramenter */
	int i;
	for(i = 0; i < RECV_THREAD_SIZE; i++)
	{
		/* push thread into free stack */
		Pstack_push(&freeRecvThreadStack, i);
		/* id current thread */
		recvThreadParaTable[i].pthreadId = i;
		/* clock current thread */
		pthread_mutex_lock(&(recvThreadParaTable[i].mutex));
	}
	
	/* create threads */
	for(i = 0; i < RECV_THREAD_SIZE; i++)
	{
		int id = i;
		int ret = pthread_create(&(recvThreadParaTable[i].PthId), NULL, (void *)recvMessage, &id);
		/* suspend for a while to avoid following thread read dirty words */
		usleep(100);
		if(ret != 0)
		{
			printf("create new thread failed!\n");
			return 0;
		}
	}
	return 1;
}


