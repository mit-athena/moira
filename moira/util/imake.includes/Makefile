#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/util/imake.includes/Makefile,v $
#	$Author: jtkohl $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/util/imake.includes/Makefile,v 4.0 1989-01-16 21:38:58 jtkohl Exp $
#
# Copyright 1988 by the Massachusetts Institute of Technology.
#
# For copying and distribution information, please see the file
# <mit-copyright.h>.
#
# Makefile for copying Imake configuration files
SOURCES = \
	${SRCDIR}/util/imake.includes/Imake.rules \
	${SRCDIR}/util/imake.includes/Imake.template \
	${SRCDIR}/util/imake.includes/config.Imakefile
DESTDIR =
SRCDIR = /mit/kerberos/src
CP = /bin/cp
RM = rm -f

config: $(SOURCES)
	-mkdir $(DESTDIR)/util
	-mkdir $(DESTDIR)/util/imake.includes
	for i in $(SOURCES); do \
		$(RM) ${DESTDIR}/util/imake.includes/`basename $$i`; \
		$(CP) $$i ${DESTDIR}/util/imake.includes/`basename $$i`; done

