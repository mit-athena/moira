#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/Makefile,v $
#	$Author: mar $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/Makefile,v 1.13 1990-05-02 18:45:11 mar Exp $
#
# Revised makefile for GDB library.
#

GDBCSRCS= gdb_struct.c gdb_stype.c gdb.c gdb_trans.c gdb_trans2.c \
	gdb_conn.c gdb_serv.c gdb_fserv.c gdb_db.c gdb_debug.c gdb_ops.c

GDBNONCSRCS= gdb.h gdb_lib.h dbserv.qc 

OTHERSRCS= test.c samp1a.c \
	tst.c tst2.c tcl.c tdbcl.c  tsr.c tfsr.c\
	tst3.c tst4.c  tst5.c tst6.c 

LIBOBJS = gdb.o gdb_struct.o gdb_stype.o gdb_trans.o gdb_trans2.o \
	gdb_conn.o gdb_ops.o gdb_serv.o gdb_fserv.o gdb_db.o gdb_debug.o

DOCS= library.mss gdb.mss gdb.prob gdb_ugd.mss slides.mss

SAMPLES= samp1.c 

MISC= Makefile README gdb_version

CFLAGS= -O -I../include

CCPROF= ${CC}

INGLIBS= /usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib

LINTFLAGS= -uhv -I../include

.c.o:
#	${CCPROF} -c -pg ${CFLAGS} $*.c
#	mv $*.o profiled/$*.o
	${CC} -c ${CFLAGS} $*.c

OPTS=

all: libgdb.a llib-lgdb.ln # libgdb_p.a 

libgdb.a: ${LIBOBJS}
	ar uv libgdb.a ${LIBOBJS}; ranlib libgdb.a

libgdb_p.a: ${LIBOBJS}
	(cd profiled; ar uv ../libgdb_p.a ${LIBOBJS}; ranlib ../libgdb_p.a)

lint:
	lint ${LINTFLAGS} ${GDBCSRCS} | \
	egrep -v "warning: possible pointer alignment problem"

llib-lgdb.ln: ${LIBSRC}
	lint ${LINTFLAGS} -Cgdb ${GDBCSRCS}

install:
	install -c -m 644 libgdb.a ${DESTDIR}/usr/athena/lib/libmrgdb.a
	ranlib ${DESTDIR}/usr/athena/lib/libmrgdb.a
#	install -c -m 644 libgdb_p.a ${DESTDIR}/usr/athena/lib/libmrgdb_p.a
#	ranlib ${DESTDIR}/usr/athena/lib/libmrgdb_p.a
	install -c -m 644 llib-lgdb.ln ${DESTDIR}/usr/lib/lint/llib-lmrgdb.ln

#
# Random demo programs
#
tst: tst.o libgdb.a 
	cc ${OPTS} -o tst tst.o libgdb.a

tsr: tsr.o  libgdb.a 
	cc ${OPTS} -o tsr tsr.o  libgdb.a

tfsr: tfsr.o  libgdb.a 
	cc ${OPTS} -o tfsr tfsr.o  libgdb.a

tcl: tcl.o  libgdb.a 
	cc ${OPTS} -o tcl tcl.o  libgdb.a

tdbcl: tdbcl.o  libgdb.a 
	cc ${OPTS} -o tdbcl tdbcl.o  libgdb.a

samp1a: samp1a.o  libgdb.a 
	cc ${OPTS} -o samp1a samp1a.o  libgdb.a

#
#	Documentation.
#

slides.PS: slides.mss
	scribe slides

gdb_ugd.PS: gdb_ugd.mss
	scribe gdb_ugd

library.PS: library.mss
	scribe library

ugdprint:  gdb_ugd.PS
	lpr  -Pln03-bldge40-4 gdb_ugd.PS
	echo 'GDB User Guide sent to PostScript printer'

libraryprint:  library.PS
	lpr  -Pln03-bldge40-4 library.PS
	echo 'GDB Library Reference Manual sent to PostScript printer'

slideprint:  slides.PS
	lpr  -Pln03-bldge40-4 slides.PS
	echo 'GDB slides sent to PostScript printer'

# 
# Cleanup.
#

clean:
	rm -f *.o libgdb.a libgdb_p.a tar.file *.PS *.err \#* *~ profiled/*.o
	rm -f llib-lgdb.ln TAGS

