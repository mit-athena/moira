# Makefile for first release of SMS.
#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/Makefile,v 1.4 1987-09-03 04:31:25 wesommer Exp $
#	$Author: wesommer $
#

DESTDIR=

SUBDIRS= aliasbld listmaint passwd userreg usermaint mailmaint \
	attachmaint clustermaint portmaint nfsmaint dcmmaint

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

