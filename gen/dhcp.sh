#! /bin/sh
# $Id: dhcp.sh 3956 2010-01-05 20:56:56Z zacheiss $

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

PATH=/usr/bin:/bin
TARFILE=/var/tmp/dhcp.out
BOOTGEN=/var/boot/dhcpd.conf.print
BOOTHEAD=/var/boot/dhcpd.conf.head
BOOTFOOT=/var/boot/dhcpd.conf.foot
BOOTFILE=/var/boot/dhcpd.conf
PSWDFILE=/var/boot/hp.add

# Alert if the tar file or other needed files do not exist
test -r $TARFILE || exit $MR_MISSINGFILE
test -r $BOOTHEAD || exit $MR_MISSINGFILE
test -r $BOOTFOOT || exit $MR_MISSINGFILE
test -r $PSWDFILE || exit $MR_MISSINGFILE

cd /
tar xf $TARFILE || exit $MR_TARERR

# Append passwords, etc., to the new files
for f in `find /var/boot/hp -name \*.new -print`; do
    cat $PSWDFILE >> $f
    mv $f /var/boot/hp/`basename $f .new`
done

# Build full bootptab
cat $BOOTHEAD $BOOTGEN $BOOTFOOT > $BOOTFILE

# kill and rerun dhcpd
/etc/init.d/dhcpd stop
/etc/init.d/dhcpd start

# cleanup
test -f $TARFILE && rm -f $TARFILE
test -f $0 && rm -f $0

exit 0
