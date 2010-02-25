#!/bin/sh
#
# $HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/gen/mailhub.sh $ $Id: mailhub.sh 3956 2010-01-05 20:56:56Z zacheiss $

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

if [ -r $root/etc/aliases.new ]; then
    chmod 644 $root/etc/aliases.new
else
    exit $MR_MISSINGFILE
fi

if [ ! -r $root/etc/aliases ]; then
    logger -p mail.error -t mailhub.sh "No current aliases file, aborting."
    exit $MR_NOCRED
fi

# Play it safe and be sure we have reasonable data
olines=`wc -l $root/etc/aliases |  awk '{print $1}'`
nlines=`wc -l $root/etc/aliases.new | awk '{print $1}'`
diff=`expr $nlines - $olines`
thresh=`expr $olines / 10`

# Catch the zero case
if [ $nlines -eq 0 ]; then
    logger -p mail.error -t mailhub.sh "Recieved empty aliases file, aborting."
    exit $MR_MISSINGFILE
fi

# If its a greater than 10% shift bomb out to be safe
if [ $diff -gt $thresh ]; then
    logger -p mail.error -t mailhub.sh "Alias changes threshold exceeded, aborting."
    exit $MR_NOCRED
fi

cp /dev/null $root/etc/aliases.new.db

$root/sbin/sendmail -bi -oA$root/etc/aliases.new
if [ $? != 0 ]; then
    exit $MR_MKCRED
fi

sh /etc/init.d/sendmail stop

mv $root/etc/aliases $root/etc/aliases.old
mv $root/etc/aliases.db $root/etc/aliases.old.db
mv $root/etc/aliases.new $root/etc/aliases
mv $root/etc/aliases.new.db $root/etc/aliases.db

sh /etc/init.d/sendmail start

# Make sure the sendmail daemons are indeed running
ps -ef | grep sendmail | grep -v grep > /dev/null 2>&1
if [ $? -ne 0 ]; then
    logger -p mail.error -t mailhub.sh "Sendmail failed to restart."
    exit $MR_MKCRED
fi

rm -f $0
exit 0
