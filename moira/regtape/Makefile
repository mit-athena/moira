#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/regtape/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/regtape/Makefile,v 1.1 1990-01-02 15:24:13 mar Exp $
#

#
#	Makefile for the sms stuff --mark roman 7/6/87
#
CFILES = adduser.c rafnu.c
OFILES = adduser.o rafnu.o
OUTP   = regtape

DESTDIR =

# Note that -lkrb is used, but not -ldes, since smsglue only calls 
# get_krbrlm()

LIBS	=  ../server/libsmsglue.a ../rpc/libsms.a -lcom_err -lkrb \
	/usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib
CFLAGS  = -I../include -L../lib

all:	$(OUTP)

debug:	
	cc $(CFLAGS) -DDEBUG -c adduser.c 


$(OUTP): $(OFILES)
	 cc -o $(OUTP) $(CFLAGS) $(OFILES) $(LIBS)

ci:
	ci $(CFILES) 

clean: 
	rm -f $(OUTP) $(OFILES) core
	rm -f *~ *.o

lint: 
	lint $(CFILES)

install:
	install regtape ../bin/regtape
