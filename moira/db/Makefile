# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/db/Makefile,v 1.1 1988-05-19 17:28:57 mar Exp $

INGLIB=/usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib
OBJS=newsms.o tables.o ../server/queries.o ../server/queries2.o

newsms: ${OBJS}
	cc -o newsms ${OBJS} ${INGLIB}

newsms.c: newsms.qc
	/usr/rtingres/bin/eqc -p newsms.qc

tables.c: newdb db2tbls.awk
	awk -f db2tbls.awk < newdb > tables.c

clean:
	rm -f *.o newsms.c tables.c newsms

install: newsms
	install newsms ../bin
