#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mailmaint/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mailmaint/Makefile,v 1.2 1988-08-04 13:31:47 mar Exp $
#

# Makefile for SMS client mailmaint
#
#
DESTDIR=
CFLAGS = -I../../include -O
CONFDIR = ${DESTDIR}/usr/athena
BINDIR = ${DESTDIR}/bin

SMSLIB = -L../../lib -lsms -lgdb -lcom_err

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


