# Makefile for Moira clients blanche
#
#	MIT Project Athena
#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/blanche/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/blanche/Makefile,v 1.7 1990-03-17 16:54:51 mar Exp $
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

MOIRALIB = -L../../lib -lmoira -lgdb -lcom_err -lkrb -ldes -lhesiod

PROGS= blanche

all: 	${PROGS}

blanche: blanche.o
	rm -f blanche
	cc -o blanche blanche.o ${MOIRALIB}

lint:
	lint *.c

clean:	
	rm -f *.o *~
	rm -f ${PROGS}

install:	${PROGS}
	install -c -s blanche ${DESTDIR}/usr/athena/blanche

depend:
	mkdep ${CFLAGS} blanche.c

# DO NOT DELETE THIS LINE -- mkdep uses it.

blanche.o: blanche.c ../../include/mit-copyright.h /usr/include/stdio.h
blanche.o: /usr/include/ctype.h ../../include/moira.h ../../include/mr_et.h
blanche.o: ../../include/moira_site.h
