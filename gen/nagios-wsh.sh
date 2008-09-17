#!/bin/sh
# $Id$

file=/var/nagios/etc/moira-wsh-hosts.cfg

if [ -d /var/athena ] && [ -w /var/athena ]; then
    exec >/var/athena/moira_update.log 2>&1
else 
    exec >/tmp/moira_update.log 2>&1
fi

chmod 644 $file

exit 0
