#!/bin/csh -f -x
# This script performs updates of hesiod files on hesiod servers.  
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/hesiod.sh,v $
echo	'$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/hesiod.sh,v 1.7 1988-12-29 19:07:25 mar Exp $'

# The following exit codes are defined and MUST BE CONSISTENT with the
# SMS error codes the library uses:
set SMS_HESFILE = 	47836472
set SMS_MISSINGFILE = 	47836473
set SMS_NAMED = 	47836475
set SMS_TARERR = 	47836476

umask 22

# File that will contain the necessary information to be updated
set TARFILE=/tmp/hesiod.out
# Directory into which we will empty the tarfile
set SRC_DIR=/etc/athena/_nameserver
# Directory into which we will put the final product
set DEST_DIR=/etc/athena/nameserver
# Files to verify nameserver loaded
set CHECKFILES="passwd.db pobox.db cluster.db grplist.db group.db"

# Create the destination directory if it doesn't exist
if (! -d $DEST_DIR) then
   /bin/rm -f $DEST_DIR
   /bin/mkdir $DEST_DIR
   /bin/chmod 755 $DEST_DIR
endif

# If $SRC_DIR does not already exist, make sure that it gets created
# on the same parition as $DEST_DIR.
if (! -d $SRC_DIR) then
	chdir $DEST_DIR
	mkdir ../_nameserver
	chdir ../_nameserver
	if ($SRC_DIR != `pwd`) ln -s `pwd` $SRC_DIR
endif

# Alert if tarfile doesn't exist
if (! -r $TARFILE) exit $SMS_MISSINGFILE

# Empty the tar file one file at a time and move each file to the
# appropriate place only if it is not zero length. 
cd $SRC_DIR
foreach  file (`/bin/tar tf $TARFILE | awk '{print $1}' | sed 's;/$;;'`)
   if (. == $file) continue

   rm -rf $file
   echo extracting $file
   /bin/tar xf $TARFILE $file
   # Don't put up with errors extracting the information
   if ($status) exit $SMS_TARERR
   # Make sure the file is not zero-length
   if (! -z $file) then
      /bin/mv -f $file $DEST_DIR
      if ($status != 0) exit $SMS_HESFILE
   else
      /bin/rm -f $file
      exit $SMS_MISSINGFILE
   endif
end

# Kill off the current named and remove the named.pid file.  It is
# important that this file be removed since the script uses its
# existance as evidence that named as has been successfully restarted.

# Use /bin/kill because, due to a bug in some versions of csh, failure
# of a builtin will cause the script to abort
/bin/kill -KILL `/bin/cat /etc/named.pid`
rm -f /etc/named.pid

# Restart named.
(unlimit; /etc/named)

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

# Did it time out?
if ($i == $TIMEOUT) exit $SMS_NAMED

# Verify that the nameserver is answering queries for the new data
cd $DEST_DIR
foreach f ( $CHECKFILES )
        set temp=`tac $f | egrep -v '^;' | head -1`
        set hes=`/bin/echo $temp | awk '{print $1}'`
        set ent=`echo $hes | rev | sed 's/\(.*\)\.\(.*\)/\2/' | rev`
        set type=`echo $hes | rev | sed 's/\(.*\)\.\(.*\)/\1/' | rev`
        hesinfo $ent $type > /dev/null
	if ($status == 1) then
                exit $SMS_HESFILE
        endif
end

# Clean up!
/bin/rm -f $TARFILE
/bin/rm -f $0

exit 0
