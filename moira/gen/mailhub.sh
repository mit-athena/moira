#!/bin/sh
PATH=/bin:/usr/ucb:/usr/bin
root=/usr/local/sendmail

MR_MKCRED=47836474

cat $root/etc/aliases.legacy > $root/etc/aliases.tmp
cat $root/etc/aliases.new >> $root/etc/aliases.tmp
cat $root/etc/aliases.local >> $root/etc/aliases.tmp
mv $root/etc/aliases.tmp $root/etc/aliases.new

cp /dev/null $root/etc/aliases.new.db

$root/sbin/sendmail -bi -oA$root/etc/aliases.new
if [ $? != 0 ]; then
	exit $MR_MKCRED
fi

kill `ps -el | grep "sendmail" | egrep -v "grep|mqueue.stall" | awk '{print $4}'`
sleep 60

mv $root/etc/aliases $root/etc/aliases.old
mv $root/etc/aliases.db $root/etc/aliases.old.db
mv $root/etc/aliases.new $root/etc/aliases
mv $root/etc/aliases.new.db $root/etc/aliases.db

cd /usr/spool/mqueue
rm -f xf* tf* lf* nf*

#$root/sbin/sendmail -bd
#$root/sbin/sendmail -q20m

sh /etc/init.d/sendmail start

rm -f $0
exit 0

# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/mailhub.sh,v 1.9 2001-06-04 21:07:49 zacheiss Exp $
