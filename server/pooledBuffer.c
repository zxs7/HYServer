//pooledBuffer.c

#include "pooledBuffer.h"



MemPool *bufferPool;		/* buffer pool for the whole server */




/**
 * init bufferPool
 *
 * Allocate memory as requested, and complete the structure of bufferPool
 *
 * return :
 * if succeed, return the current number of blocks in bufferPool
 * if failed, return -1
 */
int initBufferPool(int bufferSize)
{
	bufferPool = (MemPool *)malloc(sizeof(MemPool));
	if(bufferPool == NULL)
		return -1;

	if(bufferSize < 1)
	{
		bufferPool->head = NULL;
		bufferPool->tail = NULL;
		bufferPool->blockNumber = 0;
		return 0;
	}

	/* allocate the first MemBlock */
	MemBlock *newBlock;
	newBlock = (MemBlock *)malloc(sizeof(MemBlock));
	if(newBlock == NULL)
		return 0;
	bufferPool->head = newBlock;
	bufferPool->tail = newBlock;
	bufferPool->blockNumber = 1;

	/* allocate the rest MemBlock */
	while(--bufferSize)
	{
		newBlock = (MemBlock *)malloc(sizeof(MemBlock));
		if(newBlock == NULL)
			return bufferPool->blockNumber;
		bufferPool->tail->next = newBlock;
		bufferPool->tail = newBlock;
		bufferPool->blockNumber ++;
	}

	/* return blocks number */
	bufferPool->tail->next = NULL;
	return bufferPool->blockNumber;
}




/**
 * destroy bufferPool
 *
 * free all the memory that has been allocated to bufferPool
 */
void destroyBufferPool()
{
	MemBlock *head;
	MemBlock *headNext;

	if(bufferPool->blockNumber == 0)
	{
		free(bufferPool);
		return ;
	}

	/* free all blocks in bufferPool */
	head = bufferPool->head;
	headNext = head->next;
	while(headNext)
	{
		free(head);
		head = headNext;
		headNext = head->next;
	}
	free(head);

	/* free bufferPool */
	free(bufferPool);
	bufferPool = NULL;
	return ;
}




/**
 * increase number of blocks in bufferPool
 *
 * return :
 * the current number of blocks in bufferPool
 */
int increaseBufferPool(int number)
{
	MemBlock *newBlock;

	pthread_mutex_lock(&(bufferPool->mutex));

	while(number--)
	{
		newBlock = (MemBlock *)malloc(sizeof(MemBlock));
		if(newBlock == NULL)
			return bufferPool->blockNumber;
		bufferPool->tail->next = newBlock;
		bufferPool->tail = newBlock;
		bufferPool->blockNumber++;
	}
	bufferPool->tail->next = NULL;

	pthread_mutex_unlock(&(bufferPool->mutex));
	return bufferPool->blockNumber;
}




/**
 * decrease number of blocks in bufferPool
 *
 * return :
 * the current number of blocks in bufferPool
 */
int decreaseBufferPool(int number)
{
	if(number > bufferPool->blockNumber)
		return bufferPool->blockNumber;

	pthread_mutex_lock(&(bufferPool->mutex));

	MemBlock *headNext;
	headNext = bufferPool->head->next;

	while(number--)
	{
		free(bufferPool->head);
		bufferPool->head = headNext;
		headNext = bufferPool->head->next;
		bufferPool->blockNumber--;
	}

	pthread_mutex_unlock(&(bufferPool->mutex));
	return bufferPool->blockNumber;
}




/**
 * allocate a new block from bufferPool
 *
 * return :
 * address of new block(return NULL if failed)
 */
MemBlock *allocateBlock()
{
	MemBlock *newBlock;

	pthread_mutex_lock(&(bufferPool->mutex));

	/* if there are no more buffers to allocate, malloc a new one */
	if(bufferPool->blockNumber == 0)
	{
		newBlock = (MemBlock *)malloc(sizeof(MemBlock));
		if(newBlock != NULL)
			newBlock->next = NULL;
		pthread_mutex_unlock(&(bufferPool->mutex));
		return newBlock;
	}

	/* allocate the first buffer */
	newBlock = bufferPool->head;
	bufferPool->head = newBlock->next;
	bufferPool->blockNumber--;
	newBlock->next = NULL;

	pthread_mutex_unlock(&(bufferPool->mutex));
	return newBlock;
}




/**
 * insert a new block into bufferPool
 */
