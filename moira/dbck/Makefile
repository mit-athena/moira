.SUFFIXES:
.SUFFIXES: .o .c .qc
CFLAGS= -I../include -g

OBJS = dbck.o fix.o phase1.o phase2.o phase3.o phase4.o

EQLIBS = ../lib/libsms.a ../server/libsmsglue.a \
	/usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib

.qc.c:
	/usr/rtingres/bin/eqc -p $*.qc

all: dbck

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


clean:
	rm -f *.o 
	rm -f dbck.c fix.c phase1.c phase2.c phase3.c phase4.c dbck
	rm -f core *~
