#! /bin/sh
# $Id: printng.sh,v 1.2 1999-06-20 07:20:31 jweiss Exp $

# The following exit codes are defined and MUST BE CONSISTENT with the
# error codes the library uses:
MR_MISSINGFILE=47836473
MR_MKCRED=47836474
MR_TARERR=47836476

PATH=/bin
TARFILE=/var/tmp/print.out
PCLOCAL=/etc/printcap.local
PCGEN=/etc/printcap.moira
PRINTCAP=/etc/printcap

# Alert if the tar file or other needed files do not exist
test -r $TARFILE || exit $MR_MISSINGFILE
test -r $PCLOCAL || exit $MR_MISSINGFILE

# Unpack the tar file, getting only files that are newer than the
# on-disk copies (-u).
cd /
tar xf $TARFILE || exit $MR_TARERR

# Build full printcap and spools
cat $PCLOCAL $PCGEN > $PRINTCAP
/usr/athena/etc/checkpc -f
/usr/athena/etc/lpc reread

# cleanup
test -f $TARFILE && rm -f $TARFILE
test -f $0 && rm -f $0

exit 0

