/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/utils.c,v 1.2 1989-09-23 18:44:59 mar Exp $ */
/* $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/utils.c,v $ */


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


#include <sys/types.h>
#include <lock.h>
#include <ubik.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <rx/xdr.h>
#include <rx/rx.h>
#include <rx/rx_vab.h>
#include "print.h"
#include "prserver.h"
#include "prerror.h"

extern struct prheader cheader;
extern struct ubik_dbase *dbase;

long IDHash(x)
long x;
{
    /* returns hash bucket for x */
    return ((abs(x)) % HASHSIZE);
}

long NameHash(aname)
register unsigned char *aname;
{
    /* returns hash bucket for aname */
    register unsigned int hash=0;
    register int i;
/* stolen directly from the HashString function in the vol package */
    for (i=strlen(aname),aname += i-1;i--;aname--)
	hash = (hash*31) + (*aname-31);
    return(hash % HASHSIZE);
}


long pr_Write(tt,afd,pos,buff,len)
struct ubik_trans *tt;
long afd;
long pos;
char *buff;
long len;
{
    /* package up seek and write into one procedure for ease of use */
    long code;
    code = ubik_Seek(tt,afd,pos);
    if (code) return code;
    code = ubik_Write(tt,buff,len);
    return code;
}

long pr_Read(tt,afd,pos,buff,len)
struct ubik_trans *tt;
long afd;
long pos;
char *buff;
long len;
{
    /* same thing for read */
    long code;
    code = ubik_Seek(tt,afd,pos);
    if (code) return code;
    code = ubik_Read(tt,buff,len);
    return code;
}

pr_WriteEntry(tt, afd, pos, tentry)
struct ubik_trans *tt;
long afd;
long pos;
struct prentry *tentry;
{
    long code;
    register long i;
    struct prentry nentry;
    if (ntohl(1) != 1) {	/* Need to swap bytes. */
      nentry.flags = htonl(tentry->flags);
      nentry.id = htonl(tentry->id);
      nentry.cellid = htonl(tentry->cellid);
      nentry.next = htonl(tentry->next);
      nentry.nextID = htonl(tentry->nextID);
      nentry.nextName = htonl(tentry->nextName);
      nentry.owner = htonl(tentry->owner);
      nentry.creator = htonl(tentry->creator);
      nentry.ngroups = htonl(tentry->ngroups);
      nentry.nusers = htonl(tentry->nusers);
      nentry.count = htonl(tentry->count);
      nentry.instance = htonl(tentry->instance);
      nentry.owned = htonl(tentry->owned);
      nentry.nextOwned = htonl(tentry->nextOwned);
      nentry.parent = htonl(tentry->parent);
      nentry.sibling = htonl(tentry->sibling);
      nentry.child = htonl(tentry->child);
      strncpy(nentry.name, tentry->name, PR_MAXNAMELEN);
      for (i = 0; i < PRSIZE; i++)
	nentry.entries[i] = htonl(tentry->entries[i]);
      code = ubik_Seek(tt, afd, pos);
      if (code) return (code);
      code = ubik_Write(tt, (char *) &nentry, sizeof(struct prentry));
      return(code);
    } else {
      code = ubik_Seek(tt, afd, pos);
      if (code) return (code);
      code = ubik_Write(tt, (char *) tentry, sizeof(struct prentry));
      return(code);
    }
}

