#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/util/imake.includes/Makefile,v $
#	$Author: danw $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/util/imake.includes/Makefile,v 4.4 1997-01-29 23:35:56 danw Exp $
#
# Copyright 1988 by the Massachusetts Institute of Technology.
#
# For copying and distribution information, please see the file
# <mit-copyright.h>.
#
# Makefile for copying Imake configuration files
LNSOURCES = \
	${SRCDIR}/util/imake.includes/Imake.rules \
	${SRCDIR}/util/imake.includes/Imake.tmpl
CPSOURCES = ${SRCDIR}/util/imake.includes/config.Imakefile
DESTDIR =
SRCDIR = /mit/moiradev/src
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
	$(RM) ${DESTDIR}/util/imake.includes/config.Imakefile
	sed -e "s|/mit/moiradev/src|${SRCDIR}|" \
		${SRCDIR}/util/imake.includes/config.Imakefile \
		> ${DESTDIR}/util/imake.includes/config.Imakefile
