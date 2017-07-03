//heartBeat.c

#include "heartBeat.h"



/**
 * heartBeat processing flow：
 * 
 * 1. server create socket and wait for devices to connect with
 * 2. device request to build connection with server
 * 3. server accept device's request，then register device a new Id and password for device
 * 4. server fill device's infomation in device table, and send deviceId and password to device 
 *    message format：
 *       #heartBeat#     deviceId       password
 *    |<-  11byte  ->|<-  34byte  ->|<-  27byte  ->|
 * 5. device receive register message and record it, then send server a heartBeat message every 10 seconds
 *    message format is the same as above
 * 6. server receive heartBeat message，then update device's Id, password and timestamp, and send a feedback message to device
 *    feedback message format：(1 for success，0 for failure)
 *       #heartBeat#            111....111
 *    |<-  11byte  ->|<-          61byte         ->|
 * 7. server scan device table every 120 seconds, if some device is not lost connection beyond 60 seconds, 
 *    then remove it from device table 
 *
 */

char *serverIP = "172.18.11.117";			/* server IP */

const int serverPort = 8000;				/* server Port */




/**
 * make heart beat message
 * put deviceId and password into message
 */
void makeHeartBeatMSG(char *message, char *deviceId, char *password)
{
	memset(message, '\0', sizeof(message));

	strncpy(message, "#heartBeat#", MESSAGE_HEAD);
	strncpy(message+MESSAGE_HEAD, deviceId, DEVICEID_SIZE);
	strcpy(message+MESSAGE_HEAD+DEVICEID_SIZE, password);
	return ;
}




/**
 * analysis heart beat message
 * get deviceId and password from message
 */
void analysisHeartBeatMSG(char * message, char *deviceId, char *password)
{
	strncpy(deviceId, message+MESSAGE_HEAD, DEVICEID_SIZE);
	strcpy(password, message+MESSAGE_HEAD+DEVICEID_SIZE);
	return ;
}




/**
 * analysis the type of a heartBeat message
 *
 * return :
 * 0  -->  register message
 * 1  -->  feedBack message(succeed)
 * 2  -->  feddBack message(failed)
 */
int heartBeatMessageType(char * message)
{
	int i;

	for(i = MESSAGE_HEAD; i < HEART_BEAT_MESSAGE_SIZE; ++i)
		if(message[i] != '1')
			break;
	if(i == HEART_BEAT_MESSAGE_SIZE)
		return 1;


	for(i = MESSAGE_HEAD; i < HEART_BEAT_MESSAGE_SIZE; ++i)
		if(message[i] != '0')
			break;
	if(i == HEART_BEAT_MESSAGE_SIZE)
		return 2;

	return 0;
}




/**
 * create [deviceNumber] devices, and built connection with server,
 * then add them into epoll 
 *
 * return :
 * 0  -->  fail
 * 1  -->  success
 */
int createSocketsAndConnect(char *serverIp, int serverPort, int deviceNumber)
{
	int sockfd;
	int i;
	struct epoll_event ev;
	ev.events = EPOLLIN|EPOLLET;

	for(i = 0; i < deviceNumber; ++i)
	{
		sockfd = createAndConnect(serverIp, serverPort);
		if(sockfd == -1)
		{
			printf("Create socket failed!\n");
			return 0;
		}
		ev.data.fd = sockfd;
		addEpollEvent(epfd, sockfd, &ev);
		deviceTable[sockfd].enable = 1;
	}
	return 1;
}




/**
 * create a new device
 *
 * create a new socket, connect to server, 
 * reset the devicenode, then register it to epoll
 */
 void createANewDevice()
 {
 	int newfd = createAndConnect(serverIP, serverPort);
 	if(newfd == -1)
 	{
 		printf("Create new socket failed!\n");
 		return ;
 	}

 	struct epoll_event ev;
 	ev.data.fd = newfd;
	ev.events = EPOLLIN|EPOLLET;
	addEpollEvent(epfd, newfd, &ev);
	resetDevice(deviceTable, newfd);
	deviceTable[newfd].enable = 1;
	return ;

 }




