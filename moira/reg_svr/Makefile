#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/Makefile,v $
#	$Author: danw $
#	$Locker:  $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/Makefile,v 1.20 1996-07-08 21:48:52 danw Exp $
#
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.


CFLAGS= -O -I../include -I../lib
SRCS = requests.c reg_svr.c startreg.c
OBJS = requests.o reg_svr.o

all: reg_svr startreg

reg_svr: $(OBJS)
	cc ${CFLAGS} -o reg_svr $(OBJS) -L../lib \
		../server/libmoiraglue.a /mit/gdss/vax/libgdss.a \
		-lmoira -lkadm -lkrb -ldes -lzephyr -lhesiod -lcom_err \
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
