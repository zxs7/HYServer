//recvMessage.h

#ifndef _RECV_MESSAGE_H
#define _RECV_MESSAGE_H

#include "socketTools.h"


#define MESSAGE_LABEL_SIZE		(11)
#define MESSAGE_LENGTH_SIZE		(27)
#define RECV_MESSAGE_SIZE 		(38)


extern char *recvMessage(int sockfd);



#endif


