/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/ptutils.c,v 1.3 1989-09-24 15:35:04 mar Exp $ */
/* $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/ptutils.c,v $ */


/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

/*	
             Sherri Nichols
             Information Technology Center
       November, 1988

       Modified May, 1989 by Jeff Schiller to keep disk file in
       network byte order

*/

#include <stdio.h>
#include <strings.h>
#include <lock.h>
#define UBIK_INTERNALS
#include <ubik.h>
#include <rx/xdr.h>
#include "print.h"
#include "prserver.h"
#include "prerror.h"
#include <netinet/in.h>

extern struct prheader cheader;
extern struct ubik_dbase *dbase;
extern struct afsconf_dir *prdir;

long CreateEntry(at, aname, aid, idflag,flag,oid,creator)  
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
    newEntry = AllocBlock(at);
    if (!newEntry) return PRDBFAIL;
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
	if (tentry.id < ntohl(cheader.maxGroup)) {
	    cheader.maxGroup = htonl(tentry.id);
	    code = pr_Write(at,0,16,(char *) &cheader.maxGroup,sizeof(cheader.maxGroup));
	    if (code) return PRDBFAIL;
	}
    }
    else if (flag & PRFOREIGN) {
	if (tentry.id > ntohl(cheader.maxForeign)) {
	    cheader.maxForeign = htonl(tentry.id);
	    code = pr_Write(at,0,24,(char *) &cheader.maxForeign,sizeof(cheader.maxForeign));
	    if (code) return PRDBFAIL;
	}
    }
    else {
	if (tentry.id > ntohl(cheader.maxID)) {
	    cheader.maxID = htonl(tentry.id);
	    code = pr_Write(at,0,20,(char *) &cheader.maxID,sizeof(cheader.maxID));
	    if (code) return PRDBFAIL;
	}
    }
    tentry.creator = creator;
    *aid = tentry.id;
    strncpy(tentry.name, aname, PR_MAXNAMELEN);
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
        temp = ntohl(cheader.groupcount) + 1;
	cheader.groupcount = htonl(temp);
	code = pr_Write(at,0,40,(char *)&cheader.groupcount,sizeof(cheader.groupcount));
	if (code) return PRDBFAIL;
    }
    else if (tentry.flags & PRFOREIGN) {
        temp = ntohl(cheader.foreigncount) + 1;
	cheader.foreigncount = htonl(temp);
	code = pr_Write(at,0,44,(char *)&cheader.foreigncount,sizeof(cheader.foreigncount));
	if (code) return PRDBFAIL;
    }
    else if (tentry.flags & PRINST) {
        temp = ntohl(cheader.instcount) + 1;
	cheader.instcount = htonl(temp);
	code = pr_Write(at,0,48,(char *)&cheader.instcount,sizeof(cheader.instcount));
	if (code) return PRDBFAIL;
    }
    else {
        temp = ntohl(cheader.usercount) + 1;
	cheader.usercount = htonl(temp);
	code = pr_Write(at,0,36,(char *)&cheader.usercount,sizeof(cheader.usercount));
	if (code) return PRDBFAIL;
    }
    return PRSUCCESS;
}
    


