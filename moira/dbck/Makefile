# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dbck/Makefile,v 1.3 1990-03-19 18:43:48 mar Exp $
# Database consistency checker
#
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.


.SUFFIXES:
.SUFFIXES: .o .c .qc
CFLAGS= -I../include -O

SRCS = dbck.c fix.c phase1.c phase2.c phase3.c phase4.c members.c
OBJS = dbck.o fix.o phase1.o phase2.o phase3.o phase4.o

EQLIBS = ../lib/libmoira.a ../server/libmoiraglue.a \
	/usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib

.qc.c:
	/usr/rtingres/bin/eqc -p $*.qc

all: dbck members

dbck: ${OBJS}
	cc -o $@ ${CFLAGS} ${OBJS} ${EQLIBS}

dbck.c: dbck.qc
dbck.o: dbck.h dbck.c
fix.o: fix.c dbck.h
fix.c: fix.qc
phase1.o: phase1.c dbck.h
phase1.c: phase1.qc
phase2.o: phase2.c
phase2.c: phase2.qc
phase3.o: phase3.c
phase3.c: phase3.qc
phase4.o: phase4.c
phase4.c: phase4.qc

members: members.o
	cc -o members members.o ${EQLIBS}

clean:
	rm -f *.o 
	rm -f dbck.c fix.c phase1.c phase2.c phase3.c phase4.c dbck
	rm -f core *~

install: dbck
	install -c dbck ../bin/dbck

depend: ${SRCS}
	mkdep ${CFLAGS} ${SRCS}

# DO NOT DELETE THIS LINE -- mkdep uses it.

dbck.o: dbck.c ../include/mit-copyright.h /usr/include/stdio.h
dbck.o: /usr/include/strings.h /usr/include/signal.h dbck.h
dbck.o: ../include/moira_site.h
fix.o: fix.c ../include/mit-copyright.h ../include/moira.h ../include/mr_et.h
fix.o: /usr/include/stdio.h dbck.h ../include/moira_site.h
phase1.o: phase1.c ../include/mit-copyright.h ../include/moira.h
phase1.o: ../include/mr_et.h dbck.h ../include/moira_site.h
phase2.o: phase2.c ../include/mit-copyright.h /usr/include/stdio.h
phase2.o: ../include/moira.h ../include/mr_et.h dbck.h
phase2.o: ../include/moira_site.h
phase3.o: phase3.c ../include/mit-copyright.h dbck.h ../include/moira_site.h
phase4.o: phase4.c ../include/mit-copyright.h ../include/moira.h
phase4.o: ../include/mr_et.h dbck.h ../include/moira_site.h
members.o: members.c /usr/include/stdio.h /usr/include/signal.h
members.o: ../include/moira.h ../include/mr_et.h
