#!/bin/csh -f
# This script performs updates of hesiod files on hesiod servers.
# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/hesiod.sh,v 1.14 1998-01-05 19:52:44 danw Exp $

set path=(/etc /bin /usr/bin /usr/etc /usr/athena/etc)

# The following exit codes are defined and MUST BE CONSISTENT with the
# MR error codes the library uses:
set MR_HESFILE = 	47836472
set MR_MISSINGFILE = 	47836473
set MR_NAMED = 		47836475
set MR_TARERR = 	47836476

umask 22

# File that will contain the necessary information to be updated
set TARFILE=/var/tmp/hesiod.out
# Directory into which we will empty the tarfile
set SRC_DIR=/etc/athena/_nameserver
# Directory into which we will put the final product
set DEST_DIR=/etc/athena/nameserver

# Create the destination directory if it doesn't exist
if (! -d $DEST_DIR) then
   rm -f $DEST_DIR
   mkdir $DEST_DIR
   chmod 755 $DEST_DIR
endif

# If $SRC_DIR does not already exist, make sure that it gets created
# on the same parition as $DEST_DIR.
if (! -d $SRC_DIR) then
	chdir $DEST_DIR
	mkdir ../_nameserver
	chdir ../_nameserver
	if ($SRC_DIR != `pwd`) then
		ln -s `pwd` $SRC_DIR
	endif
endif

# Alert if tarfile doesn't exist
if (! -r $TARFILE) exit $MR_MISSINGFILE

# Empty the tar file one file at a time and move each file to the
# appropriate place only if it is not zero length.
cd $SRC_DIR
foreach  file (`tar tf $TARFILE | awk '{print $1}' | sed 's;/$;;'`)
   if (. == $file) continue

   rm -rf $file
   echo extracting $file
   tar xf $TARFILE $file
   # Don't put up with errors extracting the information
   if ($status) exit $MR_TARERR
   # Make sure the file is not zero-length
   if (! -z $file) then
      mv -f $file $DEST_DIR
      if ($status != 0) exit $MR_HESFILE
   else
      rm -f $file
      exit $MR_MISSINGFILE
   endif
end

# Kill off the current named and remove the named.pid file.  It is
# important that this file be removed since the script uses its
# existance as evidence that named as has been successfully restarted.

# First, get statistics
rm -f /usr/tmp/named.stats
ln -s /var/named.stats /usr/tmp/named.stats
kill -6 `cat /etc/named.pid`
sleep 1
# Use /bin/kill because, due to a bug in some versions of csh, failure
# of a builtin will cause the script to abort
kill -KILL `cat /etc/named.pid`
rm -f /etc/named.pid

# Restart named.
# mv /etc/named.pid /etc/named.pid.restart
#(unlimit; /etc/named&)
/etc/named
#sleep 5
echo named started

# This timeout is implemented by having the shell check TIMEOUT times
# for the existance of /etc/named.pid and to sleep INTERVAL seconds
# between each check.

set TIMEOUT=60			# number of INTERVALS until timeout
set INTERVAL=60			# number of seconds between checks
set i = 0
while ($i < $TIMEOUT)
   sleep $INTERVAL
   if (-f /etc/named.pid) break
   @ i++
end
echo out of timeout loop
# Did it time out?
if ($i == $TIMEOUT) exit $MR_NAMED
echo no timeout
# Clean up!
rm -f $TARFILE
echo removed tarfile
rm -f $0
echo removed self

exit 0