long RemoveFromEntry(at,aid,bid)
register struct ubik_trans *at;
register long aid;
register long bid;
{
    /* remove aid from bid's entries list, freeing a continuation entry if appropriate */

    register long code;
    struct prentry tentry;
    struct contentry centry;
    struct contentry hentry;
    long temp;
    long first;
    long i,j;
    long nptr;
    long hloc = 0;
    
    bzero(&hentry,sizeof(hentry));
    temp = FindByID(at,bid);
    code = pr_ReadEntry(at, 0, temp, &tentry);
    if (code != 0) return code;
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
    if (tentry.next != NULL) {
	nptr = tentry.next;
	first = 1;
	while (nptr != NULL) {
	    code = pr_ReadCoEntry(at,0,nptr,&centry);
	    if (code != 0) return code;
	    for (i=0;i<COSIZE;i++) {
		if (centry.entries[i] == aid) {
		    centry.entries[i] = PRBADID;
		    tentry.count--;
		    code = pr_WriteEntry(at,0,temp,&tentry);
		    if (code) return PRDBFAIL;
		    for (j=0;j<COSIZE;j++)
			if (centry.entries[j] != PRBADID && centry.entries[j] != 0) break;
		    if (j == COSIZE) {   /* can free this block */
			if (first) {
			    tentry.next = centry.next;
			    code = pr_WriteEntry(at,0,temp,&tentry);
			    if (code != 0) return code;
			}
			else {
			    hentry.next = centry.next;
			    code = pr_WriteCoEntry(at,0,hloc,(char *) &hentry);
			    if (code != 0) return code;
			}
			code = FreeBlock(at,nptr);
			return code;
		    }
		    else { /* can't free it yet */
			code = pr_WriteCoEntry(at,0,nptr,&centry);
			if (code != 0) return code;
			return PRSUCCESS;
		    }
		}
		if (centry.entries[i] == 0) return PRNOENT;
	    }
	    hloc = nptr;
	    nptr = centry.next;
	    bcopy(&centry,&hentry,sizeof(centry));
	}
    }
    else return PRNOENT;
    return PRSUCCESS;
}

long DeleteEntry(at,aid,cid)
register struct ubik_trans *at;
long aid;
long cid;
{
    /* delete the entry aid, removing it from all groups, putting groups owned by it on orphan chain, and freeing the space */
    register long code;
    long temp;
    long temp1;
    struct prentry tentry;
    struct contentry centry;
    struct prentry nentry;
    register long  i;
    long nptr;
    long noAuth;

    noAuth = afsconf_GetNoAuthFlag(prdir);
    bzero(&tentry,sizeof(tentry));
    temp = FindByID(at,aid);
    if (!temp) return PRNOENT;
    code = pr_ReadEntry(at,0,temp,&tentry);
    if (code != 0) return PRDBFAIL;
    if (tentry.owner != cid && !IsAMemberOf(at,cid,SYSADMINID) && !IsAMemberOf(at,cid,tentry.owner) && !noAuth) return PRPERM;
    for (i=0;i<PRSIZE;i++) {
	if (tentry.entries[i] == 0) break;
	RemoveFromEntry(at,aid,tentry.entries[i]);
    }
    nptr = tentry.next;
    while (nptr != NULL) {
	code = pr_ReadCoEntry(at,0,nptr,&centry);
	if (code != 0) return PRDBFAIL;
	for (i=0;i<COSIZE;i++) {
	    if (centry.entries[i] == 0) break;
	    RemoveFromEntry(at,aid,centry.entries[i]);
	}
	nptr = centry.next;
    }
    if (tentry.flags & PRGRP) {
	if (FindByID(at,tentry.owner)) {
	    code = RemoveFromOwnerChain(at,aid,tentry.owner);
	    if (code) return code;
	}
	else {
	    code = RemoveFromOrphan(at,aid);
	    if (code) return code;
	}
    }
    if (tentry.owned) {
	nptr = tentry.owned;
	while (nptr != NULL) {
	    code = pr_ReadEntry(at,0,nptr,&nentry);
	    if (code != 0) return PRDBFAIL;
	    if (nentry.id != aid)  /* don't add this entry to orphan chain! */
		code = AddToOrphan(at,nentry.id);
	    nptr = nentry.nextOwned;
	}
    }
    code = RemoveFromIDHash(at,tentry.id,&temp);
    if (code != PRSUCCESS) return code;
    code = RemoveFromNameHash(at,tentry.name,&temp);
    if (code != PRSUCCESS) return code;
   if (tentry.flags & PRGRP) {
	temp1 = ntohl(cheader.groupcount) + 1;
	cheader.groupcount = htonl(temp1);
	code = pr_Write(at,0,40,(char *)&cheader.groupcount,sizeof(cheader.groupcount));
	if (code) return PRDBFAIL;
    }
    else if (tentry.flags & PRFOREIGN) {
	temp1 = ntohl(cheader.foreigncount) + 1;
	cheader.foreigncount = htonl(temp1);
	code = pr_Write(at,0,44,(char *)&cheader.foreigncount,sizeof(cheader.foreigncount));
	if (code) return PRDBFAIL;
    }
    else if (tentry.flags & PRINST) {
	temp1 = ntohl(cheader.instcount) + 1;
	cheader.instcount = htonl(temp1);
	code = pr_Write(at,0,48,(char *)&cheader.instcount,sizeof(cheader.instcount));
	if (code) return PRDBFAIL;
    }
    else {
	temp1 = ntohl(cheader.usercount) + 1;
	cheader.usercount = htonl(temp1);
	code = pr_Write(at,0,36,(char *)&cheader.usercount,sizeof(cheader.usercount));
	if (code) return PRDBFAIL;
    }
    FreeBlock(at,temp);
    return PRSUCCESS;
}




