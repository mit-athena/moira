#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/Makefile,v $
#	$Author: mar $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/Makefile,v 1.3 1988-02-08 15:13:29 mar Exp $
#
#	Makefile for SMS library.
#
LIBSRC=sms_auth.c sms_call.c sms_connect.c sms_data.c sms_init.c \
	sms_shutdown.c sms_query.c sms_param.c sms_access.c sms_misc.c \
	sms_al_filsys.c sms_al_pobox.c menu.c admin_call.c admin_util.c \
	sms_do_update.c fixname.c strs.c fixhost.c nfsparttype.c ureg_err.h

CFILES=${LIBSRC}

LIBOBJ=sms_auth.o sms_call.o sms_connect.o sms_data.o sms_et.o \
	sms_init.o sms_shutdown.o sms_query.o sms_param.o sms_access.o \
	sms_misc.o sms_al_filsys.o sms_al_pobox.o krb_et.o menu.o \
	admin_call.o admin_util.o admin_err.o sms_do_update.o \
	fixname.o strs.o fixhost.o nfsparttype.o ureg_err.o

COPTS= -O

INCDIRS=-I../include

CFLAGS= ${INCDIRS} ${COPTS}
LINTFLAGS= -uhv

ALL=libsms.a llib-lsms.ln libsms_p.a

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

krb_et.o: krb_et.et
	compile_et krb_et.et
	cp krb_et.o profiled/krb_et.o

sms_et.o: sms_et.et
	compile_et sms_et.et
	cp sms_et.o profiled/sms_et.o

sms_et.h: sms_et.o

ureg_err.o: ureg_err.et
	compile_et ureg_err.et
	cp ureg_err.o profiled/ureg_err.o

admin_err.o: admin_err.et
	compile_et admin_err.et
	cp admin_err.o profiled/admin_err.o

admin_err.h: admin_err.o

../include/sms_et.h: sms_et.h

