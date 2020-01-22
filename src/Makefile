CC= cc
CFLAGS=
OBJS=	l_check.o l_end.o l_exec.o l_fs.o l_in.o l_init.o l_load.o l_main.o l_mnem.o l_quit.o l_storage.o l_svc.o l_trace.o

all:	lilith

lilith:	${OBJS}
	${CC} -o lilith ${OBJS}

clean:
	rm -f ${OBJS}
