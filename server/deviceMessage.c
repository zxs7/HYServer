//deviceMessage.c

#include "deviceMessage.h"



/**
 * make heart beat message
 * put deviceId and password into message
 *
 * format:
 *    #heartBeat#     deviceId       password
 * |<-  11byte  ->|<-  34byte  ->|<-  27byte  ->|
 */
void makeRegisterMessage(char *message, char *deviceId, char *password)
{
	memset(message, '\0', sizeof(message));
	strncpy(message, "#heartBeat#", MESSAGE_TYPE_SIZE);
	strncpy(message + MESSAGE_TYPE_SIZE, deviceId, DEVICEID_SIZE);
	strcpy(message + MESSAGE_TYPE_SIZE + DEVICEID_SIZE, password);
	return ;
}




/**
 * make feedback message
 *
 * format:
 *
 * success(feedbackType = 1):
 *    #heartBeat#            111....111
 * |<-  11byte  ->|<-          61byte         ->|
 *
 * fail(feedbackType = 0):
 *    #heartBeat#            000....000
 * |<-  11byte  ->|<-          61byte         ->|
 */
void makeFeedBackMessage(char *message, int feedbackType)
{
	int i;
	if(feedbackType == 1)
	{
		for(i = 0; i < MESSAGE_HEAD_SIZE; ++i)
			message[i] = '1';
	}	
	else
	{
		for(i = 0; i < MESSAGE_HEAD_SIZE; ++i)
			message[i] = '0';
	}
	
	strncpy(message, "#heartBeat#", MESSAGE_TYPE_SIZE);
	return ;
}




/**
 * make message head to send to user
 *
 * format:
 *
 *    #USmessage#   followMSGlen  
 * |<-  11byte  ->|<-  27byte  ->|
 */
void makeMessageHeadToUser(char *sendBuffer, int messageLen)
{
	memset(sendBuffer, '\0', sizeof(sendBuffer));
	sprintf(sendBuffer, "#USmessage#");
	sprintf(sendBuffer + MESSAGE_TYPE_SIZE, "%d", messageLen);
	return ;
}




/**
 * analysis heart beat message
 * get deviceId and password from message
 */
void analysisHeartBeatMessage(char *message, char *deviceId, char *password)
{
	strncpy(deviceId, message + MESSAGE_TYPE_SIZE, DEVICEID_SIZE);
	strcpy(password, message + MESSAGE_TYPE_SIZE + DEVICEID_SIZE);
	return ;
}




/**
 * analysis message receive from device, and need to send to user
 */
void analysisMessageToUser(char *recvBuffer, int *userSockfd, int *messageLen)
{
	sscanf(recvBuffer + MESSAGE_TYPE_SIZE, "%d", userSockfd);
	sscanf(recvBuffer + MESSAGE_TYPE_SIZE + USERSOCKFD_SIZE, "%d", messageLen);
	return ;
}




/**
 * send feedback message to device
 *
 * if feedbackType is 1, that should be a success message
 * else if feedbackType is 0, that should be a fail message
 */
int sendFeedbackMessage(int sockfd, int feedbackType)
{
	char feedbackMessage[MESSAGE_HEAD_SIZE];
	makeFeedBackMessage(feedbackMessage, feedbackType);
	return writeNonBlocking_n(sockfd, feedbackMessage, MESSAGE_HEAD_SIZE);
}




/**
 * send register message to device
 *
 * the register message include deviceId and password of the current device
 */
int sendRegisterMessage(int sockfd, char *deviceId, char *password)
{
	char registerMessage[MESSAGE_HEAD_SIZE];
	makeRegisterMessage(registerMessage, deviceId, password);
	return writeNonBlocking_n(sockfd, registerMessage, MESSAGE_HEAD_SIZE);
}




/*==============================================================================================*/
/*************************** accept device connection thread pool *******************************/
/*==============================================================================================*/


/* parameter table of threads which accept device's connection request */
threadInfo deviceAcceptThreadInfoTable[DEVICE_ACCEPT_THREAD_SIZE + 1];		


/* free stack of threads which accept device's connection request */
Pstack freeDeviceAcceptThreadStack;	

