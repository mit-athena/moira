#!/bin/sh
MR_MKCRED=47836474
/site/lookup/db_update
if [ $? != 0 ]; then
	exit $MR_MKCRED
	fi
/bin/rm -f $0
exit 0

# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/passwd.sh,v 1.2 1990-03-19 19:07:39 mar Exp $
