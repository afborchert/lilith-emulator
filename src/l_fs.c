#ifdef LT
#define TRACE
#endif LT
/*
 *	FileSystem for Lilith
 */

#include	<stdio.h>
#include	"l.h"

#define	ER	0x8000
#define	EF	0x4000
#define	RD	0x2000
#define	WR	0x1000
#define	AG	0x0800
#define	BYTEMODE 0x0400

#define	BUFSIZE	512

extern	word	*stack;

FILE * symopen();

typedef struct {
	FILE	*fp;
	char	*md_name;
	char	*un_name;
	char	zap;
	} Info;

typedef	struct {
	word	res;

	word	bufa;
	word	ela;
	word	ina;
	word	topa;

	word	elodd;
	word	inodd;
	word	eof;
	word	flags;

	/* for UNIX */

	Info	*ptr;
	} File;

File	*al_file();

#ifdef	TRACE
fs_show (f)
File	*f;
{	char	buf[BUFSIZE+1];
	int	index;
	File	*file;

	file = al_file ( f );
	trace("fs_show : file = `%s'",file->ptr->un_name);
	if ( file->ptr->md_name ) trace("\n");
	else trace(" [ tempfile ]\n");
	if ( file->flags & RD ) trace("READ ");
	if ( file->flags & WR ) trace("WRITE ");
	if ( file->flags & BYTEMODE ) trace("BYTEMODE ");
	if ( file->res ) trace("response <> done ");
	trace("\n");
	strncpy ( buf , & stack[file->bufa] , BUFSIZE );
	for ( index = 0 ; index < BUFSIZE ; ++index )
		if ( !isvisible(buf[index]) )
			buf[index] = '?';
	buf[BUFSIZE] = '\0';
	trace("buffer :\n`%s'\n",buf);
	if ( file->topa - file->bufa > BUFSIZ/2 )
	{	trace("illegal position of file->topa\n");
		crash("s.o");
	}
	if ( file->ela < file->topa ) {
		for ( index = 0 ; index < file->elodd + (file->ela - file->bufa)*2 ; ++index )
			trace(" ");
		trace(" ^ ela\n");
		}
	if ( file->ina < file->topa ) {
		for ( index = 0 ; index < (file->ina - file->bufa)*2 + file->inodd ; ++index )
			trace(" ");
		trace(" ^ ina\n");
		}
	for ( index = 0 ; index < (file->topa-file->bufa)*2 ; ++index )
		trace(" ");
	trace(" ^ topa\n");
	fr_file(f,file);
}
#endif TRACE

fs_open ( f , name , len )
File	*f;
char	*name;
word	len;
{
	File	*file;
	char	*fn;
	static	int	index = 0;
	char	*mode;

	Link(f);
	file = al_file(f);
	if ( name == 0 ) {		/* temporary file */
		CALLOC(fn,11,sizeof(char));
		strcpy(fn,"TMP.XXXXXX");
		fn = mktemp(fn);
		mode = "a";
		}
	else {
		CALLOC(fn,len+1,sizeof(char));
		strncpy(fn,name,len);
		fn[len] = '\0';
		mode = "r";
		name = fn;
		}
	CALLOC(file->ptr,1,sizeof(Info));
	file->ptr->md_name = name;
	file->ptr->un_name = fn;
	file->ptr->zap = 0;
	if ( (file->ptr->fp = fopen(fn,mode)) == NULL )
	{	if (mode[0] == 'r' && (file->ptr->fp = symopen(fn)))
			file->ptr->zap = 1;
		else
		{	file->res = 1;
			fr_file(f,file);
			return;
		}
	}
#ifdef TRACE
	trace("fs_open: `%s' has been opened\n",fn);
#endif TRACE
	allocate ( 0 , BUFSIZE/2 );
	file->res = 0;
	file->bufa = stack[0];
	file->ela = stack[0];
	file->ina = stack[0];
	file->topa = stack[0]+BUFSIZE/2;
	file->elodd = 0;
	file->inodd = 0;
	file->eof = 0;
	file->flags = 0;
	fr_file(f,file);
}

/*
 *	for the modula-2 compiler :
 *	check for archived symbol file
 */

