#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/Makefile,v 1.17 1990-05-02 13:20:25 mar Exp $
#

#
# Makefile for 'update' directory.
#
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.

CC=	cc
INCS=	-I../include
DEFS=
CFLAGS=	-g ${INCS} ${DEFS}
LINTFLAGS= ${INCS}
LDFLAGS= -x

# server object files
SOBJS=	update_server.o auth_001.o inst_001.o xfer_002.o exec_002.o \
	hostname.o log.o get_file.o checksum.o

# client object files
COBJS=	client.o ticket.o send_file.o checksum.o hostname.o

SSRCS=	update_server.c auth_001.c inst_001.c xfer_002.c exec_002.c \
	hostname.c log.c get_file.c checksum.c

CSRCS=	client.c ticket.c send_file.c checksum.c hostname.c

LIBS=	-L../lib -lmoira -lgdb -lkrb -ldes -lcom_err

all:	moira_update.o update_server

update_server: ${SOBJS}
	cc ${CFLAGS} -o update_server ${SOBJS} ${LIBS}

moira_update.o: ${COBJS}
	ld -r -o moira_update.o ${COBJS}


clean:	
	-rm -f ${COBJS} ${SOBJS}
	-rm -f update_server moira_update.o
	-rm -f core a.out *~ \#*

install: update_server
	install -c -s update_server ${DESTDIR}/usr/etc/update_server

lint:
	lint ${LINTFLAGS} ${SSRCS}
	lint ${LINTFLAGS} ${CSRCS}

smskey:	smskey.c
	${CC} ${CFLAGS} -o smskey smskey.c -ldes

depend:
	mkdep ${CFLAGS} ${SSRCS} ${CSRCS}

# DO NOT DELETE THIS LINE -- mkdep uses it.

update_server.o: update_server.c ../include/mit-copyright.h
update_server.o: /usr/include/stdio.h ../include/gdb.h
update_server.o: /usr/include/sys/types.h /usr/include/sys/time.h
update_server.o: /usr/include/sys/time.h /usr/include/errno.h
update_server.o: /usr/include/strings.h ../include/moira.h ../include/mr_et.h
update_server.o: /usr/include/sys/file.h /usr/include/sys/ioctl.h
update_server.o: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
update_server.o: ../include/update.h
auth_001.o: auth_001.c ../include/mit-copyright.h /usr/include/stdio.h
auth_001.o: /usr/include/strings.h ../include/gdb.h /usr/include/sys/types.h
auth_001.o: /usr/include/sys/time.h /usr/include/sys/time.h ../include/krb.h
auth_001.o: ../include/des.h ../include/krb_et.h /usr/include/netinet/in.h
auth_001.o: /usr/include/errno.h
inst_001.o: inst_001.c ../include/mit-copyright.h /usr/include/stdio.h
inst_001.o: ../include/gdb.h /usr/include/sys/types.h /usr/include/sys/time.h
inst_001.o: /usr/include/sys/time.h
xfer_002.o: xfer_002.c ../include/mit-copyright.h /usr/include/stdio.h
xfer_002.o: ../include/gdb.h /usr/include/sys/types.h /usr/include/sys/time.h
xfer_002.o: /usr/include/sys/time.h /usr/include/ctype.h
xfer_002.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
xfer_002.o: /usr/include/sys/signal.h /usr/include/sys/file.h
xfer_002.o: /usr/include/strings.h ../include/moira.h ../include/mr_et.h
exec_002.o: exec_002.c ../include/mit-copyright.h /usr/include/stdio.h
exec_002.o: /usr/include/sys/wait.h /usr/include/sys/signal.h
exec_002.o: ../include/gdb.h /usr/include/sys/types.h /usr/include/sys/time.h
exec_002.o: /usr/include/sys/time.h ../include/moira.h ../include/mr_et.h
exec_002.o: ../include/update.h
hostname.o: hostname.c ../include/mit-copyright.h /usr/include/stdio.h
hostname.o: /usr/include/sys/types.h /usr/include/netinet/in.h
hostname.o: /usr/include/netdb.h /usr/include/ctype.h /usr/include/strings.h
log.o: log.c ../include/mit-copyright.h /usr/include/stdio.h
log.o: ../include/com_err.h /usr/include/varargs.h ../include/update.h
log.o: ../include/krb.h ../include/des.h
get_file.o: get_file.c ../include/mit-copyright.h /usr/include/stdio.h
get_file.o: ../include/gdb.h /usr/include/sys/types.h /usr/include/sys/time.h
get_file.o: /usr/include/sys/time.h /usr/include/ctype.h
get_file.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
get_file.o: /usr/include/sys/signal.h /usr/include/sys/file.h
get_file.o: ../include/moira.h ../include/mr_et.h ../include/update.h
checksum.o: checksum.c ../include/mit-copyright.h /usr/include/stdio.h
checksum.o: /usr/include/sys/file.h
client.o: client.c ../include/mit-copyright.h /usr/include/stdio.h
client.o: /usr/include/strings.h ../include/gdb.h /usr/include/sys/types.h
client.o: /usr/include/sys/time.h /usr/include/sys/time.h
client.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
client.o: /usr/include/sys/signal.h /usr/include/sys/wait.h
client.o: /usr/include/sys/socket.h ../include/update.h /usr/include/errno.h
client.o: ../include/dcm.h ../include/moira.h ../include/mr_et.h
client.o: ../include/moira_site.h ../include/krb.h ../include/des.h
ticket.o: ticket.c ../include/mit-copyright.h /usr/include/stdio.h
ticket.o: ../include/krb.h ../include/des.h /usr/include/sys/types.h
ticket.o: /usr/include/sys/stat.h /usr/include/strings.h ../include/update.h
ticket.o: ../include/com_err.h ../include/krb_et.h
send_file.o: send_file.c ../include/mit-copyright.h /usr/include/stdio.h
send_file.o: ../include/com_err.h ../include/gdb.h /usr/include/sys/types.h
send_file.o: /usr/include/sys/time.h /usr/include/sys/time.h ../include/dcm.h
send_file.o: ../include/moira.h ../include/mr_et.h /usr/include/sys/file.h
send_file.o: /usr/include/sys/stat.h ../include/update.h
checksum.o: checksum.c ../include/mit-copyright.h /usr/include/stdio.h
checksum.o: /usr/include/sys/file.h
hostname.o: hostname.c ../include/mit-copyright.h /usr/include/stdio.h
hostname.o: /usr/include/sys/types.h /usr/include/netinet/in.h
hostname.o: /usr/include/netdb.h /usr/include/ctype.h /usr/include/strings.h
