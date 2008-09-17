#!/bin/sh

if [ -d /var/athena ] && [ -w /var/athena ]; then
    exec >/var/athena/moira_update.log 2>&1
else 
    exec >/tmp/moira_update.log 2>&1
fi
 
root=/usr/local/nic/db
PATH=/sbin:/bin:root/bin
 
if [ ! -r $root/data/moira.input ]; then
        echo "no data found"
        exit 1
fi
 
echo `date` loading database >> /var/log/dbload
$root/bin/moirain $root/data/moira.input >> /var/log/dbload 2>&1
if [ $? != 0 ]; then
        exit $MR_MKCRED
fi
 
rm -f $0
exit 0 

# $Header$
