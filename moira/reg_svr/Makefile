#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/Makefile,v $
#	$Author: mar $
#	$Locker:  $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/Makefile,v 1.15 1989-09-08 18:44:18 mar Exp $
#
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.


CFLAGS= -O -I../include
OBJS = requests.o reg_svr.o admin_call.o admin_err.o

all: reg_svr startreg

reg_svr: $(OBJS)
	cc ${CFLAGS} -o reg_svr $(OBJS) -L../lib \
		../server/libsmsglue.a \
		-lsms -lcom_err -lkrb -ldes -lzephyr \
		/usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib

reg_svr.o requests.o: reg_svr.h admin_err.h

admin_err.c admin_err.h: admin_err.et
	../et/compile_et admin_err.et

startreg: startreg.o
	cc -o startreg startreg.o

clean: /tmp
	rm -f a.out core *.o
	rm -f reg_svr startreg
	rm -f unreg_user.c unreg_user

install: all
	install reg_svr ../bin/reg_svr
	install startreg ../bin/startreg
