#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/Makefile,v 1.3 1987-08-22 23:46:47 wesommer Exp $
#

#
# Makefile for 'update' directory.
#

CC=	cc
INCS=	-I../include
DEFS=
CFLAGS=	-g ${INCS} ${DEFS}
LINTFLAGS= ${INCS}
LDFLAGS= -x

# server object files
SOBJS=	dispatch.o initialize_server.o \
	quit.o xfer_001.o misc.o inst_001.o sync.o auth_001.o \
	exec_001.o get_file.o xfer_002.o exec_002.o

# client object files
COBJS=	client2.o ticket.o send_file.o

# common object files
COMOBJS= log_error.o hostname.o smsu_int.o log.o checksum.o

SSRCS=	dispatch.c initialize_server.c \
	quit.c xfer_001.c misc.c inst_001.c sync.c auth_001.c \
	exec_001.c get_file.c xfer_002.c exec_002.c

CSRCS=	client.c ticket.c send_file.c client2.c

COMSRCS= log_error.c hostname.c log.c checksum.c

# libmisc.a includes libcom_err.a and a working malloc()
LIBS=	-L../lib -lgdb -lkrb -ldes -lcom_err

all:	server sms_update.o test

clean:	
	-rm -f ${COBJS} ${SOBJS} ${COMOBJS} client.o
	-rm -f server sms_update.o test test.o smsu_int.h
	-rm -f core a.out *~ \#*

lint:
	lint ${LINTFLAGS} ${SSRCS} ${COMSRCS}
	lint ${LINTFLAGS} ${CSRCS} ${COMSRCS}

server:	${SOBJS} ${COMOBJS}
	rm -f server
	${CC} ${CFLAGS} -o server ${SOBJS} ${COMOBJS} ${LIBS}

sms_update.o:	${COBJS} ${COMOBJS}
	ld ${LDFLAGS} -r -o sms_update.o ${COBJS} ${COMOBJS}
	-chmod -x sms_update.o

smsu_int.o smsu_int.h: smsu_int.et
	compile_et smsu_int

test.o: test.c
	-rm -f test.o
	${CC} ${CFLAGS} -c test.c

test:	test.o sms_update.o
	rm -f test
	${CC} ${CFLAGS} -o test test.o sms_update.o ${LIBS}

SRCS=	${SSRCS} ${CSRCS} ${COMSRCS}
depend:	smsu_int.h
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
#
# the last line in the makefile should be...
# DO NOT DELETE THIS LINE

