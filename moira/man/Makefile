#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/man/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/man/Makefile,v 1.3 1988-06-08 16:23:39 mar Exp $
#	$Author: mar $
#

DESTDIR=
MAN1PAGES= listmaint.1 usermaint.1 attachmaint.1 clustermaint.1 mailmaint.1 \
	chfn.1 chpobox.1 chsh.1 dcmmaint.1 nfsmaint.1 rvdmaint.1 smscheck.1
MAN8PAGES= smstest.8

all:
install:
	for i in ${MAN1PAGES}; do \
		install -c $$i -m 0644 ${DESTDIR}/usr/man/man1/$$i; done
	for i in ${MAN8PAGES}; do \
		install -c $$i -m 0644 ${DESTDIR}/usr/man/man8/$$i; done

clean:
	rm -f *~
