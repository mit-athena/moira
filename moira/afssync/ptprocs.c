/* Copyright (C) 1989 Transarc Corporation - All rights reserved */

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

#ifndef lint
static char rcsid[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/ptprocs.c,v 1.3 1990-09-25 00:06:06 mar Exp $";
#endif

/*	
       Sherri Nichols
       Information Technology Center
       November, 1988
*/

#include <afs/param.h>
#include <ctype.h>
#include <stdio.h>
#include <lock.h>
#include <ubik.h>
#include <rx/xdr.h>
#include <rx/rx.h>
#include <rx/rx_vab.h>
#include <rx/rxkad.h>
#include <afs/auth.h>
#include <netinet/in.h>
#include "ptserver.h"
#include "pterror.h"
#include <strings.h>

#ifdef AFS_ATHENA_STDENV
#include <krb.h>
#endif

extern struct ubik_dbase *dbase;
extern long Initdb();
extern int pr_noAuth;

static int CreateOK (ut, cid, oid, flag, admin)
  struct ubik_trans *ut;
  long cid;				/* id of caller */
  long oid;				/* id of owner */
  long flag;				/* indicates type of entry */
  int  admin;				/* sysadmin membership */
{
    if (flag & (PRGRP | PRFOREIGN)) {
	/* Allow anonymous group creation only if owner specified and running
         * noAuth. */
	if (cid == ANONYMOUSID) {
	    if ((oid == 0) || !pr_noAuth) return 0;
	}
    }
    else { /* creating a user */
	if (!admin && !pr_noAuth) return 0;
    }
    return 1;				/* OK! */
}

#if 0
/*
 * WhoIsThis() has been replaced for the Moira-AFS synchronization.
 */ 
long WhoIsThis (acall, at, aid)
  struct rx_call *acall;
  struct ubik_trans *at;
  long *aid;
{
    /* aid is set to the identity of the caller, if known, ANONYMOUSID otherwise */
    /* returns -1 and sets aid to ANONYMOUSID on any failure */
    register struct rx_connection *tconn;
    struct rxvab_conn *tc;
    register long code;
    char tcell[MAXKTCREALMLEN];
    long exp;
    char name[MAXKTCNAMELEN];
    char inst[MAXKTCNAMELEN];
    int  ilen;
    char vname[256];

    *aid = ANONYMOUSID;
    tconn = rx_ConnectionOf(acall);
    code = rx_SecurityClassOf(tconn);
    if (code == 0) return 0;
    else if (code == 1) {		/* vab class */
	tc = (struct rxvab_conn *) tconn->securityData;
	if (!tc) goto done;
	*aid = ntohl(tc->viceID);
	code = 0;
    }
    else if (code == 2) {		/* kad class */
	if (code = rxkad_GetServerInfo
	    (acall->conn, (long *) 0, &exp, name, inst, tcell, (long *) 0))
	    goto done;
	if (exp < FT_ApproxTime()) goto done;
	if (strlen (tcell)) {
	    extern char *pr_realmName;
#ifdef AFS_ATHENA_STDENV
	    static char local_realm[REALM_SZ] = "";
	    if (!local_realm[0]) {
		krb_get_lrealm (local_realm, 0);
	    }
#endif
	    if (
#ifdef AFS_ATHENA_STDENV
		strcasecmp (local_realm, tcell) &&
#endif
		strcasecmp (pr_realmName, tcell))
		goto done;

	}
	strncpy (vname, name, sizeof(vname));
	if (ilen = strlen (inst)) {
	    if (strlen(vname) + 1 + ilen >= sizeof(vname)) goto done;
	    strcat (vname, ".");
	    strcat (vname, inst);
	}
	lcstring(vname, vname, sizeof(vname));
	code = NameToID(at,vname,aid);
    }
  done:
    if (code && !pr_noAuth) return -1;
    return 0;
}
#else /* AFS-Moira synchronization */
#define USERSMS 14487

long WhoIsThis(acall, at, aid)
struct rx_call *acall;
struct ubik_trans *at;
long *aid;
{
    *aid = USERSMS;
    return 0;
}
#endif

long PR_INewEntry(call,aname,aid,oid)
  struct rx_call *call;
  char aname[PR_MAXNAMELEN];
  long aid;
  long oid;
{
    /* used primarily for conversion - not intended to be used as usual means
       of entering people into the database. */
    struct ubik_trans *tt;
    register long code;
    long gflag = 0;
    long cid;
    int  admin;

#define abort_with(code) return (ubik_AbortTrans(tt),code)

    stolower(aname);
    code = Initdb();
    if (code != PRSUCCESS) return code;
    code = ubik_BeginTrans(dbase,UBIK_WRITETRANS, &tt);
    if (code) return code;
    code = ubik_SetLock(tt, 1,1,LOCKWRITE);
    if (code) abort_with (code);

    code = WhoIsThis(call,tt,&cid);
    if (code) abort_with (PRPERM);
    admin = IsAMemberOf(tt,cid,SYSADMINID);

    /* first verify the id is good */
    if (aid == 0) abort_with (PRPERM);
    if (aid < 0) {
	gflag |= PRGRP;
	/* only sysadmin can reuse a group id */
	if (!admin && !pr_noAuth && (aid != ntohl(cheader.maxGroup)-1))
	    abort_with (PRPERM);
    }
    if (FindByID (tt, aid)) abort_with (PRIDEXIST);

    /* check a few other things */
    if (!CreateOK (tt, cid, oid, gflag, admin)) abort_with (PRPERM);

    code = CreateEntry (tt,aname,&aid,1,gflag,oid,cid);
    if (code != PRSUCCESS) {
	ubik_AbortTrans(tt);
	return code;
    }
    /* finally, commit transaction */
    code = ubik_EndTrans(tt);
    if (code) return code;
    return PRSUCCESS;
}


long PR_NewEntry (call, aname, flag, oid, aid)
  struct rx_call *call;
  char aname[PR_MAXNAMELEN];
  long flag;
  long oid;
  long *aid;
{
    register long code;
    struct ubik_trans *tt;
    long cid;
    int  admin;

    stolower(aname);
    code = Initdb();
    if (code) return code;
    code = ubik_BeginTrans(dbase,UBIK_WRITETRANS,&tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKWRITE);
    if (code) {
      abort:
	ubik_AbortTrans(tt);
	return code;
    }
    code = WhoIsThis(call,tt,&cid);
    if (code) {
      perm:
	ubik_AbortTrans(tt);
	return PRPERM;
    }
    admin = IsAMemberOf(tt,cid,SYSADMINID);

    if (!CreateOK (tt, cid, oid, flag, admin)) goto perm;

    code = CreateEntry (tt,aname,aid,0,flag,oid,cid);
    if (code != PRSUCCESS) goto abort;

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
      abort:
	ubik_AbortTrans(tt);
	return code;
    }
    code = pr_ReadEntry(tt, 0, apos, aentry);
    if (code) goto abort;

    if (aentry->flags & PRCONT) {	/* wrong type, get coentry instead */
	code = pr_ReadCoEntry(tt, 0, apos, aentry);
	if (code) goto abort;
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
    long cid;

    code = Initdb();
    if (code != PRSUCCESS) return code;
    if (gid == ANYUSERID || gid == AUTHUSERID) return PRPERM;
    if (aid == ANONYMOUSID) return PRPERM;
    code = ubik_BeginTrans(dbase,UBIK_WRITETRANS,&tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKWRITE);
    if (code) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = WhoIsThis(call, tt, &cid);
    if (code) {
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
    if (!AccessOK (tt, cid, &tentry, PRP_ADD_MEM, PRP_ADD_ANY)) {
	ubik_AbortTrans(tt);
	return PRPERM;
    }
    
    code = AddToEntry (tt, &tentry, tempg, aid);
    if (code != PRSUCCESS) {
	ubik_AbortTrans(tt);
	return code;
    }
    /* now, modify the user's entry as well */
    code = AddToEntry (tt, &uentry, tempu, gid);
    if (code != PRSUCCESS) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = ubik_EndTrans(tt);
    if (code) return code;
    return PRSUCCESS;
}

long PR_NameToID (call, aname, aid)
  struct rx_call *call;
  namelist *aname;
  idlist *aid;
{
    register long code;
    struct ubik_trans *tt;
    long i;
    int size;
    int count = 0;

    /* must do this first for RPC stub to work */
    size = aname->namelist_len;
    if ((size <= 0) || (size > PR_MAXLIST)) size = 0;
    aid->idlist_val = (long *)malloc(size*sizeof(long));
    aid->idlist_len = 0;
    if (aname->namelist_len == 0) return 0;
    if (size == 0) return PRTOOMANY;	/* rxgen will probably handle this */

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
	if (count++ > 50) IOMGR_Poll(), count = 0;
    }
    aid->idlist_len = aname->namelist_len;

    code = ubik_EndTrans(tt);
    if (code) return code;
    return PRSUCCESS;
}

long PR_IDToName (call, aid, aname)
  struct rx_call *call;
  idlist *aid;
  namelist *aname;
{
    register long code;
    struct ubik_trans *tt;
    long i;
    int size;
    int count = 0;

    /* leave this first for rpc stub */
    size = aid->idlist_len;
    if ((size <= 0) || (size > PR_MAXLIST)) size = 0;
    aname->namelist_val = (prname *)malloc(size*PR_MAXNAMELEN);
    aname->namelist_len = 0;
    if (aid->idlist_len == 0) return 0;
    if (size == 0) return PRTOOMANY;	/* rxgen will probably handle this */

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
	if (count++ > 50) IOMGR_Poll(), count = 0;
    }
    aname->namelist_len = aid->idlist_len;

    code = ubik_EndTrans(tt);
    if (code) return code;
    return PRSUCCESS;
}