/* clock of free stack of threads which accept device's connection request */				
pthread_mutex_t	freeDeviceAcceptThreadStackMutex;


/**
 * accept device's connection request
 */
void *acceptDeviceConnect(int *id)
{
	int threadIndex = *id;			/* thread index */
	int listenfd;					/* listenfd for accept connection */
	int connfd;						/* sockfd for new connection */
	int ret;						/* return value of operation */
	struct sockaddr_in clientaddr;	/* client sock address */
	socklen_t clientaddr_size;		/* client address size */
	struct epoll_event ev;			/* epoll event */
	
	pthread_detach(pthread_self());

wait_unlock:

	/* unlock thread */
	pthread_mutex_lock( &(deviceAcceptThreadInfoTable[threadIndex].threadMutex) );

	/* read listenfd */
	listenfd = deviceAcceptThreadInfoTable[threadIndex].sockfd;


	/* accept connection request */
	connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientaddr_size);
	if(connfd <= 0)
	{
		printf("accept connect failed!\n");
		goto set_free;
	}

	/* set socket to be nonblocking */
	makeSocketNonBlocking(connfd);
			
	/* register new connection into epoll */
	ev.data.fd = connfd;
	ev.events = EPOLLIN|EPOLLET;
	addEpollEvent(deviceEpollfd, connfd, &ev);

	/* create a new device */
	deviceNode *newdevice;
	newdevice = createDevice(connfd);

	/* insert new device into device table */
	insertDevice(smartDeviceTable, newdevice->deviceId, newdevice);

	/* send deviceId and password to device */
	ret = sendRegisterMessage(connfd, newdevice->deviceId, newdevice->password);
	if(ret != 1)
	{
		printf("socket:%d , sending register message failed!\n",connfd);
		close(connfd);
		deleteEpollEvent(deviceEpollfd, connfd);
		goto set_free;
	}

set_free:

	/* set thread free */
	pthread_mutex_lock(&freeDeviceAcceptThreadStackMutex);
	Pstack_push(&freeDeviceAcceptThreadStack, threadIndex);
	pthread_mutex_unlock(&freeDeviceAcceptThreadStackMutex);

	goto wait_unlock;
 
thread_exit:
	/* thread exit */
	pthread_exit(NULL);
	
}




/**
 * create device accept thread pool
 *
 * return :
 * 1  -->  succeed
 * 0  -->  failed
 */
int initDeviceAcceptThreadPool()
{
	int i;
	int ret;
	/* fill in index for threads */
	for(i = 0; i < DEVICE_ACCEPT_THREAD_SIZE; i++)
	{
		/* put new thread into stack */
		Pstack_push(&freeDeviceAcceptThreadStack, i);
		/* fill in thread index */
		deviceAcceptThreadInfoTable[i].index = i;
		/* lock thread */
		pthread_mutex_lock(&(deviceAcceptThreadInfoTable[i].threadMutex));
	}
	
	/* create thread pool */
	for(i = 0; i < DEVICE_ACCEPT_THREAD_SIZE; i++)
	{
		int id = i;
		ret = pthread_create(&(deviceAcceptThreadInfoTable[i].threadId), NULL, (void *)acceptDeviceConnect, &id);
		/* after creating each thread, wait a while to avoid thread read dirty id */
		usleep(100);
		if(ret != 0)
		{
			printf("create new device accepting thread failed!\n");
			return 0;
		}
	}
	return 1;
}



/*==============================================================================================*/
/***************************** receive device message thread pool *******************************/
/*==============================================================================================*/


/* parameter table of threads which receive message from devcie */
threadInfo deviceThreadInfoTable[DEVICE_MESSAGE_THREAD_SIZE+1];		


/* free stack of threads which receive message from device */
Pstack freeDeviceThreadStack;	

/* clock of free stack of threads which receive message from device */				
pthread_mutex_t	freeDeviceThreadStackMutex;


/**
 * server may receive two kinds of message from devices as follow ：
 *
 * 1. format：
 *       #heartBeat#     deviceId       password
 *    |<-  11byte  ->|<-  34byte  ->|<-  27byte  ->|
 *    it means ：it's a heartBeat message
 *    if deviceId is 000...00, it means the device haven't been registered, server should register this device
 *    else , means it's a normal heartBeat message, server should update device's status
 *
 * 2. format：
 *       #USmessage#    userSockfd    followMSGlen      followMSG
 *    |<-  11byte  ->|<-  34byte  ->|<-  27byte  ->|<-    Xbyte   ->|
 *    it means ：it's a feedback message to user, server should transmit it to user
 *
 */
