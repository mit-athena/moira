#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/Makefile,v $
#	$Author: mar $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/Makefile,v 1.12 1990-02-15 14:22:15 mar Exp $
#
#	Makefile for SMS library.
#
LIBSRC=sms_auth.c sms_call.c sms_connect.c sms_data.c sms_init.c \
	sms_query.c sms_param.c sms_access.c sms_ops.c \
	fixname.c strs.c fixhost.c nfsparttype.c sq.c hash.c \
	idno.c critical.c

CFILES=${LIBSRC}

LIBOBJ=sms_auth.o sms_call.o sms_connect.o sms_data.o sms_init.o \
	sms_query.o sms_param.o sms_access.o sms_ops.o \
	fixname.o strs.o fixhost.o nfsparttype.o sq.o hash.o \
	idno.o critical.o sms_et.o ureg_err.o krb_et.o

COPTS= -O

INCDIRS=-I../include

CFLAGS= ${INCDIRS} ${COPTS}
LINTFLAGS= -uhv

ALL=libsms.a llib-lsms.ln # libsms_p.a

.c.o:
	${CC} -c -pg ${CFLAGS} $*.c
	ld -x -r $*.o
	mv a.out profiled/$*.o
	${CC} -c ${CFLAGS} $*.c
	ld -x -r $*.o
	mv a.out $*.o

all: ${ALL}

libsms.a: ${LIBOBJ}
	ar cruv $@ `lorder ${LIBOBJ} | tsort`
	ranlib $@
	(cd profiled; ar cruv ../libsms_p.a ${LIBOBJ}; ranlib ../libsms_p.a)

smslib.dvi: smslib.tex
	latex smslib.tex

krb_et.c krb_et.h: krb_et.et ../et/compile_et
	../et/compile_et krb_et.et

sms_et.c sms_et.h: sms_et.et ../et/compile_et
	../et/compile_et sms_et.et

ureg_err.c ureg_err.h: ureg_err.et ../et/compile_et
	../et/compile_et ureg_err.et

../et/compile_et:
	cd ../et; make compile_et

../include/sms_et.h: sms_et.h