depend: $(GDBCSRCS)
	mkdep ${CFLAGS} ${GDBCSRCS}

# DO NOT DELETE THIS LINE -- mkdep uses it.

gdb_struct.o: gdb_struct.c ../include/mit-copyright.h /usr/include/stdio.h
gdb_struct.o: /usr/include/strings.h ../include/gdb.h
gdb_struct.o: /usr/include/sys/types.h /usr/include/sys/time.h
gdb_struct.o: /usr/include/sys/time.h
gdb_stype.o: gdb_stype.c ../include/mit-copyright.h /usr/include/stdio.h
gdb_stype.o: /usr/include/strings.h ../include/gdb.h /usr/include/sys/types.h
gdb_stype.o: /usr/include/sys/time.h /usr/include/sys/time.h
gdb_stype.o: /usr/include/netinet/in.h
gdb.o: gdb.c /usr/include/stdio.h /usr/include/strings.h
gdb.o: /usr/include/signal.h /usr/include/pwd.h ../include/gdb.h
gdb.o: /usr/include/sys/types.h /usr/include/sys/time.h
gdb.o: /usr/include/sys/time.h gdb_lib.h /usr/include/errno.h
gdb_trans.o: gdb_trans.c ../include/mit-copyright.h /usr/include/sys/types.h
gdb_trans.o: /usr/include/strings.h /usr/include/stdio.h
gdb_trans.o: /usr/include/varargs.h /usr/include/errno.h ../include/gdb.h
gdb_trans.o: /usr/include/sys/time.h /usr/include/sys/time.h
gdb_trans2.o: gdb_trans2.c ../include/mit-copyright.h
gdb_trans2.o: /usr/include/sys/types.h /usr/include/errno.h
gdb_trans2.o: /usr/include/stdio.h ../include/gdb.h /usr/include/sys/time.h
gdb_trans2.o: /usr/include/sys/time.h /usr/include/sys/uio.h
gdb_trans2.o: /usr/include/sys/socket.h
gdb_conn.o: gdb_conn.c ../include/mit-copyright.h /usr/include/stdio.h
gdb_conn.o: /usr/include/strings.h ../include/gdb.h /usr/include/sys/types.h
gdb_conn.o: /usr/include/sys/time.h /usr/include/sys/time.h
gdb_conn.o: /usr/include/sys/uio.h /usr/include/sys/socket.h
gdb_conn.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
gdb_conn.o: /usr/include/sys/ttydev.h /usr/include/netinet/in.h
gdb_conn.o: /usr/include/netdb.h /usr/include/errno.h
gdb_serv.o: gdb_serv.c ../include/mit-copyright.h /usr/include/stdio.h
gdb_serv.o: /usr/include/strings.h ../include/gdb.h /usr/include/sys/types.h
gdb_serv.o: /usr/include/sys/time.h /usr/include/sys/time.h
gdb_serv.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
gdb_serv.o: /usr/include/sys/ttydev.h
gdb_fserv.o: gdb_fserv.c ../include/mit-copyright.h /usr/include/stdio.h
gdb_fserv.o: /usr/include/sys/types.h /usr/include/sys/uio.h
gdb_fserv.o: /usr/include/sys/socket.h /usr/include/sys/wait.h
gdb_fserv.o: /usr/include/sys/signal.h ../include/gdb.h
gdb_fserv.o: /usr/include/sys/time.h /usr/include/sys/time.h
gdb_fserv.o: /usr/include/sys/resource.h
gdb_db.o: gdb_db.c ../include/mit-copyright.h /usr/include/stdio.h
gdb_db.o: /usr/include/strings.h ../include/gdb.h /usr/include/sys/types.h
gdb_db.o: /usr/include/sys/time.h /usr/include/sys/time.h
gdb_debug.o: gdb_debug.c ../include/mit-copyright.h /usr/include/stdio.h
gdb_debug.o: ../include/gdb.h /usr/include/sys/types.h
gdb_debug.o: /usr/include/sys/time.h /usr/include/sys/time.h
gdb_ops.o: gdb_ops.c ../include/mit-copyright.h /usr/include/stdio.h
gdb_ops.o: ../include/gdb.h /usr/include/sys/types.h /usr/include/sys/time.h
gdb_ops.o: /usr/include/sys/time.h /usr/include/netinet/in.h
gdb_ops.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
gdb_ops.o: /usr/include/sys/ttydev.h
