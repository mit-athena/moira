#!/bin/sh
# $Id: nagios-printers.sh 3956 2010-01-05 20:56:56Z zacheiss $

file=/var/nagios/etc/moira-printers.cfg

if [ -d /var/athena ] && [ -w /var/athena ]; then
    exec >/var/athena/moira_update.log 2>&1
else 
    exec >/tmp/moira_update.log 2>&1
fi

chmod 644 $file

exit 0