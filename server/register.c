//register.c

#include "register.h"


/**
 * Return the UNIX time in microseconds 
 */
long long ustime() 
{
    struct timeval tv;
    long long ust;

    gettimeofday(&tv, NULL);
    ust = ((long long)tv.tv_sec)*1000000;
    ust += tv.tv_usec;
    return ust;
}




/**
 * register a new ID for a device
 * 
 * the new ID is consist of :
 *    timestamp +  clientIP +  clientPort
 * |<- 16byte ->|<- 12byte ->|<-  6byte ->|
 *
 * (IP format : ***.***.***.***)
 */
void IDRegister(char *ip, int port, char *newID)
{
	/* current timestamp */
	long long currentTime = ustime();

	/* ip string */
	char IP_str[13];
	memset(IP_str, '\0', sizeof(IP_str));
	strncpy(IP_str, ip, 3);
	strncpy(IP_str+3, ip+4, 3);
	strncpy(IP_str+6, ip+8, 3);
	strncpy(IP_str+9, ip+12, 3);

	sprintf(newID, "%lld%s%06d", currentTime, IP_str, port);

	return ;
}

