#!/bin/sh
PATH=/bin:/usr/ucb:/usr/bin

if [ -d /var/athena ] && [ -w /var/athena ]; then
    exec >/var/athena/moira_update.log 2>&1
else
    exec >/tmp/moira_update.log 2>&1
fi

MR_MKCRED=47836474

cat /usr/lib/aliases.legacy > /usr/lib/aliases.tmp
cat /usr/lib/aliases.new >> /usr/lib/aliases.tmp
cat /usr/lib/aliases.local >> /usr/lib/aliases.tmp
mv aliases.tmp aliases.new

cp /dev/null /usr/lib/aliases.new.dir
cp /dev/null /usr/lib/aliases.new.pag

/usr/lib/sendmail -bi -oA/usr/lib/aliases.new
if [ $? != 0 ]; then
	exit $MR_MKCRED
	fi

kill `ps ax | grep "sendmail" | egrep -v "grep|mqueue.stall" | awk '{print $1}'`
sleep 60

mv /usr/lib/aliases /usr/lib/aliases.old
mv /usr/lib/aliases.dir /usr/lib/aliases.old.dir
mv /usr/lib/aliases.pag /usr/lib/aliases.old.pag
mv /usr/lib/aliases.new /usr/lib/aliases
mv /usr/lib/aliases.new.dir /usr/lib/aliases.dir
mv /usr/lib/aliases.new.pag /usr/lib/aliases.pag

cd /usr/spool/mqueue
rm -f xf* tf* lf* nf*

/usr/lib/sendmail -bd
/usr/lib/sendmail -q20m

rm -f $0
exit 0

# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/mailhub.sh,v 1.8 2000-05-08 18:30:30 zacheiss Exp $
