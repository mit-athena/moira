#!/bin/sh

PATH=/bin:/usr/ucb:/usr/bin
MR_MKCRED=47836474

/site/lookup/db_update
if [ $? != 0 ]; then
	exit $MR_MKCRED
	fi
rm -f $0
exit 0

# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/passwd.sh,v 1.3 1991-09-03 15:48:24 mar Exp $
