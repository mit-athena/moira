# Makefile for SMS clients passwd/chfn/chhome/chsh
#
#	MIT Project Athena
#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/Makefile,v 1.2 1987-07-31 01:24:43 ambar Exp $
#	$Author: ambar $
#	$Log: not supported by cvs2svn $
#
DESTDIR=
DEFINES = -O
CFLAGS = -I../../include ${DEFINES}
CONFDIR = ${DESTDIR}/usr/athena
BINDIR = ${DESTDIR}/bin

SMSLIB =  ../../lib/libsms.a ../../lib/libgdb.a ../../lib/libmisc.a

PROGS= passwd chfn chhome chsh

all: 	${PROGS}

passwd:	passwd.o
	cc -o passwd passwd.o -lkrb -ldes

chfn:	chfn.o
	cc -o chfn chfn.o ${SMSLIB} -lkrb -ldes

chhome:	chhome.o
	cc -o chhome chhome.o ${SMSLIB} -lkrb -ldes

chsh:	chsh.o
	cc -o chsh chsh.o ${SMSLIB} -lkrb -ldes

lint:
	lint *.c

clean:	
	rm -f *.o *~
	rm -f ${PROGS}

install:	${PROGS}
	install passwd ${DESTDIR}/usr/athena/passwd
	install chsh ${DESTDIR}/usr/athena/chsh
	install chfn ${DESTDIR}/usr/athena/chfn
	install chhome ${DESTDIR}/usr/athena/chhome
