#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/Makefile,v 1.9 1988-12-27 18:23:29 mar Exp $
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

LIBS=	-L../lib -lsms -lgdb -lkrb -ldes -lcom_err

all:	sms_update.o update_server

update_server: ${SOBJS}
	cc ${CFLAGS} -o update_server ${SOBJS} ${LIBS}

sms_update.o: ${COBJS}
	ld -r -o sms_update.o ${COBJS}


clean:	
	-rm -f ${COBJS} ${SOBJS}
	-rm -f update_server sms_update.o
	-rm -f core a.out *~ \#*

install: update_server
	install -s update_server ${DESTDIR}/usr/etc/update_server

lint:
	lint ${LINTFLAGS} ${SSRCS}
	lint ${LINTFLAGS} ${CSRCS}


smskey:	smskey.c
	${CC} ${CFLAGS} -o smskey smskey.c -ldes

SRCS=	${SSRCS} ${CSRCS}
depend:
	-rm -f eddep makedep
	${CC} -M ${CFLAGS} ${SRCS} | \
	sed -e 's; ./; ;' \
	    -e ':loop' \
	    -e 's/\.\.\/[^ /]*\/\.\./../' \
	    -e 't loop' | \
	awk ' { if ($$1 != prev) { print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		       else rec = rec " " $$2 } } \
	      END { print rec } ' > makedep
	echo '/^# DO NOT DELETE THIS LINE/+1,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	-rm -f Makefile.bak
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm -f eddep makedep

# the last line in the makefile should be...
# DO NOT DELETE THIS LINE

update_server.o: update_server.c ../include/mit-copyright.h
update_server.o: /usr/include/stdio.h ../include/gdb.h /usr/include/sys/types.h
update_server.o: /usr/include/sys/time.h /usr/include/time.h
update_server.o: /usr/include/errno.h /usr/include/strings.h ../include/sms.h
update_server.o: ../include/sms_et.h /usr/include/sys/file.h
update_server.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
update_server.o: /usr/include/sys/ttydev.h update.h kludge.h
auth_001.o: auth_001.c ../include/mit-copyright.h /usr/include/stdio.h
auth_001.o: /usr/include/strings.h ../include/gdb.h /usr/include/sys/types.h
auth_001.o: /usr/include/sys/time.h /usr/include/time.h kludge.h
auth_001.o: /usr/include/krb.h ../include/mit-copyright.h /usr/include/des.h
auth_001.o: ../include/mit-copyright.h /usr/include/des_conf.h
auth_001.o: ../include/mit-copyright.h /usr/include/netinet/in.h
auth_001.o: /usr/include/errno.h
inst_001.o: inst_001.c ../include/mit-copyright.h /usr/include/stdio.h
inst_001.o: ../include/gdb.h /usr/include/sys/types.h /usr/include/sys/time.h
inst_001.o: /usr/include/time.h kludge.h
xfer_002.o: xfer_002.c ../include/mit-copyright.h /usr/include/stdio.h
xfer_002.o: ../include/gdb.h /usr/include/sys/types.h /usr/include/sys/time.h
xfer_002.o: /usr/include/time.h /usr/include/ctype.h /usr/include/sys/param.h
xfer_002.o: /usr/include/machine/machparam.h /usr/include/signal.h
xfer_002.o: /usr/include/sys/types.h /usr/include/sys/file.h
xfer_002.o: /usr/include/strings.h ../include/sms.h ../include/sms_et.h
xfer_002.o: kludge.h
exec_002.o: exec_002.c ../include/mit-copyright.h /usr/include/stdio.h
exec_002.o: /usr/include/sys/wait.h /usr/include/signal.h ../include/gdb.h
exec_002.o: /usr/include/sys/types.h /usr/include/sys/time.h
exec_002.o: /usr/include/time.h ../include/sms.h ../include/sms_et.h update.h
exec_002.o: kludge.h
hostname.o: hostname.c ../include/mit-copyright.h /usr/include/stdio.h
hostname.o: /usr/include/sys/types.h /usr/include/netinet/in.h
hostname.o: /usr/include/netdb.h /usr/include/ctype.h /usr/include/strings.h
log.o: log.c ../include/mit-copyright.h /usr/include/stdio.h
log.o: ../include/com_err.h ../include/mit-sipb-copyright.h
log.o: /usr/include/varargs.h update.h /usr/include/krb.h
log.o: ../include/mit-copyright.h /usr/include/des.h ../include/mit-copyright.h
log.o: /usr/include/des_conf.h ../include/mit-copyright.h
get_file.o: get_file.c ../include/mit-copyright.h /usr/include/stdio.h
get_file.o: ../include/gdb.h /usr/include/sys/types.h /usr/include/sys/time.h
get_file.o: /usr/include/time.h /usr/include/ctype.h /usr/include/sys/param.h
get_file.o: /usr/include/machine/machparam.h /usr/include/signal.h
get_file.o: /usr/include/sys/types.h /usr/include/sys/file.h ../include/sms.h
get_file.o: ../include/sms_et.h update.h kludge.h
checksum.o: checksum.c ../include/mit-copyright.h /usr/include/stdio.h
checksum.o: /usr/include/sys/file.h
client.o: client.c ../include/mit-copyright.h /usr/include/stdio.h
client.o: /usr/include/strings.h ../include/gdb.h /usr/include/sys/types.h
client.o: /usr/include/sys/time.h /usr/include/time.h /usr/include/sys/param.h
client.o: /usr/include/machine/machparam.h /usr/include/signal.h
client.o: /usr/include/sys/types.h /usr/include/sys/wait.h ../include/update.h
client.o: /usr/include/errno.h ../include/dcm.h ../include/sms.h
client.o: ../include/sms_et.h /usr/include/krb.h ../include/mit-copyright.h
client.o: /usr/include/des.h ../include/mit-copyright.h /usr/include/des_conf.h
client.o: ../include/mit-copyright.h
ticket.o: ticket.c ../include/mit-copyright.h /usr/include/stdio.h
ticket.o: /usr/include/krb.h ../include/mit-copyright.h /usr/include/des.h
ticket.o: ../include/mit-copyright.h /usr/include/des_conf.h
ticket.o: ../include/mit-copyright.h /usr/include/sys/types.h
ticket.o: /usr/include/sys/stat.h /usr/include/strings.h ../include/update.h
ticket.o: ../include/com_err.h ../include/mit-sipb-copyright.h
send_file.o: send_file.c ../include/mit-copyright.h /usr/include/stdio.h
send_file.o: ../include/com_err.h ../include/mit-sipb-copyright.h
send_file.o: ../include/gdb.h /usr/include/sys/types.h /usr/include/sys/time.h
send_file.o: /usr/include/time.h ../include/dcm.h ../include/sms.h
send_file.o: ../include/sms_et.h /usr/include/sys/file.h
send_file.o: /usr/include/sys/stat.h ../include/update.h
checksum.o: checksum.c ../include/mit-copyright.h /usr/include/stdio.h
checksum.o: /usr/include/sys/file.h
hostname.o: hostname.c ../include/mit-copyright.h /usr/include/stdio.h
hostname.o: /usr/include/sys/types.h /usr/include/netinet/in.h
hostname.o: /usr/include/netdb.h /usr/include/ctype.h /usr/include/strings.h
