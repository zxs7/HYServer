//device.c

#include "device.h"


deviceNode *deviceTable;			/* device table */


/**
 * Return the UNIX time in microseconds 
 */
long long ustime() 
{
    struct timeval tv;
    long long ust;

    gettimeofday(&tv, NULL);
    ust = ((long long)tv.tv_sec)*1000000;
    ust += tv.tv_usec;
    return ust;
}




/**
 * reset a device node
 *
 */
void resetDevice(deviceNode* dt, int sockfd)
{
	pthread_mutex_lock(&(dt[sockfd].mutex));

	memset(dt[sockfd].deviceId, '0', sizeof(dt[sockfd].deviceId));
	dt[sockfd].deviceId[DEVICEID_SIZE] = '\0';
	memset(dt[sockfd].password, '\0', sizeof(dt[sockfd].password));
	strcpy(dt[sockfd].password, "123456");
	dt[sockfd].enable = 0;
	dt[sockfd].lastHeartBeatTime = ustime();
	dt[sockfd].deviceSockfd = sockfd;

	pthread_mutex_unlock(&(dt[sockfd].mutex));
	return ;
}




/**
 * init device table
 *
 * return : 
 * 0  -->  fail
 * 1  -->  success
 */
int initDeviceTable(int dtlen)
{
	deviceTable = (deviceNode*)malloc(sizeof(deviceNode) * dtlen);
	if(deviceTable == NULL)
		return 0;
	int i;
	for(i = 0; i < dtlen; ++i)
	resetDevice(deviceTable, i);
	return 1;
}




/**
 * update a device node
 */
void updateDevice(deviceNode* dt, int sockfd, char *deviceId, char *password)
{
	pthread_mutex_lock(&(dt[sockfd].mutex));

	strncpy(dt[sockfd].deviceId, deviceId, DEVICEID_SIZE);
	strcpy(dt[sockfd].password, password);
	dt[sockfd].lastHeartBeatTime = ustime();
	dt[sockfd].enable = 1;

	pthread_mutex_unlock(&(dt[sockfd].mutex));
	return ;
}

