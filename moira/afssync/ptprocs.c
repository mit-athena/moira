/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/ptprocs.c,v 1.1 1990-09-21 15:18:27 mar Exp $ */
/* $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/ptprocs.c,v $ */


/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

/*	
       Sherri Nichols
       Information Technology Center
       November, 1988
*/


#include <ctype.h>
#include <stdio.h>
#include <strings.h>
#include <lock.h>
#include <ubik.h>
#include <rx/xdr.h>
#include <rx/rx.h>
#include "print.h"
#include "prserver.h"
#include "prerror.h"

extern struct ubik_dbase *dbase;
extern struct afsconf_dir *prdir;
extern long Initdb();

PR_INewEntry(call,aname,aid,oid)
struct rx_call *call;
char aname[PR_MAXNAMELEN];
long aid;
long oid;
{
    /* used primarily for conversion - not intended to be used as usual means of entering people into the database. */
    struct ubik_trans *tt;
    register long code;
    long temp = 0;
    long gflag = 0;
    long cid;
    long noAuth;
    char *check;
    long tid;
    char oname[PR_MAXNAMELEN];
    
    stolower(aname);
    code = Initdb();
    if (code != PRSUCCESS) return code;
    noAuth =afsconf_GetNoAuthFlag(prdir);
    code = ubik_BeginTrans(dbase,UBIK_WRITETRANS, &tt);
    if (code) return code;
    code = ubik_SetLock(tt, 1,1,LOCKWRITE);
    if (code) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = WhoIsThis(call,tt,&cid);
    if (code && !noAuth) {
	ubik_AbortTrans(tt);
	return PRPERM;
    }
    if (aid == 0) {
	ubik_AbortTrans(tt);
	return PRPERM;
    }
    if (aid < 0) gflag |= PRGRP;

    if (!noAuth) {
	if (gflag & PRGRP) {
	    if (oid != cid && !IsAMemberOf(tt,cid,SYSADMINID)) {
		ubik_AbortTrans(tt);
		return PRPERM;
	    }
	}
	else {
	    if (!IsAMemberOf(tt,cid,SYSADMINID)) {
		ubik_AbortTrans(tt);
		return PRPERM;
	    }
	}
    }

    /* is this guy already in the database? */
    temp = FindByID(tt,aid);
    if (temp) {
	ubik_AbortTrans(tt);
	return PRIDEXIST;
    }
    if (gflag & PRGRP) {
	if (cid == ANONYMOUSID) {
	    ubik_AbortTrans(tt);
	    return PRPERM;
	}
	if ((check = index(aname,':')) == 0) {
	    /* groups should have owner's name prepended */
	    ubik_AbortTrans(tt);
	    return PRBADNAM;
	}
	if (oid) {
	    if (!IsAMemberOf(tt,cid,SYSADMINID) && !IsOwnerOf(tt,cid,oid)) {
		/* must be a sysadmin to specify another owner, unless other owner is group owned by you */
		ubik_AbortTrans(tt);
		return PRPERM;
	    }
	}
	else oid = cid;
	bzero(oname,PR_MAXNAMELEN);
	/* check prepended part */
	strncpy(oname,aname,check-aname);
	if (!strcmp(oname,"system")) {
	    /* groups with system: at beginning are owned by SYSADMIN */
	    tid = SYSADMINID;
	}
	else {
	    code = NameToID(tt,oname,&tid);
	    if (code || tid == ANONYMOUSID) {
		/* owner doesn't exist */
		ubik_AbortTrans(tt);
		return PRNOENT;
	    }
	}
	if (oid > 0) {
	    /* owner's name should be prepended */
	    if (tid != oid) {
		ubik_AbortTrans(tt);
		return PRBADNAM;
	    }
	}
	else {
	    /* if owner is a group, group owner's name should be prepended. */
	    if (tid != OwnerOf(tt,oid)) {
		ubik_AbortTrans(tt);
		return PRPERM;
	    }
	}
    }
    temp = FindByName(tt,aname);
    if (temp) {
	ubik_AbortTrans(tt);
	return PREXIST;
    }
    code = CreateEntry(tt,aname,&aid,1,gflag,oid,cid);
    if (code != PRSUCCESS) {
	ubik_AbortTrans(tt);
	return code;
    }
    /* finally, commit transaction */
    code = ubik_EndTrans(tt);
    if (code) return code;
    return PRSUCCESS;
}


