#!/bin/sh -x
#
#	Nightly script for backing up Moira.
#
#
BKUPDIRDIR=/u3/sms_backup
PATH=/bin:/usr/bin:/usr/ucb:/usr/new; export PATH
chdir ${BKUPDIRDIR}

# /u1/sms/bin/counts </dev/null	

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
if /u1/sms/bin/mrbackup ${BKUPDIRDIR}/in_progress/
then
	echo "Backup successful"
else
	echo "Backup failed!"
	exit 1
fi

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
su smsdba -fc "rdist -c ${BKUPDIRDIR} themis:/site/sms/sms_backup"
su smsdba -fc "rdist -c ${BKUPDIRDIR} odysseus:/site/sms/sms_backup"

if [ "`/usr/bin/find /u1/sms/critical.log -mtime -1 -print`" = "/u1/sms/critical.log" ]; then
	(/bin/echo "To: dbadmin";\
	 /bin/echo "Subject: Moira update status";\
	 /usr/ucb/tail /u1/sms/critical.log) | /bin/mail dbadmin
fi

exit 0
