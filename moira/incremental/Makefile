# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/incremental/Makefile,v 1.3 1990-03-19 19:39:31 mar Exp $

SRCS = afs.c ksrvtgt.c
CFLAGS = -I../include -O
LIBS = -L../lib -lmoira -lcom_err -lzephyr -lkrb -ldes


all: afs.incr ksrvtgt

afs.incr: afs.o
	cc -o afs.incr afs.o ${LIBS}

ksrvtgt: ksrvtgt.o
	cc -o ksrvtgt ksrvtgt.o ${LIBS}

clean:
	rm -f *.o
	rm -f afs.incr ksrvtgt

install: all
	install -c afs.incr ../bin

depend:
	mkdep ${CFLAGS} ${SRCS}

# DO NOT DELETE THIS LINE -- mkdep uses it.

afs.o: afs.c ../include/moira.h ../include/mr_et.h ../include/moira_site.h
afs.o: ../include/mit-copyright.h /usr/include/sys/file.h
ksrvtgt.o: ksrvtgt.c /usr/include/stdio.h /usr/include/sys/param.h
ksrvtgt.o: /usr/include/machine/machparam.h /usr/include/sys/signal.h
ksrvtgt.o: /usr/include/sys/types.h ../include/krb.h
ksrvtgt.o: ../include/mit-copyright.h ../include/des.h
