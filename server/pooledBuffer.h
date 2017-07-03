//pooledBuffer.h

#ifndef _POOLED_BUFFER_H
#define _POOLED_BUFFER_H


#include "socketTools.h"

#define MEMBLOCK_SIZE		(1024)		/* size of each memory block(byte) */
#define BUFFER_POOL_SIZE	(100000)	/* number of MemBlocks in buffer pool */


/**
 * memory block
 */
typedef struct MemBlock
{
	char buffer[MEMBLOCK_SIZE];	/* buffer of current block */
	struct MemBlock *next;		/* pointer to next block */

}MemBlock; 


/**
 * memory pool
 */
typedef struct MemPool
{
	MemBlock *head;			/* pointer to the first MemBlock in current buffer pool */
	MemBlock *tail;			/* pointer to the last MemBlock in current buffer pool */
	int blockNumber;		/* number of MemBlocks in current buffer pool */
	pthread_mutex_t mutex;

}MemPool;

extern MemPool *bufferPool;


/**
 * socket buffer
 */
typedef struct socketBuffer
{
	MemBlock *head;			/* pointer to the first MemBlock in those are used by current socket */
	MemBlock *tail;			/* pointer to the last MemBlock in those are used by current socket */
	int usedByte;			/* number of bytes which have been used */
	int leftByte;			/* number of bytes which haven't been used */
	char *lastByte;			/* position of the last byte */

}socketBuffer;

#endif



extern int initBufferPool(int bufferSize);

extern void destroyBufferPool();

extern int increaseBufferPool(int number);

extern int decreaseBufferPool(int number);

extern MemBlock *allocateBlock();

extern void insertBlock(MemBlock *newBlock);

extern void socketBufferInit(socketBuffer *SOBuffer);

extern MemBlock *socketBufferApplyNewBlock(socketBuffer *SOBuffer);

extern void socketBufferFreeBlock(socketBuffer *SOBuffer);

extern int socketBufferReadMessage(int sockfd, socketBuffer *SOBuffer, int byteNumber);

extern int socketBufferWriteMessage(int sockfd, socketBuffer *SOBuffer);



