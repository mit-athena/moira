# Makefile for MOIRA clients passwd/chfn/chpobox/chsh
#
#	MIT Project Athena
#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/Makefile,v 1.12 1990-03-17 17:20:44 mar Exp $
#	$Author: mar $
# 
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.
#

DESTDIR=
DEFINES = -O
CFLAGS = -I../../include ${DEFINES}
CONFDIR = ${DESTDIR}/usr/athena
BINDIR = ${DESTDIR}/bin

MOIRALIB = -L../../lib -lmoira -lgdb -lcom_err -lkrb -ldes -lhesiod

PROGS= chfn chsh chpobox

all: 	${PROGS}

chfn:	chfn.o
	cc -o chfn chfn.o ${MOIRALIB}

chpobox:	chpobox.o
	cc -o chpobox chpobox.o ${MOIRALIB}

chsh:	chsh.o
	cc -o chsh chsh.o ${MOIRALIB}

lint:
	lint *.c

clean:	
	rm -f *.o *~
	rm -f ${PROGS}

install:	${PROGS}
	install -c -s chsh ${DESTDIR}/usr/athena/chsh
	install -c -s chfn ${DESTDIR}/usr/athena/chfn
	install -c -s chpobox ${DESTDIR}/usr/athena/chpobox

depend:
	mkdep ${CFLAGS} chfn.c chpobox.c chsh.c

# DO NOT DELETE THIS LINE -- mkdep uses it.

chfn.o: chfn.c /usr/include/sys/types.h /usr/include/stdio.h
chfn.o: /usr/include/strings.h /usr/include/sys/file.h ../../include/krb.h
chfn.o: ../../include/mit-copyright.h ../../include/des.h
chfn.o: /usr/include/ctype.h /usr/include/errno.h ../../include/moira.h
chfn.o: ../../include/mr_et.h ../../include/moira_site.h
chpobox.o: chpobox.c /usr/include/sys/types.h /usr/include/stdio.h
chpobox.o: /usr/include/pwd.h /usr/include/strings.h /usr/include/ctype.h
chpobox.o: /usr/include/errno.h ../../include/moira.h ../../include/mr_et.h
chpobox.o: ../../include/moira_site.h ../../include/mit-copyright.h
chsh.o: chsh.c /usr/include/sys/types.h /usr/include/stdio.h
chsh.o: /usr/include/strings.h /usr/include/sys/file.h ../../include/krb.h
chsh.o: ../../include/mit-copyright.h ../../include/des.h
chsh.o: /usr/include/ctype.h /usr/include/errno.h ../../include/moira.h
chsh.o: ../../include/mr_et.h ../../include/moira_site.h
