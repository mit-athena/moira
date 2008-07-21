#! /bin/sh
# $Id: boot.sh,v 1.3 2000-05-08 18:30:30 zacheiss Exp $

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

PATH=/usr/bin
TARFILE=/var/tmp/boot.out
BOOTGEN=/var/boot/bootptab.print
BOOTLOCAL=/var/boot/bootptab.local
BOOTFILE=/var/boot/bootptab
PSWDFILE=/var/boot/hp.add

# Alert if the tar file or other needed files do not exist
test -r $TARFILE || exit $MR_MISSINGFILE
test -r $BOOTLOCAL || exit $MR_MISSINGFILE
test -r $PSWDFILE || exit $MR_MISSINGFILE

cd /
tar xf $TARFILE || exit $MR_TARERR

# Append passwords, etc., to the new files
for f in `find /var/boot/hp -name \*.new -print`; do
    cat $PSWDFILE >> $f
    mv $f /var/boot/hp/`basename $f .new`
done

# Build full bootptab
cat $BOOTLOCAL $BOOTGEN > $BOOTFILE

# cleanup
test -f $TARFILE && rm -f $TARFILE
test -f $0 && rm -f $0

exit 0
