/* Copyright (C) 1989 Transarc Corporation - All rights reserved */
/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

#ifndef lint
static char rcsid[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/ptutils.c,v 1.4 1990-09-21 15:17:34 mar Exp $";
#endif

/*	
             Sherri Nichols
             Information Technology Center
       November, 1988

       Modified May, 1989 by Jeff Schiller to keep disk file in
       network byte order

*/

#include <afs/param.h>
#include <sys/types.h>
#include <stdio.h>
#include <lock.h>
#include <netinet/in.h>
#include <ubik.h>
#include <rx/xdr.h>
#include <afs/com_err.h>
#include "ptserver.h"
#include "pterror.h"
#include <strings.h>

extern struct ubik_dbase *dbase;
extern struct afsconf_dir *prdir;
extern int pr_noAuth;

static char *whoami = "ptserver";

/* CorrectUserName - Check to make sure a user name is OK.  It must not include
 *   either a colon (or it would look like a group) or an atsign (or it would
 *   look like a foreign user).  The length is checked as well to make sure
 *   that the user name, an atsign, and the local cell name will fit in
 *   PR_MAXNAMELEN.  This is so this user can fit in another cells database as
 *   a foreign user with our cell name tacked on.  This is a predicate, so it
 *   return one if name is OK and zero if name is bogus. */

static int CorrectUserName (name)
  char *name;
{
    extern int pr_realmNameLen;

    if (index (name, ':') || index(name, '@') || index(name, '\n')) return 0;
    if (strlen (name) >= PR_MAXNAMELEN - pr_realmNameLen - 1) return 0; 
    return 1;
}

static long CorrectGroupName (ut, aname, cid, oid, cname)
  struct ubik_trans *ut;
  char aname[PR_MAXNAMELEN];		/* name for group */
  long cid;				/* caller id */
  long oid;				/* owner of group */
  char cname[PR_MAXNAMELEN];		/* correct name for group */
{
    long  code;
    int   admin;
    char *prefix;			/* ptr to group owner part */
    char *suffix;			/* ptr to group name part */
    char  name[PR_MAXNAMELEN];		/* correct name for group */
    struct prentry tentry;

    if (strlen (aname) >= PR_MAXNAMELEN) return PRBADNAM;
    admin = pr_noAuth || IsAMemberOf (ut, cid, SYSADMINID);

    if (oid == 0) oid = cid;

    /* Determine the correct prefix for the name. */
    if (oid == SYSADMINID) prefix = "system";
    else {
	long loc = FindByID (ut, oid);
	if (loc == 0) {
	    /* let admin create groups owned by non-existent ids (probably
             * setting a group to own itself).  Check that they look like
             * groups (with a colon) or otherwise are good user names. */
	    if (admin) {
		strcpy (cname, aname);
		goto done;
	    }
	    return PRNOENT;
	}
	code = pr_Read (ut, 0, loc, &tentry, sizeof(tentry));
	if (code) return code;
	if (ntohl(tentry.flags) & PRGRP) {
	    if ((tentry.count == 0) && !admin) return PRGROUPEMPTY;
	    /* terminate prefix at colon if there is one */
	    if (prefix = index(tentry.name, ':')) *prefix = 0;
	}
	prefix = tentry.name;
    }
    /* only sysadmin allow to use 'system:' prefix */
    if ((strcmp (prefix, "system") == 0) && !admin) return PRPERM;

    strcpy (name, aname);		/* in case aname & cname are same */
    suffix = index(name, ':');
    if (suffix == 0) {
	/* sysadmin can make groups w/o ':', but they must still look like
         * legal user names. */
	if (!admin) return PRBADNAM;
	strcpy (cname, name);
    }
    else {
	if (strlen(prefix)+strlen(suffix) >= PR_MAXNAMELEN) return PRBADNAM;
	strcpy (cname, prefix);
	strcat (cname, suffix);
    }
  done:
    /* check for legal name with either group rules or user rules */
    if (suffix = index(cname, ':')) {
	/* check for confusing characters */
	if (index(cname, '@') ||	/* avoid confusion w/ foreign users */
	    index(cname, '\n') ||	/* restrict so recreate can work */
	    index(suffix+1, ':'))	/* avoid multiple colons */
	    return PRBADNAM;
    } else {
	if (!CorrectUserName (cname)) return PRBADNAM;
    }
    return 0;
}

int AccessOK (ut, cid, tentry, mem, any)
  struct ubik_trans *ut;
  long cid;				/* caller id */
  struct prentry *tentry;		/* object being accessed */
  int mem;				/* check membership in aid, if group */
  int any;				/* if set return true */
{   long flags;
    long oid;
    long aid;

    if (pr_noAuth) return 1;
    if (tentry) {
	flags = tentry->flags;
	oid = tentry->owner;
	aid = tentry->id;
    } else {
	flags = oid = aid = 0;
    }
    if (!(flags & PRACCESS))		/* provide default access */
	if (flags & PRGRP)
	    flags |= PRP_GROUP_DEFAULT;
	else
	    flags |= PRP_USER_DEFAULT;

    if (flags & any) return 1;
    if (oid) {
	if ((cid == oid) ||
	    IsAMemberOf (ut, cid, oid)) return 1;
    }
    if (aid > 0) {			/* checking on a user */
	if (aid == cid) return 1;
    } else if (aid < 0) {		/* checking on group */
	if ((flags & mem) && IsAMemberOf (ut, cid, aid)) return 1;
    }
    if (IsAMemberOf (ut, cid, SYSADMINID)) return 1;
    return 0;				/* no access */
}

long CreateEntry (at, aname, aid, idflag, flag, oid, creator)  
  register struct ubik_trans *at;
  char aname[PR_MAXNAMELEN];
  long *aid;
  long idflag;
  long flag;
  long oid;
  long creator;
{
    /* get and init a new entry */
    register long code;
    long newEntry;
    long temp;
    struct prentry tentry;
    
    bzero(&tentry, sizeof(tentry));

    if ((oid == 0) || (oid == ANONYMOUSID)) oid = creator;

    if (flag & PRGRP) {
	code = CorrectGroupName (at, aname, creator, oid, tentry.name);
	if (code) return code;
	if (strcmp (aname, tentry.name) != 0)  return PRBADNAM;
    } else {				/* non-group must not have colon */
	if (!CorrectUserName(aname)) return PRBADNAM;
	strcpy (tentry.name, aname);
    }

    if (FindByName(at,aname)) return PREXIST;

    newEntry = AllocBlock(at);
    if (!newEntry) return PRDBFAIL;
#ifdef PR_REMEMBER_TIMES
    tentry.createTime = time(0);
#endif
    if (flag & PRGRP) {
	tentry.flags |= PRGRP;
	tentry.owner = oid;
    }
    else if (flag & PRFOREIGN) {
	tentry.flags |= PRFOREIGN;
	tentry.owner = oid;
    }
    else tentry.owner = SYSADMINID;
    if (idflag) 
	tentry.id = *aid;
    else {
	code= AllocID(at,flag,&tentry.id);
	if (code != PRSUCCESS) return code;
    }
    if (flag & PRGRP) {
	/* group ids are negative */
	if (tentry.id < (long)ntohl(cheader.maxGroup)) {
	    code = set_header_word (at, maxGroup, htonl(tentry.id));
	    if (code) return PRDBFAIL;
	}
    }
    else if (flag & PRFOREIGN) {
	if (tentry.id > (long)ntohl(cheader.maxForeign)) {
	    code = set_header_word (at, maxForeign, htonl(tentry.id));
	    if (code) return PRDBFAIL;
	}
    }
    else {
	if (tentry.id > (long)ntohl(cheader.maxID)) {
	    code = set_header_word (at, maxID, htonl(tentry.id));
	    if (code) return PRDBFAIL;
	}
    }
    /* PRACCESS is off until set, defaults provided in AccessOK */
    if (flag == 0) {			/* only normal users get quota */
	tentry.flags |= PRQUOTA;
	tentry.ngroups = tentry.nusers = 20;
    }

    if (flag & (PRGRP | PRFOREIGN)) {
	long loc = FindByID (at, creator);
	struct prentry centry;
	long *nP;			/* ptr to entry to be decremented */
	long  n;			/* quota to check */

	if (loc) { /* this should only fail during initialization */
	    code = pr_Read (at, 0, loc, &centry, sizeof(centry));
	    if (code) return code;

	    if (flag & PRGRP) nP = &centry.ngroups;
	    else if (flag & PRFOREIGN) nP = &centry.nusers;
	    else nP = 0;
	    
	    if (nP) {
		if (!(ntohl(centry.flags) & PRQUOTA)) {
		    /* quota uninitialized, so do it now */
		    centry.flags = htonl (ntohl(centry.flags) | PRQUOTA);
		    centry.ngroups = centry.nusers = htonl(20);
		}
		n = ntohl(*nP);
		if (n <= 0) {
		    if (!pr_noAuth &&
			!IsAMemberOf (at, creator, SYSADMINID))
			return PRNOMORE;
		}
		else *nP = htonl(n-1);
	    }
	    code = pr_Write (at, 0, loc, &centry, sizeof(centry));
	    if (code) return code;
	} /* if (loc) */
    } /* need to check creation quota */
    tentry.creator = creator;
    *aid = tentry.id;
    code = pr_WriteEntry(at, 0, newEntry, &tentry);
    if (code) return PRDBFAIL;
    code = AddToIDHash(at,*aid,newEntry);
    if (code != PRSUCCESS) return code;
    code = AddToNameHash(at,aname,newEntry);
    if (code != PRSUCCESS) return code;
    if (tentry.flags & PRGRP) {
	code = AddToOwnerChain(at,tentry.id,oid);
	if (code) return code;
    }
    if (tentry.flags & PRGRP) {
	if (inc_header_word (at, groupcount, 1)) return PRDBFAIL;
    }
    else if (tentry.flags & PRFOREIGN) {
	if (inc_header_word (at, foreigncount, 1)) return PRDBFAIL;
    }
    else if (tentry.flags & PRINST) {
	if (inc_header_word (at, instcount, 1)) return PRDBFAIL;
    }
    else {
	if (inc_header_word (at, usercount, 1)) return PRDBFAIL;
    }
    return PRSUCCESS;
}
    

/* RemoveFromEntry - remove aid from bid's entries list, freeing a continuation
 * entry if appropriate */

long RemoveFromEntry (at, aid, bid)
  register struct ubik_trans *at;
  register long aid;
  register long bid;
{
    register long code;
    struct prentry tentry;
    struct contentry centry;
    struct contentry hentry;
    long temp;
    long i,j;
    long nptr;
    long hloc;
    
    if (aid == bid) return PRINCONSISTENT;
    bzero(&hentry,sizeof(hentry));
    temp = FindByID(at,bid);
    if (temp == 0) return PRNOENT;
    code = pr_ReadEntry(at, 0, temp, &tentry);
    if (code != 0) return code;
#ifdef PR_REMEMBER_TIMES
    tentry.removeTime = time(0);
#endif
    for (i=0;i<PRSIZE;i++) {
	if (tentry.entries[i] == aid) {  
	    tentry.entries[i] = PRBADID;
	    tentry.count--;
	    code = pr_WriteEntry(at,0,temp,&tentry);
            if (code != 0) return code;
	    return PRSUCCESS;
	}
	if (tentry.entries[i] == 0)   /* found end of list */
	    return PRNOENT;
    }
    hloc = 0;
    nptr = tentry.next;
    while (nptr != NULL) {
	code = pr_ReadCoEntry(at,0,nptr,&centry);
	if (code != 0) return code;
	if ((centry.id != bid) || !(centry.flags & PRCONT)) return PRDBBAD;
	for (i=0;i<COSIZE;i++) {
	    if (centry.entries[i] == aid) {
		centry.entries[i] = PRBADID;
		for (j=0;j<COSIZE;j++)
		    if (centry.entries[j] != PRBADID &&
			centry.entries[j] != 0) break;
		if (j == COSIZE) {   /* can free this block */
		    if (hloc == 0) {
			tentry.next = centry.next;
		    }
		    else {
			hentry.next = centry.next;
			code = pr_WriteCoEntry (at, 0, hloc, &hentry);
			if (code != 0) return code;
		    }
		    code = FreeBlock (at, nptr);
		    if (code) return code;
		}
		else { /* can't free it yet */
		    code = pr_WriteCoEntry(at,0,nptr,&centry);
		    if (code != 0) return code;
		}
		tentry.count--;
		code = pr_WriteEntry(at,0,temp,&tentry);
		if (code) return PRDBFAIL;
		return 0;
	    }
	    if (centry.entries[i] == 0) return PRNOENT;
	} /* for all coentry slots */
	hloc = nptr;
	nptr = centry.next;
	bcopy(&centry,&hentry,sizeof(centry));
    } /* while there are coentries */
    return PRNOENT;
}

/* DeleteEntry - delete the entry in tentry at loc, removing it from all
 * groups, putting groups owned by it on orphan chain, and freeing the space */

long DeleteEntry (at, tentry, loc)
  register struct ubik_trans *at;
  struct prentry *tentry;
  long loc;
{
    register long code;
    long temp1;
    struct contentry centry;
    struct prentry nentry;
    register long  i;
    long nptr;

    /* First remove the entire membership list */
    for (i=0;i<PRSIZE;i++) {
	if (tentry->entries[i] == PRBADID) continue;
	if (tentry->entries[i] == 0) break;
	code = RemoveFromEntry (at, tentry->id, tentry->entries[i]);
	if (code) return code;
    }
    nptr = tentry->next;
    while (nptr != NULL) {
	code = pr_ReadCoEntry(at,0,nptr,&centry);
	if (code != 0) return PRDBFAIL;
	for (i=0;i<COSIZE;i++) {
	    if (centry.entries[i] == PRBADID) continue;
	    if (centry.entries[i] == 0) break;
	    code = RemoveFromEntry (at, tentry->id, centry.entries[i]);
	    if (code) return code;
	}
	code = FreeBlock (at, nptr);	/* free continuation block */
	if (code) return code;
	nptr = centry.next;
    }

    /* Remove us from other's owned chain.  Note that this will zero our owned
     * field (on disk) so this step must follow the above step in case we are
     * on our own owned list. */
    if (tentry->flags & PRGRP) {
	if (tentry->owner) {
	    code = RemoveFromOwnerChain (at, tentry->id, tentry->owner);
	    if (code) return code;
	}
	else {
	    code = RemoveFromOrphan (at, tentry->id);
	    if (code) return code;
	}
    }

    code = RemoveFromIDHash(at,tentry->id,&loc);
    if (code != PRSUCCESS) return code;
    code = RemoveFromNameHash(at,tentry->name,&loc);
    if (code != PRSUCCESS) return code;

    if (tentry->flags & (PRGRP | PRFOREIGN)) {
	long loc = FindByID (at, tentry->creator);
	struct prentry centry;
	if (loc) {
	    code = pr_Read (at, 0, loc, &centry, sizeof(centry));
	    if (code) return code;
	    if (ntohl(centry.flags) & PRQUOTA) {
		if (tentry->flags & PRGRP) {
		    centry.ngroups = htonl(ntohl(centry.ngroups) + 1);
		} else if (tentry->flags & PRFOREIGN) {
		    centry.nusers = htonl(ntohl(centry.nusers) + 1);
		}
	    }
	    code = pr_Write (at, 0, loc, &centry, sizeof(centry));
	    if (code) return code;
	}
    }

    if (tentry->flags & PRGRP) {
	if (inc_header_word (at, groupcount, -1)) return PRDBFAIL;
    }
    else if (tentry->flags & PRFOREIGN) {
	if (inc_header_word (at, foreigncount, -1)) return PRDBFAIL;
    }
    else if (tentry->flags & PRINST) {
	if (inc_header_word (at, instcount, -1)) return PRDBFAIL;
    }
    else {
	if (inc_header_word (at, usercount, -1)) return PRDBFAIL;
    }
    code = FreeBlock(at, loc);
    return code;
}

/* AddToEntry - add aid to entry's entries list, alloc'ing a continuation block
 * if needed.
 *
 * Note the entry is written out by this routine. */

long AddToEntry (tt, entry, loc, aid)
  struct ubik_trans *tt;
  struct prentry *entry;
  long loc;
  long aid;
{
    register long code;
    long i;
    struct contentry nentry;
    struct contentry aentry;
    long nptr;
    long last;				/* addr of last cont. block */
    long first = 0;
    long cloc;
    long slot = -1;

    if (entry->id == aid) return PRINCONSISTENT;
#ifdef PR_REMEMBER_TIMES
    entry->addTime = time(0);
#endif
    for (i=0;i<PRSIZE;i++) {
	if (entry->entries[i] == aid)
	    return PRIDEXIST;
	if (entry->entries[i] == PRBADID) { /* remember this spot */
	    first = 1;
	    slot = i;
	}
	else if (entry->entries[i] == 0) { /* end of the line */
	    if (slot == -1) {
		first = 1;
		slot = i;
	    }
	    break;
	}
    }
    last = 0;
    nptr = entry->next;
    while (nptr != NULL) {
 	code = pr_ReadCoEntry(tt,0,nptr,&nentry);
	if (code != 0) return code;
	last = nptr;
	if (!(nentry.flags & PRCONT)) return PRDBFAIL;
	for (i=0;i<COSIZE;i++) {
	    if (nentry.entries[i] == aid)
		return PRIDEXIST;
	    if (nentry.entries[i] == PRBADID) {
		if (slot == -1) {
		    slot = i;
		    cloc = nptr;
		}
	    }
	    else if (nentry.entries[i] == 0) {
		if (slot == -1) {
		    slot = i;
		    cloc = nptr;
		}
		break;
	    }
	}
	nptr = nentry.next;
    }
    if (slot != -1) {			/* we found a place */
	entry->count++;
	if (first) {  /* place is in first block */
	    entry->entries[slot] = aid;
	    code = pr_WriteEntry (tt, 0, loc, entry);
	    if (code != 0) return code;
	    return PRSUCCESS;
	}
	code = pr_WriteEntry (tt, 0, loc, entry);
	if (code) return code;
	code = pr_ReadCoEntry(tt,0,cloc,&aentry);
	if (code != 0) return code;
	aentry.entries[slot] = aid;
	code = pr_WriteCoEntry(tt,0,cloc,&aentry);
	if (code != 0) return code;
	return PRSUCCESS;
    }
    /* have to allocate a continuation block if we got here */
    nptr = AllocBlock(tt);
    if (last) {
	/* then we should tack new block after last block in cont. chain */
	nentry.next = nptr;
	code = pr_WriteCoEntry(tt,0,last,&nentry);
	if (code != 0) return code;
    }
    else {
	entry->next = nptr;
    }
    bzero(&aentry,sizeof(aentry));
    aentry.flags |= PRCONT;
    aentry.id = entry->id;
    aentry.next = NULL;
    aentry.entries[0] = aid;
    code = pr_WriteCoEntry(tt,0,nptr,&aentry);
    if (code != 0) return code;
    /* don't forget to update count, here! */
    entry->count++;
    code = pr_WriteEntry (tt, 0, loc, entry);
    return code;
	
}

long AddToPRList (alist, sizeP, id)
  prlist *alist;
  int *sizeP;
  long id;
{
    if (alist->prlist_len >= PR_MAXGROUPS) return PRTOOMANY;
    if (alist->prlist_len >= *sizeP) {
	*sizeP = *sizeP + 100;
	if (*sizeP > PR_MAXGROUPS) *sizeP = PR_MAXGROUPS;
	alist->prlist_val =
	    (long *) ((alist->prlist_val) ?
		      realloc (alist->prlist_val, (*sizeP)*sizeof(long)) :
		      malloc ((*sizeP)*sizeof(long)));
    }
    alist->prlist_val[alist->prlist_len++] = id;
    return 0;
}

long GetList (at, tentry, alist, add)
  struct ubik_trans *at;
  struct prentry *tentry;
  prlist *alist;
  long add;
{
    register long code;
    long i;
    struct contentry centry;
    long nptr;
    int size;
    int count = 0;
    extern long IDCmp();

    size = 0;
    alist->prlist_val = 0;
    alist->prlist_len = 0;

    for (i=0;i<PRSIZE;i++) {
	if (tentry->entries[i] == PRBADID) continue;
	if (tentry->entries[i] == 0) break;
	code = AddToPRList (alist, &size, tentry->entries[i]);
	if (code) return code;
    }

    nptr = tentry->next;
    while (nptr != NULL) {
	/* look through cont entries */
	code = pr_ReadCoEntry(at,0,nptr,&centry);
	if (code != 0) return code;
	for (i=0;i<COSIZE;i++) {
	    if (centry.entries[i] == PRBADID) continue;
	    if (centry.entries[i] == 0) break;
	    code = AddToPRList (alist, &size, centry.entries[i]);
	    if (code) return code;
	}
	nptr = centry.next;
	if (count++ > 50) IOMGR_Poll(), count = 0;
    }

    if (add) { /* this is for a CPS, so tack on appropriate stuff */
	if (tentry->id != ANONYMOUSID && tentry->id != ANYUSERID) {
	    if ((code = AddToPRList (alist, &size, ANYUSERID)) ||
		(code = AddToPRList (alist, &size, AUTHUSERID)) ||
		(code = AddToPRList (alist, &size, tentry->id))) return code;
	}
	else {
	    if ((code = AddToPRList (alist, &size, ANYUSERID)) ||
		(code = AddToPRList (alist, &size, tentry->id))) return code;
	}
    }
    if (alist->prlist_len > 100) IOMGR_Poll();
    qsort(alist->prlist_val,alist->prlist_len,sizeof(long),IDCmp);
    return PRSUCCESS;
}

long GetOwnedChain (ut, next, alist)
  struct ubik_trans *ut;
  long next;
  prlist *alist;
{   register long code;
    struct prentry tentry;
    int size;
    int count = 0;

    size = 0;
    alist->prlist_val = 0;
    alist->prlist_len = 0;

    while (next) {
	code = pr_Read (ut, 0, next, &tentry, sizeof(tentry));
	if (code) return code;
	code = AddToPRList (alist, &size, ntohl(tentry.id));
	if (code) return code;
	next = ntohl(tentry.nextOwned);
	if (count++ > 50) IOMGR_Poll(), count = 0;
    }
    if (alist->prlist_len > 100) IOMGR_Poll();
    qsort(alist->prlist_val,alist->prlist_len,sizeof(long),IDCmp);
    return PRSUCCESS;
}

long GetMax(at,uid,gid)
register struct ubik_trans *at;
long *uid;
long *gid;
{
    *uid = ntohl(cheader.maxID);
    *gid = ntohl(cheader.maxGroup);
    return PRSUCCESS;
}

long SetMax(at,id,flag)
register struct ubik_trans *at;
long id;
long flag;
{
    register long code;
    if (flag & PRGRP) {
	cheader.maxGroup = htonl(id);
	code = pr_Write(at,0,16,(char *)&cheader.maxGroup,sizeof(cheader.maxGroup));
	if (code != 0) return code;
    }
    else {
	cheader.maxID = htonl(id);
	code = pr_Write(at,0,20,(char *)&cheader.maxID,sizeof(cheader.maxID));
	if (code != 0) return code;
    }
    return PRSUCCESS;
}

int pr_noAuth;

long Initdb()
{
    long code;
    struct ubik_trans *tt;
    long len;
    static long initd=0;
#if 0
    static struct ubik_version curver;
    struct ubik_version newver;
#endif

    /* init the database.  We'll try reading it, but if we're starting from scratch, we'll have to do a write transaction. */

    pr_noAuth = afsconf_GetNoAuthFlag(prdir);

    code = ubik_BeginTrans(dbase,UBIK_READTRANS, &tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKREAD);
    if (code) {
	ubik_AbortTrans(tt);
	return code;
    }
    if (!initd) {
	initd = 1;
#if 0
	bzero(&curver,sizeof(curver));
#endif
    } else if (!ubik_CacheUpdate (tt)) {
	code = ubik_EndTrans(tt);
	return code;
    }
#if 0
    code = ubik_GetVersion(tt,&newver);
    if (vcmp(curver,newver) == 0) {
	/* same version */
	code = ubik_EndTrans(tt);
	if (code) return code;
	return PRSUCCESS;
    }
    bcopy(&newver,&curver,sizeof(struct ubik_version));
#endif

    len = sizeof(cheader);
    code = pr_Read(tt, 0, 0, (char *) &cheader, len);
    if (code != 0) {
	com_err (whoami, code, "couldn't read header");
	ubik_AbortTrans(tt);
	return code;
    }
    if ((ntohl(cheader.version) == PRDBVERSION) &&
	ntohl(cheader.headerSize) == sizeof(cheader) &&
	ntohl(cheader.eofPtr) != NULL &&
	FindByID(tt,ANONYMOUSID) != 0){
	/* database exists, so we don't have to build it */
	code = ubik_EndTrans(tt);
	if (code) return code;
	return PRSUCCESS;
    }
    /* else we need to build a database */
    code = ubik_EndTrans(tt);
    if (code) return code;

    /* Only rebuild database if the db was deleted (the header is zero) and we
       are running noAuth. */
    {   char *bp = (char *)&cheader;
	int i;
	for (i=0; i<sizeof(cheader); i++)
	    if (bp[i])  {
		code = PRDBBAD;
		com_err (whoami, code,
			 "Can't rebuild database because it is not empty");
		return code;
	    }
    }
    if (!pr_noAuth) {
	code = PRDBBAD;
	com_err (whoami, code,
		 "Can't rebuild database because not running NoAuth");
	return code;
    }

    code = ubik_BeginTrans(dbase,UBIK_WRITETRANS, &tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKWRITE);
    if (code) {
	ubik_AbortTrans(tt);
	return code;
    }

    /* Initialize the database header */

    if ((code = set_header_word (tt, version, htonl(PRDBVERSION))) ||
	(code = set_header_word (tt, headerSize, htonl(sizeof(cheader)))) ||
	(code = set_header_word (tt, eofPtr, cheader.headerSize))) {
	com_err (whoami, code, "couldn't write header words");
	ubik_AbortTrans(tt);
	return code;
    }

#define InitialGroup(id,name) do {    \
    long temp = (id);		      \
    long flag = (id) < 0 ? PRGRP : 0; \
    code = CreateEntry		      \
	(tt, (name), &temp, /*idflag*/1, flag, SYSADMINID, SYSADMINID); \
    if (code) {			      \
	com_err (whoami, code, "couldn't create %s with id %di.", 	\
		 (name), (id));	      \
	ubik_AbortTrans(tt);	      \
	return code;		      \
    }				      \
} while (0)

    InitialGroup (SYSADMINID, "system:administrators");
    InitialGroup (ANYUSERID, "system:anyuser");
    InitialGroup (AUTHUSERID, "system:authuser");
    InitialGroup (ANONYMOUSID, "anonymous");

    /* Well, we don't really want the max id set to anonymousid, so we'll set
     * it back to 0 */
    code = set_header_word (tt, maxID, 0); /* correct in any byte order */
    if (code) {
	com_err (whoami, code, "couldn't reset max id");
	ubik_AbortTrans(tt);
	return code;
    }

    code = ubik_EndTrans(tt);
    if (code) return code;
    return PRSUCCESS;
}

