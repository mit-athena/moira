#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mailmaint/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mailmaint/Makefile,v 1.7 1990-03-17 16:55:04 mar Exp $
#
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.

# Makefile for Moira client mailmaint
#
#
DESTDIR=
CFLAGS = -I../../include -O
CONFDIR = ${DESTDIR}/usr/athena
BINDIR = ${DESTDIR}/bin

MOIRALIB = -L../../lib -lmoira -lgdb -lcom_err

SRCS =	mailmaint.c

OBJECTS = mailmaint.o

all:	mailmaint

mailmaint:	${OBJECTS}
	cc ${CFLAGS} -o mailmaint ${OBJECTS} ${MOIRALIB} \
	-lcurses -ltermcap -lkrb -ldes -lhesiod

mailmaint.o:		mailmaint.c
	cc -c ${CFLAGS} mailmaint.c

lint:
	lint -I../../include ${SRCS}

clean:	
	rm -f *.o *~ core \#*
	rm -f mailmaint

install:	${PROGS}
	install -c -s mailmaint ${DESTDIR}/usr/athena/mailmaint

depend:
	mkdep ${CFLAGS} ${SRCS}

# DO NOT DELETE THIS LINE -- mkdep uses it.

mailmaint.o: mailmaint.c /usr/include/stdio.h /usr/include/pwd.h
mailmaint.o: /usr/include/signal.h /usr/include/strings.h
mailmaint.o: /usr/include/curses.h /usr/include/sgtty.h
mailmaint.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
mailmaint.o: /usr/include/sys/ttydev.h /usr/include/sys/types.h
mailmaint.o: /usr/include/varargs.h ../../include/com_err.h
mailmaint.o: /usr/include/ctype.h ../../include/moira.h ../../include/mr_et.h
mailmaint.o: ../../include/moira_site.h ../../include/mit-copyright.h