pr_ReadEntry(tt, afd, pos, tentry)
struct ubik_trans *tt;
long afd;
long pos;
struct prentry *tentry;
{
    long code;
    register long i;
    struct prentry nentry;
    code = ubik_Seek(tt, afd, pos);
    if (code) return (code);
    if (ntohl(1) == 1) {	/* no swapping needed */
      code = ubik_Read(tt, (char *) tentry, sizeof(struct prentry));
      return(code);
    }
    code = ubik_Read(tt, (char *) &nentry, sizeof(struct prentry));
    if (code) return (code);
    tentry->flags = ntohl(nentry.flags);
    tentry->id = ntohl(nentry.id);
    tentry->cellid = ntohl(nentry.cellid);
    tentry->next = ntohl(nentry.next);
    tentry->nextID = ntohl(nentry.nextID);
    tentry->nextName = ntohl(nentry.nextName);
    tentry->owner = ntohl(nentry.owner);
    tentry->creator = ntohl(nentry.creator);
    tentry->ngroups = ntohl(nentry.ngroups);
    tentry->nusers = ntohl(nentry.nusers);
    tentry->count = ntohl(nentry.count);
    tentry->instance = ntohl(nentry.instance);
    tentry->owned = ntohl(nentry.owned);
    tentry->nextOwned = ntohl(nentry.nextOwned);
    tentry->parent = ntohl(nentry.parent);
    tentry->sibling = ntohl(nentry.sibling);
    tentry->child = ntohl(nentry.child);
    strncpy(tentry->name, nentry.name, PR_MAXNAMELEN);
    for (i = 0; i < PRSIZE; i++)
      tentry->entries[i] = ntohl(nentry.entries[i]);
    return(code);
}

pr_WriteCoEntry(tt, afd, pos, tentry)
struct ubik_trans *tt;
long afd;
long pos;
struct contentry *tentry;
{
    long code;
    register long i;
    struct contentry nentry;
    if (ntohl(1) == 1) {	/* No need to swap */
      code = ubik_Seek(tt, afd, pos);
      if (code) return(code);
      code = ubik_Write(tt, (char *) tentry, sizeof(struct contentry));
      return(code);
    }
    nentry.flags = htonl(tentry->flags);
    nentry.id = htonl(tentry->id);
    nentry.cellid = htonl(tentry->cellid);
    nentry.next = htonl(tentry->next);
    for (i = 0; i < COSIZE; i++)
      nentry.entries[i] = htonl(tentry->entries[i]);
    code = ubik_Seek(tt, afd, pos);
    if (code) return (code);
    code = ubik_Write(tt, (char *) &nentry, sizeof(struct contentry));
    return(code);
}

pr_ReadCoEntry(tt, afd, pos, tentry)
struct ubik_trans *tt;
long afd;
long pos;
struct contentry *tentry;
{
    long code;
    register long i;
    struct contentry nentry;
    code = ubik_Seek(tt, afd, pos);
    if (code) return (code);
    if (ntohl(1) == 1) {	/* No swapping needed. */
      code = ubik_Read(tt, (char *) tentry, sizeof(struct contentry));
      return(code);
    }
    code = ubik_Read(tt, (char *) &nentry, sizeof(struct contentry));
    if (code) return (code);
    tentry->flags = ntohl(nentry.flags);
    tentry->id = ntohl(nentry.id);
    tentry->cellid = ntohl(nentry.cellid);
    tentry->next = ntohl(nentry.next);
    for (i = 0; i < COSIZE; i++)
      tentry->entries[i] = ntohl(nentry.entries[i]);
    return(code);
}

long AllocBlock(at)
register struct ubik_trans *at;
{
    /* allocate a free block of storage for entry, returning address of new entry  */
    register long code;
    long temp;
    struct prentry tentry;

    if (cheader.freePtr) {
	/* allocate this dude */
	temp = ntohl(cheader.freePtr);      
	code = pr_ReadEntry(at, 0, temp, &tentry);
	if (code) return 0;
	cheader.freePtr = htonl(tentry.next);
	code = pr_Write(at, 0, 8, (char *)&cheader.freePtr, sizeof(cheader.freePtr));
	if (code != 0) return 0;
	return temp;
    }
    else {
	/* hosed, nothing on free list, grow file */
	temp = ntohl(cheader.eofPtr);	/* remember this guy */
	cheader.eofPtr = htonl(temp + ENTRYSIZE);
	code = pr_Write(at, 0, 12,(char *) &cheader.eofPtr, sizeof(cheader.eofPtr));
	if (code != 0) return 0;
	return temp;
    }
}