void insertBlock(MemBlock *newBlock)
{
	pthread_mutex_lock(&(bufferPool->mutex));

	if(bufferPool->blockNumber == 0)
	{
		bufferPool->head = newBlock;
		bufferPool->tail = newBlock;
		bufferPool->blockNumber = 1;
		pthread_mutex_unlock(&(bufferPool->mutex));
		return ;
	}

	bufferPool->tail->next = newBlock;
	bufferPool->tail = newBlock;
	bufferPool->blockNumber++;

	pthread_mutex_unlock(&(bufferPool->mutex));
	return ;
}




/**
 * socket buffer apply a new block
 *
 * Let bufferPool allocate a new block for socket, then modify the related parameters of socketBuffer
 * return :
 * new block address(return NULL if failed)
 */
MemBlock *socketBufferApplyNewBlock(socketBuffer *SOBuffer)
{
	MemBlock *newBlock;
	newBlock = allocateBlock();

	if(newBlock == NULL)
		return newBlock;

	if(SOBuffer->head == NULL)
	{
		SOBuffer->head = newBlock;
		SOBuffer->tail = newBlock;
		SOBuffer->usedByte = 0;
		SOBuffer->leftByte = MEMBLOCK_SIZE;
		SOBuffer->lastByte = NULL;
		return newBlock;
	}

	SOBuffer->tail->next = newBlock;
	SOBuffer->tail = newBlock;
	SOBuffer->leftByte += MEMBLOCK_SIZE;
	return newBlock;

}




/**
 * init socketBuffer
 */
void socketBufferInit(socketBuffer *SOBuffer)
{
	SOBuffer->head = NULL;
	SOBuffer->tail = NULL;
	SOBuffer->usedByte = 0;
	SOBuffer->leftByte = 0;
	SOBuffer->lastByte = NULL;
	return ;
}




/**
 * socket buffer free all the blocks it owns
 *
 * Insert freed blocks into bufferPool 
 */
void socketBufferFreeBlock(socketBuffer *SOBuffer)
{
	MemBlock *head;
	MemBlock *headNext;

	head = SOBuffer->head;
	if(head == NULL)
		return ;

	headNext = head->next;
	while(headNext)
	{
		head->next = NULL;
		insertBlock(head);
		head = headNext;
		headNext = head->next;
	}
	
	insertBlock(head);

	socketBufferInit(SOBuffer);

	return ;
}




/**
 * read specific bytes of data from socket, and put them into socketBuffer
 *
 * return :
 * 0  -->  failed
 * 1  -->  succeed
 */
int socketBufferReadMessage(int sockfd, socketBuffer *SOBuffer, int byteNumber)
{
	int bytesReadOnce;			/* bytes need to be read current step */
	int ret;
	MemBlock *newBlock;

	while(byteNumber)
	{
		/* if there is no byte available, apply a new MemBlock */
		if(SOBuffer->leftByte == 0)
		{
			newBlock = socketBufferApplyNewBlock(SOBuffer);
			SOBuffer->lastByte = newBlock->buffer;
		}

		bytesReadOnce = SOBuffer->leftByte < byteNumber ? SOBuffer->leftByte : byteNumber;
		ret = readNonBlocking_n(sockfd, SOBuffer->lastByte, bytesReadOnce);
		if(ret != 1)
			return 0;
		SOBuffer->usedByte += bytesReadOnce;
		SOBuffer->leftByte -= bytesReadOnce;
		SOBuffer->lastByte += bytesReadOnce;
		byteNumber -= bytesReadOnce;
		
	}
	return 1;
}




/**
 * write all the data in socketBuffer to socket 
 *
 * return :
 * 0  -->  failed
 * 1  -->  succeed
 */
int socketBufferWriteMessage(int sockfd, socketBuffer *SOBuffer)
{
	int byteNumber;				/* bytes need to be written */
	MemBlock *currentBlock;		/* current block to write */
	int bytesWriteOnce;			/* bytes need to be written current step */
	int ret;

	byteNumber = SOBuffer->usedByte;
	currentBlock = SOBuffer->head;
	while(byteNumber)
	{
		bytesWriteOnce = byteNumber < MEMBLOCK_SIZE ? byteNumber : MEMBLOCK_SIZE;
		ret = writeNonBlocking_n(sockfd, currentBlock->buffer, bytesWriteOnce);
		if(ret != 1)
			return 0;
		byteNumber -= bytesWriteOnce;
		if(byteNumber)
			currentBlock = currentBlock->next;
	}
	return 1;
}



