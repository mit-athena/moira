#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/Makefile,v $
#	$Author: ostlund $
#	$Locker:  $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/Makefile,v 1.1 1986-08-21 18:05:46 ostlund Exp $
#

CFLAGS= -O
TEXT= welcome network_down username_blurb password_blurb

all: userreg
userreg: userreg.o display.o libuserinfo.a
	cc ${CFLAGS} -o userreg userreg.o display.o -lcurses -ltermcap \
		libuserinfo.a

userreg.o: userreg.c userreg.h files.h
display.o: display.c userreg.h files.h

clean: /tmp
	rm -f a.out core userreg.o display.o userreg

install: all
	install -c -o root -g staff -m 700 userreg.sh \
		${DESTDIR}/usr/athena/userreg.lockout
	rm -rf ${DESTDIR}/usr/athena/lib/userreg
	mkdir ${DESTDIR}/usr/athena/lib/userreg
	install -c -s -o root -g staff -m 4755 userreg \
		${DESTDIR}/usr/athena/userreg
	for i in ${TEXT}; do \
		install -c $$i \
			${DESTDIR}/usr/athena/lib/userreg/$$i; \
		done;

