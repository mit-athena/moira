# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/incremental/Makefile,v 1.2 1989-11-29 14:58:18 mar Exp $

SRCS = afs.c ksrvtgt.c
CFLAGS = -I../include -O
LIBS = -L../lib -lsms -lcom_err -lzephyr -lkrb -ldes


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
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.

afs.o: afs.c ../include/sms.h /usr/include/sms_et.h ../include/sms_app.h
afs.o: ../include/mit-copyright.h /usr/include/sys/file.h
ksrvtgt.o: ksrvtgt.c /usr/include/stdio.h /usr/include/sys/param.h
ksrvtgt.o: /usr/include/machine/machparam.h /usr/include/signal.h
ksrvtgt.o: /usr/include/sys/types.h ../include/krb.h ../include/mit-copyright.h
ksrvtgt.o: ../include/des.h ../include/mit-copyright.h

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY
