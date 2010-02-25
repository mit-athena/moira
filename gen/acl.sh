#!/bin/sh
# $Id: acl.sh 3956 2010-01-05 20:56:56Z zacheiss $
if [ -d /var/athena ] && [ -w /var/athena ]; then
    exec >/var/athena/moira_update.log 2>&1
else 
    exec >/tmp/moira_update.log 2>&1 
fi

# The following exit codes are defined and MUST BE CONSISTENT with the
# error codes the library uses:
MR_NOCRED=47836470
MR_MISSINGFILE=47836473
MR_MKCRED=47836474
MR_TARERR=47836476

status=0

PATH=/bin:/usr/bin
TARFILE=/var/tmp/acl.out
SRCDIR=/var/tmp/acltmp

# Alert if the tar file does not exist
test -r $TARFILE || exit $MR_MISSINGFILE

# Make a temporary directory to unpack the tar file into
rm -rf $SRCDIR
mkdir $SRCDIR || exit $MR_MKCRED
cd $SRCDIR || exit $MR_MKCRED
tar xpf $TARFILE || exit $MR_TARERR

# Copy over each file which is new or has changed
for file in `find . -type f -print | sed -e 's/^\.//'`; do
    if [ $file = /etc/passwd -o $file = /etc/passwd.local ]; then
	# Make sure that there is a head file, or that the generated
	# file contains an entry for root.
	if [ ! -f $file.head ]; then
	    if egrep -s ^root: .$file; then
		:
	    else
		status=$MR_MISSINGFILE
		break
	    fi
        fi
    elif [ $file = /etc/group -o $file = /etc/group.local ]; then
	# Make sure that there is a head file, or that the generated
	# file contains a group with gid 0.
	if [ ! -f $file.head ]; then
	    if awk -F: '$3 == "0" { exit 1; }' $file; then
		status=$MR_MISSINGFILE
		break
	    fi
        fi
    fi

    if [ -f $file.head ]; then
	head=$file.head
    else
	head=
    fi
    if [ -f $file.tail ]; then
	tail=$file.tail
    else
	tail=
    fi

    # Note that "$file" is a full pathname, and so ".$file" means
    # the copy of file in the directory hierarchy rooted at ".",
    # not "$file with a . prepended to its basename".

    # Create a tmp file with the correct owner and mode
    if [ -f $file ]; then
	cp -p $file $file.$$
    else
	cp -p .$file $file.$$
    fi

    # Now dump the correct data into the tmp file without changing its
    # owner and mode
    cat $head .$file $tail > $file.$$

    if cmp -s $file.$$ $file; then
	rm -f $file.$$
    else
	mv $file.$$ $file
    fi
done

# Test if a site-specific post dcm script exists, and run it if it does
if [ -x /etc/athena/postacldcm ]; then
    /etc/athena/postacldcm >/dev/null 2>&1
    if [ $? != 0 ]; then
	exit $MR_NOCRED
    fi
elif [ -x /usr/local/sbin/postacldcm ]; then
    /usr/local/sbin/postacldcm >/dev/null 2>&1
    if [ $? != 0 ]; then
	exit $MR_NOCRED
    fi
fi

# cleanup
cd /
rm -rf $SRCDIR
test -f $TARFILE && rm -f $TARFILE
test -f $0 && rm -f $0

exit $status
