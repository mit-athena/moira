# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/Makefile,v 1.3 1992-05-31 21:38:34 probe Exp $

AFSDIR = /mit/afsdev/bld/dest
CFLAGS = -I$(AFSDIR)/include -I$(AFSDIR)/include/afs -I../include -O

OBJS= ptprocs.o utils.o ubik.o
LIBS= -L../lib -lmoira -lcom_err -lkrb -ldes \
	 /usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib

AFSLIBS=$(AFSDIR)/lib/afs/libprot.a   $(AFSDIR)/lib/afs/libauth.a \
	$(AFSDIR)/lib/libubik.a       $(AFSDIR)/lib/afs/util.a \
	$(AFSDIR)/lib/librxkad.a      $(AFSDIR)/lib/afs/libsys.a \
	$(AFSDIR)/lib/librx.a         $(AFSDIR)/lib/liblwp.a

.SUFFIXES: .qc

.qc.c: 
	rm -f $*.c
	/usr/rtingres/bin/eqc -p $*

all: sync ptdump

sync: sync.o $(OBJS)
	$(CC) -o sync sync.o $(OBJS) $(AFSLIBS) $(LIBS)
sync.c: sync.qc

ptdump: ptdump.o
	cc -o ptdump ptdump.o $(AFSLIBS) -lkrb -ldes

clean:
	rm -f *.o resync.c sync.c resync sync startsync prnewlist prnewuser
