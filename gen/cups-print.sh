#! /bin/sh
# $Id: cups-print.sh,v 1.2 2008-10-31 20:46:11 zacheiss Exp $

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

PATH=/bin
TARFILE=/var/tmp/cups-print.out
CUPSLOCAL=/etc/cups

# Alert if the tar file or other needed files do not exist
test -r $TARFILE || exit $MR_MISSINGFILE
test -d $CUPSLOCAL || exit $MR_MISSINGFILE

# Unpack the tar file, getting only files that are newer than the
# on-disk copies (-u).
cd /
tar xf $TARFILE || exit $MR_TARERR

# Now, make a stab at the PPD file.
/etc/cups/bin/gen-ppd.pl

/etc/init.d/cups restart
if [ $? != 0 ]; then
    exit $MR_MKCRED
fi

# cleanup
test -f $TARFILE && rm -f $TARFILE
test -f $0 && rm -f $0

exit 0

