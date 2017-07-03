//device.h

#ifndef _DEVICE_H
#define _DEVICE_H

#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#include "hash.h"
#include "socketTools.h"
#include "register.h"
#include "epollTools.h"

#define PASSWORD_SIZE		(27)		/* length of device password */
#define DEVICEID_SIZE		(34)		/* length of deviceId */
#define DEVICETABLE_SIZE	(100000)	/* length of device table */


/**
 * device infomation 
 */
typedef struct deviceNode
{
	char deviceId[DEVICEID_SIZE+1];		/* deviceId */
	char password[PASSWORD_SIZE+1];		/* device password */ 
	long long lastHeartBeatTime;		/* timestamp of last heartbeat message */
	int deviceSockfd;					/* socket fd of device */
	struct deviceNode* next;			/* pointer points to next device which share the hash key with current device */

}deviceNode;


/**
 * one list in device table 
 */
typedef struct deviceList
{
	deviceNode head;					/* head of current list */
	pthread_mutex_t mutex;				/* clock of current list */
}deviceList;


/**
 * device table 
 */
typedef struct deviceTable
{
	deviceList *table;					/* pointer of device table */
	int size;							/* size of device table */
	int hashMask;						/* mask of hash，equals to size-1 */
	//int used;							/* number of device in table */

}deviceTable;

/*
typedef struct dict
{
	deviceTable *dt0;
	deviceTable *dt1;
	int rehash;
}dict;
*/

extern deviceTable *smartDeviceTable;			/* device table */




extern int initDeviceTable(int dtlen);

extern unsigned int tableHashKey(deviceTable *dt, char *deviceID);

extern deviceNode* findDevice(deviceTable *dt, char *deviceID);

extern deviceNode* findDevicePrior(deviceTable *dt, char *deviceID);

extern deviceNode* createDevice(int sockfd);

extern void deleteDevice(deviceNode* device, int epfd);

extern void insertDevice(deviceTable *dt, char *deviceID, deviceNode *newDevice);

extern int updateDevice(deviceTable *dt, char *deviceID, char *password, long long lastHeartBeatTime);


#endif

