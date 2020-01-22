/*
 *	Storage Module for Lilith
 */

#include	"l.h"

extern	word	*stack;
extern	int	datasize;
extern	int	stacksize;
	int	heapsize;
static	int	heapindex = 0;

dispose ( index , size )
word	index;
word	size;		/* in Worten */
{
}

allocate ( index , size )
word	index;
word	size;		/* in Worten */
{

	if ( heapindex + size < heapsize ) {
		if ( size >= 2 && heapindex % 2 ) {
			heapindex &= 0xFFFFFFFE;
			heapindex += 2;
			}
		stack[index] = heapindex+datasize+stacksize;
		heapindex += size;
		}
	else
		quit("heap segment overflow");
#ifdef TRACE
	trace("ALLOCATE: stack[index] = %o\n",stack[index]);
#endif TRACE
}
