//command.h

#ifndef _COMMAND_H
#define _COMMAND_H


#include "socketTools.h"
#include "recvMessage.h"

#define DEVICEID_SIZE			(34)
#define SEND_MESSAGE_SIZE		(72)
#define OPERATION_TYPE_SIZE		(3)

extern int readCommand(char **command);


#endif



