# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrcheck/Makefile,v 1.4 1990-02-14 11:34:40 mar Exp $
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.

DESTDIR=
CFLAGS = -I../../include -O
LDFLAGS =  -L../../lib
CONFDIR = ${DESTDIR}/usr/athena
BINDIR = ${DESTDIR}/bin

SMSLIB = ${LDFLAGS} -lsms -lgdb -lcom_err
LIBS = ${SMSLIB} -lkrb -ldes -lhesiod


SRCS =	mrcheck.c
OBJS = mrcheck.o

all:	mrcheck

mrcheck: ${OBJS}
	rm -f mrcheck
	cc ${CFLAGS} -o mrcheck ${OBJS} ${LIBS}

mrcheck.o: mrcheck.c

lint:
	lint -I../../include *.c

clean:
	rm -f *.o *~
	rm -f mrcheck

install:	${PROGS}
	install -c -s mrcheck ${DESTDIR}/usr/athena/mrcheck

depend:
	mkdep ${CFLAGS} ${SRCS}

# DO NOT DELETE THIS LINE -- mkdep uses it.

mrcheck.o: mrcheck.c /usr/include/stdio.h ../../include/sms.h
mrcheck.o: /usr/include/sms_et.h ../../include/sms_app.h
mrcheck.o: ../../include/mit-copyright.h
