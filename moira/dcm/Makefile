#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dcm/Makefile,v $
#	$Author: mar $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dcm/Makefile,v 1.8 1990-03-19 18:52:41 mar Exp $
#
# (c) Copyright 1988 by the Massachusetts Institute of Technology.
# For copying and distribution information, please see the file
# <mit-copyright.h>.

SMS_DIR = ..
INGRES_DIR = /usr/rtingres
UPDATE_DIR = ${SMS_DIR}/update

INCDIR = -I${SMS_DIR}/include
CFLAGS = ${INCDIR} -g
LDFLAGS = -L${SMS_DIR}/lib -L${SMS_DIR}/server

QLIBS= ${INGRES_DIR}/lib/libqlib ${INGRES_DIR}/lib/compatlib

# Libraries that talk to the MOIRA server:
SERVER_LIBS = -lmoiraglue -lmoira -lgdb -lzephyr -lkrb -ldes -lcom_err $(QLIBS)

DCM_SRCS = dcm.c utils.c
DCM_OBJS = dcm.o utils.o ../update/moira_update.o


all: dcm startdcm

dcm: ${DCM_OBJS}
	rm -f $@
	${CC} ${LDFLAGS} -o $@ ${DCM_OBJS} ${SERVER_LIBS}

startdcm: startdcm.o
	${CC} ${LDFLAGS} -o startdcm startdcm.o

clean: 
	rm -f *.o core *~
	rm -f dcm startdcm .saber

install: dcm startdcm
	install -c dcm ${SMS_DIR}/bin/dcm
	install -c startdcm ${SMS_DIR}/bin/startdcm

lint:
	lint ${INCDIR} ${DCM_SRCS} | grep -v "possible pointer alignment"


.saber: Makefile
	rm -rf $@
	echo "setopt(\"load_options\", \"${CFLAGS} ${LDFLAGS}\");" >$@
	echo "#define DCM_SRCS \"${DCM_SRCS} ${SMS_UPDATE}\"" >>$@
	echo "#define FOO_SRCS \"${FOO_SRCS}\"" >>$@
	echo "#define GLUE_LIBS \"${GLUE_LIBS}\"" >>$@
	echo "#define SERVER_LIBS \"${SERVER_LIBS}\"" >>$@
	echo "suppress(285); " >>$@

depend:
	mkdep ${CFLAGS} ${DCM_SRCS}

# DO NOT DELETE THIS LINE -- mkdep uses it.

dcm.o: dcm.c /usr/include/stdio.h ../include/update.h /usr/include/sys/file.h
dcm.o: /usr/include/sys/time.h /usr/include/sys/time.h
dcm.o: /usr/include/sys/wait.h /usr/include/signal.h /usr/include/ctype.h
dcm.o: ../include/moira.h ../include/mr_et.h ../include/moira_site.h
dcm.o: ../include/mit-copyright.h ../include/dcm.h
utils.o: utils.c ../include/mit-copyright.h /usr/include/stdio.h
utils.o: /usr/include/strings.h /usr/include/errno.h /usr/include/varargs.h
utils.o: /usr/include/sys/types.h /usr/include/sys/time.h
utils.o: /usr/include/sys/time.h /usr/include/sys/file.h ../include/moira.h
utils.o: ../include/mr_et.h ../include/moira_site.h ../include/dcm.h
