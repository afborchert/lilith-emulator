#include	<stdio.h>

extern	int	errno;
extern	int	sys_nerr;
extern	char	*sys_errlist[];

quit ( s , p1 , p2 , p3 , p4 , p5 , p6 )
{
	fprintf(stderr,s,p1,p2,p3,p4,p5,p6);
	fprintf(stderr," - QUIT\n");
#ifdef TRACE
	eox();
#endif TRACE
	exit(1);
}

pquit ( s )
char	*s;
{
	if ( errno < sys_nerr )
		quit("%s: %s",s,sys_errlist[errno]);
	else
		quit("%s: unknown error (code = %d)",s,errno);
}
