#! /bin/sh
TARFILE=/tmp/hesiod
SRC_DIR=/site/_nameserver
DEST_DIR=/etc/athena/nameserver
if [ ! -d $SRC_DIR ]; then
	/bin/rm -f $SRC_DIR
	/bin/mkdir $SRC_DIR
	/bin/chmod 755 $SRC_DIR
fi
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

/bin/rm -f $TARFILE
kill -KILL `/bin/cat /etc/named.pid`

csh -fc "unlimit; /etc/named"

while true; do
	sleep 60
	if [ -e /tmp/named.success ]
		exit 0
	fi
	if [ -e /tmp/named.failure ]
		exit 1
	fi
	if [ -e /etc/named.pid ]
		exit 1
	fi
done

rm -f $0
exit
#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/hesiod.sh,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/hesiod.sh,v 1.3 1988-03-14 15:12:12 mar Exp $
#
