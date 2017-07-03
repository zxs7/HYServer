//userRequest.h


#ifndef _USERREQUEST_H
#define _USERREQUEST_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "heartBeat.h"
#include "RPC.h"
#include "recvMessage.h"

#define MESSAGELEN_SIZE		(27)		/* bytes of variable which indicates length of followed message */
#define USERSOCKFD_SIZE		(34)		/* bytes of variable which indicates value of user socket fd */


extern void makeUserMessage(char *sendBuffer, int userSockfd, int messageLen, char *message);

extern void analysisUserMessage(char *recvBuffer, int *userSockfd, int *messageLen);

extern char *handleUserRequest(int userSockfd, int deviceSockfd, char *message, int messageBodyLen, int *retMessageLen);

#endif

