# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/Makefile,v 1.1 1988-06-20 12:53:15 mar Exp $

INGLIB=/usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib
CFLAGS= -I../include
.SUFFIXES: .qc

.qc.c: 
	rm -f $*.c
	/usr/rtingres/bin/eqc -p $*

all: passwd.gen aliases.gen hesiod.gen

passwd.gen: passwd.o util.o
	cc -o passwd.gen passwd.o util.o ${INGLIB}

passwd.o: passwd.c

aliases.gen: aliases.o util.o
	cc -o aliases.gen aliases.o util.o ../lib/libsms.a ${INGLIB}

aliases.o: aliases.c

hesiod.gen: hesiod.o util.o
	cc -o hesiod.gen hesiod.o util.o ../lib/libsms.a ${INGLIB}

hesiod.o: hesiod.c

clean:
	rm -f *.o 
	rm -f passwd.c passwd.gen
	rm -f aliases.c aliases.gen
	rm -f hesiod.c hesiod.gen

install: all
	install passwd.gen ../bin
	install aliases.gen ../bin
	install hesiod.gen ../bin
