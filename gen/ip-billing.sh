#!/bin/sh
# 
# $Header$

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
MR_MKCRED=47836474

OUTFILE=/var/tmp/ip-billing.out
OUTDIR=/var/tmp/ip-billing.dir

# Alert if the output file doesn't exist
test -r $OUTFILE || exit $MR_MISSINGFILE

# Make a temporary directory to unpack files into.
rm -rf $OUTDIR
mkdir $OUTDIR || exit $MR_MKCRED
cd $OUTDIR || exit $MR_MKCRED

# unpack the file
tar xpf $OUTFILE || exit $MR_TARERR
for i in ctl dat; do
    if [ ! -f moira.$i ]; then
	exit $MR_MISSINGFILE
    fi
    mv moira.$i /var/tmp
done

# cleanup
cd /
rm -rf $OUTDIR
test -f $OUTFILE && rm -f $OUTFILE
test -f $0 && rm -f $0

exit 0
