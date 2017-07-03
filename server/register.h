//register.h

#ifndef _REGISTER_H
#define _REGISTER_H

#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>


extern void IDRegister(char *ip, int port, char *newID);

extern long long ustime();


#endif

