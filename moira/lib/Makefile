#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/Makefile,v $
#	$Author: mar $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/Makefile,v 1.16 1990-05-02 15:56:06 mar Exp $
#
#	Makefile for Moira library.
#
LIBSRC=mr_auth.c mr_call.c mr_connect.c mr_data.c mr_init.c \
	mr_query.c mr_param.c mr_access.c mr_ops.c \
	fixname.c strs.c fixhost.c nfsparttype.c sq.c hash.c \
	idno.c critical.c

CFILES=${LIBSRC}

LIBOBJ=mr_auth.o mr_call.o mr_connect.o mr_data.o mr_init.o \
	mr_query.o mr_param.o mr_access.o mr_ops.o \
	fixname.o strs.o fixhost.o nfsparttype.o sq.o hash.o \
	idno.o critical.o mr_et.o ureg_err.o krb_et.o

COPTS= -O

INCDIRS=-I../include

CFLAGS= ${INCDIRS} ${COPTS}
LINTFLAGS= -uhv

ALL=libmoira.a llib-lmoira.ln # libmoira_p.a

.c.o:
#	${CC} -c -pg ${CFLAGS} $*.c
#	ld -x -r $*.o
#	mv a.out profiled/$*.o
	${CC} -c ${CFLAGS} $*.c
	ld -x -r $*.o
	mv a.out $*.o

all: ${ALL}

libmoira.a: ${LIBOBJ}
	ar cruv $@ `lorder ${LIBOBJ} | tsort`
	ranlib $@

libmoira_p.a: ${LIBOBJ}
	(cd profiled; ar cruv ../libmoira_p.a ${LIBOBJ}; ranlib ../libmoira_p.a)

mrlib.dvi: mrlib.tex
	latex mrlib.tex

krb_et.c krb_et.h: krb_et.et ../et/compile_et
	../et/compile_et krb_et.et

mr_et.c mr_et.h: mr_et.et ../et/compile_et
	../et/compile_et mr_et.et

ureg_err.c ureg_err.h: ureg_err.et ../et/compile_et
	../et/compile_et ureg_err.et

../et/compile_et:
	cd ../et; make compile_et

../include/mr_et.h: mr_et.h

