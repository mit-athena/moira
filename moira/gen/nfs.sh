TARFILE=/tmp/nfs
TMPDIR=/tmp/nfs.dir
if [ ! -d $TMPDIR ]; then
	/bin/rm -f $TMPDIR
	/bin/mkdir $TMPDIR
	/bin/chmod 755 $TMPDIR
fi
# cd $TMPDIR; tar xf $TARFILE
for QFILE in $TMPDIR/\\*; do
	while :; do
		read login dir uid gid quota
		if [ $? != 0 ]; then exit 0; fi
		path=$dir/$login
		echo mkdir $path
		echo chown $uid $path
		echo chgrp $gid $path
		echo setquota `expr $QFILE : '[^\]*\(.*\)' \
| sed 's,\\\\,/,g'` $uid $quota
	done <$QFILE
done
#
# 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/nfs.sh,v $
#	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/nfs.sh,v 1.2 1987-09-05 18:32:18 poto Exp $
#