long FreeBlock(at, pos)
register struct ubik_trans *at;
long pos;
{
    /* add a block of storage to the free list */
    register long code;
    struct prentry tentry;

    bzero(&tentry,sizeof(tentry));
    tentry.next = ntohl(cheader.freePtr);
    tentry.flags |= PRFREE;
    cheader.freePtr = htonl(pos);
    code = pr_Write(at,0,8, (char *) &cheader.freePtr,sizeof(cheader.freePtr));
    if (code != 0) return code;
    code = pr_WriteEntry(at,0,pos,&tentry);
    if (code != 0) return code;
    return PRSUCCESS;
}

long FindByID(at,aid)
register struct ubik_trans *at;
long aid;
{
    /* returns address of entry if found, 0 otherwise */
    register long code;
    long i;
    struct prentry tentry;
    long entry;

    i = IDHash(aid);
    entry = ntohl(cheader.idHash[i]);
    if (entry == 0) return entry;
    bzero(&tentry,sizeof(tentry));
    code = pr_ReadEntry(at, 0, entry, &tentry);
    if (code != 0) return 0;
    if (aid == tentry.id) return entry;
    entry = tentry.nextID;
    while (entry != NULL) {
	bzero(&tentry,sizeof(tentry));
	code = pr_ReadEntry(at,0,entry,&tentry);
	if (code != 0) return 0;
	if (aid == tentry.id) return entry;
	entry = tentry.nextID;
    }
    return 0;
}
 


long FindByName(at,aname)    
register struct ubik_trans *at;
char aname[PR_MAXNAMELEN];
{
    /* ditto */
    register long code;
    long i;
    struct prentry tentry;
    long entry;

    i = NameHash(aname);
    entry = ntohl(cheader.nameHash[i]);
    if (entry == 0) return entry;
    bzero(&tentry,sizeof(tentry));
    code = pr_ReadEntry(at, 0, entry,&tentry);
    if (code != 0) return 0;
    if ((strncmp(aname,tentry.name,PR_MAXNAMELEN)) == 0) return entry;
    entry = tentry.nextName;
    while (entry != NULL) {
	bzero(&tentry,sizeof(tentry));
	code = pr_ReadEntry(at,0,entry,&tentry);
	if (code != 0) return 0;
	if ((strncmp(aname,tentry.name,PR_MAXNAMELEN)) == 0) return entry;
	entry = tentry.nextName;
    }
    return 0;
}

long AllocID(at,flag,aid)
register struct ubik_trans *at;
long flag;
long *aid;
{
    /* allocs an id from the proper area of address space, based on flag */
    register long code = 1;
    register long i = 0;
    register maxcount = 50;  /* to prevent infinite loops */
    
    if (flag & PRGRP) {
	*aid = ntohl(cheader.maxGroup);
	while (code && i<maxcount) {
	    --(*aid);
	    code = FindByID(at,*aid);
	    i++;
	}
	if (code) return PRNOIDS;
	cheader.maxGroup = htonl(*aid);
	code = pr_Write(at,0,16,(char *)&cheader.maxGroup,sizeof(cheader.maxGroup));
	if (code) return PRDBFAIL;
	return PRSUCCESS;
    }
    else if (flag & PRFOREIGN) {
	*aid = ntohl(cheader.maxForeign);
	while (code && i<maxcount) {
	    ++(*aid);
	    code = FindByID(at,*aid);
	    i++;
	}
	if (code) return PRNOIDS;
	cheader.maxForeign = htonl(*aid);
	code = pr_Write(at,0,24,(char *)&cheader.maxForeign,sizeof(cheader.maxForeign));
	if (code) return PRDBFAIL;
	return PRSUCCESS;
    }
    else {
	*aid = ntohl(cheader.maxID);
	while (code && i<maxcount) {
	    ++(*aid);
	    code = FindByID(at,*aid);
	    i++;
	}
	if (code) return PRNOIDS;
	cheader.maxID = htonl(*aid);
	code = pr_Write(at,0,20,(char *)&cheader.maxID,sizeof(cheader.maxID));
	if (code) return PRDBFAIL;
	return PRSUCCESS;
    }
}

