# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/db/Makefile,v 1.6 1989-12-15 17:35:36 mar Exp $

# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.


INGLIB=/usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib
SRCS=newmoira.c tables.c imembers.c class.c
OBJS=newmoira.o tables.o ../server/queries2.o
CFLAGS= -I../include -O
.SUFFIXES: .qc

.qc.c: 
	rm -f $*.c
	/usr/rtingres/bin/eqc -p $*

all: newmoira class imembers

imembers: imembers.o
	cc -o imembers imembers.o ../lib/libsms.a ${INGLIB}
imembers.c: imembers.qc

newmoira: ${OBJS}
	cc -o newmoira ${OBJS} ${INGLIB}

newmoira.c: newmoira.qc

tables.c: newdb db2tbls.awk
	awk -f db2tbls.awk < newdb > tables.c

class: class.o
	cc -o class class.o ../lib/libsms.a ${INGLIB}

class.c: class.qc

clean:
	rm -f *.o newmoira.c tables.c newmoira extractdb extract.c class class.c
	rm -f imembers.c imembers

install: newmoira
	install newmoira ../bin

depend: ${SRCS}
	mkdep ${CFLAGS} ${SRCS}

# DO NOT DELETE THIS LINE -- mkdep uses it.

newmoira.o: newmoira.c ../include/mit-copyright.h /usr/include/stdio.h
newmoira.o: ../server/query.h
tables.o: tables.c
imembers.o: imembers.c /usr/include/stdio.h ../include/sms.h
imembers.o: ../include/sms_et.h
class.o: class.c ../include/mit-copyright.h ../include/sms.h
class.o: ../include/sms_et.h
