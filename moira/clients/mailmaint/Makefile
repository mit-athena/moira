#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mailmaint/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mailmaint/Makefile,v 1.1 1987-08-22 22:31:24 wesommer Exp $
#

# Makefile for SMS client mailmaint
#
#
DESTDIR=
CFLAGS = -I../../include -O
CONFDIR = ${DESTDIR}/usr/athena
BINDIR = ${DESTDIR}/bin

SMSLIB =  ../../lib/libsms.a ../../lib/libgdb.a ../../lib/libcom_err.a

SRCS =	mailmaint.c

OBJECTS = mailmaint.o

all:	mailmaint

mailmaint:	${OBJECTS}
	cc ${CFLAGS} -o mailmaint ${OBJECTS} ${SMSLIB} \
	-lcurses -ltermcap -lkrb -ldes

mailmaint.o:		mailmaint.c
	cc -c ${CFLAGS} mailmaint.c

lint:
	lint -I../../include ${SRCS}

clean:	
	rm -f *.o *~ core \#*
	rm -f mailmaint

install:	${PROGS}
	install mailmaint ${DESTDIR}/usr/athena/mailmaint


