# Makefile for first release of SMS.
#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/Makefile,v 1.5 1987-09-06 11:37:58 ambar Exp $
#	$Author: ambar $
#

DESTDIR=

SUBDIRS= aliasbld attachmaint clustermaint dcmmaint listmaint \
	mailmaint nfsmaint passwd portmaint rvdmaint \
	usermaint userreg

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

