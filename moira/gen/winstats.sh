#!/bin/sh

dir=/var/ops/populate/stats
tarfile=/var/tmp/winstats.out

PATH="/etc:/bin:/usr/bin:/usr/etc:/usr/athena/etc"

if [ -d /var/athena ] && [ -w /var/athena ]; then
    exec >/var/athena/moira_update.log 2>&1
else
    exec >/tmp/moira_update.log 2>&1
fi

# The following exit codes are defined and MUST BE CONSISTENT with the
# error codes the library uses:
MR_MISSINGFILE=47836473
MR_TARERR=47836476

if [ ! -f $tarfile ]; then
    exit $MR_MISSINGFILE
fi

cd $dir
tar xf $tarfile

if [ $? != 0 ]; then
    exit $MR_TARERR;
fi

chmod 644 *

rm -f $0
exit 0


