#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/Makefile,v 1.10 1990-02-14 11:39:58 mar Exp $
#
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.


LIBS = ../rpc/libsms.a ../gdb/libgdb.a 
LLIBS= -L../lib -lhesiod -lkrb -ldes -lhesiod -lss -lcom_err -lm -lc 

SRCS= test.c

COPTS= -O

INCDIRS=-I../include

CFLAGS= ${INCDIRS} ${COPTS}

.SUFFIXES: .ct .c .o

.ct.o: ; ../ss/make_commands $*.ct

all: mrtest

mrtest: test_cmds.o test.o ${LIBS} 
	rm -f mrtest
	cc ${COPTS} -o $@ test_cmds.o test.o ${LIBS} ${LLIBS}

clean:
	rm -f *.o core \#* *~ gmon.out
	rm -f mrtest

install: mrtest
	install -c -s mrtest ${DESTDIR}/usr/etc/mrtest

depend:
	mkdep ${CFLAGS} ${SRCS}

# DO NOT DELETE THIS LINE -- mkdep uses it.

test.o: test.c ../include/mit-copyright.h /usr/include/stdio.h
test.o: /usr/include/sys/file.h /usr/include/ctype.h ../include/sms.h
test.o: ../include/sms_et.h ../include/ss.h
