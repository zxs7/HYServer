//heartBeat.h


#ifndef _HEARTBEAT_H
#define _HEARTBEAT_H


#include "socketTools.h"
#include "device.h"
#include "epollTools.h"

#define HEARTBEAT_DEBUG

#define MESSAGE_HEAD 				(11)			/* message head length */
#define HEART_BEAT_MESSAGE_SIZE		(72)			/* heartbeat message size */
#define HEARTBEAT_THREAD_SIZE		(30)			/* size of heartBeat threads to send heartBeat message*/
#define HEARTBEAT_INTERVAL			(10000000)		/* heartbeat sending interval(microseconds) */
#define HEARTBEAT_TIMEOUT			(60000000)		/* heartbeat timeout interval(microseconds) */

extern char *serverIP;				/* server IP */
extern const int serverPort;		/* server Port */



typedef struct HBthreadPara
{
	int first;						/* deviceId of the first one */
	int last;						/* deviceId behind the last one */
	int threadId;					/* id of current thread */
	pthread_t PthId;				/* Pid of current thread  */

}HBthreadPara;

extern HBthreadPara HBthreadParaTable[];			/* Sending heart beat thread parameter table */


extern void makeHeartBeatMSG(char *message, char *deviceId, char *password);

extern void analysisHeartBeatMSG(char * message, char *deviceId, char *password);

extern int heartBeatMessageType(char * message);

extern void createANewDevice();

extern int createSocketsAndConnect(char *serverIp, int serverPort, int deviceNumber);

extern void sentHeartBeat(deviceNode* dt, int sockfd);

extern int initHBThreadPool();


#endif

