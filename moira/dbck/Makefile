# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dbck/Makefile,v 1.5 1993-01-12 20:43:33 genoa Exp $
# Database consistency checker
#
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.


.SUFFIXES:
.SUFFIXES: .o .c .dc .sc
CFLAGS= -I/usr/athena/include -I../include -I../lib -O

AWK=awk

SRCS = dbck.c fix.c phase1.c phase2.c phase3.c phase4.c members.c
OBJS = dbck.o fix.o phase1.o phase2.o phase3.o phase4.o

EQLIBS = ../lib/libmoira.a ../server/libmrglue.a /usr/ingres/lib/libingres.a -lm

.dc.sc:
	$(AWK) -f /mit/moiradev/src/util/imake.includes/ingres.awk < $< > $*.sc

.sc.c:
	/usr/ingres/bin/esqlc -p $*.sc

all: dbck members

dbck: ${OBJS}
	cc -o $@ ${CFLAGS} ${OBJS} ${EQLIBS}

dbck.c: dbck.sc
dbck.o: dbck.h dbck.c
fix.o: fix.c dbck.h
fix.c: fix.sc
phase1.o: phase1.c dbck.h
phase1.c: phase1.sc
phase2.o: phase2.c
phase2.c: phase2.sc
phase3.o: phase3.c
phase3.c: phase3.sc
phase4.o: phase4.c
phase4.c: phase4.sc
members.c: members.sc
members.o: members.c

members: members.o
	cc -o members members.o ${EQLIBS}

clean:
	rm -f *.o 
	rm -f dbck.sc fix.sc phase1.sc phase2.sc phase3.sc phase4.sc 
	rm -f dbck.c fix.c phase1.c phase2.c phase3.c phase4.c dbck
	rm -f members.sc members.c members
	rm -f core *~

install: dbck
	install -c dbck ../bin/dbck

depend: ${SRCS}
	makedepend ${CFLAGS} ${SRCS}