long AddToEntry(tt,entry,loc,aid)
struct ubik_trans *tt;
struct prentry entry;
long loc;
long aid;
{
    /* add aid to entry's entries list, alloc'ing a continuation block if needed */
    register long code;
    long i;
    struct contentry nentry;
    struct contentry aentry;
    long nptr;
    long last = 0;
    long first = 0;
    long cloc;
    long slot = -1;

    bzero(&nentry,sizeof(nentry));
    bzero(&aentry,sizeof(aentry));
    for (i=0;i<PRSIZE;i++) {
	if (entry.entries[i] == aid)
	    return PRIDEXIST;
	if (entry.entries[i] == PRBADID) { /* remember this spot */
	    first = 1;
	    slot = i;
	}
	if (entry.entries[i] == 0) { /* end of the line */
	    if (slot == -1) {
		first = 1;
		slot = i;
	    }
	    break;
	}
    }
    nptr = entry.next;
    while (nptr != NULL) {
	code = pr_ReadCoEntry(tt,0,nptr,&nentry);
	if (code != 0) return code;
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
	    if (nentry.entries[i] == 0) {
		if (slot == -1) {
		    slot = i;
		    cloc = nptr;
		}
		break;
	    }
	}
	last = nptr;
	nptr = nentry.next;
    }
    if (slot != -1) {  /* we found a place */
	entry.count++;
	if (first) {  /* place is in first block */
	    entry.entries[slot] = aid;
	    code = pr_WriteEntry(tt,0,loc,&entry);
	    if (code != 0) return code;
	    return PRSUCCESS;
	}
	code = pr_WriteEntry(tt,0,loc,&entry);
	code = pr_ReadCoEntry(tt,0,cloc,&aentry);
	if (code != 0) return code;
	aentry.entries[slot] = aid;
	code = pr_WriteCoEntry(tt,0,cloc,&aentry);
	if (code != 0) return code;
	return PRSUCCESS;
    }
    /* have to allocate a continuation block if we got here */
    nptr = AllocBlock(tt);
    if (nentry.flags & PRCONT) {
	/* then we should tack new block here */
	nentry.next = nptr;
	code = pr_WriteCoEntry(tt,0,last,&nentry);
	if (code != 0) return code;
    }
    else {
	entry.next = nptr;
	code = pr_WriteEntry(tt,0,loc,&entry);
	if (code != 0) return code;
    }
    aentry.flags |= PRCONT;
    aentry.id = entry.id;
    aentry.next = NULL;
    aentry.entries[0] = aid;
    code = pr_WriteCoEntry(tt,0,nptr,&aentry);
    if (code != 0) return code;
    /* don't forget to update count, here! */
    entry.count++;
    code = pr_WriteEntry(tt,0,loc,&entry);
    return PRSUCCESS;
	
}

