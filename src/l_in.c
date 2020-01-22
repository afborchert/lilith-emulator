#include	<stdio.h>
#include	"l.h"

unsigned	int	sy;
int	fs;

static	FILE	*in;
static	unsigned int	nextn;

initincode ( fp )
FILE	*fp;
{
	in = fp;
	readword(&nextn);
	fs = 0;
}

readword ( n )
unsigned	int	*n;
{
	word	x;

	if ( ! fread ( &x , sizeof(word) , 1 , in ) ) {
		sy = EOFSY;
		fs = 0;
		*n = 0;
		}
	else
		*n = (unsigned	int) x;
}

getnum ( n )
unsigned	int	*n;
{
	if ( fs == 0 )
		quit("frame size error");
	else {
		--fs;
		if ( feof(in) ) {
			*n = 0;
			if ( fs == 0 )
				quit("frame size error");
			}
		else {
			*n = nextn;
			readword(&nextn);
			}
		}
}

skip ()
{
	unsigned	int	n;

	while ( fs > 0 )
		getnum ( &n );
}

getsy ()
{
	unsigned	int	n;

	if ( fs > 0 ) {
		quit("frame size error");
		skip();
		}
	if ( feof(in) )
		sy = EOFSY;
	else {
		fs = 1;
		getnum ( &n );
		if ( 0200 <= n && n <= 0305 ) {
			sy = n;
			fs = 1;
			getnum ( &fs );
			}
		else {
			sy = EOFSY;
			quit("illegal symbol read: %o",n);
			}
		}
}
