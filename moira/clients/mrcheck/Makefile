# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrcheck/Makefile,v 1.1 1988-12-01 15:00:52 mar Exp $
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.

DESTDIR=
CFLAGS = -I../../include -O
LDFLAGS =  -L../../lib
CONFDIR = ${DESTDIR}/usr/athena
BINDIR = ${DESTDIR}/bin

SMSLIB = ${LDFLAGS} -lsms -lgdb -lcom_err
LIBS = ${SMSLIB} -lcurses -ltermcap -lkrb -ldes


SRCS =	smscheck.c
OBJS = smscheck.o

all:	smscheck

smscheck: ${OBJS}
	rm -f smscheck
	cc ${CFLAGS} -o smscheck ${OBJS} ${LIBS}

smscheck.o: smscheck.c

lint:
	lint -I../../include *.c

clean:
	rm -f *.o *~
	rm -f smscheck

install:	${PROGS}
	install -s smscheck ${DESTDIR}/usr/athena/smscheck
