#!/bin/sh
#
#
DIR=/warehouse/transfers/moirausr
TAR_FILE=$DIR/moira.tar.Z

PATH="/etc:/bin:/usr/bin:/usr/etc:/usr/athena/etc"

if [ -d /var/athena ] && [ -w /var/athena ]; then
    exec >/var/athena/moira_update.log 2>&1
else 
    exec >/tmp/moira_update.log 2>&1
fi

# The following exit codes are defined and MUST BE CONSISTENT with the
# error codes the library uses:
set MR_MISSINGFILE = 	47836473
set MR_TARERR = 	47836476

cd $DIR

echo extracting $TAR_FILE

if [ ! -f $TAR_FILE ]; then
	exit $MR_MISSINGFILE
fi

if zcat < $TAR_FILE | tar xvf -; then
	echo Extract successful
else
	exit $MR_TARERR
fi

chmod 640 username_id.*

rm -f $0
echo removed self

exit 0