long IDCmp(a,b)
long *a;
long *b;
{
    /* used to sort CPS's so that comparison with acl's is easier */
    if (*a > *b) return 1;
    if (*a == *b) return 0;
    if (*a < *b) return -1;
}

long RemoveFromIDHash(tt,aid,loc)
struct ubik_trans *tt;
long aid;
long *loc;
{
    /* remove entry designated by aid from id hash table */
    register long code;
    long current, trail, i;
    struct prentry tentry;
    struct prentry bentry;

    i = IDHash(aid);
    current = ntohl(cheader.idHash[i]);
    bzero(&tentry,sizeof(tentry));
    bzero(&bentry,sizeof(bentry));
    trail = 0;
    if (current == NULL) return PRNOENT;
    code = pr_ReadEntry(tt,0,current,&tentry);
    if (code) return PRDBFAIL;
    while (aid != tentry.id) {
	trail = current;
	current = tentry.nextID;
	if (current == NULL) break;
	code = pr_ReadEntry(tt,0,current,&tentry);
	if (code) return PRDBFAIL;
    }
    if (current == NULL) return PRNOENT;  /* we didn't find him */
    if (trail == NULL) {
	/* it's the first entry! */
	cheader.idHash[i] = htonl(tentry.nextID);
	code = pr_Write(tt,0,72+HASHSIZE*4+i*4,(char *)&cheader.idHash[i],sizeof(cheader.idHash[i]));
	if (code) return PRDBFAIL;
    }
    else {
	code = pr_ReadEntry(tt,0,trail, &bentry);
	if (code) return PRDBFAIL;
	bentry.nextID = tentry.nextID;
	code = pr_WriteEntry(tt,0,trail,&bentry);
    }
    *loc = current;
    return PRSUCCESS;
}

long AddToIDHash(tt, aid, loc)
struct ubik_trans *tt;
long aid;
long loc;
{
    /* add entry at loc designated by aid to id hash table */
    register long code;
    long i;
    struct prentry tentry;

    i = IDHash(aid);
    bzero(&tentry,sizeof(tentry));
    code = pr_ReadEntry(tt,0,loc,&tentry);
    if (code) return PRDBFAIL;
    tentry.nextID = ntohl(cheader.idHash[i]);
    cheader.idHash[i] = htonl(loc);
    code = pr_WriteEntry(tt,0,loc,&tentry);
    if (code) return PRDBFAIL;
    code = pr_Write(tt,0,72+HASHSIZE*4+i*4,(char *)&cheader.idHash[i],sizeof(cheader.idHash[i]));
    if (code) return PRDBFAIL;
    return PRSUCCESS;
}

long RemoveFromNameHash(tt,aname,loc)
struct ubik_trans *tt;
char *aname;
long *loc;
{
    /* remove from name hash */
    register long code;
    long current, trail, i;
    struct prentry tentry;
    struct prentry bentry;

    i = NameHash(aname);
    current = ntohl(cheader.nameHash[i]);
    bzero(&tentry,sizeof(tentry));
    bzero(&bentry,sizeof(bentry));
    trail = 0;
    if (current == NULL) return PRNOENT;
    code = pr_ReadEntry(tt,0,current,&tentry);
    if (code) return PRDBFAIL;
    while (strcmp(aname,tentry.name)) {
	trail = current;
	current = tentry.nextName;
	if (current == NULL) break;
	code = pr_ReadEntry(tt,0,current,&tentry);
	if (code) return PRDBFAIL;
    }
    if (current == NULL) return PRNOENT;  /* we dnamen't find him */
    if (trail == NULL) {
	/* it's the first entry! */
	cheader.nameHash[i] = htonl(tentry.nextName);
	code = pr_Write(tt,0,72+i*4,(char *)&cheader.nameHash[i],sizeof(cheader.nameHash[i]));
	if (code) return PRDBFAIL;
    }
    else {
	code = pr_ReadEntry(tt,0,trail, &bentry);
	if (code) return PRDBFAIL;
	bentry.nextName = tentry.nextName;
	code = pr_WriteEntry(tt,0,trail,&bentry);
    }
    *loc = current;
    return PRSUCCESS;
}

