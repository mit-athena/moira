# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/xregister/Makefile,v 1.1 1996-07-02 02:05:46 danw Exp $

CFLAGS = -I/mit/x11 -I/mit/motif/include
XLIBS =  -L/mit/x11/$${HOSTTYPE}lib /mit/motif/`machtype`lib/libWc.a -lXaw -lXmu -lXt -lX11 \
	-L/usr/athena/lib -lkrb -ldes -lhesiod -lm

OBJS = register.o AriRegAll.o

all: register

register: $(OBJS)
	cc -o register $(OBJS) $(XLIBS)

