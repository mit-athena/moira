#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/Makefile,v 1.6 1988-08-04 14:27:42 mar Exp $
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
COBJS=	client.o ticket.o send_file.o checksum.o hostname.o

# common object files
COMOBJS= log_error.o hostname.o smsu_int.o log.o checksum.o

SSRCS=	dispatch.c initialize_server.c \
	quit.c xfer_001.c misc.c inst_001.c sync.c auth_001.c \
	exec_001.c get_file.c xfer_002.c exec_002.c

CSRCS=	client.c ticket.c send_file.c checksum.c hostname.c

COMSRCS= log_error.c hostname.c log.c checksum.c

LIBS=	-L../lib -lgdb -lkrb -ldes -lcom_err

all:	sms_update.o # server test smskey

sms_update.o: ${COBJS}
	ld -r -o sms_update.o ${COBJS}

client.o: client.c


clean:	
	-rm -f ${COBJS} ${SOBJS} ${COMOBJS} client.o
	-rm -f server sms_update.o test test.o smsu_int.h smskey
	-rm -f core a.out *~ \#*

install:
# This is probably wrong..
	install server ../bin/update_server
	install test ../bin/update_test
	install smskey ../bin/smskey

lint:
	lint ${LINTFLAGS} ${SSRCS} ${COMSRCS}
	lint ${LINTFLAGS} ${CSRCS} ${COMSRCS}
	lint ${LINTFLAGS} smskey.c

server:	${SOBJS} ${COMOBJS}
	rm -f server
	${CC} ${CFLAGS} -o server ${SOBJS} ${COMOBJS} ${LIBS}

smskey:	smskey.c
	${CC} ${CFLAGS} -o smskey smskey.c -ldes

# sms_update.o:	${COBJS} ${COMOBJS}
#	ld ${LDFLAGS} -r -o sms_update.o ${COBJS} ${COMOBJS}
#	-chmod -x sms_update.o

smsu_int.o smsu_int.h: smsu_int.et
	compile_et smsu_int

test.o: test.c
	-rm -f test.o
	${CC} ${CFLAGS} -c test.c

test:	test.o sms_update.o
	rm -f test
	${CC} ${CFLAGS} -o test test.o sms_update.o ${LIBS}

SRCS=	${SSRCS} ${CSRCS} ${COMSRCS} smskey.c
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
