# 	This is the Makefile for allmaint, the SMS client that allows
#       a user to maintaint most important parts of the SMS database.
# 	It Contains: The Makefile
# 	
# 	Created: 	5/9/88
# 	By:		Chris D. Peterson
# 
#       $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/Makefile,v $
#       $Author: mar $
#       $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/Makefile,v 1.11 1990-02-14 12:25:34 mar Exp $
# 	
#   	Copyright 1987, 1988 by the Massachusetts Institute of Technology.
# 
# 	For further information on copyright and distribution 
# 	see the file mit-copyright.h
# 

DESTDIR=
CFLAGS = -I../../include -I. -O
LDFLAGS =  -L../../lib
CONFDIR = ${DESTDIR}/usr/athena
BINDIR = ${DESTDIR}/bin

SMSLIB = -lsms -lgdb -lcom_err
LIBS = ${SMSLIB} -lcurses -ltermcap -lkrb -ldes -lhesiod


SRCS = utils.c attach.c cluster.c delete.c globals.c lists.c main.c \
	menu.c menus.c nfs.c pobox.c quota.c user.c dcmmaint.c printer.c \
	misc.c
OBJS = attach.o cluster.o delete.o globals.o lists.o main.o menu.o \
	menus.o nfs.o pobox.o quota.o user.o utils.o dcmmaint.o printer.o \
	misc.o

all:		moira

moira:	 	${OBJS}
		rm -f moira
		cc ${CFLAGS} ${LDFLAGS} -o moira ${OBJS} ${LIBS}

lint:
	lint -I../../include *.c

install: moira
	install -c -s moira ${DESTDIR}/usr/athena/moira
	rm -f ${DESTDIR}/usr/athena/usermaint
	rm -f ${DESTDIR}/usr/athena/listmaint
	rm -f ${DESTDIR}/usr/athena/dcmmaint
	ln -s /usr/athena/moira ${DESTDIR}/usr/athena/usermaint
	ln -s /usr/athena/moira ${DESTDIR}/usr/athena/listmaint
	ln -s /usr/athena/moira ${DESTDIR}/usr/athena/dcmmaint

clean:	
	rm -f *.o *~
	rm -f moira
	cd Doc; make clean

# Typing 'make saber' at the command line will allow load and link against the
# correct libraries when using saber C.

saber: ;
	/mit/kaufer/saber -I/mit/smsdev/include -DDEBUG ${LDFLAGS} \
	${SRCS} ${LIBS}

depend:
	mkdep ${CFLAGS} ${SRCS}

# DO NOT DELETE THIS LINE -- mkdep uses it.

utils.o: utils.c /usr/include/stdio.h /usr/include/strings.h
utils.o: ../../include/sms.h /usr/include/sms_et.h ../../include/sms_app.h
utils.o: ../../include/mit-copyright.h menu.h /usr/include/ctype.h defs.h
utils.o: f_defs.h globals.h /usr/include/netdb.h
attach.o: attach.c /usr/include/stdio.h /usr/include/strings.h
attach.o: ../../include/sms.h /usr/include/sms_et.h ../../include/sms_app.h
attach.o: ../../include/mit-copyright.h menu.h defs.h f_defs.h globals.h
cluster.o: cluster.c /usr/include/stdio.h /usr/include/strings.h
cluster.o: ../../include/sms.h /usr/include/sms_et.h ../../include/sms_app.h
cluster.o: ../../include/mit-copyright.h menu.h defs.h f_defs.h globals.h
delete.o: delete.c /usr/include/stdio.h /usr/include/strings.h
delete.o: ../../include/sms.h /usr/include/sms_et.h ../../include/sms_app.h
delete.o: ../../include/mit-copyright.h menu.h defs.h f_defs.h globals.h
globals.o: globals.c ../../include/mit-copyright.h defs.h
lists.o: lists.c /usr/include/stdio.h /usr/include/strings.h
lists.o: ../../include/sms.h /usr/include/sms_et.h ../../include/sms_app.h
lists.o: ../../include/mit-copyright.h menu.h defs.h f_defs.h globals.h
main.o: main.c /usr/include/pwd.h /usr/include/signal.h /usr/include/stdio.h
main.o: /usr/include/strings.h /usr/include/sys/types.h ../../include/sms.h
main.o: /usr/include/sms_et.h menu.h ../../include/mit-copyright.h defs.h
main.o: f_defs.h globals.h
menu.o: menu.c ../../include/mit-copyright.h /usr/include/sys/types.h
menu.o: /usr/include/stdio.h /usr/include/signal.h /usr/include/curses.h
menu.o: /usr/include/sgtty.h /usr/include/sys/ioctl.h
menu.o: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
menu.o: /usr/include/ctype.h /usr/include/strings.h /usr/include/varargs.h
menu.o: ../../include/com_err.h menu.h
menus.o: menus.c /usr/include/stdio.h menu.h ../../include/mit-copyright.h
menus.o: defs.h f_defs.h globals.h
nfs.o: nfs.c /usr/include/stdio.h /usr/include/strings.h ../../include/sms.h
nfs.o: /usr/include/sms_et.h ../../include/sms_app.h
nfs.o: ../../include/mit-copyright.h menu.h defs.h f_defs.h globals.h
pobox.o: pobox.c /usr/include/stdio.h /usr/include/strings.h
pobox.o: /usr/include/ctype.h ../../include/sms.h /usr/include/sms_et.h
pobox.o: ../../include/sms_app.h ../../include/mit-copyright.h menu.h defs.h
pobox.o: f_defs.h globals.h
quota.o: quota.c /usr/include/stdio.h /usr/include/strings.h
quota.o: ../../include/sms.h /usr/include/sms_et.h ../../include/sms_app.h
quota.o: ../../include/mit-copyright.h menu.h defs.h f_defs.h globals.h
user.o: user.c /usr/include/stdio.h /usr/include/strings.h
user.o: ../../include/sms.h /usr/include/sms_et.h ../../include/sms_app.h
user.o: ../../include/mit-copyright.h menu.h /usr/include/ctype.h defs.h
user.o: f_defs.h globals.h
dcmmaint.o: dcmmaint.c /usr/include/stdio.h /usr/include/strings.h
dcmmaint.o: /usr/include/ctype.h ../../include/sms.h /usr/include/sms_et.h
dcmmaint.o: ../../include/sms_app.h ../../include/mit-copyright.h menu.h
dcmmaint.o: defs.h f_defs.h globals.h
printer.o: printer.c /usr/include/stdio.h /usr/include/strings.h
printer.o: /usr/include/ctype.h ../../include/sms.h /usr/include/sms_et.h
printer.o: ../../include/sms_app.h ../../include/mit-copyright.h menu.h
printer.o: defs.h f_defs.h globals.h
misc.o: misc.c /usr/include/stdio.h /usr/include/strings.h
misc.o: ../../include/sms.h /usr/include/sms_et.h ../../include/sms_app.h
misc.o: ../../include/mit-copyright.h menu.h /usr/include/sys/types.h
misc.o: /usr/include/netdb.h /usr/include/sys/socket.h
misc.o: /usr/include/netinet/in.h defs.h f_defs.h globals.h
