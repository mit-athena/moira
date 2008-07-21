#!/bin/sh
# $Id: nagios-printers.sh,v 1.1 2006-01-31 21:50:25 zacheiss Exp $

file=/var/nagios/etc/moira-printers.cfg

if [ -d /var/athena ] && [ -w /var/athena ]; then
    exec >/var/athena/moira_update.log 2>&1
else 
    exec >/tmp/moira_update.log 2>&1
fi

chmod 644 $file

exit 0