PR_NewEntry(call,aname,flag,oid,aid)
struct rx_call *call;
char aname[PR_MAXNAMELEN];
long flag;
long oid;
long *aid;
{
    register long code;
    struct ubik_trans *tt;
    long temp;
    long cid;
    char oname[PR_MAXNAMELEN];
    long noAuth;
    char *check;
    long tid;

    stolower(aname);
    code = Initdb();
    noAuth = afsconf_GetNoAuthFlag(prdir);
    if (code != PRSUCCESS) return code;
    code = ubik_BeginTrans(dbase,UBIK_WRITETRANS,&tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKWRITE);
    if (code) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = WhoIsThis(call,tt,&cid);
    if (code && !noAuth) {
	ubik_AbortTrans(tt);
	return PRPERM;
    }
    if (flag & PRGRP) {
	if (cid == ANONYMOUSID) {
	    ubik_AbortTrans(tt);
	    return PRPERM;
	}
	if ((check = index(aname,':')) == 0) {
	    /* groups should have owner's name prepended */
	    ubik_AbortTrans(tt);
	    return PRBADNAM;
	}
	if (oid) {
	    if (!IsAMemberOf(tt,cid,SYSADMINID) && !IsOwnerOf(tt,cid,oid)) {
		/* must be a sysadmin to specify another owner, unless other owner is group owned by you */
		ubik_AbortTrans(tt);
		return PRPERM;
	    }
	}
	else oid = cid;
	bzero(oname,PR_MAXNAMELEN);
	/* check prepended part */
	strncpy(oname,aname,check-aname);
	if (!strcmp(oname,"system")) {
	    /* groups with system: at beginning are owned by SYSADMIN */
	    tid = SYSADMINID;
	}
	else {
	    code = NameToID(tt,oname,&tid);
	    if (code || tid == ANONYMOUSID) {
		/* owner doesn't exist */
		ubik_AbortTrans(tt);
		return PRNOENT;
	    }
	}
	if (oid > 0) {
	    /* owner's name should be prepended */
	    if (tid != oid) {
		ubik_AbortTrans(tt);
		return PRBADNAM;
	    }
	}
	else {
	    /* if owner is a group, prepend group owner's name. */
	    if (tid != OwnerOf(tt,oid)) {
		ubik_AbortTrans(tt);
		return PRPERM;
	    }
	}
	temp = FindByName(tt,aname);
	if (temp) {
	    ubik_AbortTrans(tt);
	    return PREXIST;
	}
	code = CreateEntry(tt,aname,aid,0,flag,oid,cid);
	if (code != PRSUCCESS) {
	    ubik_AbortTrans(tt);
	    return code;
	}
    }
    else {
	if (!(flag & PRFOREIGN)) {
	    if (!IsAMemberOf(tt,cid,SYSADMINID) && !noAuth) {
		ubik_AbortTrans(tt);
		return PRPERM;
	    }
	    if (oid != SYSADMINID) oid = SYSADMINID;
	}
	temp = FindByName(tt,aname);
	if (temp) {
	    ubik_AbortTrans(tt);
	    return PREXIST;
	}
	code = CreateEntry(tt,aname,aid,0,flag,oid,cid);
	if (code != PRSUCCESS) {
	    ubik_AbortTrans(tt);
	    return code;
	}
    }
    code = ubik_EndTrans(tt);
    if (code) return code;
    return PRSUCCESS;
}



PR_WhereIsIt(call,aid,apos)
struct rx_call *call;
long aid;
long *apos;
{
    register long code;
    struct ubik_trans *tt;
    long temp;

    code = Initdb();
    if (code != PRSUCCESS) return code;
    code = ubik_BeginTrans(dbase,UBIK_READTRANS,&tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKREAD);
    if (code) {
	ubik_AbortTrans(tt);
	return code;
    }
    temp = FindByID(tt,aid);
    if (!temp) {
	ubik_AbortTrans(tt);
	return PRNOENT;
    }
    *apos = temp;
    code = ubik_EndTrans(tt);
    if (code) return code;
    return PRSUCCESS;
}


PR_DumpEntry(call,apos, aentry)
struct rx_call *call;
long apos;
struct prdebugentry *aentry;
{
    register long code;
    struct ubik_trans *tt;

    code = Initdb();
    if (code != PRSUCCESS) return code;
    code = ubik_BeginTrans(dbase,UBIK_READTRANS,&tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKREAD);
    if (code) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = pr_ReadEntry(tt, 0, apos, aentry);
    if (code != 0) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = ubik_EndTrans(tt);
    if (code) return code;
    return PRSUCCESS;
}

