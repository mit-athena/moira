#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/man/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/man/Makefile,v 1.1 1988-01-29 11:53:32 mar Exp $
#	$Author: mar $
#

DESTDIR=
MAN1PAGES= listmaint.1 usermaint.1 attachmaint.1 clustermaint.1 mailmaint.1
MAN8PAGES= smstest.8

all:
install:
	for i in ${MAN1PAGES}; do \
		install -c $$i -m 0644 ${DESTDIR}/usr/man/man1/$$i; done
	for i in ${MAN8PAGES}; do \
		install -c $$i -m 0644 ${DESTDIR}/usr/man/man8/$$i; done

clean:
	rm *~
