# Makefile for SMS clients passwd/chfn/chpobox/chsh
#
#	MIT Project Athena
#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/Makefile,v 1.4 1987-08-03 05:13:22 wesommer Exp $
#	$Author: wesommer $
#	$Log: not supported by cvs2svn $
# Revision 1.3  87/08/02  19:46:26  ambar
# changed name of chhome to chpobox
# 
# Revision 1.2  87/07/31  01:24:43  ambar
# added install target
# 
#
DESTDIR=
DEFINES = -O
CFLAGS = -I../../include ${DEFINES}
CONFDIR = ${DESTDIR}/usr/athena
BINDIR = ${DESTDIR}/bin

SMSLIB =  ../../lib/libsms.a ../../lib/libgdb.a ../../lib/libcom_err.a

PROGS= passwd chfn chpobox chsh

all: 	${PROGS}

passwd:	passwd.o
	cc -o passwd passwd.o -lkrb -ldes

chfn:	chfn.o
	cc -o chfn chfn.o ${SMSLIB} -lkrb -ldes

chpobox:	chpobox.o
	cc -o chpobox chpobox.o ${SMSLIB} -lkrb -ldes

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
	install chpobox ${DESTDIR}/usr/athena/chpobox
