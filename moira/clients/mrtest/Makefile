#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/Makefile,v 1.4 1988-07-21 15:06:22 mar Exp $
#


LIBS = ../rpc/libsms.a ../gdb/libgdb.a 
LLIBS= -L../lib -lkrb -ldes -lss -lcom_err -lm -lc 

COPTS= -O

INCDIRS=-I../include

CFLAGS= ${INCDIRS} ${COPTS}

.SUFFIXES: .ct

.ct.o: ; make_commands $*.ct

all: smstest

smstest: test_cmds.o test.o execute_cmd.o ${LIBS} 
	rm -f smstest
	cc ${COPTS} -o $@ test_cmds.o test.o execute_cmd.o \
		${LIBS} ${LLIBS}

execute_cmd.o: execute_cmd.c
	cc -I/paris/source/4.3/athena.lib/ss -O -c execute_cmd.c

clean:
	rm -f *.o core \#* *~ gmon.out
	rm -f smstest

install:
	install smstest ../bin