void *recvDeviceMessage(int *id)
{
	int threadIndex = *id;		/* thread index */
	int deviceSockfd;			/* socket fd of device */
	int ret;					/* return value of operation */
	struct epoll_event ev;		/* epoll event */
	char messageHeadBuffer[MESSAGE_HEAD_SIZE];		/* buffer to receive message head */
	socketBuffer SOBuffer;		/* socket Buffer, used to receive message body */
	/* init socketBuffer */
	socketBufferInit(&SOBuffer);

	pthread_detach(pthread_self());

wait_unlock:

	/* unlock thread */
	pthread_mutex_lock( &(deviceThreadInfoTable[threadIndex].threadMutex) );

	/* read sockfd of device */
	deviceSockfd = deviceThreadInfoTable[threadIndex].sockfd;

	/* remove current sockfd from epoll */
	deleteEpollEvent(deviceEpollfd, deviceSockfd);


read_message:

	/* read message head */
	memset(messageHeadBuffer, '\0', sizeof(messageHeadBuffer));
	ret = readNonBlocking_n(deviceSockfd, messageHeadBuffer, MESSAGE_HEAD_SIZE);

	/* if reading message failed, close socket */
	if(ret != 1)
	{
		printf("socket:%d , receiving heartbeat failed!\n",deviceSockfd);
		close(deviceSockfd);
		goto set_free;
	}


	/* if received heartBeat message */
	if(strncmp(messageHeadBuffer, "#heartBeat#", MESSAGE_TYPE_SIZE) == 0)
	{
		long long currentTime = ustime();
		char deviceId[DEVICEID_SIZE+1];
		char password[PASSWORD_SIZE+1];
		analysisHeartBeatMessage(messageHeadBuffer, deviceId, password);

#ifdef DEVICE_MESSAGE_DEBUG
		printf("=== Received heartBeat message from socket : %d\n", deviceSockfd);
		printf("=== deviceId : %s\n", deviceId);
		printf("=== password : %s\n", password);
#endif

		/* find device and update its status */
		int updateret = updateDevice(smartDeviceTable, deviceId, password, currentTime);
		
		/* if can not find this device, register a new one */
		if(updateret == 0)
		{
			printf("Not find this device!\n");
			/* create a device */
			deviceNode *newdevice;
			newdevice = createDevice(deviceSockfd);

			/* insert new device into device table */
			insertDevice(smartDeviceTable, newdevice->deviceId, newdevice);

			/* send its new deviceId and password */
			ret = sendRegisterMessage(deviceSockfd, newdevice->deviceId, newdevice->password);

			if(ret != 1)
			{
				printf("socket:%d , receiving heartbeat failed!\n",deviceSockfd);
				close(deviceSockfd);
				goto set_free;
			}			
		}
		/* if find this device, tell it the server received its heartBeat message */
		else
		{
			int feedbackret = sendFeedbackMessage(deviceSockfd, 1);
			if(feedbackret != 1)
			{
				close(deviceSockfd);
				goto set_free;
			}
		}
	}
	/* if it's a user message */
	else if(strncmp(messageHeadBuffer, "#USmessage#", MESSAGE_TYPE_SIZE) == 0)
	{
		int userSockfd;
		int messageLen;
		analysisMessageToUser(messageHeadBuffer, &userSockfd, &messageLen);

#ifdef DEVICE_MESSAGE_DEBUG
		printf("=== Received message from device : %d\n", deviceSockfd);
		printf("=== Resend to user : %d\n", userSockfd);
		printf("=== Message length : %d\n", messageLen);
#endif

		/* read message body to socketBuffer */
		ret = socketBufferReadMessage(deviceSockfd, &SOBuffer, messageLen);
		if(ret != 1)
		{
			close(deviceSockfd);
			socketBufferFreeBlock(&SOBuffer);
			goto set_free;
		}


		/* send message head to user */
		char userMessageHead[MESSAGE_TYPE_SIZE + MESSAGELEN_SIZE];
		makeMessageHeadToUser(userMessageHead, messageLen);
		ret = writeNonBlocking_n(userSockfd, userMessageHead, MESSAGE_TYPE_SIZE + MESSAGELEN_SIZE);
		if(ret != 1)
		{
			close(userSockfd);
		}
		else
		{
			/* send message body to user */
			ret = socketBufferWriteMessage(userSockfd, &SOBuffer);
			if(ret != 1)
			{
				close(userSockfd);
			}
		}

	}
	/* if neither, means it's an undefined message */
	else
	{
		printf("*** Error : received bad message head!\n");

#ifdef DEVICE_MESSAGE_DEBUG
		printf("*** Message content : %s\n", messageHeadBuffer);
#endif

		int feedbackret = sendFeedbackMessage(deviceSockfd, 0);
		if(feedbackret != 1)
		{
			close(deviceSockfd);
			goto set_free;
		}
	}

	/* free socketBuffer */
	socketBufferFreeBlock(&SOBuffer);

	/* check device sockfd status */
	ret = socketHasData(deviceSockfd);
	/* if sockfd has data to read, goto read message step */
	if(ret == 1)
		goto read_message;
	/* if socket closed or exception occured, closed it */
	else if(ret == 0 || ret == -2)
	{
		close(deviceSockfd);
		goto set_free;
	}

	/* put current sockfd into epoll again */
	ev.data.fd = deviceSockfd;
	ev.events = EPOLLIN|EPOLLET;
	addEpollEvent(deviceEpollfd, deviceSockfd, &ev);


set_free:

	/* set thread free */
	pthread_mutex_lock(&freeDeviceThreadStackMutex);
	Pstack_push(&freeDeviceThreadStack, threadIndex);
	pthread_mutex_unlock(&freeDeviceThreadStackMutex);

	goto wait_unlock;
 
thread_exit:
	/* thread exit */
	pthread_exit(NULL);
	
}




