#!/bin/sh

MR_TARERR=47836476
MR_MISSINGFILE=47836473

INFILE=/tmp/prlogger.out
OUTFILE=/usr/spool/quota/users/prlogger

if [ ! -r $INFILE ]; then
	exit $MR_MISSINGFILE
	fi

/bin/mv $INFILE $OUTFILE
if [ $? != 0 ]; then
	exit $MR_TARERR
	fi

/usr/spool/quota/users/moira_update &

/bin/rm -f $0 $INFILE
exit 0

# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/prlogger.sh,v 1.1 1991-09-03 15:48:56 mar Exp $
