#
# Makefile for error-table routines
#
# Copyright 1987, 1989 MIT Student Information Processing Board
# For copyright info, see mit-sipb-copyright.h.
#
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/util/et/Makefile,v 1.1 1996-07-08 22:28:45 danw Exp $
#	$Locker:  $
#

CFLAGS=	-O -I../../include
LINTFLAGS= -uhvpb
LINTFILES= error_message.c et_name.c init_et.c com_err.c
LIBOBJS= error_message.o et_name.o init_et.o com_err.o
DEPEND=$(BUILDTOP)/util/makedepend/makedepend

BINDIR=/usr/athena
INCDIR=/usr/include
LIBDIR=/usr/athena/lib
DOCDIR=/usr/doc/athena
DESTDIR=

FILES=	Makefile et_name.c error_message.c compile_et.c \
		error_table.y et_lex.lex.l init_et.c \
		com_err.c com_err.h \
		error_table.h mit-sipb-copyright.h \
		test.c test1.et test2.et \
		compiler.h internal.h

CFILES=	compile_et.c error_table.c error_message.c et_name.c \
	init_et.c com_err.c

#
# what to build...
#
all:	compile_et libcom_err.a

lint:	llib-lcom_err.ln

archive:	et.tar

doc:	com_err.dvi

#
# rules
#
.SUFFIXES: .h .c .et .ps .x9700 .mss .dvi .texinfo

.et.c:
	./compile_et $*.et

.et.h:
	./compile_et $*.et

.texinfo.dvi:
	tex $<

.dvi.ps:
	rm -f $@.new
	dvi2ps -r $< > $@.new
	mv $@.new $@

.c.o:
	${CC} -c ${CFLAGS} $*.c

#
# real entries...
#
compile_et:	compile_et.o error_table.o
	${CC} ${CFLAGS} -o compile_et compile_et.o error_table.o -ll

et.tar:	${FILES}
	rm -f et.tar
	tar cfrlv et.tar ${FILES}

tags:	TAGS

com_err.ps : com_err.dvi
com_err.dvi: com_err.texinfo

install: all
#	install -c -s compile_et ${DESTDIR}${BINDIR}/compile_et
#	install -c -m 444 com_err.h ${DESTDIR}${INCDIR}/com_err.h
#	install -c -m 444 mit-sipb-copyright.h \
#				${DESTDIR}${INCDIR}/mit-sipb-copyright.h
#	install -c libcom_err.a ${DESTDIR}${LIBDIR}/libcom_err.a
#	ranlib ${DESTDIR}${LIBDIR}/libcom_err.a
#	install -c libcom_err_p.a ${DESTDIR}${LIBDIR}/libcom_err_p.a
#	ranlib ${DESTDIR}${LIBDIR}/libcom_err_p.a
#	install -c com_err.texinfo ${DESTDIR}${DOCDIR}/com_err.texinfo
#	install -c com_err.3 ${DESTDIR}/usr/man/man3/com_err.3
#	install -c compile_et.1 ${DESTDIR}/usr/man/man1/compile_et.1

config:
	-mkdir $(DESTDIR)/util/et
	for i in $(FILES); do \
		rm -f ${DESTDIR}/util/et/`basename $$i`; \
		ln -s $(SRCDIR)/util/et/$$i ${DESTDIR}/util/et/`basename $$i`; done
	rm -f ${DESTDIR}/util/et/Makefile
	cp ${SRCDIR}/util/et/Makefile ${DESTDIR}/util/et/Makefile
	chmod 644 ${DESTDIR}/util/et/Makefile


TAGS:	et_name.c error_message.c compile_et.c error_table.c \
		lex.yy.c perror.c init_et.c
	etags et_name.c error_message.c compile_et.c \
		error_table.c perror.c init_et.c

libcom_err.a:	$(LIBOBJS)
	ar cruv libcom_err.a $(LIBOBJS)
	ranlib libcom_err.a

libcom_err_p.a:	$(LIBOBJS)
	(cd profiled; ar uv ../libcom_err_p.a $(LIBOBJS); \
		ranlib ../libcom_err_p.a)

libcom_err.o:	$(LIBOBJS)
	ld -r -s -o libcom_err.o $(LIBOBJS)
	chmod -x libcom_err.o

llib-lcom_err.ln: $(LINTFILES)
	lint -Ccom_err $(LINTFLAGS) $(LINTFILES)

clean:
	rm -f *~ \#* *.bak \
		*.otl *.aux *.toc *.PS *.dvi *.x9700 *.ps \
		*.cp *.fn *.ky *.log *.pg *.tp *.vr \
		*.o profiled/*.o libcom_err.a libcom_err_p.a \
		com_err.o compile_et \
		et.ar TAGS y.tab.c lex.yy.c error_table.c \
		et_lex.lex.c \
		test1.h test1.c test2.h test2.c test \
		eddep makedep

# for testing
test:	test.o test1.o test2.o libcom_err.a
	cc ${CFLAGS} -o test test.o test1.o test2.o libcom_err.a
test.o:	test1.h test2.h
test1.o : test1.c
test1.c : test1.et
test2.o : test2.c
test2.c : test2.et
# 'make depend' code
depend: ${CFILES} et_lex.lex.c
	@${DEPEND} ${CFLAGS} ${CFILES}

# DO NOT DELETE THIS LINE -- make depend depends on it.

compile_et.o: /usr/include/stdio.h /usr/include/sys/types.h
compile_et.o: /usr/include/sys/file.h /usr/include/string.h
compile_et.o: /usr/include/strings.h /usr/include/sys/param.h
compile_et.o: /usr/include/machine/machparam.h /usr/include/sys/signal.h
compile_et.o: ../../include/mit-sipb-copyright.h compiler.h
error_table.o: /usr/include/stdio.h /usr/include/string.h
error_table.o: /usr/include/strings.h /usr/include/assert.h
error_table.o: /usr/include/ctype.h /usr/include/sys/types.h
error_table.o: /usr/include/sys/time.h /usr/include/sys/time.h internal.h
error_table.o: ../../include/mit-sipb-copyright.h error_table.h et_lex.lex.c
error_message.o: /usr/include/stdio.h error_table.h
error_message.o: ../../include/mit-sipb-copyright.h internal.h
et_name.o: error_table.h ../../include/mit-sipb-copyright.h internal.h
init_et.o: /usr/include/stdio.h error_table.h
init_et.o: ../../include/mit-sipb-copyright.h
com_err.o: /usr/include/stdio.h ../../include/mit-sipb-copyright.h
com_err.o: /usr/include/varargs.h error_table.h internal.h com_err.h
