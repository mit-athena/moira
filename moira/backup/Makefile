# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/Makefile,v 1.5 1988-09-12 13:49:13 mar Exp $
#
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.

.SUFFIXES:
.SUFFIXES: .qc .o
CFLAGS= -O

EQLIBS = /usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib

.qc.o:
	/usr/rtingres/bin/eqc $*.qc
	rm -f $*.o
	cc -c ${CFLAGS} $*.c
	rm -f $*.c

all: smsbackup smsrestore

smsbackup: dump_db.o bkup1.o bkup.o dumprest.o
	cc -o $@ ${CFLAGS} dump_db.o bkup1.o bkup.o dumprest.o ${EQLIBS}

smsrestore: rest_db.o rest1.o rest.o dumprest.o
	cc -o $@ ${CFLAGS} rest_db.o rest1.o rest.o dumprest.o ${EQLIBS}

copy_backups: copy_backups.c
	cc -o $@ ${CFLAGS} -I../include copy_backups.c \
		 ../update/sms_update.o -L../lib \
		-lsms -lgdb -lkrb -ldes -lmisc

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
	rm -f *.o smsbackup bkup.qc bkup1.qc smsrestore rest.qc rest1.qc
	rm -f core *~
	rm -f copy_backups counts
	rm -f bkup.c bkup1.c counts.c dump_db.c dumprest.c
	rm -f rest.c rest1.c rest_db.c

install:
	install smsbackup ../bin/smsbackup
	install smsrestore ../bin/smsrestore
	install nightly.sh ../bin/nightly.sh