clean:
	rm -f ${ALL}
	rm -f *.o *~ sms_et.h krb_et.h ureg_err.h profiled/*.o \#*
	rm -f krb_et.c sms_et.c ureg_err.c
	rm -f *.PS *.aux *.dvi *.log
	rm -f *.bak


install:
	install -c -m 644 libsms.a ${DESTDIR}/usr/athena/lib/libsms.a
	ranlib ${DESTDIR}/usr/athena/lib/libsms.a
	install -c -m 644 libsms_p.a ${DESTDIR}/usr/athena/lib/libsms_p.a
	ranlib ${DESTDIR}/usr/athena/lib/libsms_p.a
	install -c -m 644 llib-lsms.ln ${DESTDIR}/usr/lib/lint/llib-lsms.ln

TAGS:	$(CFILES)
	-etags $(CFILES)

lint:	${CFILES} llib-lsms.ln
	lint ${LINTFLAGS} ${INCDIRS} sms_main.c llib-lsms.ln

llib-lsms.ln:	${LIBSRC}
	lint -Csms $(LINTFLAGS) $(INCDIRS) ${LIBSRC}

depend: $(CFILES) sms_et.h ureg_err.h krb_et.h
	mkdep ${CFLAGS} ${CFILES}

# DO NOT DELETE THIS LINE -- mkdep uses it.

sms_auth.o: sms_auth.c ../include/mit-copyright.h sms_private.h
sms_auth.o: ../include/sms_proto.h /usr/include/stdio.h ../include/gdb.h
sms_auth.o: /usr/include/sys/types.h /usr/include/sys/time.h
sms_auth.o: /usr/include/sys/time.h ../include/sms.h sms_et.h
sms_auth.o: ../include/krb.h ../include/des.h krb_et.h
sms_call.o: sms_call.c ../include/mit-copyright.h sms_private.h
sms_call.o: ../include/sms_proto.h /usr/include/stdio.h ../include/gdb.h
sms_call.o: /usr/include/sys/types.h /usr/include/sys/time.h
sms_call.o: /usr/include/sys/time.h ../include/sms.h sms_et.h
sms_connect.o: sms_connect.c ../include/mit-copyright.h sms_private.h
sms_connect.o: ../include/sms_proto.h /usr/include/stdio.h ../include/gdb.h
sms_connect.o: /usr/include/sys/types.h /usr/include/sys/time.h
sms_connect.o: /usr/include/sys/time.h ../include/sms.h sms_et.h
sms_connect.o: ../include/sms_app.h /usr/include/strings.h
sms_data.o: sms_data.c ../include/mit-copyright.h sms_private.h
sms_data.o: ../include/sms_proto.h /usr/include/stdio.h ../include/gdb.h
sms_data.o: /usr/include/sys/types.h /usr/include/sys/time.h
sms_data.o: /usr/include/sys/time.h ../include/sms.h sms_et.h
sms_init.o: sms_init.c ../include/mit-copyright.h sms_private.h
sms_init.o: ../include/sms_proto.h /usr/include/stdio.h ../include/gdb.h
sms_init.o: /usr/include/sys/types.h /usr/include/sys/time.h
sms_init.o: /usr/include/sys/time.h ../include/sms.h sms_et.h
sms_query.o: sms_query.c ../include/mit-copyright.h sms_private.h
sms_query.o: ../include/sms_proto.h /usr/include/stdio.h ../include/gdb.h
sms_query.o: /usr/include/sys/types.h /usr/include/sys/time.h
sms_query.o: /usr/include/sys/time.h ../include/sms.h sms_et.h
sms_param.o: sms_param.c ../include/mit-copyright.h /usr/include/sys/types.h
sms_param.o: /usr/include/netinet/in.h sms_private.h ../include/sms_proto.h
sms_param.o: /usr/include/stdio.h ../include/gdb.h /usr/include/sys/time.h
sms_param.o: /usr/include/sys/time.h ../include/sms.h sms_et.h
sms_access.o: sms_access.c ../include/mit-copyright.h sms_private.h
sms_access.o: ../include/sms_proto.h /usr/include/stdio.h ../include/gdb.h
sms_access.o: /usr/include/sys/types.h /usr/include/sys/time.h
sms_access.o: /usr/include/sys/time.h ../include/sms.h sms_et.h
sms_ops.o: sms_ops.c ../include/mit-copyright.h sms_private.h
sms_ops.o: ../include/sms_proto.h /usr/include/stdio.h ../include/gdb.h
sms_ops.o: /usr/include/sys/types.h /usr/include/sys/time.h
sms_ops.o: /usr/include/sys/time.h ../include/sms.h sms_et.h
fixname.o: fixname.c ../include/mit-copyright.h /usr/include/strings.h
fixname.o: /usr/include/ctype.h
strs.o: strs.c ../include/mit-copyright.h /usr/include/sys/types.h
strs.o: /usr/include/strings.h /usr/include/ctype.h
fixhost.o: fixhost.c ../include/mit-copyright.h /usr/include/sys/types.h
fixhost.o: /usr/include/sys/socket.h /usr/include/netinet/in.h
fixhost.o: /usr/include/arpa/nameser.h /usr/include/arpa/resolv.h
fixhost.o: /usr/include/netdb.h /usr/include/stdio.h /usr/include/strings.h
fixhost.o: /usr/include/ctype.h
nfsparttype.o: nfsparttype.c ../include/mit-copyright.h ../include/sms.h
nfsparttype.o: sms_et.h /usr/include/stdio.h /usr/include/strings.h
nfsparttype.o: /usr/include/ctype.h
sq.o: sq.c ../include/mit-copyright.h ../include/sms.h sms_et.h
hash.o: hash.c ../include/mit-copyright.h /usr/include/ctype.h
hash.o: ../include/sms.h sms_et.h
idno.o: idno.c ../include/mit-copyright.h /usr/include/strings.h
idno.o: /usr/include/ctype.h
critical.o: critical.c ../include/mit-copyright.h /usr/include/stdio.h
critical.o: /usr/include/sys/file.h /usr/include/zephyr/zephyr.h
critical.o: /usr/include/zephyr/mit-copyright.h
critical.o: /usr/include/zephyr/zephyr_err.h
critical.o: /usr/include/zephyr/zephyr_conf.h /usr/include/errno.h
critical.o: /usr/include/sys/types.h /usr/include/netinet/in.h
critical.o: /usr/include/sys/time.h /usr/include/sys/time.h ../include/krb.h
critical.o: ../include/des.h ../include/sms_app.h
