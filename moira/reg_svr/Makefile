#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/Makefile,v $
#	$Author: qjb $
#	$Locker:  $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/Makefile,v 1.9 1988-08-05 17:47:44 qjb Exp $
#

CFLAGS= -O -I../include
OBJS = requests.o reg_svr.o

all: reg_svr startreg

reg_svr: $(OBJS)
	cc ${CFLAGS} -o reg_svr $(OBJS) -L../lib \
		../server/libsmsglue.a \
		-lsms -lcom_err -lkrb -ldes -lzephyr \
		/usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib

startreg: startreg.c
	cc -o startreg startreg.c

clean: /tmp
	rm -f a.out core *.o
	rm -f reg_svr startreg
	rm -f unreg_user.c unreg_user

install: all
	install reg_svr ../bin/reg_svr
	install startreg ../bin/startreg
