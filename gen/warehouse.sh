#!/bin/sh
# $Id: warehouse.sh,v 1.5 2007-01-03 20:31:59 zacheiss Exp $

if [ -d /var/athena ] && [ -w /var/athena ]; then
    exec >/var/athena/moira_update.log 2>&1
else 
    exec >/tmp/moira_update.log 2>&1
fi

# The following exit codes are defined and MUST BE CONSISTENT with the
# error codes the library uses:
MR_MISSINGFILE=47836473

PATH="/etc:/bin:/usr/bin:/usr/etc:/usr/athena/etc"
OUTFILE=/warehouse/transfers/moirausr/username_id.map.txt

# Alert if the output file doesn't exist
test -r $OUTFILE || exit $MR_MISSINGFILE

# Set the perms usefully
chown root $OUTFILE
chgrp 0 $OUTFILE
chmod 644 $OUTFILE

# cleanup
test -f $0 && rm -f $0

exit 0
