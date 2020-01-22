/*
 *	Lilith - Interpreter
 */

#include	<stdio.h>
#include	<signal.h>

extern	int	stacksize;
extern	int	codesize;
extern	int	datasize;
extern	int	heapsize;

extern	int	crash();

	int	no_checks = 0;
char	usage[] = { "Usage: %s [-d datasize] [-s stacksize] [-c codesize] [-h heap] [-n] base" };

main ( argc , argv )
int	argc;
char	**argv;
{
	char	*lilith_name;

	if ( lilith_name = rindex(*argv,'/') )
		++lilith_name;
	else
		lilith_name = *argv;
	stacksize = 4; /* K */
	codesize = 20; /* K */
	datasize = 10; /* K */
	heapsize = 10; /* K */

	while ( --argc && **++argv == '-' ) switch ( argv[0][1] ) {
	case 'c' :
		if ( --argc )
			sscanf ( *++argv , "%d" , & codesize );
		else
			quit(usage,lilith_name);
		break;
	case 'd' :
		if ( --argc )
			sscanf ( *++argv , "%d" , & datasize );
		else
			quit(usage,lilith_name);
		break;
	case 'h' :
		if ( --argc )
			sscanf ( *++argv , "%d" , & heapsize );
		else
			quit(usage,lilith_name);
		break;
	case 's' :
		if ( --argc )
			sscanf ( *++argv , "%d" , &stacksize );
		else
			quit(usage,lilith_name);
		break;
	case 'n' : /* no checks */
                ++no_checks;
		break;
	default :
		quit(usage,lilith_name);
	}
	if ( argc != 1 )
		quit(usage,lilith_name);
	init ();
	load ( *argv );
	if (! signal(SIGQUIT, SIG_IGN))
		signal(SIGQUIT, crash);
	execute ();
}