FILE * symopen(filename)
	char * filename;
{	char buf[64];
	extern char * libdir;

	if (strfind(filename, NULL, ".sy", NULL, NULL))
	{	sprintf(buf, "ar x SYM %s 2>/dev/null", filename);
		if (system(buf))	/* command failed ??? */
		{	sprintf(buf, "ar x %sSYM %s 2>/dev/null",
			    libdir, filename);
			if (system(buf))
				return NULL;
		}
		return fopen(filename, "r");
	}
	return NULL;
}

fs_close ( f )
File	*f;
{
	File	*file;

	Unlink(f);
	file = al_file(f);
	fs_doio ( file );	/* geg.falls Buffer leeren */
	if (file->ptr->zap)
		unlink(file->ptr->un_name);
	dispose ( file->bufa , BUFSIZE/2 );
	fclose(file->ptr->fp);
	file->res = 0;
#ifdef TRACE
	trace("fs_close(%s)\n",file->ptr->un_name);
#endif TRACE
	cfree(file->ptr->un_name);
	cfree(file->ptr);
	fr_file(f,file);
}

fs_rename ( f , name , len )
File	*f;
char	*name;
word	len;
{
	File	*file;
	char	buf[14+14+5];
	char	*ptr;

	file = al_file(f);
	if ( name ) {
		if ( len > 14 )
			len = 14;
		CALLOC(ptr,len+1,sizeof(char));
		strncpy(ptr,name,len);
		ptr[len] = '\0';
		sprintf(buf,"mv %s %s",file->ptr->un_name,ptr);
#ifdef TRACE
		trace("fs_rename: %s\n",buf);
#endif TRACE
		if ( ptr[0] )
			system(buf);
		cfree(file->ptr->un_name);
		file->ptr->un_name = ptr;
		}
	else
		file->ptr->md_name = 0;
	file->res = 0;
	fr_file(f,file);
}

fs_setread ( f )
File	*f;
{
	File	*file;
	long	pos;

	file = al_file(f);
	if ( file->flags & WR )
		fs_doio ( file );
	pos = fs_getpos(file);
	if ( (file->ptr->fp = freopen(file->ptr->un_name,"r",file->ptr->fp))
		== NULL ) {
		file->flags = 0;
		file->res = 1;
		fr_file(f,file);
		return;
		}
	file->flags |= RD;
	file->flags &= ~WR;
	file->ina = file->bufa;
#ifdef TRACE
	trace("fs_setread: file = %s, file->flags = %o\n",file->ptr->un_name,file->flags);
#endif TRACE
	fs_setpos ( file , pos );
	fs_doio ( file );
	fr_file(f,file);
}

fs_setwrite ( f )
File	*f;
{
	File	*file;
	long	pos;

	file = al_file(f);
	fs_doio ( file );
	pos = fs_getpos(file);
	if ( (file->ptr->fp = freopen(file->ptr->un_name,"a",file->ptr->fp))
		== NULL ) {
		file->flags = 0;
		file->res = 1;
		fr_file(f,file);
		return;
		}
	file->flags |= WR;
	file->flags &= ~RD;
	file->flags &= ~BYTEMODE;
	file->ela = file->bufa;
	file->res = 0;
	file->eof = 0;
#ifdef TRACE
	trace("fs_setwrite: file = %s, file->flags = %o\n",file->ptr->un_name,file->flags);
#endif TRACE
	fs_setpos(file,pos);
	fr_file(f,file);
}

fs_setopen ( f )
File	*f;
{
	File	*file;

	file = al_file(f);
	fs_doio(file);
	file->flags = 0;
	file->res = 0;
#ifdef TRACE
	trace("fs_setopen: file = %s\n",file->ptr->un_name);
#endif TRACE
	file->eof = 0;
	file->ela = file->bufa;
	file->ina = file->bufa;
	fr_file(f,file);
}

