# Makefile for SMS clients blanche
#
#	MIT Project Athena
#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/blanche/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/blanche/Makefile,v 1.2 1988-09-12 17:32:01 mar Exp $
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
