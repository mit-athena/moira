# Makefile for SMS clients passwd/chfn/chpobox/chsh
#
#	MIT Project Athena
#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/Makefile,v 1.8 1988-08-07 21:33:08 mar Exp $
#	$Author: mar $
# 
#
DESTDIR=
DEFINES = -O
CFLAGS = -I../../include ${DEFINES}
CONFDIR = ${DESTDIR}/usr/athena
BINDIR = ${DESTDIR}/bin

SMSLIB = -L../../lib -lsms -lgdb -lcom_err -lkrb -ldes

PROGS= chfn chsh # chpobox

all: 	${PROGS}

chfn:	chfn.o
	cc -o chfn chfn.o ${SMSLIB}

chpobox:	chpobox.o
	cc -o chpobox chpobox.o ${SMSLIB}

chsh:	chsh.o
	cc -o chsh chsh.o ${SMSLIB}

lint:
	lint *.c

clean:	
	rm -f *.o *~
	rm -f ${PROGS}

install:	${PROGS}
	install -s chsh ${DESTDIR}/usr/athena/chsh
	install -s chfn ${DESTDIR}/usr/athena/chfn
	install -s chpobox ${DESTDIR}/usr/athena/chpobox