long GetList(at,aid,alist,add)
struct ubik_trans *at;
long aid;
prlist *alist;
long add;
{
    register long code;
    long temp;
    long i;
    long count;
    struct prentry tentry;
    struct contentry centry;
    long nptr;
    long size;
    extern long IDCmp();

    temp = FindByID(at,aid);
    if (!temp) return PRNOENT;
    code = pr_ReadEntry(at,0,temp,&tentry);
    if (code != 0) return code;
    alist->prlist_val = (long *)malloc(100*sizeof(long));
    size = 100;
    alist->prlist_len = 0;
    count = 0;
    for (i=0;i<PRSIZE;i++) {
	if (tentry.entries[i] == PRBADID) continue;
	if (tentry.entries[i] == 0) break;
	alist->prlist_val[count]= tentry.entries[i];
	count++;
	alist->prlist_len++;
    }
    nptr = tentry.next;
    while (nptr != NULL) {
	/* look through cont entries */
	code = pr_ReadCoEntry(at,0,nptr,&centry);
	if (code != 0) return code;
	for (i=0;i<COSIZE;i++) {
	    if (centry.entries[i] == PRBADID) continue;
	    if (centry.entries[i] == 0) break;
	    if (alist->prlist_len >= size) {
		alist->prlist_val = (long *)realloc(alist->prlist_val,(size+100)*sizeof(long));
		size += 100;
	    }
	    alist->prlist_val[count] = centry.entries[i];
	    count++;
	    alist->prlist_len++;
	}
	nptr = centry.next;
    }
    if (add) { /* this is for a CPS, so tack on appropriate stuff */
	if (aid != ANONYMOUSID && aid != ANYUSERID) {
	    if (alist->prlist_len >= size)
		alist->prlist_val = (long *)realloc(alist->prlist_val,(size + 3)*sizeof(long));
	    alist->prlist_val[count] = ANYUSERID;
	    count++;
	    alist->prlist_val[count] = AUTHUSERID;
	    count++;
	    alist->prlist_val[count] = aid;
	    count++;
	    alist->prlist_len += 3;
	}
	else {
	    if (alist->prlist_len >= size)
		alist->prlist_val = (long *)realloc(alist->prlist_val,(size + 2)*sizeof(long));
	    alist->prlist_val[count] = ANYUSERID;
	    count++;
	    alist->prlist_val[count] = aid;
	    count++;
	    alist->prlist_len += 2;
	}
    }
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

Initdb()
{
    long code;
    struct ubik_trans *tt;
    long len;
    long temp;
    long flag = 0;
    static long initd=0;
    static struct ubik_version curver;
    struct ubik_version newver;
    struct ubik_hdr header;

    /* init the database.  We'll try reading it, but if we're starting from scratch, we'll have to do a write transaction. */

    code = ubik_BeginTrans(dbase,UBIK_READTRANS, &tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKREAD);
    if (code) {
	ubik_AbortTrans(tt);
	return code;
    }
    if (!initd) {
	initd = 1;
	bzero(&curver,sizeof(curver));
    }
    len = sizeof(cheader);
    code = pr_Read(tt, 0, 0, (char *) &cheader, len);
    if (code != 0) {
	printf("prserver: couldn't read header -code is %d\n",code);
	ubik_AbortTrans(tt);
	return code;
    }
    if (ntohl(cheader.headerSize) == sizeof(cheader) && ntohl(cheader.eofPtr) != NULL && FindByID(tt,ANONYMOUSID) != 0){
	/* database exists, so we don't have to build it */
	code = ubik_EndTrans(tt);
	if (code) return code;
	return PRSUCCESS;
    }
    /* else we need to build a database */
    code = ubik_EndTrans(tt);
    if (code) return code;
    fprintf(stderr, "Creating new database\n");
    code = ubik_BeginTrans(dbase,UBIK_WRITETRANS, &tt);
    if (code) return code;
    code = ubik_SetLock(tt,1,1,LOCKWRITE);
    if (code) {
	ubik_AbortTrans(tt);
	return code;
    }
    header.magic = htonl(UBIK_MAGIC);
    header.pad1 = 0;
    header.size = 0;
    header.version.epoch = header.version.counter = htonl(1);
    code = pr_Write(tt, 0, -(HDRSIZE), (char *)&header, sizeof(header));
    if (code != 0) {
	printf("prserver: couldn't write ubik header - code is %d.\n", code);
	return code;
    }
    cheader.headerSize = htonl(sizeof(cheader));
    code = pr_Write(tt,0,4,(char *)&cheader.headerSize,sizeof(cheader.headerSize));
    if (code != 0) {
	printf("prserver:  couldn't write header size - code is %d.\n",code);
	ubik_AbortTrans(tt);
	return code;
    }
    cheader.eofPtr = cheader.headerSize; /* already in network order! */
    code = pr_Write(tt,0,12,(char *)&cheader.eofPtr,sizeof(cheader.eofPtr));
    if (code != 0) {
	printf("prserver:  couldn't write eof Ptr - code is %d.\n",code);
	ubik_AbortTrans(tt);
	return code;
    }
    temp = SYSADMINID;
    if (FindByID(tt,SYSADMINID) == 0) {
	/* init sysadmin */
	flag |= PRGRP;
	code = CreateEntry(tt,"system:administrators",&temp,1,flag,SYSADMINID,SYSADMINID);
	if (code != PRSUCCESS) {
	    printf("prserver: couldn't create system:administrators.\n");
	    ubik_AbortTrans(tt);
	    return code;
	}
	flag = 0;
    }
    temp = ANYUSERID;
    if ( FindByID(tt,temp) == 0) { /* init sysadmin */
	flag |= PRGRP;
	code = CreateEntry(tt,"system:anyuser",&temp,1,flag,SYSADMINID,SYSADMINID);
	if (code != PRSUCCESS) {
	    printf("prserver:  couldn't create system:anyuser.\n");
	    ubik_AbortTrans(tt);
	    return code;
	}
	flag = 0;
    }
    temp = AUTHUSERID;
    if (FindByID(tt,temp) == 0) { /* init sysadmin */
	flag |= PRGRP;
	code = CreateEntry(tt,"system:authuser",&temp,1,flag,SYSADMINID,SYSADMINID);
	if (code != PRSUCCESS) {
	    printf("prserver:  couldn't create system:authuser.\n");
	    ubik_AbortTrans(tt);
	    return code;
	}
	flag = 0;
    }
    temp = ANONYMOUSID;
    if (FindByID(tt,temp) == 0) { /* init sysadmin */
	code = CreateEntry(tt,"anonymous",&temp,1,flag,SYSADMINID,SYSADMINID);
	if (code != PRSUCCESS) {
	    printf("prserver:  couldn't create anonymous.\n");
	    ubik_AbortTrans(tt);
	    return code;
	}
	/* well, we don't really want the max id set to anonymousid, so we'll set it back to 0 */
	cheader.maxID = 0; /* Zero is in correct byte order no matter what! */
	code = pr_Write(tt,0,20,(char *)&cheader.maxID,sizeof(cheader.maxID));
	if (code) {
	    printf("prserver: couldn't set max id - code is %d.\n");
	    ubik_AbortTrans(tt);
	    return code;
	}
    }
    code = ubik_EndTrans(tt);
    if (code) return code;
    return PRSUCCESS;
}

long NameToID(at, aname, aid)
register struct ubik_trans *at;
char aname[PR_MAXNAMELEN];
long *aid;
{
    register long code;
    long temp;
    struct prentry tentry;

    temp = FindByName(at,aname);
    if (!temp) return PRNOENT;
    code = pr_ReadEntry(at, 0, temp, &tentry);
    if (code != 0) return code;
    *aid = tentry.id;
    return PRSUCCESS;
}

long IDToName(at, aid, aname)
register struct ubik_trans *at;
long aid;
char aname[PR_MAXNAMELEN];
{
    long temp;
    struct prentry tentry;
    register long code;

    temp = FindByID(at,aid);
    if (!temp) return PRNOENT;
    code = pr_ReadEntry(at,0,temp,&tentry);
    if (code != 0) return code;
    strncpy(aname,tentry.name,PR_MAXNAMELEN);
    return PRSUCCESS;
}

long ChangeEntry(at, aid,cid,name,oid,newid)
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
    long noAuth;
    char *check;
    long tid;
    char holder[PR_MAXNAMELEN];
    char temp[PR_MAXNAMELEN];

    noAuth = afsconf_GetNoAuthFlag(prdir);
    bzero(holder,PR_MAXNAMELEN);
    bzero(temp,PR_MAXNAMELEN);
    loc = FindByID(at,aid);
    if (!loc) return PRNOENT;
    code = pr_ReadEntry(at,0,loc,&tentry);
    if (code) return PRDBFAIL;
    if (tentry.owner != cid && !IsAMemberOf(at,cid,SYSADMINID) && !IsAMemberOf(at,cid,tentry.owner) && !noAuth)
	return PRPERM;
    if (aid != newid && newid != 0) { /* then we're actually trying to change the id */
	pos = FindByID(at,newid);
	if (pos) return PRIDEXIST;  /* new id already in use! */
	if ((aid < 0 && newid) > 0 || (aid > 0 && newid < 0)) return PRPERM;
	/* if new id is not in use, rehash things */
	code = RemoveFromIDHash(at,aid,&loc);
	if (code != PRSUCCESS) return code;
	tentry.id = newid;
	code = pr_WriteEntry(at,0,loc,&tentry);
	code = AddToIDHash(at,tentry.id,loc);
	if (code) return code;
    }
    if (tentry.owner != oid && oid) {
	if (tentry.flags & PRGRP) {
	    /* switch owner chains before we lose old owner */
	    if (FindByID(at,tentry.owner)) /* if it has an owner */
		code = RemoveFromOwnerChain(at,tentry.id,tentry.owner);
	    else  /* must be an orphan */
		code = RemoveFromOrphan(at,tentry.id);
	    if (code) return code;
	    code = AddToOwnerChain(at,tentry.id,oid);
	    if (code) return code;
	}
	tentry.owner = oid;
	if ((tentry.flags & PRGRP) && (strlen(name) == 0)) {
	    /* if we change the owner of a group, it's name will change as well */
	    if (tentry.owner < 0) {
		code = IDToName(at,tentry.owner,temp);
		if (code) return code;
		check = index(temp,':');
		strncpy(holder,temp,check - temp);
	    }
	    else {
		code = IDToName(at,tentry.owner,holder);
		if (code) return code;
	    }
	    strncat(holder,":",PR_MAXNAMELEN);
	    /* now the rest of the name */
	    check = index(tentry.name,':');
	    strncat(holder,++check,PR_MAXNAMELEN);
	    if (strcmp(holder,tentry.name)) {
		/* then the name really did change */
		pos = FindByName(at,holder);
		if (pos) return PREXIST;
		code = RemoveFromNameHash(at,tentry.name,&loc);
		if (code != PRSUCCESS) return code;
		strncpy(tentry.name,holder,PR_MAXNAMELEN);
		code = AddToNameHash(at,tentry.name,loc);
		if (code != PRSUCCESS) return code;
	    }
	}
	code = pr_WriteEntry(at,0,loc,&tentry);
	if (code) return PRDBFAIL;
    }
    if ((strcmp(tentry.name,name)) && (strlen(name)!= 0)) {
	if (tentry.flags & PRGRP) {
	    if ((check = index(name,':')) == NULL) return PRBADNAM;
	    strncpy(temp,name,check-name);
	    code = NameToID(at,temp,&tid);
	    if (tid != tentry.owner) return PRPERM;
	}
	else
	    /* if it's not a group, shouldn't have a : in it */
	    if ((check = index(name,':')) != NULL) 
		return PRBADNAM;
	pos = FindByName(at,name);
	if (pos) return PREXIST;
	code = RemoveFromNameHash(at,tentry.name,&loc);
	if (code != PRSUCCESS) return code;
	strncpy(tentry.name,name,PR_MAXNAMELEN);
	code = pr_WriteEntry(at,0,loc,&tentry);
	if (code) return PRDBFAIL;
	code = AddToNameHash(at,tentry.name,loc);
	if (code != PRSUCCESS) return code;
    }
    return PRSUCCESS;
}
