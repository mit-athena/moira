#!/bin/sh
# $Id: lpcaccess.sh,v 1.1 2008-07-10 16:58:41 zacheiss Exp $

outfile=/home/cups/lpcaccess.out
loadprog=/home/cups/update_acls

PATH="/etc:/bin:/usr/bin:/usr/etc:/usr/athena/etc"

if [ -d /var/athena ] && [ -w /var/athena ]; then
    exec >/var/athena/moira_update.log 2>&1
else
    exec >/tmp/moira_update.log 2>&1
fi

# The following exit codes are defined and MUST BE CONSISTENT with the
# error codes the library uses:
MR_MISSINGFILE=47836473
MR_MKCRED=47836474

if [ ! -f $outfile ]; then
    exit $MR_MISSINGFILE
fi

if [ ! -x $loadprog ]; then
    exit $MR_MISSINGFILE
fi

$loadprog

# Ignore exit status from $loadprog for now.
#if [ $? != 0 ]; then
#    exit $MR_MKCRED
#fi

exit 0
