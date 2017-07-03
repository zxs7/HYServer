//RPC.c

#include "RPC.h"

/**
 * read operand from expression
 *
 * return the operand
 */
int readOperand(char** expression)
{
	int operand = 0;
	int head = 0;
	int len = strlen(*expression);

	while(head < len && ((*expression)[head] >= '0' && (*expression)[head] <= '9'))
	{
		operand *= 10;
		operand += (*expression)[head] - '0';
		head ++;
	}

	(*expression) = (*expression) + head;

	return operand;
}




/**
 * read operator from expression
 *
 * return the operator
 */
char readOperator(char** expression)
{
	char operator;
	int head = 0;
	int len = strlen(*expression);

	while(head < len && (*expression)[head] == ' ')
		head ++;

	operator = (*expression)[head];
	head++;

	while(head < len && (*expression)[head] == ' ')
		head ++;

	(*expression) = (*expression) + head;
	return operator;
}




/**
 * RPC
 *
 * return the result
 */
char *RPC(char *expression)
{
	int operand1;
	int operand2;
	char operator;
	int result;
	char ret[100];
	char *retMessage;

	operand1 = readOperand(&expression);
	operator = readOperator(&expression);
	operand2 = readOperand(&expression);

	switch(operator)
	{
		case '+' :
			result = operand1 + operand2;
			sprintf(ret, "The result is : %d", result);
			break;

		case '-' :
			result = operand1 - operand2;
			sprintf(ret, "The result is : %d", result);
			break;

		case '*' :
			result = operand1 * operand2;
			sprintf(ret, "The result is : %d", result);
			break;

		case '/' :
			if(operand2 == 0)
			{
				sprintf(ret, "Error : divided by 0!");
				break;
			}
			result = operand1 / operand2;
			sprintf(ret, "The result is : %d", result);
			break;

		default :
			sprintf(ret, "Undefined operation!");
			break;
	}

	int len = strlen(ret);
	retMessage = (char *)malloc(sizeof(char) * len);
	strcpy(retMessage, ret);

	return retMessage;
}