clean:
	rm -f ${ALL}
	rm -f *.o *~ admin_err.h sms_et.h krb_et.h ureg_err.h profiled/*.o \#*
	rm -f *.PS *.aux *.dvi *.log
	rm -f *.bak


install:
	install -m 644 libsms.a ${DESTDIR}/usr/athena/lib/libsms.a
	ranlib ${DESTDIR}/usr/athena/lib/libsms.a
	install -m 644 libsms_p.a ${DESTDIR}/usr/athena/lib/libsms_p.a
	ranlib ${DESTDIR}/usr/athena/lib/libsms_p.a
	-mkdir ${DESTDIR}/usr/athena/lib/lint
	install -m 644 llib-lsms.ln ${DESTDIR}/usr/athena/lib/lint/llib-lsms.ln

TAGS:	$(CFILES)
	-etags $(CFILES)

lint:	${CFILES} llib-lsms.ln
	lint ${LINTFLAGS} ${INCDIRS} sms_main.c llib-lsms.ln

llib-lsms.ln:	${LIBSRC}
	lint -Csms $(LINTFLAGS) $(INCDIRS) ${LIBSRC}

depend: $(CFILES) sms_et.h admin_err.h
	$(CC) -M $(CFLAGS) $(CFILES) | \
	sed -e 's; ./; ;' \
	    -e ':loop' \
	    -e 's/\.\.\/[^ /]*\/\.\./../' \
	    -e 't loop' | \
	awk ' { if ($$1 != prev) { print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		       else rec = rec " " $$2 } } \
	      END { print rec } ' > makedep
	echo '/^# DO NOT DELETE THIS LINE/+1,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep
#
# the last constant line in the makefile should be...
# DO NOT DELETE THIS LINE

sms_auth.o: sms_auth.c sms_private.h sms_proto.h /usr/include/stdio.h
sms_auth.o: ../include/gdb.h /usr/include/sys/types.h /usr/include/sys/time.h
sms_auth.o: /usr/include/time.h ../include/sms.h ../include/sms_et.h
sms_auth.o: /usr/include/krb.h /usr/include/des.h
sms_call.o: sms_call.c sms_private.h sms_proto.h /usr/include/stdio.h
sms_call.o: ../include/gdb.h /usr/include/sys/types.h /usr/include/sys/time.h
sms_call.o: /usr/include/time.h ../include/sms.h ../include/sms_et.h
sms_connect.o: sms_connect.c sms_private.h sms_proto.h /usr/include/stdio.h
sms_connect.o: ../include/gdb.h /usr/include/sys/types.h
sms_connect.o: /usr/include/sys/time.h /usr/include/time.h ../include/sms.h
sms_connect.o: ../include/sms_et.h
sms_data.o: sms_data.c sms_private.h sms_proto.h /usr/include/stdio.h
sms_data.o: ../include/gdb.h /usr/include/sys/types.h /usr/include/sys/time.h
sms_data.o: /usr/include/time.h ../include/sms.h ../include/sms_et.h
sms_init.o: sms_init.c sms_private.h sms_proto.h /usr/include/stdio.h
sms_init.o: ../include/gdb.h /usr/include/sys/types.h /usr/include/sys/time.h
sms_init.o: /usr/include/time.h ../include/sms.h ../include/sms_et.h
sms_shutdown.o: sms_shutdown.c sms_private.h sms_proto.h /usr/include/stdio.h
sms_shutdown.o: ../include/gdb.h /usr/include/sys/types.h
sms_shutdown.o: /usr/include/sys/time.h /usr/include/time.h ../include/sms.h
sms_shutdown.o: ../include/sms_et.h
sms_query.o: sms_query.c sms_private.h sms_proto.h /usr/include/stdio.h
sms_query.o: ../include/gdb.h /usr/include/sys/types.h /usr/include/sys/time.h
sms_query.o: /usr/include/time.h ../include/sms.h ../include/sms_et.h
sms_param.o: sms_param.c /usr/include/sys/types.h /usr/include/netinet/in.h
sms_param.o: sms_private.h sms_proto.h /usr/include/stdio.h ../include/gdb.h
sms_param.o: /usr/include/sys/types.h /usr/include/sys/time.h
sms_param.o: /usr/include/time.h ../include/sms.h ../include/sms_et.h
sms_access.o: sms_access.c sms_private.h sms_proto.h /usr/include/stdio.h
sms_access.o: ../include/gdb.h /usr/include/sys/types.h /usr/include/sys/time.h
sms_access.o: /usr/include/time.h ../include/sms.h ../include/sms_et.h
sms_misc.o: sms_misc.c /usr/include/strings.h
sms_al_filsys.o: sms_al_filsys.c ../include/sms_et.h /usr/include/stdio.h
sms_al_pobox.o: sms_al_pobox.c ../include/sms_et.h /usr/include/stdio.h
sms_al_pobox.o: /usr/include/ctype.h
menu.o: menu.c /usr/include/sys/types.h /usr/include/stdio.h
menu.o: /usr/include/signal.h /usr/include/curses.h /usr/include/stdio.h
menu.o: /usr/include/sgtty.h /usr/include/sys/ioctl.h
menu.o: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
menu.o: /usr/include/ctype.h /usr/include/strings.h /usr/include/varargs.h
menu.o: ../include/com_err.h ../include/mit-sipb-copyright.h ../include/menu.h
admin_call.o: admin_call.c /usr/include/sys/errno.h /usr/include/sys/types.h
admin_call.o: /usr/include/sys/time.h /usr/include/time.h
admin_call.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
admin_call.o: /usr/include/sys/ttydev.h /usr/include/sys/socket.h
admin_call.o: /usr/include/netinet/in.h /usr/include/netdb.h
admin_call.o: /usr/include/strings.h /usr/include/stdio.h admin_err.h
admin_call.o: admin_server.h prot.h /usr/include/krb.h /usr/include/des.h
admin_util.o: admin_util.c /usr/include/sys/types.h /usr/include/sys/errno.h
admin_util.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
admin_util.o: /usr/include/sys/ttydev.h /usr/include/sys/signal.h
admin_util.o: /usr/include/sys/file.h /usr/include/sys/param.h
admin_util.o: /usr/include/machine/machparam.h /usr/include/signal.h
admin_util.o: /usr/include/sys/types.h /usr/include/stdio.h
admin_util.o: /usr/include/strings.h /usr/include/krb.h /usr/include/des.h
admin_util.o: /usr/include/setjmp.h /usr/include/pwd.h admin_server.h prot.h
admin_util.o: admin_err.h
sms_do_update.o: sms_do_update.c sms_private.h sms_proto.h /usr/include/stdio.h
sms_do_update.o: ../include/gdb.h /usr/include/sys/types.h
sms_do_update.o: /usr/include/sys/time.h /usr/include/time.h ../include/sms.h
sms_do_update.o: ../include/sms_et.h
fixname.o: fixname.c /usr/include/strings.h /usr/include/ctype.h
strs.o: strs.c /usr/include/sys/types.h /usr/include/strings.h
strs.o: /usr/include/ctype.h
fixhost.o: fixhost.c /usr/include/sys/types.h /usr/include/sys/socket.h
fixhost.o: /usr/include/netinet/in.h /usr/include/arpa/nameser.h
fixhost.o: /usr/include/arpa/resolv.h /usr/include/netdb.h /usr/include/stdio.h
fixhost.o: /usr/include/strings.h /usr/include/ctype.h
nfsparttype.o: nfsparttype.c ../include/sms.h ../include/sms_et.h
nfsparttype.o: /usr/include/stdio.h /usr/include/strings.h /usr/include/ctype.h