long PR_Delete (call, aid)
  struct rx_call *call;
  long aid;
{
    register long code;
    struct ubik_trans *tt;
    long cid;
    struct prentry tentry;
    long loc, nptr;
    int count;

    code = Initdb();
    if (code) return code;
    if (code != PRSUCCESS) return code;
    if (aid == SYSADMINID || aid == ANYUSERID || aid == AUTHUSERID || aid == ANONYMOUSID) return PRPERM;
    code = ubik_BeginTrans(dbase,UBIK_WRITETRANS,&tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKWRITE);
    if (code) {
      abort:
	ubik_AbortTrans(tt);
	return code;
    }
    code = WhoIsThis(call,tt,&cid);
    if (code) {
	ubik_AbortTrans(tt);
	return PRPERM;
    }

    /* Read in entry to be deleted */
    loc = FindByID (tt, aid);
    if (loc == 0) {code = PRNOENT; goto abort;}
    code = pr_ReadEntry (tt, 0, loc, &tentry);
    if (code) {code = PRDBFAIL; goto abort;}

    /* Do some access checking */
    if (tentry.owner != cid &&
	!IsAMemberOf (tt, cid, SYSADMINID) &&
	!IsAMemberOf (tt, cid, tentry.owner) && !pr_noAuth)
	{code = PRPERM; goto abort;}

    /* Delete each continuation block as a separate transaction so that no one
     * transaction become to large to complete. */
    nptr = tentry.next;
    while (nptr != NULL) {
	struct contentry centry;
	int i;

	code = pr_ReadCoEntry(tt, 0, nptr, &centry);
	if (code != 0) {code = PRDBFAIL; goto abort;}
	for (i=0;i<COSIZE;i++) {
	    if (centry.entries[i] == PRBADID) continue;
	    if (centry.entries[i] == 0) break;
	    code = RemoveFromEntry (tt, aid, centry.entries[i]);
	    if (code) goto abort;
	    tentry.count--;		/* maintain count */
	    if ((i&3) == 0) IOMGR_Poll();
	}
	tentry.next = centry.next;	/* thread out this block */
	code = FreeBlock (tt, nptr);	/* free continuation block */
	if (code) goto abort;
	code = pr_WriteEntry (tt, 0, loc, &tentry); /* update main entry */
	if (code) goto abort;

	/* end this trans and start a new one */
	code = ubik_EndTrans(tt);
	if (code) return code;
	IOMGR_Poll();			/* just to keep the connection alive */
	code = ubik_BeginTrans(dbase,UBIK_WRITETRANS,&tt);
	if (code) return code;
	code = ubik_SetLock(tt,1,1,LOCKWRITE);
	if (code) goto abort;

	/* re-read entry to get consistent uptodate info */
	loc = FindByID (tt, aid);
	if (loc == 0) {code = PRNOENT; goto abort;}
	code = pr_ReadEntry (tt, 0, loc, &tentry);
	if (code) {code = PRDBFAIL; goto abort;}

	nptr = tentry.next;
    }

    /* Then move the owned chain, except possibly ourself to the orphan list.
     * Because this list can be very long and so exceed the size of a ubik
     * transaction, we start a new transaction every 50 entries. */
    count = 0;
    nptr = tentry.owned;
    while (nptr != NULL) {
	struct prentry nentry;

	code = pr_ReadEntry (tt, 0, nptr, &nentry);
	if (code) {code = PRDBFAIL; goto abort;}
	nptr = tentry.owned = nentry.nextOwned;	/* thread out */
	
	if (nentry.id != tentry.id) {	/* don't add us to orphan chain! */
	    code = AddToOrphan (tt, nentry.id);
	    if (code) goto abort;
	    count++;
	    if ((count & 3) == 0) IOMGR_Poll();
	}
	if (count < 50) continue;
	code = pr_WriteEntry (tt, 0, loc, &tentry); /* update main entry */
	if (code) goto abort;

	/* end this trans and start a new one */
	code = ubik_EndTrans(tt);
	if (code) return code;
	IOMGR_Poll();			/* just to keep the connection alive */
	code = ubik_BeginTrans(dbase,UBIK_WRITETRANS,&tt);
	if (code) return code;
	code = ubik_SetLock(tt,1,1,LOCKWRITE);
	if (code) goto abort;

	/* re-read entry to get consistent uptodate info */
	loc = FindByID (tt, aid);
	if (loc == 0) {code = PRNOENT; goto abort;}
	code = pr_ReadEntry (tt, 0, loc, &tentry);
	if (code) {code = PRDBFAIL; goto abort;}

	nptr = tentry.owned;
    }

    /* now do what's left of the deletion stuff */
    code = DeleteEntry (tt, &tentry, loc);
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

    code = Initdb();
    if (code != PRSUCCESS) return code;
    code = ubik_BeginTrans(dbase,UBIK_WRITETRANS,&tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKWRITE);
    if (code) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = WhoIsThis(call,tt,&cid);
    if (code) {
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
    if (!AccessOK (tt, cid, &gentry, PRP_REMOVE_MEM, 0)) {
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

long PR_GetCPS (call, aid, alist, over)
  struct rx_call *call;
  long aid;
  prlist *alist;
  long *over;
{
    register long code;
    struct ubik_trans *tt;
    long temp;
    long cid;
    struct prentry tentry;

    alist->prlist_len = 0;
    alist->prlist_val = (long *) 0;
    code = Initdb();
    if (code != PRSUCCESS) goto done;
    code = ubik_BeginTrans(dbase,UBIK_READTRANS,&tt);
    if (code) goto done;
    code = ubik_SetLock(tt,1,1,LOCKREAD);
    if (code) {
      abort:
	ubik_AbortTrans(tt);
	goto done;
    }


    temp = FindByID(tt,aid);
    if (!temp) {code = PRNOENT; goto abort;}
    code = pr_ReadEntry (tt, 0, temp, &tentry);
    if (code) goto abort;

    if (0) {				/* afs doesn't authenticate yet */
	code = WhoIsThis (call, tt, &cid);
	if (code || !AccessOK (tt, cid, &tentry, PRP_MEMBER_MEM, PRP_MEMBER_ANY)) {
	    code = PRPERM;
	    goto abort;
	}
    }
	
    code = GetList(tt, &tentry, alist, 1);
    *over = 0;
    if (code == PRTOOMANY) *over = 1;
    else if (code != PRSUCCESS) {
	ubik_AbortTrans(tt);
	goto done;
    }

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

    code = Initdb();
    if (code != PRSUCCESS) return code;
    code = ubik_BeginTrans(dbase,UBIK_WRITETRANS,&tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKWRITE);
    if (code) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = WhoIsThis(call,tt,&cid);
    if (code) {
	ubik_AbortTrans(tt);
	return PRPERM;
    }
    if (!AccessOK (tt, cid, 0, 0, 0)) {
	ubik_AbortTrans(tt);
	return PRPERM;
    }
    if (((gflag & PRGRP) && (aid > 0)) ||
	(!(gflag & PRGRP) && (aid < 0))) {
	code = PRBADARG;
	goto abort;
    }

    code = SetMax(tt,aid,gflag);
    if (code != PRSUCCESS) {
      abort:
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
    long cid;
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
    code = WhoIsThis(call,tt,&cid);
    if (code) {
      perm:
	ubik_AbortTrans(tt);
	return PRPERM;
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
    if (!AccessOK (tt, cid, &tentry, PRP_STATUS_MEM, PRP_STATUS_ANY)) goto perm;

    aentry->flags = tentry.flags >> PRIVATE_SHIFT;
    if (aentry->flags == 0)
	if (tentry.flags & PRGRP)
	    aentry->flags = PRP_GROUP_DEFAULT >> PRIVATE_SHIFT;
	else aentry->flags = PRP_USER_DEFAULT >> PRIVATE_SHIFT;
    aentry->flags;
    aentry->owner = tentry.owner;
    aentry->id = tentry.id;
    strncpy(aentry->name,tentry.name,PR_MAXNAMELEN);
    aentry->creator = tentry.creator;
    aentry->ngroups = tentry.ngroups;
    aentry->nusers = tentry.nusers;
    aentry->count = tentry.count;
    bzero (aentry->reserved, sizeof(aentry->reserved));
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

    stolower(name);
    code = Initdb();
    if (code) return code;
    if (aid == ANYUSERID || aid == AUTHUSERID || aid == ANONYMOUSID || aid == SYSADMINID) return PRPERM;
    if (code != PRSUCCESS) return code;
    code = ubik_BeginTrans(dbase,UBIK_WRITETRANS,&tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKWRITE);
    if (code) {
	ubik_AbortTrans(tt);
	return code;
    }
    code = WhoIsThis(call,tt,&cid);
    if (code) {
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

PR_SetFieldsEntry(call, id, mask, flags, ngroups, nusers, spare1, spare2)
  struct rx_call *call;
  long id;
  long mask;				/* specify which fields to update */
  long flags, ngroups, nusers;
  long spare1, spare2;
{
    register long code;
    struct ubik_trans *tt;
    long pos;
    long cid;
    struct prentry tentry;
    long tflags;

    if (mask == 0) return 0;		/* no-op */
    code = Initdb();
    if (code) return code;
    if (id == ANYUSERID || id == AUTHUSERID || id == ANONYMOUSID)
	return PRPERM;
    if (code != PRSUCCESS) return code;
    code = ubik_BeginTrans(dbase,UBIK_WRITETRANS,&tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKWRITE);
    if (code) {
      abort:
	ubik_AbortTrans(tt);
	return code;
    }
    code = WhoIsThis(call,tt,&cid);
    if (code) {
      perm:
	ubik_AbortTrans(tt);
	return PRPERM;
    }
    pos = FindByID(tt,id);
    if (!pos) { 
	ubik_AbortTrans(tt);
	return PRNOENT;
    }
    code = pr_ReadEntry (tt, 0, pos, &tentry);
    if (code) goto abort;
    tflags = tentry.flags;

    if (mask & (PR_SF_NGROUPS | PR_SF_NUSERS)) {
	if (!AccessOK (tt, cid, 0, 0, 0)) goto perm;
	if ((tflags & PRQUOTA) == 0) {	/* default if only setting one */
	    tentry.ngroups = tentry.nusers = 20;
	}
    } else {
	if (!AccessOK (tt, cid, &tentry, 0, 0)) goto perm;
    }

    if (mask & 0xffff) {		/* if setting flag bits */
	long flagsMask = mask & 0xffff;
	tflags &= ~(flagsMask << PRIVATE_SHIFT);
	tflags |= (flags & flagsMask) << PRIVATE_SHIFT;
	tflags |= PRACCESS;
    }

    if (mask & PR_SF_NGROUPS) {		/* setting group limit */
	if (ngroups < 0) {code = PRBADARG; goto abort;}
	tentry.ngroups = ngroups;
	tflags |= PRQUOTA;
    }

    if (mask & PR_SF_NUSERS) {		/* setting foreign user limit */
	if (nusers < 0) {code = PRBADARG; goto abort;}
	tentry.nusers = nusers;
	tflags |= PRQUOTA;
    }
    tentry.flags = tflags;

    code = pr_WriteEntry (tt, 0, pos, &tentry);
    if (code) goto abort;

    code = ubik_EndTrans(tt);
    return code;
}

long PR_ListElements (call, aid, alist, over)
  struct rx_call *call;
  long aid;
  prlist *alist;
  long *over;
{
    register long code;
    struct ubik_trans *tt;
    long cid;
    long temp;
    struct prentry tentry;

    alist->prlist_len = 0;
    alist->prlist_val = (long *) 0;

    code = Initdb();
    if (code != PRSUCCESS) goto done;
    code = ubik_BeginTrans(dbase,UBIK_READTRANS,&tt);
    if (code) goto done;
    code = ubik_SetLock(tt,1,1,LOCKREAD);
    if (code) {
      abort:
	ubik_AbortTrans(tt);
	goto done;
    }
    code = WhoIsThis(call,tt,&cid);
    if (code) {
      perm:
	code = PRPERM;
	ubik_AbortTrans(tt);
	goto done;
    }

    temp = FindByID(tt,aid);
    if (!temp) {code = PRNOENT; goto abort;}
    code = pr_ReadEntry (tt, 0, temp, &tentry);
    if (code) goto abort;
    if (!AccessOK (tt, cid, &tentry, PRP_MEMBER_MEM, PRP_MEMBER_ANY))
	goto perm;
	
    code = GetList (tt, &tentry, alist, 0);
    *over = 0;
    if (code == PRTOOMANY) *over = 1;
    else if (code != PRSUCCESS) goto abort;

    code = ubik_EndTrans(tt);

done:
    if (!alist->prlist_val)
	alist->prlist_val = (long *) malloc(0);	/* make calling stub happy */
    return code;
}

/* List the entries owned by this id.  If the id is zero return the orphans
 * list. */

PR_ListOwned (call, aid, alist, over)
  struct rx_call *call;
  long aid;
  prlist *alist;
  long *over;
{
    register long code;
    struct ubik_trans *tt;
    long cid;
    struct prentry tentry;
    long head;

    alist->prlist_len = 0;
    alist->prlist_val = (long *) 0;

    code = Initdb();
    if (code != PRSUCCESS) goto done;
    code = ubik_BeginTrans(dbase,UBIK_READTRANS,&tt);
    if (code) goto done;
    code = ubik_SetLock(tt,1,1,LOCKREAD);
    if (code) {
      abort:
	ubik_AbortTrans(tt);
	goto done;
    }
    code = WhoIsThis(call,tt,&cid);
    if (code) {
      perm:
	code = PRPERM;
	ubik_AbortTrans(tt);
	goto done;
    }

    if (aid) {
	long loc = FindByID (tt, aid);
	if (loc == 0) { code =  PRNOENT; goto abort; }
	code = pr_ReadEntry (tt, 0, loc, &tentry);
	if (code) goto abort;
;
	if (!AccessOK (tt, cid, &tentry, -1, PRP_OWNED_ANY)) goto perm;
	head = tentry.owned;
    } else {
	if (!AccessOK (tt, cid, 0, 0, 0)) goto perm;
	head = ntohl(cheader.orphan);
    }

    code = GetOwnedChain (tt, head, alist);
    *over = 0;
    if (code == PRTOOMANY) *over = 1;
    else if (code) goto abort;

    code = ubik_EndTrans(tt);

done:
    if (!alist->prlist_val)
	alist->prlist_val = (long *) malloc(0);	/* make calling stub happy */
    return code;
}

PR_IsAMemberOf(call,uid,gid,flag)
struct rx_call *call;
long uid;
long gid;
long *flag;
{
    register long code;
    struct ubik_trans *tt;

    code = Initdb();
    if (code != PRSUCCESS) return code;
    code = ubik_BeginTrans(dbase,UBIK_READTRANS,&tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKREAD);
    if (code) {
      abort:
	ubik_AbortTrans(tt);
	return code;
    }
    {	long cid;
	long uloc = FindByID (tt, uid);
	long gloc = FindByID (tt, gid);
	struct prentry uentry, gentry;

	if (!uloc || !gloc) {
	    code = PRNOENT;
	    goto abort;
	}
	code = WhoIsThis(call, tt, &cid);
	if (code) {
	  perm:
	    code = PRPERM;
	    goto abort;
	}
	code = pr_ReadEntry (tt, 0, uloc, &uentry);
	if (code) goto abort;
	code = pr_ReadEntry (tt, 0, gloc, &gentry);
	if (code) goto abort;
	if ((uentry.flags & PRGRP) || !(gentry.flags & PRGRP)) {
	    code = PRBADARG;
	    goto abort;
	}
	if (!AccessOK (tt, cid, &uentry, 0, PRP_MEMBER_ANY) &&
	    !AccessOK (tt, cid, &gentry, PRP_MEMBER_MEM, PRP_MEMBER_ANY)) goto perm;
    }
	
    *flag = IsAMemberOf(tt,uid,gid);
    code = ubik_EndTrans(tt);
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
