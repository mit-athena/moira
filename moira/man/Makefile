#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/man/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/man/Makefile,v 1.8 1990-03-20 11:31:19 mar Exp $
#	$Author: mar $
#

DESTDIR=
MAN1PAGES= blanche.1 chfn.1 chpobox.1 chsh.1 dcmmaint.1 listmaint.1 \
	mailmaint.1 moira.1 mrcheck.1 usermaint.1 userreg.1
MAN3PAGES= moira.3
MAN8PAGES= dcm.8 mrtest.8 reg_svr.8 moirad.8 startdcm.8 startreg.8 \
	startmoira.8 update_server.8

all:

install:
	for i in ${MAN1PAGES}; do \
		install -c -m 0644 $$i ${DESTDIR}/usr/man/man1/$$i; done
	for i in ${MAN3PAGES}; do \
		install -c -m 0644 $$i ${DESTDIR}/usr/man/man3/$$i; done
	for i in ${MAN8PAGES}; do \
		install -c -m 0644 $$i ${DESTDIR}/usr/man/man8/$$i; done

clean:
	rm -f *~

depend:
