#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/Makefile,v $
#	$Author: wesommer $
#	$Locker:  $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/Makefile,v 1.3 1987-08-23 01:42:02 wesommer Exp $
#

CFLAGS= -O -I../../include

TEXT= welcome network_down username_blurb password_blurb

all: userreg reg_svr

userreg: userreg.o display.o reg_stubs.o
	cc ${CFLAGS} -o userreg userreg.o display.o reg_stubs.o ureg_err.o \
		-L../../lib -lcom_err -lkrb -ldes -lsms -lcurses -ltermcap

reg_svr: reg_svr.o ureg_err.o 
	cc ${CFLAGS} -o reg_svr reg_svr.o ureg_err.o \
		-L../../lib ../../rpc/sms_al_filsys.o \
		../../rpc/sms_al_pobox.o \
		../../rpc/sms_misc.o \
		../../rpc/admin_call.o \
		../../rpc/admin_err.o \
		../../server/libsmsglue.a \
		-lcom_err -lkrb -ldes /usr/rtingres/lib/libqlib \
		/usr/rtingres/lib/compatlib

userreg.o: userreg.c userreg.h files.h ureg_err.h
display.o: display.c userreg.h files.h

ureg_err.o: ureg_err.et
	compile_et ureg_err.et

ureg_err.h: ureg_err.o

clean: /tmp
	rm -f a.out core *.o
	rm -f userreg reg_svr unreg_user
	rm -f ureg_err.h
	rm -f unreg_user.c

install: all
	install -c -o root -g staff -m 700 userreg.sh \
		${DESTDIR}/usr/athena/userreg.lockout
	rm -rf ${DESTDIR}/usr/athena/lib/userreg
	mkdir ${DESTDIR}/usr/athena/lib/userreg
	install -c -s userreg ${DESTDIR}/usr/athena/userreg
	for i in ${TEXT}; do \
		install -c $$i \
			${DESTDIR}/usr/athena/lib/userreg/$$i; \
		done;
	install reg_svr ../../bin/reg_svr


