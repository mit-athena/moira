# Makefile for SMS 2.0 clients.
#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/Makefile,v 1.9 1988-09-12 17:03:41 mar Exp $
#	$Author: mar $
#
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.

DESTDIR=

SUBDIRS= dcmmaint mailmaint passwd sms userreg blanche

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

