#!/bin/sh -x
#
#	Nightly script for backing up Moira.
#
#
BKUPDIRDIR=/u3/sms_backup
PATH=/bin:/usr/bin:/usr/ucb:/usr/new; export PATH
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
if /moira/bin/mrbackup ${BKUPDIRDIR}/in_progress/
then
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
echo -n "3"
mv backup_2 backup_3
echo -n "2"
mv backup_1 backup_2
echo -n "1"
mv in_progress backup_1
echo 
echo -n "deleting last backup"
rm -rf stale
echo "Shipping over the net:"
rcp -rp ${BKUPDIRDIR}/* oregano:/var/moira
rcp -rp ${BKUPDIRDIR}/* plover:/backup/moira

if [ "`/usr/bin/find /moira/critical.log -mtime -1 -print`" = "/moira/critical.log" ]; then
	(/bin/echo "To: dbadmin";\
	 /bin/echo "Subject: Moira update status";\
	 /usr/ucb/tail /moira/critical.log) | /bin/mail dbadmin
fi

exit 0
