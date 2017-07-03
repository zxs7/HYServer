//socketTools.c

#include "socketTools.h"



/**
 * set socket -> non-blocking, and close Nagle algorithm
 *
 * return:
 * -1  -->  fail
 * 0   -->  succeed
 */
int makeSocketNonBlocking (int sfd)
{
	int flags, s;

	flags = fcntl (sfd, F_GETFL, 0);
	if(flags == -1) 
	{
		perror ("fcntl");
		return -1;
	}

	flags |= O_NONBLOCK;
	s = fcntl(sfd, F_SETFL, flags);
	if (s == -1) 
	{
		perror ("fcntl");
		return -1;
	}

	/* close Nagle algorithm */
	int on = 1;
	setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));

	return 0;
}




/** 
 * create a socket and connect to server, then set socket non-blocking
 *
 * return :
 * -1      -->  fail
 * sockfd  -->  succeed
 */
int createAndConnect(char *serverIP, int serverPort)
{
	int sockfd;					
	struct sockaddr_in pin;			/* socket address */
	struct hostent *shost_name;		/* server info */
	
	/* get server info */
	if ((shost_name = gethostbyname(serverIP)) == 0) 
	{
		printf("Error resolving local host\n");
		return -1;
	}

	/* get socket address */
	bzero(&pin, sizeof(pin));
	pin.sin_family = AF_INET;
	pin.sin_addr.s_addr = htonl(INADDR_ANY);
	pin.sin_addr.s_addr = ((struct in_addr *)(shost_name->h_addr))->s_addr;
	pin.sin_port = htons(serverPort);

	/* create new socket */
	if((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) 
	{
		printf("Error in creating new socket\n");
		return -1;
	}

	/* connect to server */
	if(connect(sockfd, (void *)&pin, sizeof(pin)) == -1) 
	{
		printf("Error in connecting to socket\n");
		printf("Errno : %d\n",errno);
		return -1;
	}

	/* set socket non-blocking */
	makeSocketNonBlocking(sockfd);

	return sockfd;
}




/**
 * read n byte from non-blocking socket fd, and put it into buf
 * do not stop until have read n bytes
 *
 * return:
 * -1  -->  exception
 * 0   -->  connection closed
 * 1   -->  succeed
 */
int readNonBlocking_n(int fd, char * buf, int n)
{
	int count = 0;
	int ret;
	int eagainTime = 1000;

	while(1) 
	{
		if(count == n)
		return 1;

		ret = read(fd, buf+count, n-count);

		if(ret == -1) 
		{
			/* if errno == EAGAIN, that means we have read all data
			 * else, means some error occured
			 */
			if(errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) 
			{
				perror("read");
				return -1;
			} 
			else 
			{
				usleep(1000);
				if(eagainTime-- == 0)
					return -1;
				continue;
			}
		} 
		else if(ret == 0) 
		{
			return 0;
		}
		else
		{
			count += ret;
		}
	}
}




/**
 * read n byte from buf, and write it into non-blocking socket fd
 * do not stop until hava written n bytes
 *
 * return:
 * -1  -->  exception
 * 0   -->  connection closed
 * 1   -->  succeed
 */
int writeNonBlocking_n(int fd, char * buf, int n)
{
	int count = 0;
	int ret;
	int eagainTime = 1000;

	while(1) 
	{
		if(count == n)
		return 1;

		ret = write(fd, buf+count, n - count);
		
		if(ret == -1) 
		{
			/* if errno == EAGAIN, that means we have written all data
			 * else, means some error occured
			 */
			if(errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) 
			{
				perror ("write");
				return -1;
			} 
			else 
			{
				usleep(1000);
				if(eagainTime-- == 0)
					return -1;
				continue;
			}
		} 
		else if(ret == 0) 
		{
			return 0;
		}
		else
		{
			count += ret;
		}
	}
}


