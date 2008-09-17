#!/bin/sh
# $Id$

if [ -d /var/athena ] && [ -w /var/athena ]; then 
    exec >/var/athena/moira_update.log 2>&1
else 
    exec >/tmp/moira_update.log 2>&1
fi

# The following exit codes are defined and MUST BE CONSISTENT with the
# error codes the library uses:
MR_MISSINGFILE=47836473

PATH=/bin
OUTFILE=/export/home/events/feeds/moira-events.out

# Alert if the output file doesn't exist
test -r $OUTFILE || exit $MR_MISSINGFILE

# Set the perms usefully
chown root $OUTFILE
chgrp 0 $OUTFILE
chmod 644 $OUTFILE

# cleanup
test -f $0 && rm -f $0

exit 0
