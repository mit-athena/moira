# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/incremental/Makefile,v 1.1 1989-08-18 17:31:48 mar Exp $

CFLAGS = -I../include -O
LIBS = -L../lib -lsms -lcom_err -lzephyr -lauth -lsys -lkrb -ldes


all: afs.incr

afs.incr: afs.o
	cc -o afs.incr afs.o ${LIBS}

clean:
	rm -f *.o
	rm -f afs.incr

install: all
	install -c afs.incr ../bin
