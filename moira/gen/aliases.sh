#!/bin/sh
MR_MKCRED=47836474
if [ `hostname`x != MITx ]; then
	/bin/diff /usr/lib/aliases /usr/lib/aliases.new > /tmp/aliasdiff
	fi
/bin/cp /dev/null /usr/lib/aliases.new.dir
/bin/cp /dev/null /usr/lib/aliases.new.pag
/usr/lib/sendmail -bi -oA/usr/lib/aliases.new
if [ $? != 0 ]; then
	exit $MR_MKCRED
	fi
if [ `hostname`x = MITx ]; then
	kill `/bin/ps ax | /bin/grep "accepting connections" | /bin/grep -v grep | /bin/awk '{print $1}'`
	/bin/mv /usr/lib/aliases /usr/lib/aliases.old
	/bin/mv /usr/lib/aliases.dir /usr/lib/aliases.old.dir
	/bin/mv /usr/lib/aliases.pag /usr/lib/aliases.old.pag
	/bin/mv /usr/lib/aliases.new /usr/lib/aliases
	/bin/mv /usr/lib/aliases.new.dir /usr/lib/aliases.dir
	/bin/mv /usr/lib/aliases.new.pag /usr/lib/aliases.pag
	/usr/lib/sendmail -bd -q30m
	fi
/bin/rm -f $0
exit 0

# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/aliases.sh,v 1.4 1990-03-19 19:05:53 mar Exp $
