#!/bin/sh
#
#
DIR=/warehouse/transfers/moirausr
TAR_FILE=$DIR/moira.tar.Z

PATH="/etc:/bin:/usr/bin:/usr/etc:/usr/athena/etc"

# The following exit codes are defined and MUST BE CONSISTENT with the
# MR error codes the library uses:
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
