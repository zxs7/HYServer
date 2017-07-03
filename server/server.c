
#include "server.h"


int main(int argc, char **argv) 
{

	int ret;

	/* init device table */
	ret = initDeviceTable(DEVICETABLE_SIZE);
	if(smartDeviceTable == NULL)
	return 0;

	/* init bufferPool */
	ret = initBufferPool(BUFFER_POOL_SIZE);
	if(ret == -1)
		return 0;

	/* init device listen thread */
	ret = initDeviceListenThread();
	if(ret == 0)
	return 0;

	/* init user listen thread */
	ret = initUserListenThread();
	if(ret == 0)
	return 0;

	/* init detect timeout thread */
	ret = initTimeOutThread();
	if(ret == 0)
	return 0;

	pthread_join(deviceListenThreadId, NULL);
	return 0;
}


