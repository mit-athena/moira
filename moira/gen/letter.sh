#!/bin/sh
# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/letter.sh,v 1.1 1992-06-18 14:52:55 mar Exp $
# This script prints the letters confirming registration for the extra
# kerberos principal.

# The algorithm consists of first checking to see that the previous 
# day's job printed.  If so, the cached copy is deleted.  Otherwise,
# today's is appended to yesterday's and we continue working with that.

PATH=/bin:/bin/athena:/usr/ucb:/usr/bin/athena:/usr/athena; export PATH
printer=nil
newfile=/tmp/letter.out
savefile=/u1/letter.save

# These are not normally local, so may need to point somewhere else
lpquota=lpquota
lpr=lpr
colrm=colrm

last=`$lpquota -l | tail -2`
if [ "`echo $last | $colrm 1 4 | $colrm 7`" = \
     "`ls -l $savefile | $colrm 1 32 | $colrm 7`" ]; then
	mv $newfile $savefile
else
	cat $newfile >> $savefile
	echo "Reg_extra letter printing error" | /bin/mail dbadmin
fi

$lpr -P$printer -h $savefile

exit 0