dispatch.o: dispatch.c /usr/include/stdio.h ../include/gdb.h
dispatch.o: /usr/include/sys/types.h /usr/include/sys/time.h
dispatch.o: /usr/include/time.h /usr/include/errno.h /usr/include/strings.h
dispatch.o: ../include/update.h sms_update_int.h smsu_int.h kludge.h
initialize_server.o: initialize_server.c /usr/include/stdio.h ../include/gdb.h
initialize_server.o: /usr/include/sys/types.h /usr/include/sys/time.h
initialize_server.o: /usr/include/time.h
quit.o: quit.c /usr/include/stdio.h ../include/gdb.h /usr/include/sys/types.h
quit.o: /usr/include/sys/time.h /usr/include/time.h
xfer_001.o: xfer_001.c /usr/include/stdio.h /usr/include/strings.h
xfer_001.o: ../include/gdb.h /usr/include/sys/types.h /usr/include/sys/time.h
xfer_001.o: /usr/include/time.h /usr/include/ctype.h /usr/include/sys/param.h
xfer_001.o: /usr/include/machine/machparam.h /usr/include/signal.h
xfer_001.o: /usr/include/sys/types.h /usr/include/sys/file.h smsu_int.h
xfer_001.o: kludge.h
misc.o: misc.c /usr/include/stdio.h ../include/gdb.h /usr/include/sys/types.h
misc.o: /usr/include/sys/time.h /usr/include/time.h sms_update_int.h kludge.h
misc.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
misc.o: /usr/include/signal.h /usr/include/sys/types.h smsu_int.h
inst_001.o: inst_001.c /usr/include/stdio.h ../include/gdb.h
inst_001.o: /usr/include/sys/types.h /usr/include/sys/time.h
inst_001.o: /usr/include/time.h kludge.h sms_update_int.h
sync.o: sync.c /usr/include/stdio.h ../include/gdb.h /usr/include/sys/types.h
sync.o: /usr/include/sys/time.h /usr/include/time.h kludge.h smsu_int.h
auth_001.o: auth_001.c /usr/include/stdio.h /usr/include/strings.h
auth_001.o: ../include/gdb.h /usr/include/sys/types.h /usr/include/sys/time.h
auth_001.o: /usr/include/time.h smsu_int.h kludge.h /usr/include/krb.h
auth_001.o: /usr/include/des.h /usr/include/netinet/in.h /usr/include/errno.h
exec_001.o: exec_001.c /usr/include/stdio.h ../include/gdb.h
exec_001.o: /usr/include/sys/types.h /usr/include/sys/time.h
exec_001.o: /usr/include/time.h sms_update_int.h kludge.h smsu_int.h
get_file.o: get_file.c /usr/include/stdio.h ../include/gdb.h
get_file.o: /usr/include/sys/types.h /usr/include/sys/time.h
get_file.o: /usr/include/time.h /usr/include/ctype.h /usr/include/sys/param.h
get_file.o: /usr/include/machine/machparam.h /usr/include/signal.h
get_file.o: /usr/include/sys/types.h /usr/include/sys/file.h
get_file.o: ../include/update.h smsu_int.h kludge.h
xfer_002.o: xfer_002.c /usr/include/stdio.h ../include/gdb.h
xfer_002.o: /usr/include/sys/types.h /usr/include/sys/time.h
xfer_002.o: /usr/include/time.h /usr/include/ctype.h /usr/include/sys/param.h
xfer_002.o: /usr/include/machine/machparam.h /usr/include/signal.h
xfer_002.o: /usr/include/sys/types.h /usr/include/sys/file.h smsu_int.h
xfer_002.o: kludge.h /usr/include/strings.h
exec_002.o: exec_002.c /usr/include/stdio.h /usr/include/sys/wait.h
exec_002.o: ../include/gdb.h /usr/include/sys/types.h /usr/include/sys/time.h
exec_002.o: /usr/include/time.h ../include/update.h sms_update_int.h kludge.h
exec_002.o: smsu_int.h
client.o: client.c /usr/include/stdio.h ../include/gdb.h
client.o: /usr/include/sys/types.h /usr/include/sys/time.h /usr/include/time.h
client.o: /usr/include/sys/stat.h /usr/include/sys/file.h
client.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
client.o: /usr/include/signal.h /usr/include/sys/types.h ../include/update.h
client.o: /usr/include/errno.h sms_update_int.h smsu_int.h /usr/include/krb.h
client.o: /usr/include/des.h kludge.h
stubs.o: stubs.c ../include/update.h
ticket.o: ticket.c /usr/include/stdio.h /usr/include/krb.h /usr/include/des.h
ticket.o: /usr/include/sys/types.h /usr/include/sys/stat.h
ticket.o: /usr/include/strings.h sms_update_int.h ../include/com_err.h
ticket.o: ../include/mit-sipb-copyright.h kludge.h
send_file.o: send_file.c /usr/include/stdio.h ../include/com_err.h
send_file.o: ../include/mit-sipb-copyright.h ../include/gdb.h
send_file.o: /usr/include/sys/types.h /usr/include/sys/time.h
send_file.o: /usr/include/time.h smsu_int.h /usr/include/sys/file.h
send_file.o: ../include/update.h sms_update_int.h kludge.h
client2.o: client2.c /usr/include/stdio.h /usr/include/strings.h
client2.o: ../include/gdb.h /usr/include/sys/types.h /usr/include/sys/time.h
client2.o: /usr/include/time.h /usr/include/sys/stat.h /usr/include/sys/file.h
client2.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
client2.o: /usr/include/signal.h /usr/include/sys/types.h ../include/update.h
client2.o: /usr/include/errno.h sms_update_int.h smsu_int.h /usr/include/krb.h
client2.o: /usr/include/des.h kludge.h
log_error.o: log_error.c /usr/include/stdio.h /usr/include/syslog.h
hostname.o: hostname.c /usr/include/stdio.h /usr/include/sys/types.h
hostname.o: /usr/include/netinet/in.h /usr/include/netdb.h /usr/include/ctype.h
hostname.o: /usr/include/strings.h
log.o: log.c /usr/include/stdio.h ../include/com_err.h
log.o: ../include/mit-sipb-copyright.h /usr/include/varargs.h
log.o: ../include/update.h /usr/include/krb.h /usr/include/des.h
checksum.o: checksum.c /usr/include/stdio.h /usr/include/sys/file.h
