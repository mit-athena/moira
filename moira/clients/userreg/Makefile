#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/Makefile,v $
#	$Author: mar $
#	$Locker:  $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/Makefile,v 1.10 1991-03-08 10:29:33 mar Exp $
#
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.

CFLAGS= -O -I../../include

TEXT= welcome network_down username_blurb password_blurb improper_reg \
	deleted_acct not_allowed

all: userreg

userreg: userreg.o display.o reg_stubs.o disable.o
	cc ${CFLAGS} -o $@ userreg.o display.o reg_stubs.o disable.o \
		-L../../lib -lcom_err -lkrb -ldes -lmoira -lcurses \
		-ltermcap -lhesiod -lkadm

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

depend:
	mkdep ${CFLAGS} userreg.c display.c reg_stubs.c disable.c

# DO NOT DELETE THIS LINE -- mkdep uses it.

userreg.o: userreg.c ../../include/mit-copyright.h /usr/include/curses.h
userreg.o: /usr/include/stdio.h /usr/include/sgtty.h /usr/include/sys/ioctl.h
userreg.o: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
userreg.o: /usr/include/signal.h /usr/include/sys/time.h
userreg.o: /usr/include/sys/time.h /usr/include/setjmp.h /usr/include/ctype.h
userreg.o: ../../include/krb.h ../../include/des.h /usr/include/errno.h
userreg.o: userreg.h files.h ../../include/ureg_err.h
display.o: display.c ../../include/mit-copyright.h /usr/include/stdio.h
display.o: /usr/include/curses.h /usr/include/sgtty.h
display.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
display.o: /usr/include/sys/ttydev.h /usr/include/sys/time.h
display.o: /usr/include/sys/time.h userreg.h /usr/include/ctype.h files.h
reg_stubs.o: reg_stubs.c ../../include/mit-copyright.h /usr/include/stdio.h
reg_stubs.o: /usr/include/sys/types.h /usr/include/sys/time.h
reg_stubs.o: /usr/include/sys/time.h /usr/include/sys/socket.h
reg_stubs.o: /usr/include/netinet/in.h /usr/include/netdb.h
reg_stubs.o: ../../include/des.h /usr/include/errno.h ../../include/moira.h
reg_stubs.o: ../../include/mr_et.h ../../include/moira_site.h
reg_stubs.o: ../../include/ureg_err.h ../../include/ureg_proto.h
reg_stubs.o: /usr/include/strings.h
disable.o: disable.c ../../include/mit-copyright.h /usr/include/stdio.h
disable.o: /usr/include/ctype.h /usr/include/sys/types.h
disable.o: /usr/include/sys/time.h /usr/include/sys/time.h
disable.o: /usr/include/sys/file.h
