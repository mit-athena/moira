#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/util/imake.includes/Makefile,v $
#	$Author: jtkohl $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/util/imake.includes/Makefile,v 4.2 1989-04-11 10:30:57 jtkohl Exp $
#
# Copyright 1988 by the Massachusetts Institute of Technology.
#
# For copying and distribution information, please see the file
# <mit-copyright.h>.
#
# Makefile for copying Imake configuration files
LNSOURCES = \
	${SRCDIR}/util/imake.includes/Imake.rules \
	${SRCDIR}/util/imake.includes/Imake.template
CPSOURCES = ${SRCDIR}/util/imake.includes/config.Imakefile
DESTDIR =
SRCDIR = /mit/kerberos/src
CP = cp
LN = ln -s
RM = rm -f

config: $(SOURCES)
	-mkdir $(DESTDIR)/util/imake.includes
	for i in $(CPSOURCES); do \
		$(RM) ${DESTDIR}/util/imake.includes/`basename $$i`; \
		$(CP) $$i ${DESTDIR}/util/imake.includes/`basename $$i`; done
	for i in $(LNSOURCES); do \
		$(RM) ${DESTDIR}/util/imake.includes/`basename $$i`; \
		$(LN) $$i ${DESTDIR}/util/imake.includes/`basename $$i`; done

