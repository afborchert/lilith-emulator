#ifdef	LT
# define TRACE
#endif  LT
/*
 *	Lilith
 */

#include	<stdio.h>
#include	"l.h"

extern	word	*stack;
extern	int	stacksize;
extern	char	*code;
extern	int	codesize;
extern	int	datasize;
extern	int	S;

int	tocall;		/* als Information fuer den SVC 40 */

int	baseptr;	/* -> auf globale Datenflaeche nach der Base */
int	topptr;		/* -> auf das aktuelle Ende der globalen Fl. */
int	codeptr;	/* -> auf das aktuelle Ende der Codeflaeche */
int	cbptr;		/* -> auf das Ende des Codes nach der Base */

extern	unsigned int	sy;
extern	int	fs;

int	boot;

char	*libdir = "/usr/spez/diplom/borchert/lib/";

load ( filename )
char	*filename;
{
	FILE	*fp;
	unsigned int n;
	int	main_mod;
	int	main_over = FALSE;

	if ( (fp = fopen(filename,"r")) == NULL ) {
		char *ptr;

		if ( (ptr = malloc(strlen(libdir)+strlen(filename)+1)) == NULL )
			pquit("malloc");
		strcpy(ptr, libdir);
		strcat(ptr, filename);
		if ( (fp = fopen(ptr,"r")) == NULL )
		        pquit(filename);
		free(ptr);
		}
	initincode(fp);
	getsy();
	main_mod = boot;
	if ( boot ) {
		topptr = 0200;
		codeptr = 0;
		if ( sy != xBASE )
			quit("%s: Bad format",filename);
		skip();
		getsy();
		stack[4] = S;
		stack[S] = topptr;
		stack[S+1] = 0; /* dynamic link */
		/* stack[S+2] = PC */
		stack[S+3] = 0; /* M */
		stack[S+4] = S+7;
		stack[S+5] = 0; /* H + 24 */
		stack[S+6] = 0;
		stack[S+7] = 0;
		stack[DFT] = -1; /* module 0 darf nicht aufgerufen werden */
		}
	else {
		codeptr = cbptr;
		topptr = baseptr;
		main_over = TRUE;
		}
	while ( sy == xMODULE ) {
		if ( codeptr % 4 != 0 ) {	/* 4-er Kante ? */
			codeptr &= 0xFFFFFFFC;
			codeptr += 4;
			}
		getnum ( &n );	/* ModuleNumber */
		if ( main_over ) {
			main_over = FALSE;
			tocall = n;  /* fuer SVC 40 */
			}
#ifdef TRACE
		trace("load: module %o: code at %o, data at %o\n",n,codeptr,topptr);
#endif TRACE
		if ( n > 0200 - DFT )
			quit("illegal module number : %o",n);
		stack[DFT+n] = topptr;
		stack[topptr++] = codeptr/4;
		getsy(); /* xDATATEXT */
		getnum ( &n );	/* 0 */
		while ( fs ) {
			getnum ( &n );
			if ( topptr >= datasize-1 )
				quit("data segment overflow");
			stack[topptr++] = (word) n;
			}
		if ( main_mod ) {
			main_mod = FALSE;
			stack[S+2] = codeptr;	/* Startaddresse */
			}
		getsy(); /* xCODETEXT */
		while ( fs ) {
			getnum ( &n );
			if ( codeptr >= codesize-2 )
				quit("code segment overflow");
			code[codeptr++] = n / 0x100;
			code[codeptr++] = n % 0x100;
			}
		getsy();
		}
	if ( boot ) {
		boot = FALSE;
		cbptr = codeptr;
		baseptr = topptr;
		}
	fclose(fp);
}
