# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/db/Makefile,v 1.12 1990-09-26 10:00:09 kostas Exp $

# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.


INGLIB=/usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib
# SRCS=newmoira.c tables.c imembers.c class.c tables_sql.c
# OBJS=newmoira.o tables.o tables_sql.o ../server/queries2.o
SRCS=newmoira.c tables_sql.c imembers.c class.c 
OBJS=newmoira.o tables_sql.o ../server/queries2.o
CFLAGS= -I../include -O
.SUFFIXES: .qc

.qc.c: 
	rm -f $*.c
	/usr/rtingres/bin/eqc -p $*

all: newmoira class imembers

imembers: imembers.o
	cc -o imembers imembers.o ../lib/libmoira.a ${INGLIB}
imembers.c: imembers.qc

newmoira: ${OBJS}
	cc -o newmoira ${OBJS} ${INGLIB}

newmoira.c: newmoira.qc

tables.c: newdb db2tbls.awk
	awk -f db2tbls.awk < newdb > tables.c

tables_sql.c: newdb.sql db2tbls_sql.awk
	awk -f db2tbls_sql.awk < newdb.sql > tables_sql.c

class: class.o
	cc -o class class.o ../lib/libmoira.a ${INGLIB}

class.c: class.qc

clean:
	rm -f *.o newmoira.c tables.c newmoira extractdb extract.c class class.c
	rm -f imembers.c imembers
	rm -f tables_sql.c

install: newmoira
	install -c newmoira ../bin

depend: ${SRCS}
	mkdep ${CFLAGS} ${SRCS}

# DO NOT DELETE THIS LINE -- mkdep uses it.

newmoira.o: newmoira.c ../include/mit-copyright.h /usr/include/stdio.h
newmoira.o: ../server/query.h
tables.o: tables.c
tables_sql.o: tables_sql.c
imembers.o: imembers.c /usr/include/stdio.h ../include/moira.h
imembers.o: ../include/mr_et.h
class.o: class.c ../include/mit-copyright.h ../include/moira.h
class.o: ../include/mr_et.h
