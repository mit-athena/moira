# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/Makefile,v 1.6 1992-07-05 13:32:14 probe Exp $

AFSDIR = /mit/afsdev/bld/dest
CFLAGS = -I$(AFSDIR)/include -I$(AFSDIR)/include/afs -I../include \
	-I/usr/athena/include -O
LDFLAGS= -L/usr/athena/lib

OBJS= ptutils.o utils.o ubik.o
LIBS= -L../lib $(LDFLAGS) -lmoira -lcom_err -lkrb -ldes \
	 /usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib

AFSLIBS=$(AFSDIR)/lib/afs/libprot.a   $(AFSDIR)/lib/afs/libauth.a \
	$(AFSDIR)/lib/libubik.a       $(AFSDIR)/lib/afs/util.a \
	$(AFSDIR)/lib/librxkad.a      $(AFSDIR)/lib/afs/libsys.a \
	$(AFSDIR)/lib/librx.a         $(AFSDIR)/lib/liblwp.a

.SUFFIXES: .qc

.qc.c: 
	rm -f $*.c
	/usr/rtingres/bin/eqc -p $*.qc

all: sync ptdump migrate pt_util

sync: sync.o $(OBJS)
	$(CC) -o sync sync.o $(OBJS) $(AFSLIBS) $(LIBS)
sync.c: sync.qc

sync.fast: sync.fast.o $(OBJS)
	$(CC) -o sync.fast sync.fast.o $(OBJS) $(AFSLIBS) $(LIBS)
sync.fast.c: sync.fast.qc

ptdump: ptdump.o
	cc -o ptdump ptdump.o $(AFSLIBS) $(LDFLAGS) -ldes

pt_util: pt_util.o $(OBJS)
	cc -o pt_util pt_util.o $(OBJS) $(AFSLIBS) $(LDFLAGS) -lcom_err -ldes

migrate: migrate.o
	cc -o migrate migrate.o $(LIBS)

migrate.c: migrate.qc

clean:
	rm -f *.o sync.c sync migrate.c migrate pt_util ptdump
