//main.c

#include "main.h"

char *serverIP = "172.18.11.117";				/* server IP */
const int serverPort = 8001;					/* server Port */



int main()
{
	int sockfd;
	int ret;

	sockfd = createAndConnect(serverIP, serverPort);
	if(sockfd == -1)
	{
		printf("Create connection to server failed!\n");
		return -1;
	}

	while(1)
	{
		char *command;
		int commandLen;

		/* read user's command */
		commandLen = readCommand(&command);

		/* send user's command(packed) */
		ret = writeNonBlocking_n(sockfd, command, commandLen);
		if(ret != 1)
		{
			printf("Write Error !\n");
			close(sockfd);
			if(command)
			free(command);
			return -1;
		}

		free(command);

		/* receive returned message */
		char *retMessage;
		retMessage = recvMessage(sockfd);
		if(retMessage == NULL)
		{
			printf("Read Message failed!\n");
			return -1;
		}

		/* print message */
		printf("%s\n", retMessage);

		/* free message memory */
		free(retMessage);
	}
	return 0;
}


