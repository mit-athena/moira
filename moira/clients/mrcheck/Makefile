# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrcheck/Makefile,v 1.5 1990-03-17 17:16:09 mar Exp $
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.

DESTDIR=
CFLAGS = -I../../include -O
LDFLAGS =  -L../../lib
CONFDIR = ${DESTDIR}/usr/athena
BINDIR = ${DESTDIR}/bin

MOIRALIB = ${LDFLAGS} -lmoira -lgdb -lcom_err
LIBS = ${MOIRALIB} -lkrb -ldes -lhesiod


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

mrcheck.o: mrcheck.c /usr/include/stdio.h ../../include/moira.h
mrcheck.o: ../../include/mr_et.h ../../include/moira_site.h
mrcheck.o: ../../include/mit-copyright.h
