#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/Makefile,v $
#	$Author: mar $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/Makefile,v 1.7 1989-07-13 17:13:49 mar Exp $
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

MISC= Makefile .saberinit README gdb_version

CFLAGS= -O -I../include

CCPROF= ${CC}

FTP= /mit/ftp/gdb

INGLIBS= /usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib

LINTFLAGS= -uhv

.c.o:
	${CCPROF} -c -pg ${CFLAGS} $*.c
	mv $*.o profiled/$*.o
	${CC} -c ${CFLAGS} $*.c

OPTS=

all: libgdb.a libgdb_p.a llib-lgdb.ln

libgdb.a: ${LIBOBJS}
	ar uv libgdb.a ${LIBOBJS}; ranlib libgdb.a

libgdb_p.a: ${LIBOBJS}
	(cd profiled; ar uv ../libgdb_p.a ${LIBOBJS}; ranlib ../libgdb_p.a)

lint:
	lint -huv ${GDBCSRCS} | \
	egrep -v "warning: possible pointer alignment problem"

llib-lgdb.ln: ${LIBSRC}
	lint -Cgdb ${GDBCSRCS}

install:
	install -m 644 libgdb.a ${DESTDIR}/usr/athena/lib/libsmsgdb.a
	ranlib ${DESTDIR}/usr/athena/lib/libsmsgdb.a
	install -m 644 libgdb_p.a ${DESTDIR}/usr/athena/lib/libsmsgdb_p.a
	ranlib ${DESTDIR}/usr/athena/lib/libsmsgdb_p.a
	-mkdir ${DESTDIR}/usr/athena/lib/lint
	install -m 644 llib-lgdb.ln ${DESTDIR}/usr/athena/lib/lint/llib-lsmsgdb.ln

#
# Random demo programs
#
tst: tst.o libgdb.a 
	cc ${OPTS} -o tst tst.o libgdb.a

stest: stest.o  libgdb.a 
	cc ${OPTS} -o stest stest.o libgdb.a

tst2: tst2.o  libgdb.a 
	cc ${OPTS} -o tst2 tst2.o libgdb.a

tst3: tst3.o  libgdb.a 
	cc ${OPTS} -o tst3 tst3.o libgdb.a

tst4: tst4.o  libgdb.a 
	cc ${OPTS} -o tst4 tst4.o  libgdb.a

tst5: tst5.o  libgdb.a 
	cc ${OPTS} -o tst5 tst5.o  libgdb.a

tst6: tst6.o  libgdb.a 
	cc ${OPTS} -o tst6 tst6.o  libgdb.a

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
	
dbserv: dbserv.o  libgdb.a 
	cc ${OPTS} -o dbserv dbserv.o libgdb.a ${INGLIBS} -lm
	chmod 6755 dbserv

dbserv.c: dbserv.qc 
	/usr/rtingres/bin/eqc -d -fdbserv.c dbserv.qc

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
# Distribution.
#

tar.file: ${GDBCSRCS} ${GDBNONCSRCS} ${OTHERSRCS} ${DOCS} ${SAMPLES} ${MISC}
	rm -f tar.file
	tar cvf tar.file ${GDBCSRCS} ${GDBNONCSRCS} ${OTHERSRCS} \
		${DOCS} ${SAMPLES} ${MISC}
	chmod 644 tar.file

archive: tar.file
	rcp tar.file "menelaus:~noah/gdb/tar.file"

ftp: tar.file
	rm -f ${FTP}/tar.file.Z ${FTP}/README
	compress -c tar.file > ${FTP}/tar.file.Z
	cp README ${FTP}/README
	chmod 755 ${FTP}/tar.file.Z ${FTP}/README

# 
# Cleanup.
#

clean:
	rm -f *.o libgdb.a libgdb_p.a tar.file *.PS *.err \#* *~ profiled/*.o

depend: $(GDBCSRCS)
	$(CC) -M $(CFLAGS) $(GDBCSRCS) | \
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

