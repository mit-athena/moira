#!/bin/sh
#
#	Nightly script for backing up SMS.
#
#
BKUPDIRDIR=/tmp

chdir ${BKUPDIRDIR}

if [ -d in_progress ] 
then
	echo "Two backups running?"
	exit 1
fi

if mkdir in_progress 
then
	echo "Backup in progress."
else
	echo "Cannot create backup directory"
	exit 1
fi
	
if /u1/sms/backup/smsbackup ${BKUPDIRDIR}/in_progress/
then
	echo "Backup successful"
else
	echo "Backup failed!"
	exit 1
fi

if [ -d stale ]
then
	echo -n "Stale backup "
	rm -rf backup_4
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
echo "New backup:"
ls -l backup_1
exit 0
