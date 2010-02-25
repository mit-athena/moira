#! /bin/sh
# $Id: cups-print.sh 3956 2010-01-05 20:56:56Z zacheiss $

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

# We need to kill off CUPS to prevent it from overwriting
# state data whilst updating
/etc/init.d/cups stop

/etc/cups/bin/check-disabled.pl 2>/dev/null

# Unpack the tar file, getting only files that are newer than the
# on-disk copies (-u).
cd /
tar xf $TARFILE || exit $MR_TARERR

/etc/cups/bin/post-dcm-disable.pl 2>/dev/null
if [ -s /etc/cups/printers.conf.tmp ]; then
    mv /etc/cups/printers.conf.tmp /etc/cups/printers.conf
fi

/etc/init.d/cups start

# Now, make a stab at the PPD file.  This is okay to run after
# because CUPS will pick up the new PPDs later
/etc/cups/bin/gen-ppd.pl

if [ $? != 0 ]; then
    exit $MR_MKCRED
fi

# if Samba-enabled, then restart it too to have it pick up
# new definitions
if [ -x /etc/init.d/smb ]; then
       /etc/init.d/smb restart
fi

# cleanup
test -f $TARFILE && rm -f $TARFILE
test -f $0 && rm -f $0

exit 0