gdb_struct.o: gdb_struct.c ../include/mit-copyright.h /usr/include/stdio.h
gdb_struct.o: /usr/include/strings.h ../include/gdb.h /usr/include/sys/types.h
gdb_struct.o: /usr/include/sys/time.h /usr/include/time.h
gdb_stype.o: gdb_stype.c ../include/mit-copyright.h /usr/include/stdio.h
gdb_stype.o: /usr/include/strings.h ../include/gdb.h /usr/include/sys/types.h
gdb_stype.o: /usr/include/sys/time.h /usr/include/time.h
gdb_stype.o: /usr/include/netinet/in.h
gdb.o: gdb.c /usr/include/stdio.h /usr/include/strings.h /usr/include/signal.h
gdb.o: /usr/include/pwd.h ../include/gdb.h /usr/include/sys/types.h
gdb.o: /usr/include/sys/time.h /usr/include/time.h gdb_lib.h
gdb.o: /usr/include/errno.h
gdb_trans.o: gdb_trans.c ../include/mit-copyright.h /usr/include/sys/types.h
gdb_trans.o: /usr/include/strings.h /usr/include/stdio.h /usr/include/varargs.h
gdb_trans.o: /usr/include/errno.h ../include/gdb.h /usr/include/sys/types.h
gdb_trans.o: /usr/include/sys/time.h /usr/include/time.h
gdb_trans2.o: gdb_trans2.c ../include/mit-copyright.h /usr/include/sys/types.h
gdb_trans2.o: /usr/include/errno.h /usr/include/stdio.h ../include/gdb.h
gdb_trans2.o: /usr/include/sys/types.h /usr/include/sys/time.h
gdb_trans2.o: /usr/include/time.h /usr/include/sys/uio.h
gdb_trans2.o: /usr/include/sys/socket.h
gdb_conn.o: gdb_conn.c ../include/mit-copyright.h /usr/include/stdio.h
gdb_conn.o: /usr/include/strings.h ../include/gdb.h /usr/include/sys/types.h
gdb_conn.o: /usr/include/sys/time.h /usr/include/time.h
gdb_conn.o: /usr/include/sys/types.h /usr/include/sys/uio.h
gdb_conn.o: /usr/include/sys/socket.h /usr/include/sys/ioctl.h
gdb_conn.o: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
gdb_conn.o: /usr/include/netinet/in.h /usr/include/netdb.h /usr/include/errno.h
gdb_serv.o: gdb_serv.c ../include/mit-copyright.h /usr/include/stdio.h
gdb_serv.o: /usr/include/strings.h ../include/gdb.h /usr/include/sys/types.h
gdb_serv.o: /usr/include/sys/time.h /usr/include/time.h
gdb_serv.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
gdb_serv.o: /usr/include/sys/ttydev.h
gdb_fserv.o: gdb_fserv.c ../include/mit-copyright.h /usr/include/stdio.h
gdb_fserv.o: /usr/include/sys/types.h /usr/include/sys/uio.h
gdb_fserv.o: /usr/include/sys/socket.h /usr/include/sys/wait.h
gdb_fserv.o: /usr/include/sys/signal.h ../include/gdb.h
gdb_fserv.o: /usr/include/sys/types.h /usr/include/sys/time.h
gdb_fserv.o: /usr/include/time.h /usr/include/sys/resource.h
gdb_db.o: gdb_db.c ../include/mit-copyright.h /usr/include/stdio.h
gdb_db.o: /usr/include/strings.h ../include/gdb.h /usr/include/sys/types.h
gdb_db.o: /usr/include/sys/time.h /usr/include/time.h
gdb_debug.o: gdb_debug.c ../include/mit-copyright.h /usr/include/stdio.h
gdb_debug.o: ../include/gdb.h /usr/include/sys/types.h /usr/include/sys/time.h
gdb_debug.o: /usr/include/time.h
gdb_ops.o: gdb_ops.c ../include/mit-copyright.h /usr/include/stdio.h
gdb_ops.o: ../include/gdb.h /usr/include/sys/types.h /usr/include/sys/time.h
gdb_ops.o: /usr/include/time.h /usr/include/netinet/in.h
gdb_ops.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
gdb_ops.o: /usr/include/sys/ttydev.h
