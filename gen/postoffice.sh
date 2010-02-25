#!/bin/sh 
# This script performs postoffice updates.
#
# $HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/gen/postoffice.sh $ $Id: postoffice.sh 3956 2010-01-05 20:56:56Z zacheiss $

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/etc:/usr/etc:/usr/athena/bin:/usr/local/bin
export PATH

if [ -d /var/athena ] && [ -w /var/athena ]; then
    exec >/var/athena/moira_update.log 2>&1
else
    exec >/tmp/moira_update.log 2>&1
fi

# The following exit codes are defined and MUST BE CONSISTENT with the
# error codes the library uses:
MR_NOCRED=47836470
MR_MISSINGFILE=47836473
MR_MKCRED=47836474
MR_TARERR=47836476

umask 22

TARFILE=/var/tmp/po.out
SRC_DIR=/var/tmp/po.dir
IMAP_ROOT=/usr/local/cyrus
IMAP_CONFIG=${IMAP_ROOT}/config
IMAP_CHECK=${IMAP_CONFIG}/ignore_update_checks
IMAP_MBOXES=${IMAP_CONFIG}/mailboxes.input;
IMAP_QUOTA=${IMAP_CONFIG}/quota
IMAP_QINPUT=${IMAP_QUOTA}/input
IMAP_BIN=${IMAP_ROOT}/bin


#
# handle tar file
#

if [ ! -r ${TARFILE} ]; then
   	exit ${MR_TARERR}
fi

rm -rf ${SRC_DIR}
mkdir ${SRC_DIR}
chmod 700 ${SRC_DIR}

uchost=`hostname | tr '[a-z]' '[A-Z]'`
echo $uchost | egrep -e "\." > /dev/null
if [ $? != 0 ]; then
    	domain=`grep domain /etc/resolv.conf | awk '{print $2}' |tr '[a-z]' '[A-Z]'`

    	uchost=$uchost.$domain
fi

cd ${SRC_DIR}
tar xvf ${TARFILE}
if [ $? != 0 ]; then
    	exit ${MR_TARERR}
fi

if [ ! -d $uchost ]; then
    	echo "no directory for "$uchost
    	exit ${MR_MISSINGFILE}
fi

cd $uchost


#
# handle mboxes file
#

cp mailboxes ${IMAP_MBOXES}
if [ $? != 0 ]; then
	echo "failed to copy mailboxes"
	exit ${MR_NOCRED}
fi

sort -o ${IMAP_MBOXES} ${IMAP_MBOXES}


#
# handle quotas
#

rm -f ${IMAP_QINPUT}
cp quota ${IMAP_QINPUT}
if [ $? != 0 ]; then
        echo "failed to copy quota"
        exit ${MR_NOCRED}
fi


#
# save
#
cp ${IMAP_CONFIG}/mailboxes ${IMAP_CONFIG}/mailboxes.save

#
# synchronize
#
LD_LIBRARY_PATH=/usr/local/tcl/lib
export LD_LIBRARY_PATH

${IMAP_BIN}/sync.pl
if [ $? != 0 ]; then
	exit ${MR_MKCRED}
fi

#
# backup
#
cp ${IMAP_CONFIG}/mailboxes ${IMAP_CONFIG}/mailboxes.backup

#
# cleanup
#

if [ -f ${TARFILE} ]; then
  	 rm -f ${TARFILE}
fi

if [ -d ${SRC_DIR} ]; then
  	cd ${SRC_DIR}/..
  	rm -rf ${SRC_DIR}
fi

if [ -f $0 ]; then
	 rm -f $0
fi

exit 0