/**
 * create device listen thread pool
 *
 * return :
 * 1  -->  succeed
 * 0  -->  failed
 */
int initDeviceThreadPool()
{
	int i;
	/* fill in index for threads */
	for(i = 0; i < DEVICE_MESSAGE_THREAD_SIZE; i++)
	{
		/* put new thread into stack */
		Pstack_push(&freeDeviceThreadStack, i);
		/* fill in thread index */
		deviceThreadInfoTable[i].index = i;
		/* lock thread */
		pthread_mutex_lock(&(deviceThreadInfoTable[i].threadMutex));
	}
	
	/* create thread pool */
	for(i = 0; i < DEVICE_MESSAGE_THREAD_SIZE; i++)
	{
		int id = i;
		int rc = pthread_create(&(deviceThreadInfoTable[i].threadId), NULL, (void *)recvDeviceMessage, &id);
		/* after creating each thread, wait a while to avoid thread read dirty id */
		usleep(100);
		if(rc != 0)
		{
			printf("create new thread failed!\n");
			return 0;
		}
	}
	return 1;
}




/*==============================================================================================*/
/******************************** device processing main loop ***********************************/
/*==============================================================================================*/


const int deviceListenPort = 8000;				/* device listen port */

int deviceEpollfd;								/* device epoll socket fd */

/**
 * thread listens to devices
 */
