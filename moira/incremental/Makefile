# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/incremental/Makefile,v 1.5 1992-06-02 14:32:03 probe Exp $

AFSDIR = /mit/afsdev/bld/dest
AFSLIBS=$(AFSDIR)/lib/afs/libprot.a    $(AFSDIR)/lib/afs/libauth.a \
	$(AFSDIR)/lib/libubik.a        $(AFSDIR)/lib/afs/util.a \
	$(AFSDIR)/lib/librxkad.a       $(AFSDIR)/lib/afs/libsys.a \
	$(AFSDIR)/lib/librx.a          $(AFSDIR)/lib/liblwp.a

SRCS = afs.c ksrvtgt.c
DEBUG = -O
CFLAGS = -I../include -I$(AFSDIR)/include $(DEBUG)
LIBS = -L../lib -lmoira -lzephyr -lgdb -lcom_err -lhesiod -lkrb -ldes

all: afs.incr ksrvtgt

afs.incr: afs.o
	cc -o afs.incr afs.o ${AFSLIBS} ${LIBS}

ksrvtgt: ksrvtgt.o
	cc -o ksrvtgt ksrvtgt.o ${LIBS}

clean:
	rm -f *.o
	rm -f afs.incr ksrvtgt

install: all
	install -c afs.incr ../bin

depend:
	mkdep ${CFLAGS} ${SRCS}

# DO NOT DELETE THIS LINE -- mkdep uses it.

afs.o: afs.c ../include/moira.h ../include/mr_et.h ../include/moira_site.h
afs.o: ../include/mit-copyright.h /usr/include/sys/file.h
ksrvtgt.o: ksrvtgt.c /usr/include/stdio.h /usr/include/sys/param.h
ksrvtgt.o: /usr/include/machine/machparam.h /usr/include/sys/signal.h
ksrvtgt.o: /usr/include/sys/types.h ../include/krb.h
ksrvtgt.o: ../include/mit-copyright.h ../include/des.h