long ChangeEntry (at, aid, cid, name, oid, newid)
  struct ubik_trans *at;
  long aid;
  long cid;
  char *name;
  long oid;
  long newid;
{
    register long code;
    long pos;
    struct prentry tentry;
    long loc;
    long oldowner;
    char holder[PR_MAXNAMELEN];
    char temp[PR_MAXNAMELEN];
    char oldname[PR_MAXNAMELEN];

    bzero(holder,PR_MAXNAMELEN);
    bzero(temp,PR_MAXNAMELEN);
    loc = FindByID(at,aid);
    if (!loc) return PRNOENT;
    code = pr_ReadEntry(at,0,loc,&tentry);
    if (code) return PRDBFAIL;
    if (tentry.owner != cid &&
	!IsAMemberOf(at,cid,SYSADMINID) &&
	!IsAMemberOf(at,cid,tentry.owner) &&
	!pr_noAuth) return PRPERM;
#ifdef PR_REMEMBER_TIMES
    tentry.changeTime = time(0);
#endif

    /* we're actually trying to change the id */
    if (aid != newid && newid != 0) {
	if (!IsAMemberOf(at,cid,SYSADMINID) && !pr_noAuth) return PRPERM;
	pos = FindByID(at,newid);
	if (pos) return PRIDEXIST;  /* new id already in use! */
	if ((aid < 0 && newid) > 0 || (aid > 0 && newid < 0)) return PRPERM;
	/* if new id is not in use, rehash things */
	code = RemoveFromIDHash(at,aid,&loc);
	if (code != PRSUCCESS) return code;
	tentry.id = newid;
	code = pr_WriteEntry(at,0,loc,&tentry);
	if (code) return code;
	code = AddToIDHash(at,tentry.id,loc);
	if (code) return code;
	/* get current data */
	code = pr_ReadEntry(at,0,loc,&tentry);
	if (code) return PRDBFAIL;
    }

    /* Change the owner */
    if (tentry.owner != oid && oid) {
	/* only groups can have their owner's changed */
	if (!(tentry.flags & PRGRP)) return PRPERM;
	oldowner = tentry.owner;
	tentry.owner = oid;
	/* The entry must be written through first so Remove and Add routines
         * can operate on disk data */
	code = pr_WriteEntry(at,0,loc,(char *)&tentry);
	if (code) return PRDBFAIL;
	if (tentry.flags & PRGRP) {
	    /* switch owner chains */
	    if (oldowner)		/* if it has an owner */
		code = RemoveFromOwnerChain(at,tentry.id,oldowner);
	    else			/* must be an orphan */
		code = RemoveFromOrphan(at,tentry.id);
	    if (code) return code;
	    code = AddToOwnerChain(at,tentry.id,tentry.owner);
	    if (code) return code;
	}
	/* fix up the name */
	if ((tentry.flags & PRGRP) && (strlen(name) == 0)) name = tentry.name;
	/* get current data */
	code = pr_ReadEntry(at,0,loc,&tentry);
	if (code) return PRDBFAIL;
    }

    /* Change the name, if name is a ptr to tentry.name then this name change
     * is due to a chown, otherwise caller has specified a new name */
    if ((name == tentry.name) ||
	(*name && (strcmp (tentry.name, name) != 0))) {
	strncpy (oldname, tentry.name, PR_MAXNAMELEN);
	if (tentry.flags & PRGRP) {
	    code = CorrectGroupName (at, name, cid, tentry.owner, tentry.name);
	    if (code) return code;

	    if (name == tentry.name) {	/* owner fixup */
		if (strcmp (oldname, tentry.name) == 0) goto nameOK;
	    } else {			/* new name, caller must be correct */
		if (strcmp (name, tentry.name) != 0) return PRBADNAM;
	    }
	}
	else
	    if (!CorrectUserName(name)) return PRBADNAM;

	pos = FindByName(at,name);
	if (pos) return PREXIST;
	code = RemoveFromNameHash (at, oldname, &loc);
	if (code != PRSUCCESS) return code;
	strncpy (tentry.name, name, PR_MAXNAMELEN);
	code = pr_WriteEntry(at,0,loc,(char *)&tentry);
	if (code) return PRDBFAIL;
	code = AddToNameHash(at,tentry.name,loc);
	if (code != PRSUCCESS) return code;
nameOK:;
    }
    return PRSUCCESS;
}
