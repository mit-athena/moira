#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/Makefile,v $
#	$Author: kostas $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/Makefile,v 1.2 1990-10-02 09:42:16 kostas Exp $
#

CFILES = startmoira.c mr_main.c mr_sauth.c mr_scall.c \
	mr_srvdata.c mr_shutdown.c mr_util.c qrtn.c \
	qsupport.c qsubs.c queries2.c mr_sbrk.c mr_smalloc.c \
	meinitlst.c increment.c cache.c

SRVOBJ=mr_main.o mr_sauth.o mr_scall.o mr_srvdata.o mr_shutdown.o \
	mr_util.o qrtn.o queries2.o qsupport.o qsubs.o mr_sbrk.o \
	mr_smalloc.o meinitlst.o increment.o cache.o

GLUOBJS=mr_glue.o qrtn.o queries2.o qsupport.o qsubs.o \
	../rpc/mr_et.o mr_srvdata.o mr_smalloc.o meinitlst.o mr_sbrk.o \
	../rpc/krb_et.o mr_util.o increment.o cache.o

GLUPOBJS=mr_glue.o qrtn.o queries2.o qsupport.o qsubs.o \
	../../rpc/profiled/mr_et.o mr_srvdata.o mr_smalloc.o \
	meinitlst.o mr_sbrk.o ../../rpc/profiled/krb_et.o mr_util.o \
	increment.o cache.o 


LIBS=../rpc/libmoira.a ../gdb/libgdb.a 
PLIBS=../../rpc/libmoira_p.a ../../gdb/libgdb_p.a 

LLIBS= -L../lib -lzephyr -lkrb -ldes -lcom_err -lm
PLLIBS= -L../../lib -lzephyr -lkrb -ldes -lcom_err_p -lm

SLIBS= /usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib 

COPTS= -O

INCDIRS=-I../include

CFLAGS= ${INCDIRS} ${COPTS}
LINTFLAGS= -uhv

ALL=moirad startmoira libmoiraglue.a # libmoiraglue_p.a

.c.o:
#	${CC} -c -pg ${CFLAGS} $*.c
#	mv $*.o profiled/$*.o
	${CC} -c ${CFLAGS} $*.c

.SUFFIXES: .dc .qc

.qc.c: 
	rm -f $*.c
	/usr/rtingres/bin/eqc -p $*

.dc.c:
	rm -f $*.c
	awk -f ingres.awk < $< > $*.sc
	/usr/rtingres/bin/esqlc -p $*

all: ${ALL}

moirad: ${SRVOBJ} ${LIBS}
	rm -f moirad
	cc -o moirad ${COPTS} ${SRVOBJ} ${SLIBS} ${LIBS} ${LLIBS}

moirad_p: ${SRVOBJ} ${LIBS}
	(cd profiled; cc -pg -o moirad_p ${COPTS} ${SRVOBJ} ${SLIBS} ${PLIBS} \
		${PLLIBS})

libmoiraglue.a: ${GLUOBJS}
	ar cruv libmoiraglue.a ${GLUOBJS}
	ranlib libmoiraglue.a

libmoiraglue_p.a: ${GLUOBJS}
	cd profiled; ar cruv ../libmoiraglue_p.a ${GLUPOBJS}
	ranlib libmoiraglue_p.a

restart: moirad startmoira
	./startmoira

startmoira: startmoira.o
	cc -o startmoira ${COPTS} startmoira.o

install: moirad startmoira
	install -c moirad ../bin/moirad
	install -c startmoira ../bin/startmoira

