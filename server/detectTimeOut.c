//detectTimeOut.c

#include "detectTimeOut.h"



/**
 *
 * server need to find out devices who have lost contact with server for more than 60s，
 * then server need delete the device from device table.
 *
 *
 * To avoid affecting other threads to use device table, this operation do not occupy
 * clock of device list, the operation is as below:
 * Taking device p as a example, before delete p, we find the node pri_p which is in front of p,
 * then let pri_p's next pointer points to p's next, then usleep(100)
 * (to let threads which are just visiting p pass to avoid illegle ptr error), finally free(p).
 *
 */
void *detectTimeOutThread()
{
	long long startTime;			/* start time for each loop */
	long long endTime;				/* end time for each loop */
	long long currentTime;			/* current time */
	long long leftTime;				/* leftTime before next loop starts */
	int i;
	deviceNode* head;
	deviceNode* headNext;

	/* detach from parent thread */
	pthread_detach(pthread_self());

wait_unlock:

	/* record start time */
	startTime = ustime();

	/* detect timeout device */
	for(i = 0; i < DEVICETABLE_SIZE; ++i)
	{
		head = &(smartDeviceTable->table[i].head);
		headNext = head->next;

		while(headNext)
		{
			currentTime = ustime();
			/* if current device lost connection(timeout), then delete it */
			if(currentTime - headNext->lastHeartBeatTime >= TIMEOUT_INTERVAL)
			{
#ifdef TIMEOUT_DEBUG
				printf("~~~ Device is offline : %s\n", headNext->deviceId);
#endif

				if(headNext->next == NULL)
				{
					head->next = NULL;
					/* before free device, wait a while to avoid there is other thread using this address */
					usleep(70);
					deleteDevice(headNext, deviceEpollfd);
					break;
				}
				else
				{
					head->next = headNext->next;
					usleep(70);
					deleteDevice(headNext, deviceEpollfd);
					headNext = head->next;
				}
			}
			else
			{
				head = headNext;
				headNext = head->next;
			}
		}
	}

	/* record end time */
	endTime = ustime();

	/* caculate left time */
	leftTime = endTime - startTime;

	/* if the used time is less than detect interval, wait a while */
	if(leftTime > 0)
		usleep(leftTime);

	goto wait_unlock;
 
thread_exit:
	pthread_exit(NULL); 
}




/**
 * init detect timeout device thread
 *
 * return :
 * 1 -> succeed
 * 0 -> failed
 */
int initTimeOutThread()
{
	int ret;
	pthread_t 	detectThreadId;

	ret = pthread_create(&detectThreadId, NULL, (void *)detectTimeOutThread, NULL);
	if(ret != 0)
	{
		printf("Creating detect timeout device thread failed!\n");
		return 0;
	}

	return 1;
}


