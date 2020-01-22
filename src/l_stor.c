/*
 *	Storage Module for Lilith
 */

#include	"l.h"

extern	word	*stack;
	int	heapsize;

typedef	struct free {
	word	next;
	word	size;
	} freenode;

freenode first = { 0 , 0 };

dispose ( index , size )
word	index;		/* -> auf freie Flaeche */
word	size;		/* in Worten */
{
	freenode *ptr;

	if ( size % 2 )
		--size;
	if ( size >= sizeof(freenode)/sizeof(word) ) {
		ptr = (freenode *) &stack[index];
		ptr->next = first.next;
		ptr->size = size;
		first.next = index;
		}
}

allocate ( index , size )
word	index;		/* hier wird die Adr. der angelegten Flaeche abgel. */
word	size;		/* in Worten */
{
	word	ptr;
	freenode *pre;

	if ( size % 2 )
		++size;
	pre = &first;
	for ( ptr = first.next ; ptr && stack[ptr+1] < size ; ptr = stack[ptr] )
		pre = (freenode *) &stack[ptr];
	if ( ptr ) {
		if ( stack[ptr+1] - size >= sizeof(freenode)/sizeof(word) ) {
			stack[ptr+1] -= size;
			stack[index] = ptr + stack[ptr+1];
			}
		else {
			pre->next = stack[ptr];
			stack[index] = ptr;
			}
		}
	else
		quit("heap segment overflow");
#ifdef TRACE
	trace("ALLOCATE: stack[index] = %o\n",stack[index]);
#endif TRACE
}
