//userRequest.c

#include "userRequest.h"

/**
 * make message to send to user
 */
void makeUserMessage(char *sendBuffer, int userSockfd, int messageLen, char *message)
{
	memset(sendBuffer, '\0', sizeof(sendBuffer));
	sprintf(sendBuffer, "#USmessage#");
	sprintf(sendBuffer+MESSAGE_HEAD, "%d", userSockfd);
	sprintf(sendBuffer+MESSAGE_HEAD+USERSOCKFD_SIZE, "%d", messageLen);
	sprintf(sendBuffer+MESSAGE_HEAD+USERSOCKFD_SIZE+MESSAGELEN_SIZE, "%s", message);
	return ;
}




/**
 * analysis message receive from user
 */
void analysisUserMessage(char *recvBuffer, int *userSockfd, int *messageLen)
{
	sscanf(recvBuffer+MESSAGE_HEAD, "%d", userSockfd);
	sscanf(recvBuffer+MESSAGE_HEAD+USERSOCKFD_SIZE, "%d", messageLen);
	return ;
}




/**
 * analysis user's requests, and return the result
 * 
 * request format:
 * QUE -- query device's password
 * MOD -- modify device's password
 * EXE -- execute a RPC command
 */
char *analysisRequest(int deviceSockfd, char *message, int messageBodyLen)
{
	char *retMessage;

	if(strncmp(message, "QUE", 3) == 0)
	{
		int passwordLen = strlen(deviceTable[deviceSockfd].password);
		retMessage = (char *)malloc(sizeof(char) * (passwordLen));
		sprintf(retMessage, "%s", deviceTable[deviceSockfd].password);
	}
	else if(strncmp(message, "MOD", 3) == 0)
	{
		if(deviceTable[deviceSockfd].enable == 0)
		{
			retMessage = (char *)malloc(sizeof(char) * 17);
			sprintf(retMessage, "Operation failed!");
		}
		else
		{
			strncpy(deviceTable[deviceSockfd].password, message+3, messageBodyLen-3);
			deviceTable[deviceSockfd].password[messageBodyLen-3] = '\0';
			retMessage = (char *)malloc(sizeof(char) * 18);
			sprintf(retMessage, "Operation succeed!");
		}
	}
	else if(strncmp(message, "EXE", 3) == 0)
	{
		retMessage = RPC(message+3);
	}
	else
	{
		retMessage = (char *)malloc(sizeof(char) * 20);
		sprintf(retMessage, "Undefined Operation!");
	}

	return retMessage;
}




/**
 * handle user request
 */
char *handleUserRequest(int userSockfd, int deviceSockfd, char *message, int messageBodyLen, int *retMessageLen)
{
	char *messageBody = analysisRequest(deviceSockfd, message, messageBodyLen);
	int lenBody = strlen(messageBody);
	int lenAll = lenBody + MESSAGE_HEAD_SIZE;

	*retMessageLen = lenAll;
	char *messageAll = (char *)malloc(sizeof(char) * lenAll);

	makeUserMessage(messageAll, userSockfd, lenBody, messageBody);
	free(messageBody);

	return messageAll;
}

