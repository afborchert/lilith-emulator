#ifdef LT
#define TRACE
#endif LT
/*
 *	Super Visor Calls
 */

#include	<stdio.h>
#include	<assert.h>
#include	"l.h"

extern	word	*stack;
extern	word	L;
extern	word	G;
extern	word	PC;
extern	word	F;
extern	word	pop();
extern	int	tocall;

svc ( n )
word	n;
{
	static	int	readagain = FALSE;
	static	int	ch;
	char	filename[15];
	word	stat;
	register word	len;
	char	*ptr;
	register word	index;
	register word	index2;
	word	new;
	word	lowpos,highpos;
	long	pos;

	switch ( n ) {
	case  1 :	/*  PROCEDURE Create(VAR f:File;
					     mediumname: ARRAY OF CHAR); */
		len = pop() + 1;
		index2 = pop();
		index = pop();
#ifdef TRACE
		trace("Create(%o,\"%s\")\n",index,&stack[index2]);
#endif TRACE
		fs_open ( &stack[index] , NULL );
		break;
	case  2 :	/*  PROCEDURE Close(VAR f:File); */
		index = pop();
#ifdef TRACE
		trace("Close(%o)\n",index);
#endif TRACE
		fs_close ( &stack[index] );
		break;
	case  3 :	/* PROCEDURE Lookup(VAR f:File;
					    filename:ARRAY OF CHAR;
					    new: BOOLEAN); */
		new = pop();
		len = pop() + 1;
		index2 = pop();
		index = pop();
#ifdef TRACE
		trace("Lookup(%o,\"%s\",%o)\n",index,&stack[index2],new);
#endif TRACE
		fs_open ( &stack[index] , &stack[index2] , len );
		break;
	case  4 :	/* PROCEDURE Rename(VAR f:File;
					    filename:ARRAY OF CHAR); */
		len = pop() + 1;
		index2 = pop();
		index = pop();
#ifdef TRACE
		trace("Rename(%o,\"%s\")\n",index,&stack[index2]);
#endif
		fs_rename ( &stack[index] , &stack[index2] , len);
		break;
	case 5 :	/* PROCEDURE SetRead(VAR f:File); */
		index = pop();
#ifdef TRACE
		trace("SetRead(%o)\n",index);
#endif
		fs_setread ( &stack[index] );
		break;
	case 6 :	/* PROCEDURE SetWrite(VAR f:File); */
		index = pop();
#ifdef TRACE
		trace("SetWrite(%o)\n",index);
#endif
		fs_setwrite ( &stack[index] );
		break;
	case 8 :	/* PROCEDURE SetOpen(VAR f:File); */
		index = pop();
#ifdef TRACE
		trace("SetOpen(%o)\n",index);
#endif
		fs_setopen ( &stack[index] );
		break;
	case 9 :	/* PROCEDURE Doio(VAR f:File); */
		index = pop();
#ifdef TRACE
		trace("Doio(%o)\n",index);
#endif
		fs_doio ( &stack[index] );
		break;
	case 10 :	/* PROCEDURE SetPos(VAR f:File;
					    highpos, lowpos: CARDINAL); */
		lowpos = pop();
		highpos = pop();
		index = pop();
#ifdef TRACE
		trace("SetPos(%o,%o)\n",index,highpos*0x10000+lowpos);
#endif TRACE
		fs_setpos ( &stack[index] , highpos*0x10000+lowpos);
		break;
	case 11 :	/*  PROCEDURE GetPos(VAR f:File;
					     VAR highpos, lowpos: CARDINAL); */
		lowpos = pop();
		highpos = pop();
		index = pop();
		pos = fs_getpos ( &stack[index] );
		stack[highpos] = (word) pos / 0x10000;
		stack[lowpos] = (word) pos % 0x10000;
		break;
	case 13 :	/*  PROCEDURE Reset(VAR f:File); */
		index = pop();
		fs_reset ( &stack[index] );
		break;
	case 30 :	/*  PROCEDURE GetTime (VAR time: Time); */
		index = pop();
		stack[index] = 0;
		stack[index+1] = 0;
		stack[index+2] = 0;
		break;
	case 40 :	/*  PROCEDURE Call (filename: ARRAY OF CHAR;
					    flag: BOOLEAN;
					    VAR stat: Status); */
		stat = pop();
		stack[stat] = 0;   /* normal */
		pop();       /* flag wird ignoriert */
		len = pop() + 1;
		if ( len > 14 )
			len = 14;
		index = pop();
		ptr = (char *) &stack[index];
		strncpy ( filename , ptr , len );
		filename[len] = '\0';
#ifdef TRACE
		trace("\nCall(%s)\n",filename);
#endif
		assert( filename[0] );
		load(filename);

		/*
		 * jetzt ein "CX tocall 0" simulieren
		 */
		mark(G,TRUE);
		G = stack[DFT+tocall];
		F = stack[G];
		PC = 0;
		PC = next2();
		break;
	case 50 :	/* PROCEDURE ALLOCATE(VAR a: ADDRESS; n: CARDINAL); */
		len = pop();
		index = pop();
#ifdef TRACE
		trace("ALLOCATE(%o,%o)\n",index,len);
#endif TRACE
		allocate ( index , len );
		break;
	case 51 :	/* PROCEDURE DEALLOCATE(VAR a: ADDRESS;
						n: CARDINAL); */
		len = pop();
		index = pop();
#ifdef TRACE
		trace("DEALLOCATE(%o,%o)\n",index,len);
#endif TRACE
		dispose ( stack[index] , len );
		stack[index] = 0xFFFF; /* NIL - Pointer */
		break;
	case 60 :	/* PROCEDURE Read(VAR ch: CHAR); */
		index = pop();
		if ( readagain )
			readagain = FALSE;
		else
			ch = getchar();
#ifdef TRACE
		trace("Read: liefert `%c' (%o) bei stack[%o] zurueck\n",ch,ch,index);
#endif TRACE
		stack[index] = (word) ch;
		break;
	case 62 :	/* PROCEDURE ReadAgain; */
		readagain = TRUE;
#ifdef TRACE
		trace("ReadAgain called\n");
#endif TRACE
		break;
	case 63 :	/* PROCEDURE Write(ch: CHAR); */
		index = pop();
		putchar(index);
#ifdef TRACE
		trace("Write ( '%c' )\n",index);
#endif TRACE
		break;
	case 64 :	/* PROCEDURE WriteLn; */
		putchar('\n');
#ifdef TRACE
		trace("WriteLn\n");
#endif TRACE
		break;
	case 65 :	/* PROCEDURE WriteString(s: ARRAY OF CHAR); */
		len = pop() + 1;
		index = pop();
		ptr = (char *) &stack[index];
#ifdef TRACE
		trace("WriteString(\"");
#endif TRACE
		while ( len && *ptr ) {
#ifdef TRACE
			trace("%c",*ptr);
#endif TRACE
			putchar(*ptr++);
			--len;
			}
#ifdef TRACE
		trace("\")\n");
#endif TRACE
		break;
	default :
		quit("\nSVC %o not implemented !",n);
	}
}
