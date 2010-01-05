#!/bin/sh
#
# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/access.sh,v 1.1 2003-10-21 18:35:48 zacheiss Exp $

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/etc:/usr/etc:/usr/athena/bin:/usr/local/bin
export PATH

if [ -d /var/athena ] && [ -w /var/athena ]; then
    exec >/var/athena/moira_update.log 2>&1
else
    exec >/tmp/moira_update.log 2>&1
fi

# The following exit codes are defined and MUST BE CONSISTENT with
# error codes the library uses:
MR_MKCRED=47836474
MR_MISSINGFILE=47836473
MR_NOCRED=47836470

root=/usr/local/sendmail

if [ -s $root/etc/access.new ]; then
    cp /dev/null $root/etc/access.tmp
    
    if [ -s $root/etc/efl-access ]; then
	cat $root/etc/efl-access >> $root/etc/access.tmp
    fi

    cat $root/etc/access.new >> $root/etc/access.tmp
    mv $root/etc/access.tmp $root/etc/access.new
    chmod 644 $root/etc/access.new
else
    exit $MR_MISSINGFILE
fi

if [ ! -s $root/etc/access ]; then
    logger -p mail.error -t access.sh "No current access file, aborting."
    exit $MR_NOCRED
fi

# Play it safe and be sure we have reasonable data
olines=`wc -l $root/etc/access |  awk '{print $1}'`
nlines=`wc -l $root/etc/access.new | awk '{print $1}'`
diff=`expr $nlines - $olines`
thresh=`expr $olines / 10`

# Catch the zero case
if [ $nlines -eq 0 ]; then
    logger -p mail.error -t access.sh "Recieved empty access file, aborting."
    exit $MR_MISSINGFILE
fi

# If its a greater than 10% shift bomb out to be safe
if [ $diff -gt $thresh ]; then
    logger -p mail.error -t access.sh "Access changes threshold exceeded, aborting."
    exit $MR_NOCRED
fi

cp /dev/null $root/etc/access.new.db

$root/sbin/makemap btree $root/etc/access.new < $root/etc/access.new
if [ $? != 0 ]; then
    exit $MR_MKCRED
fi

mv $root/etc/access $root/etc/access.old
mv $root/etc/access.new $root/etc/access
rm -f $root/etc/access.old.db
ln $root/etc/access.db $root/etc/access.old.db
mv $root/etc/access.new.db $root/etc/access.db

rm -f $0
exit 0
