# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrcheck/Makefile,v 1.2 1989-01-21 16:29:30 mar Exp $
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
	install -s mrcheck ${DESTDIR}/usr/athena/mrcheck
