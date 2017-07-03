//command.c


#include "command.h"



int readCommand(char **command)
{
	char deviceId[DEVICEID_SIZE+1];
	char operationType[30];
	int  commandLen;

input:
	printf(">> Please input device ID:\n");
	printf(">> ");
	scanf("%s", deviceId);

	printf(">> Please input operation type:\n");
	printf(">> ");
	scanf("%s", operationType);

	if(strcmp(operationType, "help") == 0)
	{
		printf(">> Operation Types : \n");
		printf(">> query    -->    query password of device\n");
		printf(">> modify   -->    modify password of device\n");
		printf(">> excute   -->    excute RPC request\n\n");
		goto input;
	}
	else if(strcmp(operationType, "query") == 0)
	{
		commandLen = MESSAGE_LABEL_SIZE + DEVICEID_SIZE + MESSAGE_LENGTH_SIZE + OPERATION_TYPE_SIZE;
		(*command) = (char *)malloc(sizeof(char) * commandLen);
		sprintf((*command), "#USmessage#");
		sprintf((*command) + MESSAGE_LABEL_SIZE, "%s", deviceId);
		sprintf((*command) + MESSAGE_LABEL_SIZE + DEVICEID_SIZE, "%d", commandLen - SEND_MESSAGE_SIZE);
		sprintf((*command) + SEND_MESSAGE_SIZE, "QUE");
		return commandLen;
	}
	else if(strcmp(operationType, "modify") == 0)
	{
		char password[100];
		printf(">> Please input new password:\n");
		printf(">> ");
		scanf("%s", password);

		int passwordLen;
		passwordLen = strlen(password);

		commandLen = MESSAGE_LABEL_SIZE + DEVICEID_SIZE + MESSAGE_LENGTH_SIZE + OPERATION_TYPE_SIZE + passwordLen;
		(*command) = (char *)malloc(sizeof(char) * commandLen);
		sprintf((*command), "#USmessage#");
		sprintf((*command) + MESSAGE_LABEL_SIZE, "%s", deviceId);
		sprintf((*command) + MESSAGE_LABEL_SIZE + DEVICEID_SIZE, "%d", commandLen - SEND_MESSAGE_SIZE);
		sprintf((*command) + SEND_MESSAGE_SIZE, "MOD");
		sprintf((*command) + SEND_MESSAGE_SIZE + OPERATION_TYPE_SIZE, "%s", password);
		return commandLen;
	}
	else if(strcmp(operationType, "execute") == 0)
	{
		char expression[100];
		printf(">> Please input expression:\n");
		printf(">> ");
		scanf("%s", expression);

		int expressionLen;
		expressionLen = strlen(expression);

		commandLen = MESSAGE_LABEL_SIZE + DEVICEID_SIZE + MESSAGE_LENGTH_SIZE + OPERATION_TYPE_SIZE + expressionLen;
		(*command) = (char *)malloc(sizeof(char) * commandLen);
		sprintf((*command), "#USmessage#");
		sprintf((*command) + MESSAGE_LABEL_SIZE, "%s", deviceId);
		sprintf((*command) + MESSAGE_LABEL_SIZE + DEVICEID_SIZE, "%d", commandLen - SEND_MESSAGE_SIZE);
		sprintf((*command) + SEND_MESSAGE_SIZE, "EXE");
		sprintf((*command) + SEND_MESSAGE_SIZE + OPERATION_TYPE_SIZE, "%s", expression);
		return commandLen;
	}
	else
	{
		printf("This operation is not defined!\n\n");
		goto input;
	}
}



