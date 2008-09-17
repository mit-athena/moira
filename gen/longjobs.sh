#!/bin/sh

# $Id$

# Wrapper script to install the groups file output by moira, and update
# the quota database for current group membership.

spooldir=/var/spool/pbs
spool_etcdir=$spooldir/etc
exec_prefix=/usr/pbs
sbindir=${exec_prefix}/sbin
update_group=${sbindir}/update_group
groups=${spool_etcdir}/groups
quotadb=${spool_etcdir}/quotadb

if [ -d /var/athena ] && [ -w /var/athena ]; then
    exec >/var/athena/moira_update.log 2>&1
else 
    exec >/tmp/moira_update.log 2>&1
fi

# The following exit codes are defined and MUST BE CONSISTENT with the
# error codes the library uses:
MR_MISSINGFILE=47836473
MR_MKCRED=47836474

test -r ${groups}.out || exit $MR_MISSINGFILE

if [ -s ${groups}.out ]; then
  mv ${groups}.out $groups
  ${update_group} -d $quotadb -a
  if [ $? != 0 ]; then
    exit $MR_MKCRED
  fi
fi

# cleanup
test -f $0 && rm -f $0

exit 0
