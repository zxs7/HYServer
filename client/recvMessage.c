//recvMessage.c


#include "recvMessage.h"


char *recvMessage(int sockfd)
{
	char messageHead[RECV_MESSAGE_SIZE];
	int ret;
	int messageLen;

	/* read message head */
	ret = readNonBlocking_n(sockfd, messageHead, RECV_MESSAGE_SIZE);
	if(ret != 1)
	{
		printf("Read Error!\n");
		close(sockfd);
		return NULL;
	}

	/* get length of the following message */
	sscanf(messageHead + MESSAGE_LABEL_SIZE, "%d", &messageLen);
	char *message;
	message = (char *)malloc(sizeof(char) * (messageLen+1));

	/* read the following message */
	ret = readNonBlocking_n(sockfd, message, messageLen);
	if(ret != 1)
	{
		printf("Read Error!\n");
		close(sockfd);
		free(message);
		return NULL;
	}

	message[messageLen] = '\0';

	return message;
}


