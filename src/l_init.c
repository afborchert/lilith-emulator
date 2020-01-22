/*
 *	Lilith - Interpreter
 */

#include	<stdio.h>
#include	"l.h"

extern	int	stacksize;
extern	int	codesize;
extern	int	datasize;
extern	int	heapsize;
extern	word	*stack;
extern	char	*code;
extern	int	boot;
extern	int	S;

init ()
{
	stacksize *= 512;
	codesize *= 1024;
	datasize *= 512;
	heapsize *= 512;

	boot = TRUE;
	CALLOC(stack,sizeof(word),stacksize+datasize+heapsize);
	CALLOC(code,sizeof(char),codesize);
	S = datasize;
	dispose(stacksize+datasize,heapsize);
}
