#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/Makefile,v $
#	$Author: mar $
#	$Locker:  $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/Makefile,v 1.17 1990-02-14 11:23:00 mar Exp $
#
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.


CFLAGS= -O -I../include
SRCS = requests.c reg_svr.c startreg.c
OBJS = requests.o reg_svr.o

all: reg_svr startreg

reg_svr: $(OBJS)
	cc ${CFLAGS} -o reg_svr $(OBJS) -L../lib \
		../server/libsmsglue.a \
		-lsms -lkadm -lkrb -ldes -lzephyr -lcom_err \
		/usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib

reg_svr.o requests.o: reg_svr.h 

startreg: startreg.o
	cc -o startreg startreg.o

clean: /tmp
	rm -f a.out core *.o
	rm -f reg_svr startreg
	rm -f unreg_user.c unreg_user

install: all
	install -c reg_svr ../bin/reg_svr
	install -c startreg ../bin/startreg

depend:
	mkdep ${CFLAGS} ${SRCS}

# DO NOT DELETE THIS LINE -- mkdep uses it.

requests.o: requests.c ../include/mit-copyright.h /usr/include/stdio.h
requests.o: /usr/include/strings.h /usr/include/ctype.h
requests.o: /usr/include/sys/types.h /usr/include/sys/file.h ../include/krb.h
requests.o: ../include/des.h /usr/include/errno.h /usr/include/sys/socket.h
requests.o: /usr/include/netinet/in.h /usr/include/netdb.h ../include/sms.h
requests.o: ../include/sms_et.h ../include/sms_app.h reg_svr.h
requests.o: ../include/ureg_err.h ../include/ureg_proto.h
reg_svr.o: reg_svr.c ../include/mit-copyright.h /usr/include/stdio.h
reg_svr.o: /usr/include/strings.h /usr/include/ctype.h
reg_svr.o: /usr/include/sys/types.h /usr/include/sys/file.h ../include/krb.h
reg_svr.o: ../include/des.h ../include/kadm.h /usr/include/netinet/in.h
reg_svr.o: /usr/include/kadm_err.h /usr/include/krb_err.h
reg_svr.o: /usr/include/errno.h ../include/sms.h ../include/sms_et.h
reg_svr.o: ../include/sms_app.h reg_svr.h ../include/ureg_err.h
reg_svr.o: ../include/ureg_proto.h
startreg.o: startreg.c ../include/mit-copyright.h /usr/include/stdio.h
startreg.o: /usr/include/strings.h /usr/include/sys/types.h
startreg.o: /usr/include/sys/file.h /usr/include/sys/wait.h
startreg.o: /usr/include/sys/signal.h /usr/include/sys/ioctl.h
startreg.o: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
startreg.o: ../include/sms_app.h
