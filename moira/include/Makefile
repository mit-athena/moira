# Makefile for SMS 2.0 include files
#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/Makefile,v 1.1 1988-12-27 18:48:57 mar Exp $
#	$Author: mar $
#
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.

DESTDIR=
FILES= sms.h sms_app.h sms_et.h

all:

install: ${FILES}
	install -m 644 sms.h ${DESTDIR}/usr/include/sms.h
	install -m 644 sms_app.h ${DESTDIR}/usr/include/sms_app.h
	install -m 644 sms_et.h ${DESTDIR}/usr/include/sms_et.h
