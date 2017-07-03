//userMessage.c


#include "userMessage.h"



/* paramenter table of threads which receive message from users */
threadInfo userThreadInfoTable[USER_MESSAGE_THREAD_SIZE+1];		


/* free stack of threads which receive message from users */
Pstack freeUserThreadStack;				

/* clock of free stack of threads which receive message from users */	
pthread_mutex_t	freeUserThreadStackMutex;	




/**
 * make message head to send to device
 *
 * format:
 *
 *    #USmessage#    userSockfd    followMSGlen  
 * |<-  11byte  ->|<-  34byte  ->|<-  27byte  ->|
 */
void makeMessageHeadToDevice(char *sendBuffer, int userSockfd, int messageLen)
{
	memset(sendBuffer, '\0', sizeof(sendBuffer));
	sprintf(sendBuffer, "#USmessage#");
	sprintf(sendBuffer + MESSAGE_TYPE_SIZE, "%d", userSockfd);
	sprintf(sendBuffer + MESSAGE_TYPE_SIZE + USERSOCKFD_SIZE, "%d", messageLen);
	return ;
}




/**
 * analysis message receive from user
 *
 *    #USmessage#     deviceID     followMSGlen      followMSG
 * |<-  11byte  ->|<-  34byte  ->|<-  27byte  ->|<-    Xbyte   ->|
 */
void analysisUserMessage(char *recvBuffer, char *deviceId, int *messageLen)
{
	strncpy(deviceId, recvBuffer + MESSAGE_TYPE_SIZE, DEVICEID_SIZE);
	sscanf(recvBuffer + MESSAGE_TYPE_SIZE + DEVICEID_SIZE, "%d", messageLen);
	return ;
}




/**
 * user message format：
 *
 *    #USmessage#     deviceID     followMSGlen      followMSG
 * |<-  11byte  ->|<-  34byte  ->|<-  27byte  ->|<-    Xbyte   ->|
 *
 */
