# Project Description
This project is used as a TCP server of Haiyun system, which provides support for the Internet of Things. It is able to maintain TCP persistent connection for multiple devices and provide users with devices management services.


# Dependency
You just need to ensure that all the machines have installed gcc.


# How to use
## Server
### configuration Description
The meaning of the parameters are as follows: <br/>
userListenPort: port for listenning message from users <br/>
deviceListenPort: port for listenning message from devices <br/>
TIMEOUT_INTERVAL: the timeout interval for each device (microsecond) <br/>
DETECT_INTERVAL: the interval of starting detecting process(microsecond) <br/>
USER_MESSAGE_THREAD_SIZE: the number of threads to receive message from users <br/>
DEVICE_MESSAGE_THREAD_SIZE: the number of threads to receive message from devices <br/>
DEVICE_ACCEPT_THREAD_SIZE: the name of threads to accept connection request from devices <br/>
MEMBLOCK_SIZE: the size of each memory block (byte) <br/>
### Compiling
$ cd server/ and make, you will get an executable file server
### Usage
After compiling, run this server program with <br/>
$ make run (or ./server)

## Client
### configuration Description
The meaning of the parameters are as follows: <br/>
serverIP: the IP address of server <br/>
serverPort: the port of server used for client's connection <br/>
### Compiling
$ cd client/ and make, you will get an executable file client
### Usage
After compiling, run this client program with <br/>
$ make run (or ./client)

## Device
### configuration Description
The meaning of the parameters are as follows: <br/>
serverIP: the IP address of server <br/>
serverPort: the port of server used for device's connection <br/>
DEVICE_SIZE: the number of devices <br/>
HEARTBEAT_INTERVAL: the interval of sending heartbeat message <br/>
HEARTBEAT_TIMEOUT: the interval of heartbeat timeout <br/>
HEARTBEAT_THREAD_SIZE: the number of threads to send heartbeat message <br/>
RECV_THREAD_SIZE: the number of threads to receive message from server <br/>
### Compiling
$ cd device/ and make, you will get an executable file device
### Usage
After compiling, run this device program with <br/>
$ make run (or ./device)

