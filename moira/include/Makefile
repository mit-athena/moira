# Makefile for SMS 2.0 include files
#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/Makefile,v 1.4 1990-03-17 16:30:34 mar Exp $
#	$Author: mar $
#
# (c) Copyright 1988, 1990 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.

DESTDIR=
FILES= moira.h moira_site.h mr_et.h

all:

install: ${FILES}
	install -c -m 644 moira.h ${DESTDIR}/usr/include/moira.h
	install -c -m 644 moira_site.h ${DESTDIR}/usr/include/moira_site.h
	install -c -m 644 mr_et.h ${DESTDIR}/usr/include/mr_et.h

depend:

