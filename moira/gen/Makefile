# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/Makefile,v 1.7 1989-09-18 12:00:11 mar Exp $

LIBS= -L../lib -lsms -lcom_err -lzephyr -lkrb -ldes \
	 /usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib
CFLAGS= -I../include -O
.SUFFIXES: .qc

.qc.c: 
	rm -f $*.c
	/usr/rtingres/bin/eqc -p $*

all: passwd.gen aliases.gen hesiod.gen nfs.gen mailhub.gen

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

mailhub.gen: mailhub.o util.o
	cc -o mailhub.gen mailhub.o util.o ${LIBS}

mailhub.o: mailhub.c

clean:
	rm -f *.o 
	rm -f passwd.c passwd.gen
	rm -f aliases.c aliases.gen
	rm -f hesiod.c hesiod.gen
	rm -f nfs.c nfs.gen
	rm -f mailhub.c mailhub.gen

install: all
	install -c passwd.gen ../bin
	install -c aliases.gen ../bin
	install -c hesiod.gen ../bin
	install -c nfs.gen ../bin
	install -c aliases.sh ../bin
	install -c hesiod.sh ../bin
	install -c install_dirs ../bin
	install -c install_quotas ../bin
	install -c nfs.sh ../bin
	install -c passwd.sh ../bin
