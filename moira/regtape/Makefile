#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/regtape/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/regtape/Makefile,v 1.4 1990-03-08 17:25:23 mar Exp $
#

#
#	Makefile for the sms stuff --mark roman 7/6/87
#
CFILES = rafnu.c students.c employee.c

LIBS	=  -L../lib -lsms -lgdb -lcom_err -lzephyr -lkrb -ldes \
	/usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib
CFLAGS  = -I../include

.SUFFIXES: .qc

.qc.c: 
	rm -f $*.c
	/usr/rtingres/bin/eqc -p $*

all:	students employee

students: students.o rafnu.o
	cc -o students students.o rafnu.o $(LIBS)

employee: employee.o rafnu.o
	cc -o employee employee.o rafnu.o $(LIBS)

clean: 
	rm -f regtape students employee core
	rm -f students.c employee.c
	rm -f *~ *.o

lint: 
	lint $(CFILES)

install: students employee
	install -c students ../bin/students
	install -c employee ../bin/employee

depend:	${CFILES}
	mkdep ${CFLAGS} ${CFILES}

# DO NOT DELETE THIS LINE -- mkdep uses it.

rafnu.o: rafnu.c /usr/include/stdio.h /usr/include/ctype.h
students.o: students.c /usr/include/stdio.h /usr/include/strings.h
students.o: /usr/include/ctype.h /usr/include/sys/time.h
students.o: /usr/include/sys/time.h ../include/sms.h ../include/sms_et.h
students.o: ../include/sms_app.h ../include/mit-copyright.h
employee.o: employee.c /usr/include/stdio.h /usr/include/strings.h
employee.o: /usr/include/ctype.h /usr/include/sys/time.h
employee.o: /usr/include/sys/time.h ../include/sms.h ../include/sms_et.h
employee.o: ../include/sms_app.h ../include/mit-copyright.h
