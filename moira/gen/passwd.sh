#!/bin/sh
SMS_MKCRED=47836474
/site/lookup/db_update
if [ $? != 0 ]; then
	exit $SMS_MKCRED
	fi
/bin/rm -f $0
exit 0

# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/passwd.sh,v 1.1 1989-08-27 20:35:10 mar Exp $
