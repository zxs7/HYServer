//detectTimeOut.h

#ifndef _DETECT_TIME_OUT_H
#define _DETECT_TIME_OUT_H


#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "device.h"
#include "register.h"
#include "deviceMessage.h"


#define TIMEOUT_DEBUG


#define TIMEOUT_INTERVAL	(60000000)	/* timeout interval(microseconds) 60 seconds */
#define DETECT_INTERVAL		(120000000)	/* detect interval(microseconds) 120 seconds */



extern int initTimeOutThread();

#endif

