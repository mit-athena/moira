#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/regtape/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/regtape/Makefile,v 1.2 1990-01-18 14:07:22 mar Exp $
#

#
#	Makefile for the sms stuff --mark roman 7/6/87
#
CFILES = adduser.c rafnu.c students.c personel.c

LIBS	=  -L../lib -lsms -lgdb -lcom_err -lzephyr -lkrb -ldes \
	/usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib
CFLAGS  = -I../include

.SUFFIXES: .qc

.qc.c: 
	rm -f $*.c
	/usr/rtingres/bin/eqc -p $*

all:	regtape students personel

regtape: adduser.o rafnu.o
	cc -o regtape adduser.o rafnu.o ../server/libsmsglue.a $(LIBS)

students: students.o rafnu.o
	cc -o students students.o rafnu.o $(LIBS)

personel: personel.o rafnu.o
	cc -o personel personel.o rafnu.o $(LIBS)

clean: 
	rm -f regtape students personal core
	rm -f *~ *.o

lint: 
	lint $(CFILES)

install: regtape students personel
	install -c regtape ../bin/regtape
	install -c students ../bin/students
	install -c personel ../bin/personel

depend:	${CFILES}
	mkdep ${CFLAGS} ${CFILES}

# DO NOT DELETE THIS LINE -- mkdep uses it.

adduser.o: adduser.c /usr/include/stdio.h /usr/include/sys/time.h
adduser.o: /usr/include/sys/time.h ../include/sms.h ../include/sms_et.h
rafnu.o: rafnu.c /usr/include/stdio.h /usr/include/ctype.h
students.o: students.c /usr/include/stdio.h /usr/include/strings.h
students.o: /usr/include/ctype.h /usr/include/sys/time.h
students.o: /usr/include/sys/time.h ../include/sms.h ../include/sms_et.h
students.o: ../include/sms_app.h ../include/mit-copyright.h
personel.o: personel.c /usr/include/stdio.h /usr/include/strings.h
personel.o: /usr/include/ctype.h /usr/include/sys/time.h
personel.o: /usr/include/sys/time.h ../include/sms.h ../include/sms_et.h
personel.o: ../include/sms_app.h ../include/mit-copyright.h
