#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/Makefile,v $
#	$Author: mar $
#	$Locker:  $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/Makefile,v 1.5 1988-08-09 17:58:56 mar Exp $
#

CFLAGS= -O -I../../include

TEXT= welcome network_down username_blurb password_blurb improper_reg \
	deleted_acct not_allowed

all: userreg

userreg: userreg.o display.o reg_stubs.o disable.o
	cc ${CFLAGS} -o $@ userreg.o display.o reg_stubs.o disable.o \
		-L../../lib -lcom_err -lkrb -ldes -lsms -lcurses -ltermcap

userreg.o display.o: userreg.h files.h

clean: /tmp
	rm -f a.out core *.o
	rm -f userreg

install:

install_real: all
	install -c -s userreg ${DESTDIR}/mit/register/userreg.`machtype`
	install -c register ${DESTDIR}/mit/register/register
	for i in ${TEXT}; do \
		install -c -m 444 $$i ${DESTDIR}/mit/register/$$i; \
		done;
