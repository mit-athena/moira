# Makefile for first release of SMS.
#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/Makefile,v 1.2 1987-08-22 16:42:47 wesommer Exp $
#	$Author: wesommer $
#

DESTDIR=

SUBDIRS= aliasbld finger listmaint servermaint usermaint userreg

all:
	for i in ${SUBDIRS}; do \
		(cd $$i; make ${MFLAGS} all); done

install:
	for i in ${SUBDIRS}; do \
		(cd $$; make ${MFLAGS} install); done

clean:
	for i in ${SUBDIRS}; do \
		(cd $$i; make ${MFLAGS} clean); done

