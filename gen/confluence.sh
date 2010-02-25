#!/bin/sh
# $Id: confluence.sh 3956 2010-01-05 20:56:56Z zacheiss $

if [ -d /var/athena ] && [ -w /var/athena ]; then
    exec >/var/athena/moira_update.log 2>&1
else 
    exec >/tmp/moira_update.log 2>&1
fi

# The following exit codes are defined and MUST BE CONSISTENT with the
# error codes the library uses:
MR_MISSINGFILE=47836473

PATH="/etc:/bin:/usr/bin:/usr/etc:/usr/athena/etc"
OUTFILE=/var/local/moira-feed/groups

# Alert if the output file doesn't exist
test -r $OUTFILE || exit $MR_MISSINGFILE

# Set the perms usefully
chgrp www $OUTFILE
chmod g+r $OUTFILE

# cleanup
test -f $0 && rm -f $0

exit 0
