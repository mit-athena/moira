#	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dcm/Makefile,v $
#	$Author: mar $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dcm/Makefile,v 1.5 1989-07-12 18:12:36 mar Exp $
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

# Libraries that talk to the SMS server:
SERVER_LIBS = -lsmsglue -lsms -lgdb -lzephyr -lkrb -ldes -lcom_err $(QLIBS)

DCM_SRCS = dcm.c utils.c
DCM_OBJS = dcm.o utils.o ../update/sms_update.o


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
	install dcm ${SMS_DIR}/bin/dcm
	install startdcm ${SMS_DIR}/bin/startdcm

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

