# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/Makefile,v 1.2 1990-04-13 17:36:59 mar Exp $

CFLAGS = -I/mit/afsdev/build/vax/include -I../include -O

OBJS = prprocs.o prutils.o utils.o ubik.o
LIBS= -L../lib -lmoira -lcom_err \
	 /usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib

.SUFFIXES: .qc

.qc.c: 
	rm -f $*.c
	/usr/rtingres/bin/eqc -p $*

all: sync resync

sync: sync.o $(OBJS)
	cc -o sync sync.o $(OBJS) $(LIBS)
sync.c: sync.qc

resync: resync.o $(OBJS)
	cc -o resync resync.o $(OBJS) $(LIBS)
resync.c: resync.qc

prnewuser: prnewuser.o $(OBJS)
	cc -o prnewuser prnewuser.o $(OBJS)

prnewlist: prnewlist.o $(OBJS)
	cc -o prnewlist prnewlist.o $(OBJS)

startsync: startsync.o
	cc -o startsync startsync.o


clean:
	rm -f *.o resync.c sync.c resync sync startsync prnewlist prnewuser
