# Makefile for SMS 2.0 include files
#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/Makefile,v 1.2 1990-02-14 11:41:36 mar Exp $
#	$Author: mar $
#
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.

DESTDIR=
FILES= sms.h sms_app.h sms_et.h

all:

install: ${FILES}
	install -c -m 644 sms.h ${DESTDIR}/usr/include/sms.h
	install -c -m 644 sms_app.h ${DESTDIR}/usr/include/sms_app.h
	install -c -m 644 sms_et.h ${DESTDIR}/usr/include/sms_et.h
