#! /bin/sh
# $Id: boot.sh,v 1.1 1999-01-27 19:39:22 danw Exp $

# The following exit codes are defined and MUST BE CONSISTENT with the
# error codes the library uses:
MR_MISSINGFILE=47836473
MR_MKCRED=47836474
MR_TARERR=47836476

PATH=...
TARFILE=/var/tmp/boot.out
BOOTGEN=/var/boot/bootptab.print
BOOTLOCAL=/var/boot/bootptab.local
BOOTFILE=/var/boot/bootptab
PSWDFILE=/var/boot/hp.add

# Alert if the tar file or other needed files do not exist
test -r $TARFILE || exit $MR_MISSINGFILE
test -r $BOOTLOCAL || exit $MR_MISSINGFILE
test -r $PSWDFILE || exit $MR_MISSINGFILE

# Unpack the tar file, getting only files that are newer than the
# on-disk copies (-u). When extracting files under /var/boot/hp,
# tack on a .new so we can recognize them in the next step
cd /
pax -ru -s '|/var/boot/hp/.*$/&.new/' -f $TARFILE || exit $MR_TARERR

# Append passwords, etc., to the new files
for f in find /var/boot/hp -name \*.new -print; do
    cat $PSWDFILE >> $f
    mv $f /var/boot/hp/`basename $f .new`
done

# Build full bootptab
cat $BOOTLOCAL $BOOTGEN > $BOOTFILE

# Signal bootpd? XXX

# Remove out-of-date hp files
find /var/boot/hp -mtime +1 -exec rm -f {} \;

# cleanup
test -f $TARFILE && rm -f $TARFILE
test -f $0 && rm -f $0

exit 0
