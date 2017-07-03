# Makefle for client

# Define macros for name of compiler
CC = gcc

TARGET = client

OBJS = main.o command.o recvMessage.o socketTools.o

CXXFLAGS = -g 
	
$(TARGET) : $(OBJS)
	$(CC) -o client $(OBJS) 

main.o : main.h command.h recvMessage.h socketTools.h
	$(CC) -c main.c $(CXXFLAGS) 

command.o : command.h recvMessage.h socketTools.h
	$(CC) -c command.c $(CXXFLAGS)

socketTools.o : socketTools.h
	$(CC) -c socketTools.c $(CXXFLAGS)

recvMessage.o : socketTools.h recvMessage.h
	$(CC) -c recvMessage.c $(CXXFLAGS)


.PHONY:run
run:		
	./client

.PHONY:clean
clean:
	rm *.o

	