long AddToNameHash(tt, aname, loc)
struct ubik_trans *tt;
char *aname;
long loc;
{
    /* add to name hash */
    register long code;
    long i;
    struct prentry tentry;

    i = NameHash(aname);
    bzero(&tentry,sizeof(tentry));
    code = pr_ReadEntry(tt,0,loc,&tentry);
    if (code) return PRDBFAIL;
    tentry.nextName = ntohl(cheader.nameHash[i]);
    cheader.nameHash[i] = htonl(loc);
    code = pr_WriteEntry(tt,0,loc,&tentry);
    if (code) return PRDBFAIL;
    code = pr_Write(tt,0,72+i*4,(char *)&cheader.nameHash[i],sizeof(cheader.nameHash[i]));
    if (code) return PRDBFAIL;
    return PRSUCCESS;
}

long AddToOwnerChain(at,gid,oid)
struct ubik_trans *at;
long gid;
long oid;
{
    /* add entry designated by gid to owner chain of entry designated by oid */
    register long code;
    long loc;
    long gloc;
    struct prentry tentry;
    struct prentry gentry;

    loc = FindByID(at,oid);
    if (!loc) return PRNOENT;
    code = pr_ReadEntry(at,0,loc,&tentry);
    if (code != 0) return PRDBFAIL;
    gloc = FindByID(at,gid);
    code = pr_ReadEntry(at,0,gloc,&gentry);
    if (code != 0) return PRDBFAIL;
    gentry.nextOwned = tentry.owned;
    tentry.owned = gloc;
    code = pr_WriteEntry(at,0,loc,&tentry);
    if (code != 0) return PRDBFAIL;
    code = pr_WriteEntry(at,0,gloc,&gentry);
    if (code != 0) return PRDBFAIL;
    return PRSUCCESS;
}

long RemoveFromOwnerChain(at,gid,oid)
struct ubik_trans *at;
long gid;
long oid;
{
    /* remove gid from owner chain for oid */
    register long code;
    long nptr;
    struct prentry tentry;
    struct prentry bentry;
    long loc;

    loc = FindByID(at,oid);
    if (!loc) return PRNOENT;
    code = pr_ReadEntry(at,0,loc,&tentry);
    if (code != 0) return PRDBFAIL;
    if (!tentry.owned) return PRNOENT;
    nptr = tentry.owned;
    bcopy(&tentry,&bentry,sizeof(tentry));
    while (nptr != NULL) {
	code = pr_ReadEntry(at,0,nptr,&tentry);
	if (code != 0) return PRDBFAIL;
	if (tentry.id == gid) {
	    /* found it */
	    if (nptr == bentry.owned) /* modifying first of chain */
		bentry.owned = tentry.nextOwned;
	    else bentry.nextOwned = tentry.nextOwned;
	    tentry.nextOwned = 0;
	    code = pr_WriteEntry(at,0,loc,&bentry);
	    if (code != 0) return PRDBFAIL;
	    code = pr_WriteEntry(at,0,nptr,&tentry);
	    if (code != 0) return PRDBFAIL;
	    return PRSUCCESS;
	}
	loc = nptr;
	nptr = tentry.nextOwned;
	bcopy(&tentry,&bentry,sizeof(tentry));
    }
    return PRNOENT;
}

long AddToOrphan(at,gid)
struct ubik_trans *at;
long gid;
{
    /* add gid to orphan list, as it's owner has died */
    register long code;
    long loc;
    struct prentry tentry;

    loc = FindByID(at,gid);
    if (!loc) return PRNOENT;
    code = pr_ReadEntry(at,0,loc,&tentry);
    if (code != 0) return PRDBFAIL;
    tentry.nextOwned = ntohl(cheader.orphan);
    cheader.orphan = htonl(loc);
    code = pr_WriteEntry(at,0,loc,&tentry);
    if (code != 0) return PRDBFAIL;
    code = pr_Write(at,0,32,(char *)&cheader.orphan,sizeof(cheader.orphan));
    if (code != 0) return PRDBFAIL;
    return PRSUCCESS;
}

