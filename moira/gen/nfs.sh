#!/bin/csh -f
# This script performs nfs updates on servers.
#
# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/nfs.sh,v 1.23 1999-09-02 21:39:03 danw Exp $

# The following exit codes are defined and MUST BE CONSISTENT with the
# error codes the library uses:
set MR_NOCRED = 47836470
set MR_MKCRED = 47836474
set MR_TARERR = 47836476

set path=(/etc /bin /usr/bin /usr/etc /usr/athena/etc)
set nonomatch

# The file containg the information for the update
set TARFILE=/var/tmp/nfs.out
# The directory into which we will empty the tarfile
set SRC_DIR=/var/tmp/nfs.dir

# Alert if the tarfile does not exist
if (! -r $TARFILE) then
   exit $MR_TARERR
endif

# Create a fresh source directory
rm -rf $SRC_DIR
mkdir $SRC_DIR
chmod 755 $SRC_DIR

set uchost=`hostname | tr '[a-z]' '[A-Z]'`
if ($uchost !~ *.*) then
    set uchost=$uchost.MIT.EDU
endif

cd $SRC_DIR

# Just extract everything since some of what we need exists as
# hardlinks and tar doesn't deal well with extracting them in isolation.
tar xf $TARFILE
if ($status) exit $MR_TARERR

foreach type (dirs quotas)
   echo "Installing ${type}:"
   foreach i ( ${uchost}*.${type} )
     if (-e $i) then
	# Convert the from the filename HOST.@dev@device.type to /dev/device
	set dev=`echo $i |  sed "s,.${type},," | sed "s,${uchost}.,," | sed "s,@,/,g"`
	echo ${uchost}:$dev
	./install_${type} $dev < $i
	if ($status) exit $status
	if ($type == quotas) ./zero_quotas $dev < $i
	if ($status) exit $status
	# save the files used here for later debugging
	mv $i /var/tmp
     endif
   end
end

# find credentials directory (not robust, but works for the current
# situation)
set creddir = /var/athena
if (-d /usr/etc) then
    set creddir = /usr/etc
endif

# build new credentials files.
rm -f $creddir/credentials.new
cp ${uchost}.cred $creddir/credentials.new
if ($status) exit $MR_NOCRED
if (-e $creddir/credentials.local) then
    cat $creddir/credentials.local >> $creddir/credentials.new
endif

# After this point, if /tmp gets cleared out by reactivate (which
# happens on a combined server/workstation) we don't care.
mkcred $creddir/credentials.new
if ($status) exit $MR_MKCRED

# Try to install the files
foreach e ( "" .dir .pag)
   mv -f $creddir/credentials.new$e $creddir/credentials$e
end

# If any of them didn't get installed, fail
foreach e ( "" .dir .pag)
   if (! -e $creddir/credentials$e) exit $MR_NOCRED
end



# cleanup
if ( -f $TARFILE ) rm -f $TARFILE
if ( -d $SRC_DIR ) then
  cd $SRC_DIR/..
  rm -rf $SRC_DIR
endif
if ( -f $0 ) rm -r $0

exit 0
