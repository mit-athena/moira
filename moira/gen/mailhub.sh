#!/bin/sh
MR_MKCRED=47836474
/bin/cp /dev/null /u1/usr/lib/aliases.new.dir
/bin/cp /dev/null /u1/usr/lib/aliases.new.pag
/usr/lib/sendmail -bi -oA/u1/usr/lib/aliases.new
if [ $? != 0 ]; then
	exit $MR_MKCRED
	fi

kill `/bin/ps ax | /bin/grep "accepting connections" | /bin/grep -v grep | /bin/awk '{print $1}'`
/bin/mv /u1/usr/lib/aliases /u1/usr/lib/aliases.old
/bin/mv /u1/usr/lib/aliases.dir /u1/usr/lib/aliases.old.dir
/bin/mv /u1/usr/lib/aliases.pag /u1/usr/lib/aliases.old.pag
/bin/mv /u1/usr/lib/aliases.new /u1/usr/lib/aliases
/bin/mv /u1/usr/lib/aliases.new.dir /u1/usr/lib/aliases.dir
/bin/mv /u1/usr/lib/aliases.new.pag /u1/usr/lib/aliases.pag
/usr/lib/sendmail -bd -q30m

/bin/rm -f $0
exit 0

# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/mailhub.sh,v 1.1 1990-04-10 17:03:26 mar Exp $
