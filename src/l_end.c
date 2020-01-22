#ifdef LT
# define TRACE
#endif LT
#ifdef TRACE
#include	<stdio.h>
#include	"l.h"

extern	word	*stack;
extern	int	stacksize;
extern	int	datasize;
extern	int	heapsize;

eox ()
{
	int	adr;
	int	ch;
	int	cmd = 'o';
	int	index;

	printf("\n\nEnd of execution....\n");
	do {
		if ( scanf("%o",&adr) )
			index = adr;
		while ( (ch = getchar()) == ' ' )
			;
		if ( ch != '\n' )
			cmd = ch;
		if ( index >= stacksize+heapsize+datasize )
			index = 0;
		else if ( index < 0 )
			index = 0;
		switch ( cmd ) {
		case 'o' :	printf("%5o %5o ",index++,stack[index]); break;
		case 's' :	printf("%5o `%s' ",index,&stack[index]);
				while ( stack[index] && index < stacksize+datasize+heapsize )
					++index;
				++index;
				break;
		case 'c' :	printf("%5o `%c' `%c' %3o %3o ",
					index,
					stack[index] / 0x100,
					stack[index] % 0x100,
					stack[index] / 0x100,
					stack[index] % 0x100 );
				++index;
				break;
		case 'x' :	exit(0);
		default  :	printf("??? "); cmd = 'o';
				break;
		}
		}
	while ( 1 );
}

#endif TRACE
