# Makefile for SMS 2.0 clients.
#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/Makefile,v 1.11 1989-01-21 16:26:43 mar Exp $
#	$Author: mar $
#
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.

DESTDIR=

SUBDIRS= mailmaint passwd moira userreg blanche mrcheck

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

