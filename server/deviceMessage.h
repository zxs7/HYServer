//deviceMessage.h

#ifndef _DEVICEMESSAGE_H
#define _DEVICEMESSAGE_H


#include "device.h"
#include "epollTools.h"
#include "register.h"
#include "pooledBuffer.h"
#include "multiThread.h"


#define DEVICE_MESSAGE_DEBUG							/* used to debug device message */



#define DEVICE_MESSAGE_THREAD_SIZE 		(200)			/* size of receiving device message thread pool */
#define DEVICE_ACCEPT_THREAD_SIZE		(100) 			/* size of accepting device connection request thread pool */
#define DEVCIE_MESSAGE_LISTEN_MAX 		(150000)		/* maximum of number of events epoll is able to listen to  */

#define MESSAGE_TYPE_SIZE 				(11)			/* length of message type */
#define USERSOCKFD_SIZE					(34)			/* bytes of variable which indicates value of user socket fd */
#define MESSAGELEN_SIZE					(27)			/* bytes of variable which indicates length of followed message */
#define MESSAGE_HEAD_SIZE				(72)			/* length of message head */



extern const int deviceListenPort;						/* port for listening device connection */

extern int deviceEpollfd;								/* epoll for devcie request */


extern pthread_t deviceListenThreadId;

extern void makeMessageHeadToUser(char *sendBuffer, int messageLen);

extern int initDeviceListenThread();


#endif