void *recvUserMessage(int *id)
{
	int threadIndex = *id;		/* thread index */	
	int userSockfd;				/* user socket fd */
	int ret;					/* return value of operation */
	struct epoll_event ev;		/* epoll event */			
	char messageHeadBuffer[MESSAGE_HEAD_SIZE];		/* buffer to receive message head */
	socketBuffer SOBuffer;		/* socket Buffer, used to receive message body */
	/* init socketBuffer */
	socketBufferInit(&SOBuffer);

	pthread_detach(pthread_self());

wait_unlock:

	/* unlock thread */
	pthread_mutex_lock( &(userThreadInfoTable[threadIndex].threadMutex) );	
	
	/* read user sockfd */
	userSockfd = userThreadInfoTable[threadIndex].sockfd;

	/* remove current sockfd from epoll */
	deleteEpollEvent(userEpollfd, userSockfd);


read_message:

	/* read message head */
	memset(messageHeadBuffer, '\0', sizeof(messageHeadBuffer));
	ret = readNonBlocking_n(userSockfd, messageHeadBuffer, MESSAGE_HEAD_SIZE);

	/* if reading message failed, close socket */
	if(ret != 1)
	{
		printf("socket:%d , receiving user message failed!\n",userSockfd);
		close(userSockfd);
		goto set_free;
	}

	/* if it is a user message */
	if(strncmp(messageHeadBuffer, "#USmessage#", MESSAGE_TYPE_SIZE) == 0)
	{
		char deviceId[DEVICEID_SIZE + 1];
		int messageLen;

		/* get deviceId and length of left message */
		analysisUserMessage(messageHeadBuffer, deviceId, &messageLen);

#ifdef USER_MESSAGE_DEBUG
		printf("=== Received user message from %d\n", userSockfd);
		printf("=== Resend to device : %s\n", deviceId);
		printf("=== Message   length : %d\n", messageLen);
#endif

		/* read message body */
		ret = socketBufferReadMessage(userSockfd, &SOBuffer, messageLen);
		if(ret != 1)
		{
			close(userSockfd);
			socketBufferFreeBlock(&SOBuffer);
			goto set_free;
		}

		/* find the device with deviceId */
		deviceNode* device;
		device = findDevice(smartDeviceTable, deviceId);

		/* if can not find device, let user know */
		if(device == NULL)
		{
			char messageHead[MESSAGE_TYPE_SIZE + MESSAGELEN_SIZE];
			char messageBody[16];
			sprintf(messageBody, "Device not find!");
			makeMessageHeadToUser(messageHead, 16);

			/* send message head */
			ret = writeNonBlocking_n(userSockfd, messageHead, MESSAGE_TYPE_SIZE + MESSAGELEN_SIZE);
			if(ret != 1)
			{
				close(userSockfd);
				socketBufferFreeBlock(&SOBuffer);
				goto set_free;
			}
			
			/* send message body */
			ret = writeNonBlocking_n(userSockfd, messageBody, 16);
			if(ret != 1)
			{
				close(userSockfd);
				socketBufferFreeBlock(&SOBuffer);
				goto set_free;
			}

		}
		else
		{
			/* get device sockfd */
			int deviceSockfd = device->deviceSockfd;

			char messageHead[MESSAGE_HEAD_SIZE];
			makeMessageHeadToDevice(messageHead, userSockfd, messageLen);

			/* send message head */
			ret = writeNonBlocking_n(deviceSockfd, messageHead, MESSAGE_HEAD_SIZE);
			if(ret != 1)
			{
				close(deviceSockfd);
				deleteEpollEvent(deviceEpollfd, deviceSockfd);
			}

			/* send message body */
			ret = socketBufferWriteMessage(deviceSockfd, &SOBuffer);
			if(ret != 1)
			{
				close(deviceSockfd);
				deleteEpollEvent(deviceEpollfd, deviceSockfd);
			}
		}
	}
	/* if it is not user message */
	else
	{
		printf("*** Error : received bad message head from user %d\n", userSockfd);

#ifdef USER_MESSAGE_DEBUG
		printf("*** Message content : %s\n", messageHeadBuffer);
#endif

		char messageHead[MESSAGE_TYPE_SIZE + MESSAGELEN_SIZE];
		char messageBody[12];
		sprintf(messageBody, "Bad message!");
		makeMessageHeadToUser(messageHead, 12);

		/* send message head */
		ret = writeNonBlocking_n(userSockfd, messageHead, MESSAGE_TYPE_SIZE + MESSAGELEN_SIZE);
		if(ret != 1)
		{
			close(userSockfd);
			goto set_free;
		}
			
		/* send message body */
		ret = writeNonBlocking_n(userSockfd, messageBody, 12);
		if(ret != 1)
		{
			close(userSockfd);
			goto set_free;
		}	
	}


	/* free socketBuffer */
	socketBufferFreeBlock(&SOBuffer);

	/* check user sockfd status */
	ret = socketHasData(userSockfd);
	/* if sockfd has data to read, goto read message step */
	if(ret == 1)
		goto read_message;
	/* if socket closed or exception occured, closed it */
	else if(ret == 0 || ret == -2)
	{
		close(userSockfd);
		goto set_free;
	}

	/* put current sockfd into epoll again */
	ev.data.fd = userSockfd;
	ev.events = EPOLLIN|EPOLLET;
	addEpollEvent(userEpollfd, userSockfd, &ev);


set_free:

	/* set thread free */
	pthread_mutex_lock(&freeUserThreadStackMutex);
	Pstack_push(&freeUserThreadStack, threadIndex);
	pthread_mutex_unlock(&freeUserThreadStackMutex);

	goto wait_unlock;
 

thread_exit:
	pthread_exit(NULL);
	
}




/**
 * create user listen thread pool
 *
 * return :
 * 1  -->  succeed
 * 0  -->  failed
 */
int initUserThreadPool()
{
	int i;

	for(i = 0; i < USER_MESSAGE_THREAD_SIZE; i++)
	{
		Pstack_push(&freeUserThreadStack, i);
		userThreadInfoTable[i].index = i;
		pthread_mutex_lock(&(userThreadInfoTable[i].threadMutex));
	}
	
	for(i = 0; i < USER_MESSAGE_THREAD_SIZE; i++)
	{
		int id = i;
		int rc = pthread_create(&(userThreadInfoTable[i].threadId), NULL, (void *)recvUserMessage, &id);
		usleep(100);
		if(rc != 0)
		{
			printf("create new thread failed!\n");
			return 0;
		}
	}
	return 1;
}




