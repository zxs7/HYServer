//multiThread.c

#include "multiThread.h"


/**
 * return value of top item
 */
int Pstack_top(Pstack *ps)
{
	return ps->q[ps->top - 1];
}

/**
 * delete top item
 */
void Pstack_pop(Pstack *ps)
{
	ps->top--;
}

/**
 * push a new item with value k to the top
 */
void Pstack_push(Pstack *ps, int k)
{
	ps->q[ps->top] = k;
	ps->top++;
}

/**
 * judge weather the stack is empty
 */
int Pstack_empty(Pstack *ps)
{
	if(ps->top == 0)
	return 1;
	return 0;
}

