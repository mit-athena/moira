# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/Makefile,v 1.7 1993-02-05 03:34:15 probe Exp $

DBCFLAGS=
DBLIBS=	-L/usr/ingres/lib -lingres -lm

AFSDIR = /mit/afsdev/bld/dest
CFLAGS = -I$(AFSDIR)/include -I$(AFSDIR)/include/afs -I../include \
	-I/usr/athena/include -O ${DBCFLAGS}
LDFLAGS= -L/usr/athena/lib

OBJS= ptutils.o utils.o ubik.o
LIBS= -L../lib $(LDFLAGS) -lmoira -lcom_err -lkrb -ldes ${DBLIBS}

AFSLIBS=$(AFSDIR)/lib/afs/libprot.a   $(AFSDIR)/lib/afs/libauth.a \
	$(AFSDIR)/lib/libubik.a       $(AFSDIR)/lib/afs/util.a \
	$(AFSDIR)/lib/librxkad.a      $(AFSDIR)/lib/afs/libsys.a \
	$(AFSDIR)/lib/librx.a         $(AFSDIR)/lib/liblwp.a

.SUFFIXES: .dc .sc

.dc.sc:
	awk -f ../util/imake.includes/ingres.awk < $< > $*.sc
.sc.c:
	/usr/ingres/bin/esqlc -p $*

all: sync ptdump pt_util

sync: sync.o $(OBJS)
	$(CC) -o sync sync.o $(OBJS) $(AFSLIBS) $(LIBS)

ptdump: ptdump.o
	cc -o ptdump ptdump.o $(AFSLIBS) $(LDFLAGS) -ldes

pt_util: pt_util.o $(OBJS)
	cc -o pt_util pt_util.o $(OBJS) $(AFSLIBS) $(LDFLAGS) -lcom_err -ldes

migrate: migrate.o
	cc -o migrate migrate.o $(LIBS)

migrate.c: migrate.qc

clean:
	rm -f *.o sync.c sync migrate.c migrate pt_util ptdump