const int userListenPort = 8001;				/* user listen port */

int userEpollfd;								/* user epoll fd */

/**
 * thread listens to users
 */
void *userListenThread()
{
	int listenfd;						/* listen connection request */
	int connfd;							/* socket fd of new conntection */
	struct sockaddr_in clientaddr;		/* client sock address */
	socklen_t clientaddr_size;			/* client address size */

	int nfds;							/* number of epoll I/O event */
	struct epoll_event ev;				/* I/O event */
	struct epoll_event events[USER_MESSAGE_LISTEN_MAX];		/* I/O event tables */
	
	
	/***************************** init ******************************/
	
	/* clear user thread stack */
	freeUserThreadStack.top = 0;

	/* init user thread pool */
	int rc = initUserThreadPool();
	if(rc != 1)
	{
		printf("Init user thread pool failed!\n");
		goto thread_exit;
	}
	printf("There are %d user threads available\n", freeUserThreadStack.top);

	/* create listen socket */
	listenfd = createAndBind(userListenPort);
	if(listenfd == -1)
	{
		printf("Error in creating listen socket!\n");
		goto thread_exit;
	}

	/* create epoll */
	userEpollfd = epoll_create(USER_MESSAGE_LISTEN_MAX);

	/* register listenfd into epoll */
	ev.data.fd = listenfd;
	ev.events = EPOLLIN;
	addEpollEvent(userEpollfd, listenfd, &ev);
	



	/***************************** listen *******************************/
	
	if(listen(listenfd, 20) == -1) 
	{
		printf("call to listen");
		goto thread_exit;
	}
	printf("Accepting connections ...\n"); 

	while(1)
	{
		int i;
		nfds = epoll_wait(userEpollfd, events, USER_MESSAGE_LISTEN_MAX, -1);
		for(i = 0; i < nfds; i ++)
		{
			/* if received connection request, then register it into epoll */
			if(events[i].data.fd == listenfd)
			{
				/* accept connection request */
				connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientaddr_size);
				if(connfd <= 0)
				{
					printf("accept connect failed!\n");
					continue;
				}

				/* set socket to be nonblocking */
				makeSocketNonBlocking(connfd);
				
				/* register new connection into epoll */
				ev.data.fd = connfd;
				ev.events = EPOLLIN|EPOLLET;
				addEpollEvent(userEpollfd, connfd, &ev);

			}
			/* if received message from connection that has been registered, wake up a thread to process it */
			else if(events[i].events&EPOLLIN)
			{
				/* if there are no threads available, wait for a while */
				while(Pstack_empty(&freeUserThreadStack))
				{
					printf("there is no user thread available!\n");
					usleep(100);
				}

				/* pick up a thread */
				pthread_mutex_lock(&freeUserThreadStackMutex);
				int availableThreadId = Pstack_top(&freeUserThreadStack);
				Pstack_pop(&freeUserThreadStack);
				pthread_mutex_unlock(&freeUserThreadStackMutex);

				/* put sockfd and index into thread's parameter table */
				userThreadInfoTable[availableThreadId].sockfd = events[i].data.fd;
				userThreadInfoTable[availableThreadId].index = availableThreadId;
				
				/* wake up the thread */
				pthread_mutex_unlock(&(userThreadInfoTable[availableThreadId].threadMutex));

			}
			else if(events[i].events&EPOLLHUP)
			{
				printf("socket:%d has been closed!\n",events[i].data.fd);
				close(events[i].data.fd);
			}
		}
	}
	close(userEpollfd);
	return 0;

thread_exit:
	pthread_exit(NULL);
}




/**
 * init user listen thread thread
 *
 * return :
 * 1 -> succeed
 * 0 -> failed
 */
int initUserListenThread()
{
	int ret;
	pthread_t userListenThreadId;
	
	ret = pthread_create(&userListenThreadId, NULL, (void *)userListenThread, NULL);
	if(ret != 0)
	{
		printf("Create user listen thread failed!\n");
		return 0;
	}

	return 1;
}

