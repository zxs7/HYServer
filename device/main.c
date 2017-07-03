//main.c

#include "main.h"



int main() 
{
	
	/* init device table */
	int initRet;
	initRet = initDeviceTable(DEVICETABLE_SIZE);
	if(initRet == 0)
	{
		printf("Init device table failed!\n");
		return 0;
	}

	/* create epoll */
	int createEpollRet;
	createEpollRet = epollCreate();
	if(createEpollRet == 0)
	{
		printf("Create epoll failed!\n");
		return 0;
	}

	/* init receiving message thread pool */
	int initRecvRet;
	initRecvRet = initRecvThreadPool();
	if(initRecvRet == 0)
	{
		printf("Init Receive thread pool failed!\n");
		return 0;
	}

	/* start to listen epoll events */
	pthread_t epollListenPid;
	int epollListenRet = pthread_create(&epollListenPid, NULL, (void *)listenEpollEvent, NULL);
	if(epollListenRet == -1)
	{
		printf("Create epoll listenning thread failed!\n");
		return 0;
	}

	/* create sockets ans connect to server */
	int createSocketRet;
	createSocketRet = createSocketsAndConnect(serverIP, serverPort, DEVICE_SIZE);
	if(createSocketRet == 0)
	{
		printf("Create socket failed!\n");
		return 0;
	}

	usleep(HEARTBEAT_INTERVAL);
	
	/* send heartbeat message */
	int initHBThreadRet;
	initHBThreadRet = initHBThreadPool();
	if(initHBThreadRet == 0)
	{
		printf("Init heart beat thread poll failed!\n");
		return 0;
	}
	
	pthread_join(epollListenPid, NULL);

	return 0;
}

