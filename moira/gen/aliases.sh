#!/bin/sh
PATH=/bin:/usr/ucb:/usr/bin

MR_MKCRED=47836474

cat /usr/lib/local-aliases >> /usr/lib/aliases.new
diff /usr/lib/aliases /usr/lib/aliases.new > /tmp/aliasdiff
cp /dev/null /usr/lib/aliases.new.dir
cp /dev/null /usr/lib/aliases.new.pag
/usr/lib/sendmail -bi -oA/usr/lib/aliases.new
if [ $? != 0 ]; then
	exit $MR_MKCRED
	fi
kill `ps ax | grep "accepting connections" | grep -v grep | awk '{print $1}'`
mv /usr/lib/aliases /usr/lib/aliases.old
mv /usr/lib/aliases.dir /usr/lib/aliases.old.dir
mv /usr/lib/aliases.pag /usr/lib/aliases.old.pag
mv /usr/lib/aliases.new /usr/lib/aliases
mv /usr/lib/aliases.new.dir /usr/lib/aliases.dir
mv /usr/lib/aliases.new.pag /usr/lib/aliases.pag
/usr/lib/sendmail -bd -q30m

rm -f $0
exit 0

# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/aliases.sh,v 1.6 1991-09-03 15:45:31 mar Exp $
