#!/bin/sh
PATH=/bin:/usr/ucb:/usr/bin

MR_MKCRED=47836474

cat /usr/lib/local-aliases >> /u1/usr/lib/aliases.new
cp /dev/null /u1/usr/lib/aliases.new.dir
cp /dev/null /u1/usr/lib/aliases.new.pag
/usr/lib/sendmail -bi -oA/u1/usr/lib/aliases.new
if [ $? != 0 ]; then
	exit $MR_MKCRED
	fi

kill `ps ax | grep "accepting connections" | grep -v grep | awk '{print $1}'`
mv /u1/usr/lib/aliases /u1/usr/lib/aliases.old
mv /u1/usr/lib/aliases.dir /u1/usr/lib/aliases.old.dir
mv /u1/usr/lib/aliases.pag /u1/usr/lib/aliases.old.pag
mv /u1/usr/lib/aliases.new /u1/usr/lib/aliases
mv /u1/usr/lib/aliases.new.dir /u1/usr/lib/aliases.dir
mv /u1/usr/lib/aliases.new.pag /u1/usr/lib/aliases.pag
/usr/lib/sendmail -bd -q30m

rm -f $0
exit 0

# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/mailhub.sh,v 1.4 1991-09-03 15:47:12 mar Exp $
