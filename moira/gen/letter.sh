#!/bin/sh
# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/letter.sh,v 1.4 1992-07-16 15:06:08 mar Exp $
# This script prints the letters confirming registration for the extra
# kerberos principal.

# The algorithm consists of first checking to see that the previous 
# day's job printed.  If so, the cached copy is deleted.  Otherwise,
# today's is appended to yesterday's and we continue working with that.

PATH=/bin:/bin/athena:/usr/ucb:/usr/bin/athena:/usr/athena; export PATH
printer=linus
newfile=/tmp/letter.out
savefile=/u1/letter.save
holdfile=/u1/letter.hold

# These are not normally local, so may need to point somewhere else
lpquota=lpquota
lpr=lpr
colrm=colrm

last=`$lpquota -l | tail -2`
if [ "`echo $last | $colrm 1 4 | $colrm 7`" = \
     "`ls -l $savefile | $colrm 1 32 | $colrm 7`" ]; then
	mv $newfile $savefile
else
	if [ -s $savefile ]; then
		if [ ! -f $holdfile ]; then
			(echo "Reg_extra letters failed to print yesterday"; \
			ls -l $savefile; \
			echo $last) | /bin/mail dbadmin thorne
		fi
		cat $newfile >> $savefile
	else
		mv $newfile $savefile
	fi
fi
rm -f $holdfile

if [ "`$lpr -P$printer -h $savefile`"x = "Printer queue is disabled."x ]; then
	touch $holdfile
fi

exit 0
