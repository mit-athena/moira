#!/bin/sh
# $Id: zephyr.sh,v 1.2 2000-01-22 03:11:42 danw Exp $

# The following exit codes are defined and MUST BE CONSISTENT with the
# error codes the library uses:
MR_MISSINGFILE=47836473
MR_MKCRED=47836474
MR_TARERR=47836476

hup=no
PATH=/bin
TARFILE=/var/tmp/zephyr.out

# Alert if the tar file does not exist
test -r $TARFILE || exit $MR_MISSINGFILE

# Make a temporary directory to unpack the tar file into
mkdir /etc/athena/zephyr/acl.new
cd /etc/athena/zephyr/acl.new || exit $MR_MKCRED
tar xf $TARFILE || exit $MR_TARERR

# Copy over each file which is new or has changed
for file in *; do
    if [ -f ../acl/$file ]; then
	if cmp -s $file ../acl/$file; then
	    mv $file ../acl/$file
	    hup=yes
	fi
    else
	mv $file ../acl/$file
	hup=yes
    fi
done

# HUP zephyrd if needed
case $hup in
yes)
    pid=`ps -ef | awk '/[^\]]zephyrd/ { print $2; }'`
    kill -HUP $pid
    sleep 5
    ;;
esac

# cleanup
rm -rf /etc/athena/zephyr/acl.new
test -f $TARFILE && rm -f $TARFILE
test -f $0 && rm -f $0

exit 0

