#!/bin/sh
# This script performs nfs updates on servers.
#
# $HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/gen/nfs.sh $ $Id: nfs.sh 3956 2010-01-05 20:56:56Z zacheiss $

# redirect output, and find the credentials directory (not robust, but
# works for now).
if [ -d /var/athena ] && [ -w /var/athena ]; then
    exec >/var/athena/moira_update.log 2>&1
else 
    exec >/tmp/moira_update.log 2>&1
fi

creddir=/var/athena
if [ -d /usr/etc ]; then
    creddir=/usr/etc
fi

# The following exit codes are defined and MUST BE CONSISTENT with the 
# error codes the library uses:
MR_NOCRED=47836470
MR_MISSINGFILE=47836473
MR_MKCRED=47836474
MR_TARERR=47836476

PATH=/etc:/bin:/usr/bin:/usr/etc:/usr/athena/etc

TARFILE=/var/tmp/nfs.out
SRCDIR=/var/tmp/nfs.dir

uchost=`hostname | tr '[a-z]' '[A-Z]'`
echo $uchost | egrep -e "\." >/dev/null
if [ $? != 0 ]; then
    uchost=$uchost.MIT.EDU
fi

# Alert if the tar file does not exist
test -r $TARFILE || exit $MR_MISSINGFILE

# Make a temporary directory to unpack the tar file into
rm -rf $SRCDIR
mkdir $SRCDIR || exit $MR_MKCRED
cd $SRCDIR || exit $MR_MKCRED

# Extract everything
tar xpf $TARFILE || exit $MR_TARERR

for type in dirs quotas; do
    echo "Installing ${type}:"
    for i in $uchost.$type; do
	if [ -f $i ]; then
	    # Convert from the filename HOST.@dev@device.type to /dev/device
	    dev=`echo $i | sed "s,.${type},," | sed "s,${uchost}.,," | \
		sed "s,@,/,g"`
	    echo ${uchost}:$dev
	    ./install_${type} $dev < $i
	    if [ $? != 0 ]; then
		exit $MR_NOCRED
	    fi
	    if [ ${type} = "quotas" ]; then
		./zero_quotas $dev < $i
		if [ $? != 0 ]; then
		    exit $MR_NOCRED
		fi
	    fi
	    mv -f $i /var/tmp
        fi
    done
done
    
# build new credentials files
rm -f $creddir/credentials.new
cp ${uchost}.cred $creddir/credentials.new
if [ $? != 0 ]; then
    exit $MR_NOCRED
fi
if [ -s $creddir/credentials.local ]; then
    cat $creddir/credentials.local >> $creddir/credentials.new
fi

# After this point, if /tmp gets cleared out by reactivate (which happens
# on a combined server/workstation), we don't care.
mkcred $creddir/credentials.new
if [ $? != 0 ]; then
    exit $MR_MKCRED
fi

# Try to install the files
for e in "" .dir .pag; do
    mv -f $creddir/credentials.new$e $creddir/credentials$e
done

# If any of them didn't get installed, fail
for e in "" .dir .pag; do
    if [ ! -f $creddir/credentials$e ]; then
	exit $MR_NOCRED
    fi
done

# cleanup
cd /
rm -rf $SRCDIR
test -f $TARFILE && rm -f $TARFILE
test -f $0 && rm -f $0

exit 0
