# Makefile for SMS clients blanche
#
#	MIT Project Athena
#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/blanche/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/blanche/Makefile,v 1.3 1990-01-10 18:30:23 mar Exp $
#	$Author: mar $
# 
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.
#

DESTDIR=
DEFINES = -O
CFLAGS = -I../../include ${DEFINES}
BINDIR = ${DESTDIR}/bin

SMSLIB = -L../../lib -lsms -lgdb -lcom_err -lkrb -ldes

PROGS= blanche

all: 	${PROGS}

blanche: blanche.o
	rm -f blanche
	cc -o blanche blanche.o ${SMSLIB}

lint:
	lint *.c

clean:	
	rm -f *.o *~
	rm -f ${PROGS}

install:	${PROGS}
	install -s blanche ${DESTDIR}/usr/athena/blanche

depend:
	mkdep ${CFLAGS} blanche.c

# DO NOT DELETE THIS LINE -- mkdep uses it.

blanche.o: blanche.c ../../include/mit-copyright.h /usr/include/stdio.h
blanche.o: /usr/include/ctype.h ../../include/sms.h /usr/include/sms_et.h
blanche.o: ../../include/sms_app.h
