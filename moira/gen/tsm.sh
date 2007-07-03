#!/bin/sh
# $Id: tsm.sh,v 1.2 2007-07-03 19:22:09 zacheiss Exp $

outfile=/tsm-mit/data/tsmlists
loadprog=/tsm-mit/bin/loadAdminTable.pl

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
