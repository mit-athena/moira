#!/bin/sh
# $Id: $

if [ -d /var/athena ] && [ -w /var/athena ]; then
    exec >/var/athena/moira_update.log 2>&1
else
    exec >/tmp/moira_update.log 2>&1
fi

# The following exit codes are defined and MUST BE CONSISTENT with the
# error codes the library uses:
MR_MISSINGFILE=47836473
MR_MKCRED=47836474

PATH=/usr/bin:/bin
TARFILE=/etc/sites/sites.moira.out

test -r $TARFILE || exit $MR_MISSINGFILE
test -f $0 && rm -f $0
exit 0
