#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/Makefile,v $
#	$Author: mar $
#	$Locker:  $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/Makefile,v 1.4 1988-02-08 15:10:56 mar Exp $
#

CFLAGS= -O -I../include

all: reg_svr startreg

reg_svr: reg_svr.o
	cc ${CFLAGS} -o reg_svr reg_svr.o -L../../lib \
		../rpc/sms_al_filsys.o \
		../rpc/sms_al_pobox.o \
		../rpc/sms_misc.o \
		../rpc/admin_call.o \
		../rpc/admin_err.o \
		../rpc/ureg_err.o \
		../server/libsmsglue.a \
		-lcom_err -lkrb -ldes /usr/rtingres/lib/libqlib \
		/usr/rtingres/lib/compatlib

startreg: startreg.c
	cc -o startreg startreg.c

clean: /tmp
	rm -f a.out core *.o
	rm -f reg_svr startreg
	rm -f unreg_user.c unreg_user

install: all
	install reg_svr ../bin/reg_svr
	install startreg ../bin/startreg
