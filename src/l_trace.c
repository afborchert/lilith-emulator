#ifdef LT
# define TRACE
#endif LT

#ifdef TRACE
#include	<stdio.h>

trace(s,p1,p2,p3,p4,p5,p6)
{
	fprintf(stderr,s,p1,p2,p3,p4,p5,p6);
}

#endif TRACE
