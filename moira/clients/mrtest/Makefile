#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/Makefile,v 1.7 1989-07-12 18:07:09 mar Exp $
#
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.


LIBS = ../rpc/libsms.a ../gdb/libgdb.a 
LLIBS= -L../lib -lkrb -ldes -lss -lcom_err -lm -lc 

COPTS= -O

INCDIRS=-I../include

CFLAGS= ${INCDIRS} ${COPTS}

.SUFFIXES: .ct

.ct.o: ; ../ss/make_commands $*.ct

all: smstest

smstest: test_cmds.o test.o ${LIBS} 
	rm -f smstest
	cc ${COPTS} -o $@ test_cmds.o test.o ${LIBS} ${LLIBS}

clean:
	rm -f *.o core \#* *~ gmon.out
	rm -f smstest

install: smstest
	install -s smstest ${DESTDIR}/usr/etc/smstest
