#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/man/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/man/Makefile,v 1.4 1988-12-01 12:43:44 mar Exp $
#	$Author: mar $
#

DESTDIR=
MAN1PAGES= chfn.1 chpobox.1 chsh.1 dcmmaint.1 listmaint.1 \
	mailmaint.1 sms.1 smscheck.1 smslist.1 usermaint.1 userreg.1
MAN3PAGES= sms.3
MAN8PAGES= dcm.8 reg_svr.8 smsd.8 smstest.8 startdcm.8 startreg.8 \
	startsms.8 update_server.8

all:
install:
	for i in ${MAN1PAGES}; do \
		install -c $$i -m 0644 ${DESTDIR}/usr/man/man1/$$i; done
	for i in ${MAN3PAGES}; do \
		install -c $$i -m 0644 ${DESTDIR}/usr/man/man3/$$i; done
	for i in ${MAN8PAGES}; do \
		install -c $$i -m 0644 ${DESTDIR}/usr/man/man8/$$i; done

clean:
	rm -f *~
