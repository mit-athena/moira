#!/bin/sh
# 
# $Header: /afs/athena.mit.edu/astaff/project/moiradev/repository/moira/gen/stel
lar.sh,v 1.1 2002/01/18 06:54:45 zacheiss Exp $

PATH=/usr/bin:/bin:/usr/sbin:/sbin

if [ -d /var/athena ] && [ -w /var/athena ]; then
    exec >/var/athena/moira_update.log 2>&1
else 
    exec >/tmp/moira_update.log 2>&1
fi

# The following exit codes are defined and MUST BE CONSISTENT with the 
# error codes the library uses:
MR_MISSINGFILE=47836473
MR_TARERR=47836476

OUTFILE=/var/tmp/ip-billing.out

# Alert if the output file doesn't exist
test -r $OUTFILE || exit $MR_MISSINGFILE

# unpack the file
(cd /var/tmp && tar xf $OUTFILE) || exit $MR_TARERR

# cleanup
rm -f $OUTFILE
test -f $0 && rm -f $0

exit 0
