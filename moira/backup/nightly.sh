#!/bin/sh
#
#	Nightly script for backing up Moira.
#
#
BKUPDIRDIR=/backup
PATH=/bin:/athena/bin:/usr/athena/bin:/usr/bin:/usr/ucb:/usr/new; export PATH
. /usr/ingres/sqluser.profile
chdir ${BKUPDIRDIR}

# /moira/bin/counts </dev/null	

if [ -d in_progress ] 
then
	echo "Two backups running?"
	exit 1
fi

trap "rm -rf ${BKUPDIRDIR}/in_progress" 0 1 15 

if mkdir in_progress 
then
	echo "Backup in progress."
else
	echo "Cannot create backup directory"
	exit 1
fi
chmod 750 in_progress
if /moira/bin/mrbackup ${BKUPDIRDIR}/in_progress/
then
	failed=`ls -s ${BKUPDIRDIR}/in_progress/ \
		| awk '	!/total/ { 
			  if ( FILENAME ~ /conf$/ ) {
			    minsize[$2]=$1;
			  } else {
			    size[$2]=$1
			  }
			} 
			END {
			  for ( i in minsize ) {
			    if ( minsize[i] > size[i] ) {
			      j+=1
			    }
			  }
			  print j
			}' ${BKUPDIRDIR}/conf -`
	if [ "$failed" -gt 0 ]
	then
		echo "Backup was incomplete!  $failed table(s) too small!"
		echo "Current file sizes:"
		ls -s in_progress/
		exit 1
	fi
	echo "Backup successful"
else
	echo "Backup failed!"
	exit 1
fi

/moira/bin/report.sh ${BKUPDIRDIR}/in_progress > ${BKUPDIRDIR}/in_progress/report

if [ -d stale ]
then
	echo -n "Stale backup "
	rm -r stale
	echo "removed"
fi
echo -n "Shifting backups "

mv backup_3 stale
echo -n "3 "
mv backup_2 backup_3
echo -n "2 "
mv backup_1 backup_2
echo -n "1 "
mv in_progress backup_1
echo
/bin/df /moira/moira.log | /usr/ucb/tail -1
echo 
echo -n "deleting last backup"
rm -rf stale
echo
echo "Shipping over the net"
rcp -rpx ${BKUPDIRDIR}/* oregano:/backup
rcp -rpx ${BKUPDIRDIR}/* nessus:/backup/moira

if [ "`/usr/bin/find /moira/critical.log -mtime -1 -print`" = "/moira/critical.log" ]; then
	(/bin/echo "To: dbadmin";\
	 /bin/echo "Subject: Moira update status";\
	 /usr/ucb/tail /moira/critical.log) | /bin/mail dbadmin
fi

plfile=/afs/athena.mit.edu/system/info/public-mailing-lists
awk -F\| '$3==1 && $4==1 && $5==0 && $6==1 {printf "%-20s %s\n", $1, $9}' \
	${BKUPDIRDIR}/backup_1/list \
	| sort > $plfile.new && mv -f $plfile.new $plfile

exit 0
