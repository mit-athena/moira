#!/bin/sh
PATH=/bin:/usr/ucb:/usr/bin
root=/usr/local/sendmail

MR_MKCRED=47836474

cat $root/etc/aliases.legacy > $root/etc/aliases.tmp
cat $root/etc/aliases.new >> $root/etc/aliases.tmp
cat $root/etc/aliases.local >> $root/etc/aliases.tmp
mv $root/etc/aliases.tmp $root/etc/aliases.new

cp /dev/null $root/etc/aliases.new.db

$root/sbin/sendmail -bi -oA$root/etc/aliases.new -C$root/etc/sendmail.cf
if [ $? != 0 ]; then
    exit $MR_MKCRED
fi

mv $root/etc/aliases $root/etc/aliases.old
mv $root/etc/aliases.db $root/etc/aliases.old.db
mv $root/etc/aliases.new $root/etc/aliases
mv $root/etc/aliases.new.db $root/etc/aliases.db

rm -f $0
exit 0

# $HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/gen/aliases.sh $ $Id: aliases.sh 3956 2010-01-05 20:56:56Z zacheiss $
