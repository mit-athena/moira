#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/Makefile,v 1.2 1987-08-23 01:43:47 wesommer Exp $
#


LIBS = ../rpc/libsms.a ../gdb/libgdb.a 
LLIBS= -L../lib -lkrb -ldes -lss -lcom_err -lm -lc 

COPTS= -O

INCDIRS=-I../include

CFLAGS= ${INCDIRS} ${COPTS}

.SUFFIXES: .ct

.ct.o: ; make_commands $*.ct

all: smstest smsfinger

smstest: test_cmds.o test.o ${LIBS} 
	cc ${COPTS} -o $@ test_cmds.o test.o ${LIBS} ${LLIBS}

smsfinger: smsfinger.o ${LIBS} 
	cc ${COPTS} -o $@ smsfinger.o ${LIBS} ${LLIBS}

clean:
	rm -f *.o core \#* *~ gmon.out
	rm -f smstest smsfinger

install:
	install smstest ../bin
# NOT /usr/athena!!!
	install smsfinger ../bin
