# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/Makefile,v 1.5 1992-06-08 17:27:21 probe Exp $

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
	/usr/rtingres/bin/eqc -p $*

all: sync ptdump migrate pt_util

sync: sync.o $(OBJS)
	$(CC) -o sync sync.o $(OBJS) $(AFSLIBS) $(LIBS)
sync.c: sync.qc

ptdump: ptdump.o
	cc -o ptdump ptdump.o $(AFSLIBS) $(LDFLAGS) -ldes

pt_util: pt_util.o $(OBJS)
	cc -o pt_util pt_util.o $(OBJS) $(AFSLIBS) $(LDFLAGS) -lcom_err -ldes

migrate: migrate.o
	cc -o migrate migrate.o $(LIBS)

migrate.c: migrate.qc

clean:
	rm -f *.o sync.c sync migrate.c migrate pt_util ptdump
