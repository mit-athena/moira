# Makefile for first release of SMS.
#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/Makefile,v 1.3 1987-08-23 01:45:17 wesommer Exp $
#	$Author: wesommer $
#

DESTDIR=

SUBDIRS= aliasbld listmaint passwd servermaint userreg usermaint mailmaint

all:
	for i in ${SUBDIRS}; do \
		(cd $$i; make ${MFLAGS} all); done

install:
	for i in ${SUBDIRS}; do \
		(cd $$i; make ${MFLAGS} DESTDIR=${DESTDIR} install); done

clean:
	rm -f *~ *.bak
	for i in ${SUBDIRS}; do \
		(cd $$i; make ${MFLAGS} clean); done

