# 	This is the Makefile for allmaint, the SMS client that allows
#       a user to maintaint most important parts of the SMS database.
# 	It Contains: The Makefile
# 	
# 	Created: 	5/9/88
# 	By:		Chris D. Peterson
# 
#       $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/Makefile,v $
#       $Author: mar $
#       $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/Makefile,v 1.5 1988-10-05 12:59:33 mar Exp $
# 	
#   	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
# 
# 	For further information on copyright and distribution 
# 	see the file mit-copyright.h
# 

DESTDIR=
CFLAGS = -I../../include -O
LDFLAGS =  -L../../lib
CONFDIR = ${DESTDIR}/usr/athena
BINDIR = ${DESTDIR}/bin

SMSLIB = -lsms -lgdb -lcom_err
LIBS = ${SMSLIB} -lcurses -ltermcap -lkrb -ldes


SRCS = utils.c attach.c cluster.c delete.c globals.c lists.c main.c \
	menus.c nfs.c pobox.c quota.c user.c dcmmaint.c printer.c misc.c
OBJS = attach.o cluster.o delete.o globals.o lists.o main.o menus.o nfs.o \
       pobox.o quota.o user.o utils.o dcmmaint.o printer.o misc.o

all:		sms

sms:	 	${OBJS}
		rm -f sms
		cc ${CFLAGS} ${LDFLAGS} -o sms ${OBJS} ${LIBS}

lint:
	lint -I../../include *.c

install: sms
	install -s sms ${DESTDIR}/usr/athena/sms
	rm -f ${DESTDIR}/usr/athena/usermaint
	rm -f ${DESTDIR}/usr/athena/listmaint
	ln -s /usr/athena/sms ${DESTDIR}/usr/athena/usermaint
	ln -s /usr/athena/sms ${DESTDIR}/usr/athena/listmaint

clean:	
	rm -f *.o *~
	rm -f sms


# Typing 'make saber' at the command line will allow load and link against the
# correct libraries when using saber C.

saber: ;
	/mit/kaufer/saber -I/mit/smsdev/include -DDEBUG ${LDFLAGS} \
	${SRCS} ${LIBS}
