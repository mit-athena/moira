#!/bin/sh
# 
# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/stellar.sh,v 1.1 2002-01-18 06:54:45 zacheiss Exp $

PATH=/usr/bin:/bin:/usr/sbin:/sbin

if [ -d /var/athena ] && [ -w /var/athena ]; then
    exec >/var/athena/moira_update.log 2>&1
else 
    exec >/tmp/moira_update.log 2>&1
fi

# The following exit codes are defined and MUST BE CONSISTENT with the 
# error codes the library uses:
MR_MISSINGFILE=47836473
MR_MKCRED=47836474
MR_TARERR=47836476

TARFILE=/var/tmp/stellar.out
SRCDIR=/var/tmp/stellar.dir

uchost=`hostname | tr '[a-z]' '[A-Z]'`
echo $uchost | egrep -e "\." >/dev/null
if [ $? != 0 ]; then
    uchost=$uchost.MIT.EDU
fi

# Alert if the tar file does not exist
test -r $TARFILE || exit $MR_MISSINGFILE

# Make a temporary directory to unpack the tar file into
rm -rf $SRCDIR
mkdir $SRCDIR || exit $MR_MKCRED
cd $SRCDIR || exit $MR_MKCRED

# Extract everything
tar xpf $TARFILE || exit $MR_TARERR

if [ -f "$uchost" ]; then
    /var/local/stellar-support/moira-to-stellar.perl
else
    exit $MR_MISSINGFILE
fi

# cleanup
cd /
rm -rf $SRCDIR
test -f $TARFILE && rm -f $TARFILE
test -f $0 && rm -f $0

exit 0
