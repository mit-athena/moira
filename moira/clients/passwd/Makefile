# Makefile for SMS clients passwd/chfn/chpobox/chsh
#
#	MIT Project Athena
#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/Makefile,v 1.10 1990-01-10 18:29:01 mar Exp $
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

SMSLIB = -L../../lib -lsms -lgdb -lcom_err -lkrb -ldes

PROGS= chfn chsh chpobox

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

depend:
	mkdep ${CFLAGS} chfn.c chpobox.c chsh.c

# DO NOT DELETE THIS LINE -- mkdep uses it.

chfn.o: chfn.c /usr/include/sys/types.h /usr/include/stdio.h
chfn.o: /usr/include/strings.h /usr/include/sys/file.h ../../include/krb.h
chfn.o: ../../include/mit-copyright.h ../../include/des.h
chfn.o: /usr/include/ctype.h /usr/include/errno.h ../../include/sms.h
chfn.o: /usr/include/sms_et.h ../../include/sms_app.h
chpobox.o: chpobox.c /usr/include/sys/types.h /usr/include/stdio.h
chpobox.o: /usr/include/pwd.h /usr/include/strings.h /usr/include/ctype.h
chpobox.o: /usr/include/errno.h ../../include/sms.h /usr/include/sms_et.h
chpobox.o: ../../include/sms_app.h ../../include/mit-copyright.h
chsh.o: chsh.c /usr/include/sys/types.h /usr/include/stdio.h
chsh.o: /usr/include/strings.h /usr/include/sys/file.h ../../include/krb.h
chsh.o: ../../include/mit-copyright.h ../../include/des.h
chsh.o: /usr/include/ctype.h /usr/include/errno.h ../../include/sms.h
chsh.o: /usr/include/sms_et.h ../../include/sms_app.h