PR_AddToGroup(call,aid,gid)
struct rx_call *call;
long aid;
long gid;
{
    register long code;
    struct ubik_trans *tt;
    long tempu;
    long tempg;
    struct prentry tentry;
    struct prentry uentry;
    long len;
    long cid;
    long noAuth;

    code = Initdb();
    if (code != PRSUCCESS) return code;
    if (gid == ANYUSERID || gid == AUTHUSERID) return PRPERM;
    if (aid == ANONYMOUSID) return PRPERM;
    noAuth = afsconf_GetNoAuthFlag(prdir);
    code = ubik_BeginTrans(dbase,UBIK_WRITETRANS,&tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKWRITE);
    if (code) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = WhoIsThis(call, tt, &cid);
    if (code && !noAuth) {
	ubik_AbortTrans(tt);
	return PRPERM;
    }
    tempu = FindByID(tt,aid);
    if (!tempu) { 
	ubik_AbortTrans(tt);
	return PRNOENT;
    }
    bzero(&uentry,sizeof(uentry));
    code = pr_ReadEntry(tt,0,tempu,&uentry);
    if (code != 0)  {
	ubik_AbortTrans(tt);
	return code;
    }
    /* we don't allow groups as members of groups at present */
    if (uentry.flags & PRGRP) {
	ubik_AbortTrans(tt);
	return PRNOTUSER;
    }
    tempg = FindByID(tt,gid);
    if (!tempg) {
	ubik_AbortTrans(tt);
	return PRNOENT;
    }
    code = pr_ReadEntry(tt,0,tempg,&tentry);
    if (code != 0) {
	ubik_AbortTrans(tt);
	return code;
    }
    /* make sure that this is a group */
    if (!(tentry.flags & PRGRP)) {
	ubik_AbortTrans(tt);
	return PRNOTGROUP;
    }
    if (tentry.owner != cid && !IsAMemberOf(tt,cid,SYSADMINID) && !IsAMemberOf(tt,cid,tentry.owner) && !noAuth) {
	ubik_AbortTrans(tt);
	return PRPERM;
    }

    
    code = AddToEntry(tt,tentry,tempg,aid);
    if (code != PRSUCCESS) {
	ubik_AbortTrans(tt);
	return code;
    }
    /* now, modify the user's entry as well */
    code = AddToEntry(tt,uentry,tempu,gid);
    if (code != PRSUCCESS) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = ubik_EndTrans(tt);
    if (code) return code;
    return PRSUCCESS;
}

PR_NameToID(call,aname,aid)
struct rx_call *call;
namelist *aname;
idlist *aid;
{
    register long code;
    struct ubik_trans *tt;
    long i;

    /* must do this first for RPC stub to work */
    aid->idlist_val = (long *)malloc(PR_MAXLIST*sizeof(long));
    aid->idlist_len = 0;

    code = Initdb();
    if (code != PRSUCCESS) return code;
    code = ubik_BeginTrans(dbase,UBIK_READTRANS,&tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKREAD);
    if (code) {
	ubik_AbortTrans(tt);
	return code;
    }
    for (i=0;i<aname->namelist_len;i++) {
	code = NameToID(tt,aname->namelist_val[i],&aid->idlist_val[i]);
	if (code != PRSUCCESS) aid->idlist_val[i] = ANONYMOUSID;
	aid->idlist_len++;
    }
    code = ubik_EndTrans(tt);
    if (code)return code;
    return PRSUCCESS;
}

PR_IDToName(call,aid,aname)
struct rx_call *call;
idlist *aid;
namelist *aname;
{
    register long code;
    struct ubik_trans *tt;
    long i;

    /* leave this first for rpc stub */
    aname->namelist_len = 0;
    aname->namelist_val = (prname *)malloc(PR_MAXLIST*PR_MAXNAMELEN);

    code = Initdb();
    if (code != PRSUCCESS) return code;
    code = ubik_BeginTrans(dbase,UBIK_READTRANS,&tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKREAD);
    if (code) {
	ubik_AbortTrans(tt);
	return code;
    }
    for (i=0;i<aid->idlist_len;i++) {
	/* interface won't allow more than PR_MAXLIST to be sent in */
	code = IDToName(tt,aid->idlist_val[i],aname->namelist_val[i]);
	if (code != PRSUCCESS)
	    sprintf(aname->namelist_val[i],"%d",aid->idlist_val[i]);
	aname->namelist_len++;
    }
    code = ubik_EndTrans(tt);
    if (code) return code;
    return PRSUCCESS;
}

