#! /bin/sh
# $Id$

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

PATH=/usr/local/samba/bin:/usr/bin:/bin; export PATH
TARFILE=/var/tmp/cups-cluster.out
CUPSLOCAL=/etc/cups
SAMBAPASSWD=`cat /etc/cups/sambapasswd`

# Alert if the tar file or other needed files do not exist
test -r $TARFILE || exit $MR_MISSINGFILE
test -d $CUPSLOCAL || exit $MR_MISSINGFILE

# We need to kill off CUPS to prevent it from overwriting
# state data whilst updating
/etc/init.d/cups stop

/etc/cups/bin/check-disabled.pl 2>/dev/null

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

test -r /etc/cups/all-queues || exit $MR_MISSINGFILE

# Generate list of all queues.
rm -f /etc/cups/all-queues.new
rm -f /etc/cups/all-queues.tmp
grep "<Printer" /etc/cups/printers.conf | awk '{print $2}' | sed -e 's/>//' > /etc/cups/all-queues.tmp
grep '^Printer' /etc/cups/classes.conf | awk '{print $2}' >> /etc/cups/all-queues.tmp
sort -u /etc/cups/all-queues.tmp > /etc/cups/all-queues.new

# Sanity check that the file isn't empty.
test -s /etc/cups/all-queues.new || exit $MR_MKCRED

rm -f /etc/cups/all-queues.tmp
mv /etc/cups/all-queues /etc/cups/all-queues.old && mv /etc/cups/all-queues.new /etc/cups/all-queues

# Generate list of new queues since the last time we ran.
newqueues=`comm -13 /etc/cups/all-queues.old /etc/cups/all-queues`
for queue in $newqueues; do
    # If PPD file doesn't exist, cupsaddsmb will bomb out.
    if [ -f /etc/cups/ppd/$queue.ppd ]; then
        # Add this queue to SMB service advertisements.
	/usr/sbin/cupsaddsmb -v -U root%$SAMBAPASSWD -W PRINTERS $queue
	if [ $? != 0 ]; then
	    echo "Failed to configure $queue for SMB printing."
	fi
    fi
done

# cleanup
test -f $TARFILE && rm -f $TARFILE
test -f $0 && rm -f $0

exit 0