void *deviceListenThread()
{
	int listenfd;						/* listen connection request */
	int connfd;							/* socket fd of new conntection */
	
	int nfds;							/* number of epoll I/O event */
	struct epoll_event ev;				/* I/O event */
	struct epoll_event events[DEVCIE_MESSAGE_LISTEN_MAX];	/* I/O event tables */
	int ret;
	
	
	/***************************** init ******************************/
	
	/* clear device accepting thread stack */
	freeDeviceAcceptThreadStack.top = 0;

	/* init device accepting thread pool */
	ret = initDeviceAcceptThreadPool();
	if(ret != 1)
	{
		printf("Init device accepting thread pool failed!\n");
		goto thread_exit;
	}
	printf("There are %d device accepting threads available\n", freeDeviceAcceptThreadStack.top);

	/* clear device thread stack */
	freeDeviceThreadStack.top = 0;

	/* init device thread pool */
	ret = initDeviceThreadPool();
	if(ret != 1)
	{
		printf("Init device thread pool failed!\n");
		goto thread_exit;
	}
	printf("There are %d device threads available\n", freeDeviceThreadStack.top);

	/* create listen socket */
	listenfd = createAndBind(deviceListenPort);
	if(listenfd == -1)
	{
		printf("Error in creating listen socket!\n");
		goto thread_exit;
	}

	/* create epoll */
	deviceEpollfd = epoll_create(DEVCIE_MESSAGE_LISTEN_MAX);

	/* register listenfd into epoll */
	ev.data.fd = listenfd;
	ev.events = EPOLLIN;
	addEpollEvent(deviceEpollfd, listenfd, &ev);
	

	/***************************** listen *******************************/

	if (listen(listenfd, 20) == -1) 
	{
		printf("*** Error in listenning to device port !\n");
		goto thread_exit;
	}
	printf("Accepting connections ...\n"); 
 
	while(1)
	{
		int i;
		nfds = epoll_wait(deviceEpollfd, events, DEVCIE_MESSAGE_LISTEN_MAX, -1);
		for(i = 0; i < nfds; i ++)
		{
			/* if received connection request, then register it into epoll */
			if(events[i].data.fd == listenfd)
			{
				/* if there are no threads available, wait for a while */
				while(Pstack_empty(&freeDeviceAcceptThreadStack))
				{
					printf("there is no device accepting thread available!\n");
					usleep(100);
				}

				/* pick up a thread */
				pthread_mutex_lock(&freeDeviceAcceptThreadStackMutex);
				int availableThreadId = Pstack_top(&freeDeviceAcceptThreadStack);
				Pstack_pop(&freeDeviceAcceptThreadStack);
				pthread_mutex_unlock(&freeDeviceAcceptThreadStackMutex);

				/* put sockfd and index into thread's parameter table */
				deviceAcceptThreadInfoTable[availableThreadId].sockfd = listenfd;
				deviceAcceptThreadInfoTable[availableThreadId].index = availableThreadId;

				/* wake up the thread */
				pthread_mutex_unlock(&(deviceAcceptThreadInfoTable[availableThreadId].threadMutex));

			}
			/* if received message from connection that has been registered, wake up a thread to process it */
			else if(events[i].events&EPOLLIN)
			{
				/* if there are no threads available, wait for a while */
				while(Pstack_empty(&freeDeviceThreadStack))
				{
					printf("there is no device receiving thread available!\n");
					usleep(100);
				}

				/* pick up a thread */
				pthread_mutex_lock(&freeDeviceThreadStackMutex);
				int availableThreadId = Pstack_top(&freeDeviceThreadStack);
				Pstack_pop(&freeDeviceThreadStack);
				pthread_mutex_unlock(&freeDeviceThreadStackMutex);

				/* put sockfd and index into thread's parameter table */
				deviceThreadInfoTable[availableThreadId].sockfd = events[i].data.fd;
				deviceThreadInfoTable[availableThreadId].index = availableThreadId;

				/* change receive buffer size */
				//int len = 256000;
				//setsockopt(events[i].data.fd, SOL_SOCKET, SO_RCVBUF, &len, sizeof(int));
				
				/* wake up the thread */
				pthread_mutex_unlock(&(deviceThreadInfoTable[availableThreadId].threadMutex));

			}
			else if(events[i].events&EPOLLHUP)
			{
				printf("socket:%d has been closed!\n",events[i].data.fd);
				close(events[i].data.fd);
			}
		}
	}
	close(deviceEpollfd);
	return 0;

thread_exit:
	pthread_exit(NULL);
}




pthread_t deviceListenThreadId;		/* device listen thread id */

/**
 * init device listen threads
 *
 * return :
 * 1 -> succeed
 * 0 -> failed
 */
int initDeviceListenThread()
{
	int ret;
	
	ret = pthread_create(&deviceListenThreadId, NULL, (void *)deviceListenThread, NULL);
	if(ret != 0)
	{
		printf("Create device listen thread failed!\n");
		return 0;
	}

	return 1;
}