fs_doio ( f )
File	*f;
{
	File	*file;

	file = al_file(f);
#ifdef TRACE
	if ( file->ela > file->bufa+BUFSIZE/2 ) {
		trace("illegal pos of ela\n");
		file->ela = file->bufa;
		fs_show(file);
		crash("s.o.");
		}
#endif TRACE
	if ( (file->flags & RD) && file->ela >= file->ina ) {
		file->ela = file->bufa;
		file->ina = fread( &stack[file->bufa] , sizeof(char) ,
				   BUFSIZE , file->ptr->fp );
#ifdef TRACE
		trace("fs_doio: reads %o characters from file %s\n",file->ina,file->ptr->un_name);
		fflush(file->ptr->fp);
#endif TRACE
		if ( file->ina == 0 ) {
			file->eof = 1;
			file->flags = 0;
			}
		else
			file->eof = 0;
		file->inodd = file->ina % 2;
		file->ina >>= 1;
		file->ina += file->bufa;
		file->res = 0;
		}
	else if ( file->flags & WR ) {
		if ( (file->ela - file->bufa)*2 + file->elodd > BUFSIZE )
			crash("illegal position of file->ela");
		if ( fwrite( &stack[file->bufa] , sizeof(char) ,
			     (file->ela - file->bufa)*2 + file->elodd , file->ptr->fp )
		     != (file->ela - file->bufa)*2 + file->elodd )
			file->res = 1;
		else {
#ifdef TRACE
		trace("fs_doio: writes %o characters to file %s\n",(file->ela - file->bufa)*2 + file->elodd ,file->ptr->un_name);
		fflush(file->ptr->fp);
#endif TRACE
			file->ela = file->bufa;
			file->res = 0;
			}
		}
	else
		file->res = 1;
	fr_file(f,file);
}

fs_setpos ( f , pos )
File	*f;
long	pos;
{
	File	*file;
	word	odd;

	file = al_file(f);
	if ( file->flags & WR )
		fs_doio(file);
	odd = pos % 2;
	pos >>= 1; pos <<= 1;
	if ( fseek(file->ptr->fp,pos, 0 ) ) {
		file->res = 1;
		file->eof = 1;
		}
	else {
		file->ela = file->ina = file->bufa;
		fs_doio ( file );
		file->res = 0;
		file->elodd = odd;
		}
#ifdef TRACE
	trace("fs_setpos: on file %s to position %o\n",file->ptr->un_name,pos+odd);
#endif TRACE
	fr_file(f,file);
}

long
fs_getpos ( f )
File	*f;
{
	File	*file;
	long	val;

	file = al_file(f);
	file->res = 0;
#ifdef TRACE
	trace("fs_getpos: file = %s, pos = %o\n",file->ptr->un_name,ftell(file->ptr->fp) + (file->ela - file->bufa)*2 + file->elodd );
#endif TRACE
	val = ftell(file->ptr->fp) + (file->ela - file->bufa)*2 + file->elodd ;
	fr_file(f,file);
	return val;
}

fs_reset ( f )
File	*f;
{
	File	*file;


	file = al_file(f);
#ifdef TRACE
	trace("fs_reset: file = %s\n",file->ptr->un_name);
#endif TRACE
	file->eof = 0;
	fs_setpos ( file , 0L );
	fr_file(f,file);
}

static
File	*
al_file ( file )
File	*file;
{
	File	*ptr;
	char	*p1,*p2;
	int	index;

	if (file % 4)
	{	CALLOC(ptr,1,sizeof(File));
		p1 = (char *) file;
		p2 = (char *) ptr;
		for ( index = 0 ; index < sizeof(File) ; ++index )
			*p2++ = *p1++;
		return(ptr);
	}
	else
		return file;
}

static
fr_file ( file , adr )
File	*file,*adr;
{
	int	index;
	char	*p1,*p2;

	if (file % 4)
	{	p1 = (char *) file;
		p2 = (char *) adr;
		for ( index = 0 ; index < sizeof(File) ; ++index )
			*p1++ = *p2++;
		cfree(adr);
	}
}

/*
 *	in case of an error close all files for better debugging
 */

struct chain {
	File	*c_file;
	struct chain * c_link;
};

struct chain * Chain = NULL;
int nounlink = 0;

Link(f)
	File * f;
{	struct chain * new;

	if ((new = calloc(sizeof(struct chain), 1)) == NULL)
		return;
	new->c_file = f;
	new->c_link = Chain;
	Chain = new;
}

Unlink(f)
	File * f;
{	struct chain * old, * ptr;

	if (nounlink)
		return;
	old = NULL;
	for (ptr = Chain; ptr && ptr->c_file != f; ptr = ptr->c_link)
		old = ptr;
	if (ptr->c_file == f)
	{	if (old)
			old->c_link = ptr->c_link;
		else
			Chain = ptr->c_link;
		cfree(ptr);
	}
}

CloseAll()
{	struct chain * ptr;

	++nounlink;
	for (ptr = Chain; ptr; ptr = ptr->c_link)
		fs_close(ptr->c_file);
}