long RemoveFromOrphan(at,gid)
struct ubik_trans *at;
long gid;
{
    /* remove gid from the orphan list */
    register long code;
    long loc;
    long nptr;
    struct prentry tentry;
    struct prentry bentry;

    loc = FindByID(at,gid);
    if (!loc) return PRNOENT;
    code = pr_ReadEntry(at,0,loc,&tentry);
    if (code != 0) return PRDBFAIL;
    if (cheader.orphan == htonl(loc)) {
	cheader.orphan = htonl(tentry.nextOwned);
	tentry.nextOwned = 0;
	code = pr_Write(at,0,32,(char *)&cheader.orphan,sizeof(cheader.orphan));
	if (code != 0) return PRDBFAIL;
	code = pr_WriteEntry(at,0,loc,&tentry);
	if (code != 0) return PRDBFAIL;
	return PRSUCCESS;
    }
    nptr = ntohl(cheader.orphan);
    bzero(&bentry,sizeof(bentry));
    loc = 0;
    while (nptr != NULL) {
	code = pr_ReadEntry(at,0,nptr,&tentry);
	if (code != 0) return PRDBFAIL;
	if (gid == tentry.id) {
	    /* found it */
	    bentry.nextOwned = tentry.nextOwned;
	    tentry.nextOwned = 0;
	    code = pr_WriteEntry(at,0,loc,&bentry);
	    if (code != 0) return PRDBFAIL;
	    code = pr_WriteEntry(at,0,nptr,&tentry);
	    if (code != 0) return PRDBFAIL;
	    return PRSUCCESS;
	}
	loc = nptr;
	nptr = tentry.nextOwned;
	bcopy(&tentry,&bentry,sizeof(tentry));
    }
    return PRNOENT;
}

long IsOwnerOf(at,aid,gid)
struct ubik_trans *at;
long aid;
long gid;
{
    /* returns 1 if aid is the owner of gid, 0 otherwise */
    register long code;
    struct prentry tentry;
    long loc;

    loc = FindByID(at,gid);
    if (!loc) return 0;
    code = pr_ReadEntry(at,0,loc,&tentry);
    if (code != 0) return 0;
    if (tentry.owner == aid) return 1;
    return 0;
}

long OwnerOf(at,gid)
struct ubik_trans *at;
long gid;
{
    /* returns the owner of gid */
    register long code;
    long loc;
    struct prentry tentry;

    loc = FindByID(at,gid);
    if (!loc) return 0;
    code = pr_ReadEntry(at,0,loc,&tentry);
    if (code != 0) return 0;
    return tentry.owner;
}
    

long WhoIsThis(acall, at, aid)
struct rx_call *acall;
struct ubik_trans *at;
long *aid;
{
    *aid = SYSADMINID;
    return 0;
}

long IsAMemberOf(at,aid,gid)
struct ubik_trans *at;
long aid;
long gid;
{
    /* returns true if aid is a member of gid */
    struct prentry tentry;
    struct contentry centry;
    register long code;
    long i;
    long loc;

    loc = FindByID(at,gid);
    if (!loc) return 0;
    bzero(&tentry,sizeof(tentry));
    code = pr_ReadEntry(at, 0, loc,&tentry);
    if (code) return 0;
    if (!(tentry.flags & PRGRP)) return 0;
    for (i= 0;i<PRSIZE;i++) {
	if (tentry.entries[i] == aid) return 1;
	if (tentry.entries[i] == 0) return 0;
    }
    if (tentry.next) {
	loc = tentry.next;
	while (loc) {
	    bzero(&centry,sizeof(centry));
	    code = pr_ReadCoEntry(at,0,loc,&centry);
	    if (code) return 0;
	    for (i=0;i<COSIZE;i++) {
		if (centry.entries[i] == aid) return 1;
		if (centry.entries[i] == 0) return 0;
	    }
	    loc = centry.next;
	}
    }
    return 0;  /* actually, should never get here */
}
