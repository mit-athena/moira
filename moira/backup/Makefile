# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/Makefile,v 1.7 1989-12-04 12:10:16 mar Exp $
#
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.

.SUFFIXES:
.SUFFIXES: .qc .c .o
CFLAGS= -O

SRCS = dump_db.c bkup1.c bkup.c dumprest.c rest_db.c rest1.c rest.c
EQLIBS = /usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib

.qc.c:
	/usr/rtingres/bin/eqc $*.qc

all: mrbackup mrrestore

mrbackup: dump_db.o bkup1.o bkup.o dumprest.o
	cc -o $@ ${CFLAGS} dump_db.o bkup1.o bkup.o dumprest.o ${EQLIBS}

mrrestore: rest_db.o rest1.o rest.o dumprest.o
	cc -o $@ ${CFLAGS} rest_db.o rest1.o rest.o dumprest.o ${EQLIBS}

copy_backups: copy_backups.c
	cc -o $@ ${CFLAGS} -I../include copy_backups.c \
		 ../update/sms_update.o -L../lib \
		-lsms -lgdb -lkrb -ldes -lmisc

bkup.qc: ../db/newdb db2bkup.awk
	cc -E ../db/newdb | awk -f db2bkup.awk > bkup.qc

bkup1.qc: bkup.qc

rest.qc: ../db/newdb db2rest.awk
	cc -E ../db/newdb | awk -f db2rest.awk > rest.qc

rest1.qc: rest.qc

lint:
	lint -h dump_db.c bkup.c bkup1.c dumprest.c | egrep -v '^II'
	lint -h rest_db.c rest.c rest1.c dumprest.c | egrep -v '^II'

clean:
	rm -f *.o mrbackup bkup.qc bkup1.qc mrrestore rest.qc rest1.qc
	rm -f core *~
	rm -f copy_backups counts
	rm -f bkup.c bkup1.c counts.c dump_db.c dumprest.c
	rm -f rest.c rest1.c rest_db.c

install:
	install mrbackup ../bin/mrbackup
	install mrrestore ../bin/mrrestore
	install nightly.sh ../bin/nightly.sh

depend: ${SRCS}
	mkdep ${CFLAGS} ${SRCS}

# DO NOT DELETE THIS LINE -- mkdep uses it.

dump_db.o: dump_db.c /usr/include/stdio.h /usr/include/sys/file.h
dump_db.o: /usr/include/ctype.h /usr/include/mit-copyright.h dump_db.h
bkup1.o: bkup1.c /usr/include/stdio.h
bkup.o: bkup.c /usr/include/stdio.h dump_db.h
dumprest.o: dumprest.c /usr/include/stdio.h /usr/include/sys/file.h
dumprest.o: /usr/include/strings.h /usr/include/mit-copyright.h
rest_db.o: rest_db.c /usr/include/sys/file.h /usr/include/stdio.h
rest_db.o: /usr/include/ctype.h /usr/include/mit-copyright.h dump_db.h
rest1.o: rest1.c /usr/include/stdio.h
rest.o: rest.c /usr/include/stdio.h
