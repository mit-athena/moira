# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/db/Makefile,v 1.2 1988-09-13 13:46:03 mar Exp $

# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.


INGLIB=/usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib
OBJS=newsms.o tables.o ../server/queries.o ../server/queries2.o
CFLAGS= -I../include -O
.SUFFIXES: .qc

.qc.c: 
	rm -f $*.c
	/usr/rtingres/bin/eqc -p $*

all: newsms class convert

newsms: ${OBJS}
	cc -o newsms ${OBJS} ${INGLIB}

newsms.c: newsms.qc

tables.c: newdb db2tbls.awk
	awk -f db2tbls.awk < newdb > tables.c

extractdb: extract.o
	cc -o extractdb extract.o ${INGLIB}

extract.c: extract.qc

class: class.o
	cc -o class class.o ../lib/libsms.a ${INGLIB}

class.c: class.qc

convert: conv.o data.o match.o
	cc -o convert conv.o data.o match.o ../lib/libsms.a ${INGLIB}

conv.c: conv.qc
data.c: data.qc
match.c: match.qc

clean:
	rm -f *.o newsms.c tables.c newsms extractdb extract.c class class.c
	rm -f convert conv.c match.c data.c

install: newsms extractdb
	install newsms ../bin
	install extractdb ../bin
