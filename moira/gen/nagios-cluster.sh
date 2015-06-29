#!/bin/sh
# $Id: nagios-cluster.sh 3956 2010-01-05 20:56:56Z zacheiss $

# The following exit codes are defined and MUST BE CONSISTENT with
# error codes the library uses:
MR_MKCRED=47836474

file=/var/nagios-cluster/etc/moira-cluster.cfg
newfile=$file.new
initscript=/etc/init.d/nagios-cluster

if [ -d /var/athena ] && [ -w /var/athena ]; then
    exec >/var/athena/moira_update.log 2>&1
else 
    exec >/tmp/moira_update.log 2>&1
fi

chmod 644 $file.new
diff -q $file $newfile
status=$?
if [ $status -eq 0 ]; then
    echo "No changes"
    exit 0
elif [ $status -eq 1 ]; then
    mv $newfile $file
    $initscript restart
    if [ $? -eq 0 ]; then 
	exit 0
    else
	exit $MR_MKCRED
    fi
else
    exit $MR_MKCRED
fi
