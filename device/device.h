//device.h

#ifndef _DEVICE_H
#define _DEVICE_H

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>



#define PASSWORD_SIZE		(27)		/* device password length */
#define DEVICEID_SIZE		(34)		/* device id length */
#define DEVICE_SIZE 		(27000)		/* size of devices */
#define DEVICETABLE_SIZE	(65537)		/* length of device table */

/**
 * device infomation
 */
typedef struct deviceNode
{
	char deviceId[DEVICEID_SIZE+1];		/* device id */
	char password[PASSWORD_SIZE+1];		/* device password */ 
	long long lastHeartBeatTime;		/* timestamp of last heartbeat message */
	int deviceSockfd;					/* device socket fd */
	int enable;							/* weather current node is available */
	pthread_mutex_t mutex;				/* device clock */

}deviceNode;

extern deviceNode *deviceTable;			/* device table */


extern long long ustime();

extern void resetDevice(deviceNode* dt, int sockfd);

extern int initDeviceTable(int dtlen);

extern void updateDevice(deviceNode* dt, int sockfd, char *deviceId, char *password);


#endif


