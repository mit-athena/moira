#! /bin/sh
TARFILE=/tmp/hesiod
SRC_DIR=/etc/athena/_nameserver
DEST_DIR=/etc/athena/nameserver
if [ ! -d $SRC_DIR ]; then
	/bin/rm -f $SRC_DIR
	/bin/mkdir $SRC_DIR
	/bin/chmod 755 $SRC_DIR
fi
#if [ ! -d $DEST_DIR ]; then
#	/bin/rm -f $DEST_DIR
#	/bin/mkdir $DEST_DIR
#	/bin/chmod 755 $DEST_DIR
#fi
cd $SRC_DIR
for file in `/bin/tar tf $TARFILE`; do 
	if [ ./ = $file ]; then continue; fi
	/bin/tar xf $TARFILE $file
	if [ -s $file ]; then
		/bin/mv -f $file $DEST_DIR
	else
		/bin/rm -f $file
	fi
done
#/bin/rm -f $TARFILE
#kill -HUP `/bin/cat /etc/named.pid`


exit
#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/hesiod.sh,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/hesiod.sh,v 1.1 1987-08-22 16:34:57 wesommer Exp $
#
