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
#include <dirent.h>
#include <sys/io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>


extern void ipFormat(char *ip);

extern void getPeerAddress(int fd, char *ip, int *port);

extern int makeSocketNonBlocking (int sfd);

extern int createAndConnect(char *serverIP, int serverPort);

extern int createAndBind(int serverPort);

extern int socketHasData(int sockfd);

extern int readNonBlocking(int fd, char * buf, int n);

extern int readNonBlocking_n(int fd, char * buf, int n);

extern int writeNonBlocking(int fd, char * buf, int n);

extern int writeNonBlocking_n(int fd, char * buf, int n);


#endif

