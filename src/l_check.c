#ifdef	LT
# define TRACE
#endif
#ifdef	TRACE

/*
 *	bei TRACE wird folgende Funktion nach jedem Befehl
 *	ausgefuehrt
 */
#include	<stdio.h>
#include	<assert.h>
#include	"l.h"


extern	word	*stack;
extern	word	G;
extern	word	PC;
extern	word	L;

check ()
{
	if ( PC == 03651 ) {
		trace("NextCh : ch = `%c' , (%o)\n",stack[G+7],stack[G+7]);
		fs_show ( &stack[041764] );
		}
}

#endif TRACE
