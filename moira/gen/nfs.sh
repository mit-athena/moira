#!/bin/csh -f
# This script performs nfs updates on servers.

# The following exit codes are defined and MUST BE CONSISTENT with the
# SMS error codes the library uses:
set SMS_NOCRED 47836470
set SMS_MKCRED 47836474
set SMS_TARERR 47836476

set path=(/etc /bin /usr/bin /usr/etc)

# The file containg the information for the update
set TARFILE=/tmp/nfs.out
# The directory into which we will empty the tarfile
set SRC_DIR=/tmp/nfs.dir

# Create a frash source directory
rm -f $SRC_DIR
mkdir $SRC_DIR
chmod 755 $SRC_DIR

# Alert if the tarfile does not exist
if (! -r $TARFILE) then
   exit $SMS_TARERR
endif

# Note that since SMS is going to be exported, assuming .MIT.EDU is 
# incorrect.  For now however, it is probably not worth the effort
# to canonicalize the hostname, especially with the upcoming update
# protocol redesign
set uchost=`/bin/hostname | tr a-z A-Z`.MIT.EDU

cd $SRC_DIR

# Only files starting with $uchost, install_, and list- are needed.
# The files starting with list- are needed because the credentials
# files are hard links to them.  Tar will fail if they are not extracted.
foreach file (`/bin/tar tf $TARFILE | awk '{print $1}'`)
   if (($file =~ ./${uchost}*) || \
       ($file =~ ./install_*) || \
       ($file =~ ./list-*)) then
      tar xf $TARFILE $file
      if ($status) exit $SMS_TARERR
   endif
end

foreach type (dirs quotas)
   echo "Installing ${type}:"
   foreach i ( ${uchost}*.${type} )
      # Convert the from the filename HOST.@dev@device.type to /dev/device
      set dev=`echo $i |  sed 's;.${type};;' | sed 's;${uchost}.;;' | \
          sed 's;@;/;g'`
      echo ${uchost}:$dev
      ./install_${type} $dev < $i
      if ($status) exit $$status
   end
end

# build new credentials files.
rm -f /usr/etc/credentials.new
cp ${uchost}.cred /usr/etc/credentials.new
if ($status) exit $SMS_SMS_NOCRED

/usr/etc/mkcred /usr/etc/credentials.new
if ($status) exit $SMS_MKCRED

# Try to install the files
foreach e ( "" .dir .pag)
   mv -f /usr/etc/credentials.new$e /usr/etc/credentials$e
end

# If any of them didn't get installed, fail
foreach e ( "" .dir .pag)
   if (! -e /usr/etc/credentials$e) exit $SMS_NOCRED
end

# cleanup
rm -f $TARFILE
cd $SRC_DIR/..
rm -rf $SRC_DIR
rm $0

exit 0

#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/nfs.sh,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/nfs.sh,v 1.5 1988-08-06 16:42:17 qjb Exp $
#
