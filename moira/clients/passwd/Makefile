# Makefile for Kerberos client software.
#
#	MIT Project Athena
#
#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/Makefile,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/passwd/Makefile,v 1.1 1987-06-23 23:10:13 ambar Exp $
#
DESTDIR=
DEFINES = -DDEBUG -O
PCDEFINES = -DPC
VXDEFINES = -DBSD42
CFLAGS = ${DEFINES} ${VXDEFINES}
CONFDIR = ${DESTDIR}/usr/athena
BINDIR = ${DESTDIR}/bin

EQLIB = -lm -lc

LIBDEP1 =  ../lib/krb/libkrb.a ../lib/des/bsd/libdes.a
LIBDEP2 = ../db/util/libkerb_db.a

PROGS= kinit kdestroy passwd list_tkts kchpass

#note -- all the include file and library dependencies are not yet
#		listed

all: 	${PROGS}

clean:	
	rm -f *.o *.b vxlogin pclogin.com *~
	rm -f ${PROGS} chsh chfn chhome list_tkts

kinit:		kinit.o $(LIBDEP1)
	cc -O -o kinit kinit.o ${LIBDEP1}

kdestroy:	kdestroy.o ${LIBDEP1}
	cc -o kdestroy kdestroy.o ${LIBDEP1}

list_tkts:	list_tkts.o ${LIBDEP1}
	cc -o list_tkts list_tkts.o ${LIBDEP1}

passwd:	passwd.o $(LIBDEP1) $(LIBDEP2) 
	cc -O -o  passwd passwd.o ${LIBDEP1} $(LIBDEP2) \
		-luserinfo ${EQLIB}

kchpass:	kchpass.o $(LIBDEP1)
	cc -o kchpass kchpass.o ${LIBDEP1}

ksu:	ksu.o $(LIBDEP1) ../services/rcmd/rcmd.o ../services/rcmd/kparse.o
	cc -o ksu ksu.o ../services/rcmd/rcmd.o ../services/rcmd/kparse.o ${LIBDEP1}

# getusershell.o: getusershell.c

passwd.o: passwd.c passwd_server.h
	cc ${CFLAGS} -c passwd.c

#pclogin.com:	clogin.b  
#	cc86 -DPC -o pclogin kinit.b \
#	    -lkrb -lbsd4.2 -ltftp -ludp -l3com -lip -lnet -ltask -ldes -lh19
		
#x.com:	x.b 
#	cc86 -lkrb -DPC -o x x.b 

install:
	install -c -s kinit ${CONFDIR}/kinit
	install -c -s kdestroy ${CONFDIR}/kdestroy
	install -c -s list_tkts ${CONFDIR}/list_tkts
	install -c -s kchpass ${CONFDIR}/kchpass
	install -c -s -m 4755 -o root ksu ${CONFDIR}/ksu
	install -c -s -m 4755 -o root passwd ${BINDIR}/passwd
	for i in chsh chfn chhome; do \
		rm -f ${BINDIR}/$$i; ln ${BINDIR}/passwd ${BINDIR}/$$i; \
	done

# .SUFFIXES: .o .c .b
# 
# .c.o:	$*.c 
# 	cc $(DEFINES) $(VXDEFINES) -c $*.c
# 
# .c.b:	$*.c 
# 	cc86 $(DEFINES) $(PCDEFINES) -c $*.c

