# Makefle for server

# Define macros for name of compiler
CC = gcc

TARGET = server

OBJS = server.o deviceMessage.o device.o socketTools.o epollTools.o register.o hash.o detectTimeOut.o userMessage.o pooledBuffer.o multiThread.o

CXXFLAGS = -g 
	
$(TARGET) : $(OBJS)
	$(CC) -o server $(OBJS) -lpthread

server.o : server.h detectTimeOut.h userMessage.h
	$(CC) -c server.c $(CXXFLAGS) -lpthread

deviceMessage.o : deviceMessage.h device.h epollTools.h register.h pooledBuffer.h multiThread.h
	$(CC) -c deviceMessage.c $(CXXFLAGS) -lpthread

device.o : device.h hash.h socketTools.h epollTools.h register.h
	$(CC) -c device.c $(CXXFLAGS) -lpthread

socketTools.o : socketTools.h
	$(CC) -c socketTools.c $(CXXFLAGS)

epollTools.o : epollTools.h
	$(CC) -c epollTools.c $(CXXFLAGS)

hash.o : hash.h
	$(CC) -c hash.c $(CXXFLAGS)

register.o : register.h
	$(CC) -c register.c $(CXXFLAGS)

detectTimeOut.o : detectTimeOut.h deviceMessage.h
	$(CC) -c detectTimeOut.c $(CXXFLAGS) -lpthread

userMessage.o : userMessage.h deviceMessage.h
	$(CC) -c userMessage.c $(CXXFLAGS) -lpthread

pooledBuffer.o : pooledBuffer.h socketTools.h
	$(CC) -c pooledBuffer.c $(CXXFLAGS) -lpthread

multiThread.o : multiThread.h
	$(CC) -c multiThread.c $(CXXFLAGS) -lpthread

.PHONY:run
run:		
	./server

.PHONY:clean
clean:
	rm *.o

	