clean:
	rm -f ${ALL}
	rm -f *.o *~ mr_et.h krb_et.h ureg_err.h profiled/*.o \#*
	rm -f krb_et.c mr_et.c ureg_err.c
	rm -f *.PS *.aux *.dvi *.log
	rm -f *.bak


install:
	install -c -m 644 libmoira.a ${DESTDIR}/usr/athena/lib/libmoira.a
	ranlib ${DESTDIR}/usr/athena/lib/libmoira.a
#	install -c -m 644 libmoira_p.a ${DESTDIR}/usr/athena/lib/libmoira_p.a
#	ranlib ${DESTDIR}/usr/athena/lib/libmoira_p.a
	install -c -m 644 llib-lmoira.ln ${DESTDIR}/usr/lib/lint/llib-lmoira.ln

TAGS:	$(CFILES)
	-etags $(CFILES)

lint:	${CFILES} llib-lmoira.ln
	lint ${LINTFLAGS} ${INCDIRS} mr_main.c llib-lmoira.ln

llib-lmoira.ln:	${LIBSRC}
	lint -Cmoira $(LINTFLAGS) $(INCDIRS) ${LIBSRC}

depend: $(CFILES) mr_et.h ureg_err.h krb_et.h
	mkdep ${CFLAGS} ${CFILES}

# DO NOT DELETE THIS LINE -- mkdep uses it.

mr_auth.o: mr_auth.c ../include/mit-copyright.h mr_private.h
mr_auth.o: ../include/mr_proto.h /usr/include/stdio.h ../include/gdb.h
mr_auth.o: /usr/include/sys/types.h /usr/include/sys/time.h
mr_auth.o: /usr/include/sys/time.h ../include/moira.h mr_et.h
mr_auth.o: /usr/include/ctype.h ../include/krb.h ../include/des.h krb_et.h
mr_call.o: mr_call.c ../include/mit-copyright.h mr_private.h
mr_call.o: ../include/mr_proto.h /usr/include/stdio.h ../include/gdb.h
mr_call.o: /usr/include/sys/types.h /usr/include/sys/time.h
mr_call.o: /usr/include/sys/time.h ../include/moira.h mr_et.h
mr_connect.o: mr_connect.c ../include/mit-copyright.h mr_private.h
mr_connect.o: ../include/mr_proto.h /usr/include/stdio.h ../include/gdb.h
mr_connect.o: /usr/include/sys/types.h /usr/include/sys/time.h
mr_connect.o: /usr/include/sys/time.h ../include/moira.h mr_et.h
mr_connect.o: ../include/moira_site.h /usr/include/strings.h
mr_data.o: mr_data.c ../include/mit-copyright.h mr_private.h
mr_data.o: ../include/mr_proto.h /usr/include/stdio.h ../include/gdb.h
mr_data.o: /usr/include/sys/types.h /usr/include/sys/time.h
mr_data.o: /usr/include/sys/time.h ../include/moira.h mr_et.h
mr_init.o: mr_init.c ../include/mit-copyright.h mr_private.h
mr_init.o: ../include/mr_proto.h /usr/include/stdio.h ../include/gdb.h
mr_init.o: /usr/include/sys/types.h /usr/include/sys/time.h
mr_init.o: /usr/include/sys/time.h ../include/moira.h mr_et.h
mr_query.o: mr_query.c ../include/mit-copyright.h mr_private.h
mr_query.o: ../include/mr_proto.h /usr/include/stdio.h ../include/gdb.h
mr_query.o: /usr/include/sys/types.h /usr/include/sys/time.h
mr_query.o: /usr/include/sys/time.h ../include/moira.h mr_et.h
mr_param.o: mr_param.c ../include/mit-copyright.h /usr/include/sys/types.h
mr_param.o: /usr/include/netinet/in.h mr_private.h ../include/mr_proto.h
mr_param.o: /usr/include/stdio.h ../include/gdb.h /usr/include/sys/time.h
mr_param.o: /usr/include/sys/time.h ../include/moira.h mr_et.h
mr_access.o: mr_access.c ../include/mit-copyright.h mr_private.h
mr_access.o: ../include/mr_proto.h /usr/include/stdio.h ../include/gdb.h
mr_access.o: /usr/include/sys/types.h /usr/include/sys/time.h
mr_access.o: /usr/include/sys/time.h ../include/moira.h mr_et.h
mr_ops.o: mr_ops.c ../include/mit-copyright.h mr_private.h
mr_ops.o: ../include/mr_proto.h /usr/include/stdio.h ../include/gdb.h
mr_ops.o: /usr/include/sys/types.h /usr/include/sys/time.h
mr_ops.o: /usr/include/sys/time.h ../include/moira.h mr_et.h
fixname.o: fixname.c ../include/mit-copyright.h /usr/include/strings.h
fixname.o: /usr/include/ctype.h
strs.o: strs.c ../include/mit-copyright.h /usr/include/sys/types.h
strs.o: /usr/include/strings.h /usr/include/ctype.h
fixhost.o: fixhost.c ../include/mit-copyright.h /usr/include/sys/types.h
fixhost.o: /usr/include/sys/socket.h /usr/include/netinet/in.h
fixhost.o: /usr/include/arpa/nameser.h /usr/include/arpa/resolv.h
fixhost.o: /usr/include/netdb.h /usr/include/stdio.h /usr/include/strings.h
fixhost.o: /usr/include/ctype.h
nfsparttype.o: nfsparttype.c ../include/mit-copyright.h ../include/moira.h
nfsparttype.o: mr_et.h /usr/include/stdio.h /usr/include/strings.h
nfsparttype.o: /usr/include/ctype.h
sq.o: sq.c ../include/mit-copyright.h ../include/moira.h mr_et.h
hash.o: hash.c ../include/mit-copyright.h /usr/include/ctype.h
hash.o: ../include/moira.h mr_et.h
idno.o: idno.c ../include/mit-copyright.h /usr/include/strings.h
idno.o: /usr/include/ctype.h
critical.o: critical.c ../include/mit-copyright.h /usr/include/stdio.h
critical.o: /usr/include/sys/file.h ../include/moira_site.h
critical.o: /usr/include/zephyr/zephyr.h /usr/include/zephyr/mit-copyright.h
critical.o: /usr/include/zephyr/zephyr_err.h
critical.o: /usr/include/zephyr/zephyr_conf.h /usr/include/errno.h
critical.o: /usr/include/sys/types.h /usr/include/netinet/in.h
critical.o: /usr/include/sys/time.h /usr/include/sys/time.h ../include/krb.h
critical.o: ../include/des.h
