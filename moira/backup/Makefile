.SUFFIXES:
.SUFFIXES: .qc .o
CFLAGS= -O

EQLIBS = /usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib

.qc.o:
	eqc $*.qc
	rm -f $*.o
	cc -c ${CFLAGS} $*.c

all: smsbackup smsrestore

smsbackup: dump_db.o bkup1.o bkup.o dumprest.o
	cc -o $@ ${CFLAGS} dump_db.o bkup1.o bkup.o dumprest.o ${EQLIBS}

smsrestore: rest_db.o rest1.o rest.o dumprest.o
	cc -o $@ ${CFLAGS} rest_db.o rest1.o rest.o dumprest.o ${EQLIBS}

bkup.qc: ../db/dbbuild db2bkup.awk
	cc -E ../db/dbbuild | awk -f db2bkup.awk > bkup.qc

bkup1.qc: bkup.qc

rest.qc: ../db/dbbuild db2rest.awk
	cc -E ../db/dbbuild | awk -f db2rest.awk > rest.qc

rest1.qc: rest.qc

lint:
	lint -h dump_db.c bkup.c bkup1.c dumprest.c | egrep -v '^II'
	lint -h rest_db.c rest.c rest1.c dumprest.c | egrep -v '^II'

clean:
	rm -f *.c *.o smsbackup bkup.qc bkup1.qc smsrestore rest.qc rest1.qc
	rm -f core *~





