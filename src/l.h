/*
 *	Lilith
 *
 *	(c) Andreas Borchert, 1983
 */

#define	TRUE	1
#define	FALSE	0

typedef	unsigned short word;

/* FrameTypes bei loadfiles */

#define	EOFSY	0
#define	xBASE  	  0300
#define	xDATATEXT 0301
#define	xCODETEXT 0302
#define	xMODULE	  0303

#define	CALLOC(ptr,n,s)	{ if ( (ptr = calloc(n,s)) == NULL )\
				quit("No space available"); }

#define	DFT	040
#define	TLC	016

#define	isvisible(ch)	((ch)>=' '&&(ch)<='~')
