#!/bin/sh
# This script performs updates of the web server.
# $HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/gen/www.sh $ $Id: www.sh 3956 2010-01-05 20:56:56Z zacheiss $

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/etc:/usr/etc:/usr/athena/bin:/usr/local/bin
export PATH

if [ -d /var/athena ] && [ -w /var/athena ]; then
    exec >/var/athena/moira_update.log 2>&1
else 
    exec >/tmp/moira_update.log 2>&1
fi

# The following exit codes are defined and MUST BE CONSISTENT with the
# error codes the library uses:
MR_HESFILE=47836472
MR_MISSINGFILE=47836473
MR_NAMED=47836475
MR_TARERR=47836476
MR_MKCRED=47836474

umask 22

TARFILE=/usr/tmp/www.out
SRC_DIR=/usr/local/apache/etc
BIN_DIR=/usr/local/apache/bin
DEST_DIR=/usr/local/apache/etc

if [ ! -r $TARFILE ]; then
	exit $MR_MISSINGFILE
fi

cd $SRC_DIR
rm -rf build
mkdir build
if [ $? != 0 ]; then
	exit $MR_TARERR
fi

rm -rf save2
if [ -d save ]; then
	mv -f save save2
fi
mkdir save
if [ $? != 0 ]; then
	exit $MR_TARERR
fi

cd build

for file in `tar tf $TARFILE | awk '{print $1}' | sed 's;/$;;'` ; do
   	if [ $file != . ]; then
   		rm -rf $file
   		echo extracting $file
   		tar xf $TARFILE $file
   		if [ $? != 0 ]; then
			 exit $MR_TARERR
		fi
		$BIN_DIR/dbmanage $file import $file.db 
		if [ $? != 0 ]; then
			exit $MR_MKCRED
		fi
		mv $DEST_DIR/$file $SRC_DIR/save/$file
		mv $DEST_DIR/$file.db $SRC_DIR/save/$file.db
		mv $SRC_DIR/build/$file $DEST_DIR/$file
		mv $SRC_DIR/build/$file.db $DEST_DIR/$file.db
  	fi
done

cd ..

rm -f $TARFILE
echo removed tarfile
rm -f $0
echo removed self

exit 0