PR_Delete(call,aid)
struct rx_call *call;
long aid;
{
    register long code;
    struct ubik_trans *tt;
    long cid;
    long noAuth;

    code = Initdb();
    noAuth = afsconf_GetNoAuthFlag(prdir);
    if (code != PRSUCCESS) return code;
    if (aid == SYSADMINID || aid == ANYUSERID || aid == AUTHUSERID || aid == ANONYMOUSID) return PRPERM;
    code = ubik_BeginTrans(dbase,UBIK_WRITETRANS,&tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKWRITE);
    if (code) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = WhoIsThis(call,tt,&cid);
    if (code && !noAuth) {
	ubik_AbortTrans(tt);
	return PRPERM;
    }
    /*protection check will occur in DeleteEntry - sigh. */
    code = DeleteEntry(tt,aid,cid);
    if (code != PRSUCCESS) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = ubik_EndTrans(tt);
    if (code) return code;
    return PRSUCCESS;
}

PR_RemoveFromGroup(call,aid,gid)
struct rx_call *call;
long aid;
long gid;
{
    register long code;
    struct ubik_trans *tt;
    long tempu;
    long tempg;
    struct prentry uentry;
    struct prentry gentry;
    long cid;
    long noAuth;

    code = Initdb();
    if (code != PRSUCCESS) return code;
    noAuth = afsconf_GetNoAuthFlag(prdir);
    code = ubik_BeginTrans(dbase,UBIK_WRITETRANS,&tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKWRITE);
    if (code) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = WhoIsThis(call,tt,&cid);
    if (code && !noAuth) {
	ubik_AbortTrans(tt);
	return PRPERM;
    }
    tempu = FindByID(tt,aid);
    if (!tempu) { 
	ubik_AbortTrans(tt);
	return PRNOENT;
    }
    tempg = FindByID(tt,gid);
    if (!tempg) {
	ubik_AbortTrans(tt);
	return PRNOENT;
    }
    bzero(&uentry,sizeof(uentry));
    bzero(&gentry,sizeof(gentry));
    code = pr_ReadEntry(tt,0,tempu,&uentry);
    if (code != 0) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = pr_ReadEntry(tt,0,tempg,&gentry);
    if (code != 0) {
	ubik_AbortTrans(tt);
	return code;
    }
    if (!(gentry.flags & PRGRP)) {
	ubik_AbortTrans(tt);
	return PRNOTGROUP;
    }
    if (uentry.flags & PRGRP) {
	ubik_AbortTrans(tt);
	return PRNOTUSER;
    }
    if (gentry.owner != cid && !IsAMemberOf(tt,cid,SYSADMINID) && !IsAMemberOf(tt,cid,gentry.owner) && !noAuth) {
	ubik_AbortTrans(tt);
	return PRPERM;
    }
    code = RemoveFromEntry(tt,aid,gid);
    if (code != PRSUCCESS) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = RemoveFromEntry(tt,gid,aid);
    if (code != PRSUCCESS) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = ubik_EndTrans(tt);
    if (code) return code;
    return PRSUCCESS;
}

PR_GetCPS(call,aid,alist,over)
struct rx_call *call;
long aid;
prlist *alist;
long *over;
{
    register long code;
    struct ubik_trans *tt;

    alist->prlist_len = 0;
    alist->prlist_val = (long *) 0;
    code = Initdb();
    if (code != PRSUCCESS) goto done;
    code = ubik_BeginTrans(dbase,UBIK_READTRANS,&tt);
    if (code) goto done;
    code = ubik_SetLock(tt,1,1,LOCKREAD);
    if (code) {
	ubik_AbortTrans(tt);
	goto done;
    }
    *over = 0;
    code = GetList(tt,aid,alist,1);
    if (code != PRSUCCESS) {
	ubik_AbortTrans(tt);
	goto done;
    }
    if (alist->prlist_len > PR_MAXGROUPS) *over = alist->prlist_len - PR_MAXGROUPS;
    code = ubik_EndTrans(tt);

done:
    /* return code, making sure that prlist_val points to malloc'd memory */
    if (!alist->prlist_val)
	alist->prlist_val = (long *) malloc(0);	/* make xdr stub happy */
    return code;
}

PR_ListMax(call,uid,gid)
struct rx_call *call;
long *uid;
long *gid;
{
    register long code;
    struct ubik_trans *tt;

    code = Initdb();
    if (code != PRSUCCESS) return code;
    code = ubik_BeginTrans(dbase,UBIK_READTRANS,&tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKREAD);
    if (code) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = GetMax(tt,uid,gid);
    if (code != PRSUCCESS) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = ubik_EndTrans(tt);
    if (code) return code;
    return PRSUCCESS;
}

