# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/Makefile,v 1.10 1990-03-19 19:05:40 mar Exp $

SRCS= passwd.c aliases.c hesiod.c nfs.c mailhub.c mitdir.c util.c setquota.c
LIBS= -L../lib -lmoira -lcom_err -lzephyr -lkrb -ldes \
	 /usr/rtingres/lib/libqlib /usr/rtingres/lib/compatlib
CFLAGS= -I../include -O
.SUFFIXES: .qc

.qc.c: 
	rm -f $*.c
	/usr/rtingres/bin/eqc -p $*

all: passwd.gen aliases.gen hesiod.gen nfs.gen mailhub.gen mitdir.gen setquota

passwd.gen: passwd.o util.o
	cc -o passwd.gen passwd.o util.o ${LIBS}

passwd.o: passwd.c

aliases.gen: aliases.o util.o
	cc -o aliases.gen aliases.o util.o ${LIBS}

aliases.o: aliases.c

hesiod.gen: hesiod.o util.o
	cc -o hesiod.gen hesiod.o util.o ${LIBS}

hesiod.o: hesiod.c

nfs.gen: nfs.o util.o
	cc -o nfs.gen nfs.o util.o ${LIBS}

nfs.o: nfs.c

mailhub.gen: mailhub.o util.o
	cc -o mailhub.gen mailhub.o util.o ${LIBS}

mailhub.o: mailhub.c

mitdir.gen: mitdir.o util.o
	cc -o mitdir.gen mitdir.o util.o ${LIBS}

mitdir.o: mitdir.c

setquota: setquota.o
	cc -o setquota setquota.o

clean:
	rm -f *.o 
	rm -f passwd.c passwd.gen
	rm -f aliases.c aliases.gen
	rm -f hesiod.c hesiod.gen
	rm -f nfs.c nfs.gen
	rm -f mailhub.c mailhub.gen
	rm -f mitdir.c mitdir.gen

install: all
	install -c passwd.gen ../bin
	install -c aliases.gen ../bin
	install -c hesiod.gen ../bin
	install -c nfs.gen ../bin
	install -c aliases.sh ../bin
	install -c hesiod.sh ../bin
	install -c install_dirs ../bin
	install -c install_quotas ../bin
	install -c nfs.sh ../bin
	install -c passwd.sh ../bin
	install -c zero_quotas ../bin

depend:	${SRCS}
	mkdep ${CFLAGS} ${SRCS}

# DO NOT DELETE THIS LINE -- mkdep uses it.

passwd.o: passwd.c ../include/mit-copyright.h /usr/include/stdio.h
passwd.o: ../include/moira.h ../include/mr_et.h /usr/include/sys/types.h
passwd.o: /usr/include/sys/stat.h /usr/include/sys/time.h
passwd.o: /usr/include/sys/time.h
aliases.o: aliases.c ../include/mit-copyright.h /usr/include/stdio.h
aliases.o: /usr/include/string.h /usr/include/strings.h ../include/moira.h
aliases.o: ../include/mr_et.h ../include/moira_site.h
aliases.o: /usr/include/sys/types.h /usr/include/sys/stat.h
aliases.o: /usr/include/sys/time.h /usr/include/sys/time.h
hesiod.o: hesiod.c ../include/mit-copyright.h /usr/include/stdio.h
hesiod.o: ../include/moira.h ../include/mr_et.h ../include/moira_site.h
hesiod.o: /usr/include/sys/types.h /usr/include/sys/stat.h
hesiod.o: /usr/include/sys/time.h /usr/include/sys/time.h
hesiod.o: /usr/include/ctype.h
nfs.o: nfs.c ../include/mit-copyright.h /usr/include/stdio.h
nfs.o: ../include/moira.h ../include/mr_et.h ../include/moira_site.h
nfs.o: /usr/include/sys/types.h /usr/include/sys/stat.h
nfs.o: /usr/include/sys/time.h /usr/include/sys/time.h
mailhub.o: mailhub.c ../include/mit-copyright.h /usr/include/stdio.h
mailhub.o: /usr/include/string.h /usr/include/strings.h /usr/include/ctype.h
mailhub.o: ../include/moira.h ../include/mr_et.h ../include/moira_site.h
mailhub.o: /usr/include/sys/types.h /usr/include/sys/stat.h
mailhub.o: /usr/include/sys/time.h /usr/include/sys/time.h
mitdir.o: mitdir.c ../include/mit-copyright.h /usr/include/stdio.h
mitdir.o: /usr/include/string.h /usr/include/strings.h ../include/moira.h
mitdir.o: ../include/mr_et.h ../include/moira_site.h /usr/include/sys/types.h
mitdir.o: /usr/include/sys/stat.h /usr/include/sys/time.h
mitdir.o: /usr/include/sys/time.h
util.o: util.c ../include/mit-copyright.h /usr/include/stdio.h
util.o: /usr/include/sys/time.h /usr/include/sys/time.h ../include/moira.h
util.o: ../include/mr_et.h ../include/moira_site.h
setquota.o: setquota.c /usr/include/stdio.h /usr/include/ctype.h
setquota.o: /usr/include/mntent.h /usr/include/sys/file.h
setquota.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
setquota.o: /usr/include/sys/signal.h /usr/include/sys/types.h
setquota.o: /usr/include/sys/time.h /usr/include/sys/time.h
setquota.o: /usr/include/ufs/quota.h
