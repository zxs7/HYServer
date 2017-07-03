# Makefle for device

# Define macros for name of compiler
CC = gcc

TARGET = device

OBJS = main.o heartBeat.o device.o socketTools.o epollTools.o recvMessage.o userRequest.o RPC.o

CXXFLAGS = -g 
	
$(TARGET) : $(OBJS)
	$(CC) -o device $(OBJS) -lpthread

main.o : main.h heartBeat.h device.h socketTools.h epollTools.h recvMessage.h userRequest.h
	$(CC) -c main.c $(CXXFLAGS) -lpthread

heartBeat.o : heartBeat.h device.h socketTools.h epollTools.h
	$(CC) -c heartBeat.c $(CXXFLAGS) -lpthread

device.o : device.h
	$(CC) -c device.c $(CXXFLAGS)

socketTools.o : socketTools.h
	$(CC) -c socketTools.c $(CXXFLAGS)

epollTools.o : epollTools.h recvMessage.h
	$(CC) -c epollTools.c $(CXXFLAGS) -lpthread

recvMessage.o : heartBeat.h device.h socketTools.h epollTools.h recvMessage.h userRequest.h
	$(CC) -c recvMessage.c $(CXXFLAGS) -lpthread

userRequest.o : userRequest.h heartBeat.h RPC.h recvMessage.h
	$(CC) -c userRequest.c $(CXXFLAGS) -lpthread

RPC.o : RPC.h
	$(CC) -c RPC.c $(CXXFLAGS)

.PHONY:run
run:		
	./device

.PHONY:clean
clean:
	rm *.o

	
