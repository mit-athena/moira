#!/bin/sh
# $Id: acl.sh,v 1.1 2000-01-06 21:13:51 danw Exp $

# The following exit codes are defined and MUST BE CONSISTENT with the
# error codes the library uses:
MR_MISSINGFILE=47836473
MR_MKCRED=47836474
MR_TARERR=47836476

status=0

PATH=/bin:/usr/bin
TARFILE=/var/tmp/acl.out

# Alert if the tar file does not exist
test -r $TARFILE || exit $MR_MISSINGFILE

# Make a temporary directory to unpack the tar file into
mkdir /var/tmp/acltmp || exit $MR_MKCRED
cd /var/tmp/acltmp || exit $MR_MKCRED
tar xf $TARFILE || exit $MR_TARERR

# Copy over each file which is new or has changed
for file in `find . -type f -print | sed -e 's/^\.//'`; do
    if [ $file = /etc/passwd -o $file = /etc/passwd.local ]; then
	# Make sure that there is a head file, or that the generated
	# file contains an entry for root.
	if [ ! -f $file.head ]; then
	    if egrep -s ^root: .$file; then
		:
	    else
		status=MR_MISSINGFILE
		break
	    fi
        fi
    elif [ $file = /etc/group -o $file = /etc/group.local ]; then
	# Make sure that there is a head file, or that the generated
	# file contains a group with gid 0.
	if [ ! -f $file.head ]; then
	    if awk -F: '$3 == "0" { exit 1; }'; then
		status=MR_MISSINGFILE
		break
	    fi
        fi
    fi

    cat $file.head .$file $file.tail > .$file.$$

    if [ -f $file ]; then
	if cmp -s .$file.$$ $file; then
	    :
	else
	    mv .$file.$$ $file
	fi
    else
	mv .$file.$$ $file
    fi
done

# cleanup
cd /
rm -rf /var/tmp/acltmp
test -f $TARFILE && rm -f $TARFILE
test -f $0 && rm -f $0

exit $status