/**
 * sent heart beat to server
 *
 * if device timeout or sending failed:
 * 1. close sockfd
 * 2. reset dt[sockfd]
 * 3. delete sockfd in epfd
 */
void sentHeartBeat(deviceNode* dt, int sockfd)
{
	char deviceId[DEVICEID_SIZE];
	char password[PASSWORD_SIZE];
	long long lastHeartBeatTime;

	pthread_mutex_lock(&(dt[sockfd].mutex));
	strncpy(deviceId, dt[sockfd].deviceId, DEVICEID_SIZE);
	strcpy(password, dt[sockfd].password);
	lastHeartBeatTime = dt[sockfd].lastHeartBeatTime;
	pthread_mutex_unlock(&(dt[sockfd].mutex));

	long long currentTime = ustime();
	/* timeout : 60 seconds */
	if(currentTime - lastHeartBeatTime >= HEARTBEAT_TIMEOUT)
	{
		close(sockfd);
		resetDevice(dt, sockfd);
		deleteEpollEvent(epfd, sockfd);
		createANewDevice();
		return ;
	}

	/* send heartBeat message */
	char message[HEART_BEAT_MESSAGE_SIZE];
	makeHeartBeatMSG(message, deviceId, password);
	int ret = writeNonBlocking_n(sockfd, message, HEART_BEAT_MESSAGE_SIZE);
	if(ret != 1)
	{
		close(sockfd);
		resetDevice(dt, sockfd);
		deleteEpollEvent(epfd, sockfd);
		createANewDevice();
		return ;
	}

#ifdef HEARTBEAT_DEBUG
	printf("--- Send heartBeat message : %s\n", message);
#endif

	return ;
}




/* Sending heart beat thread parameter table */
HBthreadPara HBthreadParaTable[HEARTBEAT_THREAD_SIZE];			

/**
 * send heart beat thread
 *
 * each thread is in charge devices arrange from first to last(not include)
 */
void *sendHBThread(int *id)
{
	int threadId = *id;
	long long beginTime;
	long long endTime;
	long long waitTime;
	int first = HBthreadParaTable[threadId].first;
	int last = HBthreadParaTable[threadId].last;
	
	pthread_detach(pthread_self());

sendHeartBeat:

	while(1)
	{
		beginTime = ustime();

		int i;
		for(i = first; i < last; ++i)
		{
			if(deviceTable[i].enable == 1)
			sentHeartBeat(deviceTable, i);
		}

		endTime = ustime();
		
		/* sending interval is 10 seconds */
		waitTime = HEARTBEAT_INTERVAL - (endTime - beginTime);
		if(waitTime > 0)
			usleep(waitTime);
	}

thread_exit:
	pthread_exit(NULL); 

}




/**
 * init sending heart beat thread pool
 *
 * return :
 * 0  -->  fail
 * 1  -->  success
 */
int initHBThreadPool()
{
	/* caculate how many device each thread is in charge of */
	int deviceNumber = DEVICETABLE_SIZE / HEARTBEAT_THREAD_SIZE;
	int deviceId = 0;
	int threadId = 0;
	int id;
	int ret;

	/* create heartBeat message threads */
	for(threadId = 0; threadId < HEARTBEAT_THREAD_SIZE; ++threadId)
	{
		HBthreadParaTable[threadId].first = deviceId;
		if(threadId == HEARTBEAT_THREAD_SIZE-1)
			HBthreadParaTable[threadId].last = DEVICETABLE_SIZE;
		else
			HBthreadParaTable[threadId].last = deviceId + deviceNumber;
		HBthreadParaTable[threadId].threadId = threadId;
		id = threadId;
		deviceId += deviceNumber;

		ret = pthread_create(&(HBthreadParaTable[threadId].PthId), NULL, (void *)sendHBThread, &id);
		if(ret != 0)
		{
			printf("Creating sending heart beat thread failed!\n");
			return 0;
		}
		usleep(100);
	}

	return 1;

}


