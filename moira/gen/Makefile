# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/Makefile,v 1.4 1989-01-30 15:51:17 mar Exp $

LIBS= -L../lib -lsms -lcom_err -lzephyr -lkrb -ldes \
	 /usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib
CFLAGS= -I../include -O
.SUFFIXES: .qc

.qc.c: 
	rm -f $*.c
	/usr/rtingres/bin/eqc -p $*

all: passwd.gen aliases.gen hesiod.gen nfs.gen

passwd.gen: passwd.o util.o
	cc -o passwd.gen passwd.o util.o ${LIBS}

passwd.o: passwd.c

aliases.gen: aliases.o util.o
	cc -o aliases.gen aliases.o util.o ${LIBS}

aliases.o: aliases.c

hesiod.gen: hesiod.o util.o
	cc -o hesiod.gen hesiod.o util.o ${LIBS}

hesiod.o: hesiod.c

nfs.gen: nfs.o util.o
	cc -o nfs.gen nfs.o util.o ${LIBS}

nfs.o: nfs.c

clean:
	rm -f *.o 
	rm -f passwd.c passwd.gen
	rm -f aliases.c aliases.gen
	rm -f hesiod.c hesiod.gen
	rm -f nfs.c nfs.gen

install: all
	install passwd.gen ../bin
	install aliases.gen ../bin
	install hesiod.gen ../bin
	install nfs.gen ../bin