clean:
	rm -f ${ALL} libmoiraglue_p.a
	rm -f *.o *~ qrtn.c qsupport.c qoldsup.c increment.c
	rm -f profiled/*.o

TAGS:	$(CFILES)
	-etags $(CFILES)

lint:
	lint ${INCDIRS} mr_*.c q*.c ../rpc/llib-lmoira.ln 

depend: $(CFILES)
	mkdep $(CFLAGS) $(CFILES)

# the last constant line in the makefile should be...
# DO NOT DELETE THIS LINE -- mkdep uses it.

startmoira.o: startmoira.c ../include/mit-copyright.h /usr/include/stdio.h
startmoira.o: /usr/include/strings.h /usr/include/sys/types.h
startmoira.o: /usr/include/sys/file.h /usr/include/sys/wait.h
startmoira.o: /usr/include/sys/signal.h /usr/include/sys/ioctl.h
startmoira.o: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
startmoira.o: ../include/moira_site.h
mr_main.o: mr_main.c ../include/mit-copyright.h /usr/include/strings.h
mr_main.o: /usr/include/sys/types.h /usr/include/sys/errno.h
mr_main.o: /usr/include/sys/signal.h /usr/include/sys/wait.h
mr_main.o: /usr/include/sys/stat.h mr_server.h /usr/include/sys/uio.h
mr_main.o: /usr/include/sys/socket.h /usr/include/netinet/in.h
mr_main.o: ../include/mr_proto.h /usr/include/stdio.h ../include/gdb.h
mr_main.o: /usr/include/sys/time.h /usr/include/sys/time.h ../include/moira.h
mr_main.o: ../include/mr_et.h ../include/moira_site.h ../include/krb.h
mr_main.o: ../include/des.h ../include/krb_et.h
mr_sauth.o: mr_sauth.c ../include/mit-copyright.h /usr/include/strings.h
mr_sauth.o: mr_server.h /usr/include/sys/types.h /usr/include/sys/uio.h
mr_sauth.o: /usr/include/sys/socket.h /usr/include/netinet/in.h
mr_sauth.o: ../include/mr_proto.h /usr/include/stdio.h ../include/gdb.h
mr_sauth.o: /usr/include/sys/time.h /usr/include/sys/time.h
mr_sauth.o: ../include/moira.h ../include/mr_et.h ../include/moira_site.h
mr_sauth.o: ../include/krb.h ../include/des.h /usr/include/ctype.h
mr_sauth.o: ../include/krb_et.h
mr_scall.o: mr_scall.c ../include/mit-copyright.h /usr/include/sys/types.h
mr_scall.o: /usr/include/sys/stat.h /usr/include/sys/file.h ../include/krb.h
mr_scall.o: ../include/des.h /usr/include/errno.h query.h mr_server.h
mr_scall.o: /usr/include/sys/uio.h /usr/include/sys/socket.h
mr_scall.o: /usr/include/netinet/in.h ../include/mr_proto.h
mr_scall.o: /usr/include/stdio.h ../include/gdb.h /usr/include/sys/time.h
mr_scall.o: /usr/include/sys/time.h ../include/moira.h ../include/mr_et.h
mr_scall.o: ../include/moira_site.h
mr_srvdata.o: mr_srvdata.c ../include/mit-copyright.h mr_server.h
mr_srvdata.o: /usr/include/sys/types.h /usr/include/sys/uio.h
mr_srvdata.o: /usr/include/sys/socket.h /usr/include/netinet/in.h
mr_srvdata.o: ../include/mr_proto.h /usr/include/stdio.h ../include/gdb.h
mr_srvdata.o: /usr/include/sys/time.h /usr/include/sys/time.h
mr_srvdata.o: ../include/moira.h ../include/mr_et.h ../include/moira_site.h
mr_srvdata.o: ../include/krb.h ../include/des.h
mr_shutdown.o: mr_shutdown.c ../include/mit-copyright.h
mr_shutdown.o: /usr/include/strings.h /usr/include/sys/errno.h mr_server.h
mr_shutdown.o: /usr/include/sys/types.h /usr/include/sys/uio.h
mr_shutdown.o: /usr/include/sys/socket.h /usr/include/netinet/in.h
mr_shutdown.o: ../include/mr_proto.h /usr/include/stdio.h ../include/gdb.h
mr_shutdown.o: /usr/include/sys/time.h /usr/include/sys/time.h
mr_shutdown.o: ../include/moira.h ../include/mr_et.h ../include/moira_site.h
mr_shutdown.o: ../include/krb.h ../include/des.h
mr_util.o: mr_util.c ../include/mit-copyright.h mr_server.h
mr_util.o: /usr/include/sys/types.h /usr/include/sys/uio.h
mr_util.o: /usr/include/sys/socket.h /usr/include/netinet/in.h
mr_util.o: ../include/mr_proto.h /usr/include/stdio.h ../include/gdb.h
mr_util.o: /usr/include/sys/time.h /usr/include/sys/time.h ../include/moira.h
mr_util.o: ../include/mr_et.h ../include/moira_site.h ../include/krb.h
mr_util.o: ../include/des.h /usr/include/ctype.h /usr/include/strings.h
qrtn.o: qrtn.c ../include/mit-copyright.h query.h mr_server.h
qrtn.o: /usr/include/sys/types.h /usr/include/sys/uio.h
qrtn.o: /usr/include/sys/socket.h /usr/include/netinet/in.h
qrtn.o: ../include/mr_proto.h /usr/include/stdio.h ../include/gdb.h
qrtn.o: /usr/include/sys/time.h /usr/include/sys/time.h ../include/moira.h
qrtn.o: ../include/mr_et.h ../include/moira_site.h ../include/krb.h
qrtn.o: ../include/des.h
qsupport.o: qsupport.c ../include/mit-copyright.h query.h sql_defs.h 
qsupport.o: mr_server.h
qsupport.o: /usr/include/sys/types.h /usr/include/sys/uio.h
qsupport.o: /usr/include/sys/socket.h /usr/include/netinet/in.h
qsupport.o: ../include/mr_proto.h /usr/include/stdio.h ../include/gdb.h
qsupport.o: /usr/include/sys/time.h /usr/include/sys/time.h
qsupport.o: ../include/moira.h ../include/mr_et.h ../include/moira_site.h
qsupport.o: ../include/krb.h ../include/des.h /usr/include/ctype.h
qsubs.o: qsubs.c ../include/mit-copyright.h ../include/moira.h
qsubs.o: ../include/mr_et.h query.h
queries2.o: queries2.c ../include/mit-copyright.h query.h ../include/mr_et.h
mr_sbrk.o: mr_sbrk.c ../include/mit-copyright.h /usr/include/sys/types.h
mr_smalloc.o: mr_smalloc.c /usr/include/stdio.h /usr/include/signal.h
mr_smalloc.o: /usr/include/sys/time.h /usr/include/sys/time.h
mr_smalloc.o: /usr/include/sys/resource.h
meinitlst.o: meinitlst.c ../include/mit-copyright.h /usr/include/sys/errno.h
meinitlst.o: /usr/include/sys/types.h
increment.o: increment.c ../include/mit-copyright.h ../include/moira.h
increment.o: ../include/mr_et.h query.h mr_server.h /usr/include/sys/types.h
increment.o: /usr/include/sys/uio.h /usr/include/sys/socket.h
increment.o: /usr/include/netinet/in.h ../include/mr_proto.h
increment.o: /usr/include/stdio.h ../include/gdb.h /usr/include/sys/time.h
increment.o: /usr/include/sys/time.h ../include/moira_site.h ../include/krb.h
increment.o: ../include/des.h
cache.o: cache.c ../include/mit-copyright.h query.h mr_server.h
cache.o: /usr/include/sys/types.h /usr/include/sys/uio.h
cache.o: /usr/include/sys/socket.h /usr/include/netinet/in.h
cache.o: ../include/mr_proto.h /usr/include/stdio.h ../include/gdb.h
cache.o: /usr/include/sys/time.h /usr/include/sys/time.h ../include/moira.h
cache.o: ../include/mr_et.h ../include/moira_site.h ../include/krb.h
cache.o: ../include/des.h
