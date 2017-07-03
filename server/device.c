//device.c

#include "device.h"




/* There are three situations of changing device table：
 * 
 * 1. When server receives a heartbeat message，and the device is in device table, 
 *    then update lastheartBeatTime of the device. Because for each device, 
 *    there can be only one thread to update its state, so here do not need any clock
 *
 * 2. When server receives a heartbeat message，and the device is not in device table,
 *    then server need to register this device , and insert to device table.
 *    For this situation, there can multi-thread execute this operation at the same time,
 *    so, we need a clock for each list to avoid confusion.
 *
 * 3. When server checks out a device who has lost contact for more than 60s，then server need delete 
 *    the device from device table.
 *    Taking device p as a example, before delete p, we need occupy clock of list which p lines in,
 *    and find the node pri_p which is in front of p, then let pri_p's next pointer points to p's next,
 *    then usleep(100)(to let threads which are just visiting p pass to avoid illegle ptr error), 
 *    finally free(p).
 *
 */
deviceTable *smartDeviceTable;			/* device table */




/**
 * init device Table
 * 
 * return :
 * 0  -->  fail
 * 1  -->  success
 */
int initDeviceTable(int dtlen)
{
	smartDeviceTable = (deviceTable*)malloc(sizeof(deviceTable));
	if(smartDeviceTable == NULL)
	{
		printf("Init device table failed!\n");
		return 0;
	}

	smartDeviceTable->size = dtlen;
	smartDeviceTable->hashMask = dtlen-1;
	smartDeviceTable->table = (deviceList*)malloc(sizeof(deviceList) * dtlen);
	if(smartDeviceTable->table == NULL)
	{
		printf("Init device table failed!\n");
		return 0;
	}

	int i;
	for(i = 0; i < dtlen; ++i)
	smartDeviceTable->table[i].head.next = NULL;
	return 1;
}




/**
 * get hash key for device in dt
 * 
 * return : hash key index
 */
unsigned int tableHashKey(deviceTable *dt, char *deviceID)
{
	int dtmask = dt->hashMask;
	int h = hashFunction(deviceID, strlen(deviceID));
	return h & dtmask;
}




/**
 * find device with ID deviceID from dt
 *
 * return :
 * NULL        -->  not find
 * devicePtr   -->  find
 */
deviceNode* findDevice(deviceTable *dt, char *deviceID)
{
	deviceNode* devicePtr = NULL;
	unsigned int listID = tableHashKey(dt, deviceID);

	devicePtr = dt->table[listID].head.next;
	while(devicePtr)
	{
		if(strcmp(deviceID, devicePtr->deviceId) == 0)
			return devicePtr;
		devicePtr = devicePtr->next;
	}
	return devicePtr;
}




/**
 * find device which is before device with ID deviceID from dt
 *
 * return :
 * NULL             -->  not find
 * devicePtrPrior   -->  find
 */
deviceNode* findDevicePrior(deviceTable *dt, char *deviceID)
{
	deviceNode *devicePtr = NULL;
	deviceNode *devicePtrPrior = NULL;
	unsigned int listID = tableHashKey(dt, deviceID);

	devicePtrPrior = &(dt->table[listID].head);
	devicePtr = dt->table[listID].head.next;
	while(devicePtr)
	{
		if(strcmp(deviceID, devicePtr->deviceId) == 0)
			return devicePtrPrior;
		devicePtr = devicePtr->next;
		devicePtrPrior = devicePtrPrior->next;
	}
	return NULL;
}




/**
 * Create a new device with socket : sockfd
 *
 * return :
 * address of new device(return NULL if failed)
 */
deviceNode* createDevice(int sockfd)
{
	/* malloc memory for new device */
	deviceNode *newdevice = (deviceNode*)malloc(sizeof(deviceNode));
	if(newdevice == NULL)
	{
		printf("*** Error : Create new device failed!\n");
		return NULL;
	}

	/* get ip and port of peer site */
	char clientIP[16];
	int clientPort;
	getPeerAddress(sockfd, clientIP, &clientPort);

	/* register a new ID */
	IDRegister(clientIP, clientPort, newdevice->deviceId);

	/* set initial password to be 123456 */
	strcpy(newdevice->password, "123456");

	/* set lateset timestamp */
	newdevice->lastHeartBeatTime = ustime();

	newdevice->next = NULL;
	newdevice->deviceSockfd = sockfd;

	return newdevice;
}




/**
 * delete device from device table, then free its memory
 */
void deleteDevice(deviceNode* device, int epfd)
{
	/* close socket */
	close(device->deviceSockfd);

	/* delete sockfd from epoll */
	deleteEpollEvent(epfd, device->deviceSockfd);

	/* free the device */
	device->next = NULL;
	free(device);

	return ;
}




/**
 * insert device into dt
 */
void insertDevice(deviceTable *dt, char *deviceID, deviceNode *newDevice)
{
	unsigned int listID = tableHashKey(dt, deviceID);
	pthread_mutex_lock(&(dt->table[listID].mutex));
	newDevice->next = dt->table[listID].head.next;
	dt->table[listID].head.next = newDevice;
	pthread_mutex_unlock(&(dt->table[listID].mutex));
	return ;
}




/**
 * update device
 *
 * return :
 * 0  -->  not find
 * 1  -->  success
 */
int updateDevice(deviceTable *dt, char *deviceID, char *password, long long lastHeartBeatTime)
{
	deviceNode* devicePtr = NULL;
	devicePtr = findDevice(dt, deviceID);
	if(devicePtr == NULL)
		return 0;
	strcmp(devicePtr->password, password);
	devicePtr->lastHeartBeatTime = lastHeartBeatTime;
	return 1;
}



