#!/bin/sh
SMS_MKCRED=47836474
/bin/diff /usr/lib/aliases /usr/lib/aliases.new > /tmp/aliasdiff
/usr/lib/sendmail -bi -oA/usr/lib/aliases.new
if [ $? != 0 ]; then
	exit $SMS_MKCRED
	fi
/bin/rm -f $0
exit 0

# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/aliases.sh,v 1.1 1989-08-16 20:39:11 mar Exp $
