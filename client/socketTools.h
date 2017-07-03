//socketTools.h


#ifndef _SOCKET_TOOL_H
#define _SOCKET_TOOL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/tcp.h>


extern int makeSocketNonBlocking (int sfd);

extern int createAndConnect(char *serverIP, int serverPort);

extern int readNonBlocking_n(int fd, char * buf, int n);

extern int writeNonBlocking_n(int fd, char * buf, int n);


#endif

