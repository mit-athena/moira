#!/bin/sh
# $Id: nagios-colo.sh,v 1.1 2007-08-17 15:26:40 zacheiss Exp $

file=/var/nagios/etc/moira-colo-hosts.cfg

if [ -d /var/athena ] && [ -w /var/athena ]; then
    exec >/var/athena/moira_update.log 2>&1
else 
    exec >/tmp/moira_update.log 2>&1
fi

chmod 644 $file

exit 0