PR_SetMax(call,aid,gflag)
struct rx_call *call;
long aid;
long gflag;
{
    register long code;
    struct ubik_trans *tt;
    long cid;
    long noAuth;

    code = Initdb();
    if (code != PRSUCCESS) return code;
    noAuth = afsconf_GetNoAuthFlag(prdir);
    code = ubik_BeginTrans(dbase,UBIK_WRITETRANS,&tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKWRITE);
    if (code) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = WhoIsThis(call,tt,&cid);
    if (code && !noAuth) {
	ubik_AbortTrans(tt);
	return PRPERM;
    }
    if (!IsAMemberOf(tt, cid,SYSADMINID) && !noAuth) {
	ubik_AbortTrans(tt);
	return PRPERM;
    }
    code = SetMax(tt,aid,gflag);
    if (code != PRSUCCESS) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = ubik_EndTrans(tt);
    if (code) return code;
    return PRSUCCESS;
}

PR_ListEntry(call,aid,aentry)
struct rx_call *call;
long aid;
struct prcheckentry *aentry;
{
    register long code;
    struct ubik_trans *tt;
    long temp;
    struct prentry tentry;

    code = Initdb();
    if (code != PRSUCCESS) return code;
    code = ubik_BeginTrans(dbase,UBIK_READTRANS,&tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKREAD);
    if (code) {
	ubik_AbortTrans(tt);
	return code;
    }
    temp = FindByID(tt,aid);
    if (!temp) { 
	ubik_AbortTrans(tt);
	return PRNOENT;
    }
    code = pr_ReadEntry(tt, 0, temp, &tentry);
    if (code != 0) {
	ubik_AbortTrans(tt);
	return code;
    } 
    aentry->owner = tentry.owner;
    aentry->id = tentry.id;
    strncpy(aentry->name,tentry.name,PR_MAXNAMELEN);
    aentry->creator = tentry.creator;
    aentry->ngroups = tentry.ngroups;
    aentry->nusers = tentry.nusers;
    aentry->count = aentry->count;
    code = ubik_EndTrans(tt);
    if (code) return code;
    return PRSUCCESS;
}

PR_ChangeEntry(call,aid,name,oid,newid)
struct rx_call *call;
long aid;
char *name;
long oid;
long newid;
{
    register long code;
    struct ubik_trans *tt;
    long pos;
    long cid;
    long noAuth;

    stolower(name);
    code = Initdb();
    if (aid == ANYUSERID || aid == AUTHUSERID || aid == ANONYMOUSID || aid == SYSADMINID) return PRPERM;
    noAuth = afsconf_GetNoAuthFlag(prdir);
    if (code != PRSUCCESS) return code;
    code = ubik_BeginTrans(dbase,UBIK_WRITETRANS,&tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKWRITE);
    if (code) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = WhoIsThis(call,tt,&cid);
    if (code && !noAuth) {
	ubik_AbortTrans(tt);
	return PRPERM;
    }
    pos = FindByID(tt,aid);
    if (!pos) { 
	ubik_AbortTrans(tt);
	return PRNOENT;
    }
    /* protection check in changeentry */
    code = ChangeEntry(tt,aid,cid,name,oid,newid);
    if (code != PRSUCCESS) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = ubik_EndTrans(tt);
    return code;
}

PR_ListElements(call,aid,alist,over)
struct rx_call *call;
long aid;
prlist *alist;
long *over;
{
    register long code;
    struct ubik_trans *tt;

    alist->prlist_len = 0;
    alist->prlist_val = (long *) 0;

    code = Initdb();
    if (code != PRSUCCESS) goto done;
    code = ubik_BeginTrans(dbase,UBIK_READTRANS,&tt);
    if (code) goto done;
    code = ubik_SetLock(tt,1,1,LOCKREAD);
    if (code) {
	ubik_AbortTrans(tt);
	goto done;
    }
    code = GetList(tt,aid,alist,0);
    if (code != PRSUCCESS) {
	ubik_AbortTrans(tt);
	goto done;
    }
    if (alist->prlist_len > PR_MAXGROUPS) *over = alist->prlist_len - PR_MAXGROUPS;
    code = ubik_EndTrans(tt);

done:
    if (!alist->prlist_val)
	alist->prlist_val = (long *) malloc(0);	/* make calling stub happy */
    return code;
}

static stolower(s)
register char *s;
{
    register int tc;
    while (tc = *s) {
	if (isupper(tc)) *s = tolower(tc);
	s++;
    }
}
