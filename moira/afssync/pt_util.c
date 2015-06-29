/*
 *
 * pt_util: Program to dump the AFS protection server database
 *         into an ascii file.
 *
 *	Assumptions: We *cheat* here and read the datafile directly, ie.
 *	             not going through the ubik distributed data manager.
 *		     therefore the database must be quiescent for the
 *		     output of this program to be valid.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/file.h>
#include <stdlib.h>
#include <errno.h>

#include <afs/param.h>
#include <lock.h>
#include <netinet/in.h>
#define UBIK_INTERNALS
#include <ubik.h>
#include <rx/xdr.h>
#include <rx/rx.h>
#include "ptint.h"
#include "ptserver.h"
#include "pterror.h"
#include "vis.h"

#define IDHash(x) (abs(x) % HASHSIZE)

int display_entry(int offset);
int print_id(int id);
void add_group(long id);
void display_groups(void);
void display_group(int id);
void fix_pre(struct prentry *pre);
char *checkin(struct prentry *pre);
char *check_core(register int id);
char *id_to_name(int id);

struct hash_entry {
    char h_name[PR_MAXNAMELEN];
    int h_id;
    struct hash_entry *next;
};
struct hash_entry *hat[HASHSIZE];

static struct contentry prco;
static struct prentry pre;
static struct prheader prh;
static struct ubik_version uv;

struct grp_list {
    struct grp_list	*next;
    long		groups[1024];
};
static struct grp_list *grp_head=0;
static long grp_count=0;

struct usr_list {
    struct usr_list *next;
    char name[PR_MAXNAMELEN];
    long uid;
};
static struct usr_list *usr_head=0;

char buffer[1024];
int dbase_fd;
FILE *dfp;

#define FMT_BASE "%-10s %d/%d %d %d %d\n"
#define FMT_MEM  "   %-8s %d\n"

#define DO_USR 1
#define DO_GRP 2
#define DO_MEM 4
#define DO_SYS 8
#define DO_OTR 16

int nflag = 0;
int wflag = 0;
int flags = 0;

int main(int argc, char **argv)
{
    register int i;
    register long code;
    long cc, upos, gpos;
    struct prentry uentry, gentry;
    struct ubik_hdr *uh;
    char *dfile = 0;
    char *pfile = "/usr/afs/db/prdb.DB0";
    char *str;
    
    while ((cc = getopt(argc, argv, "wugmxsnp:d:")) != EOF) {
	switch (cc) {
	case 'p':
	    pfile = optarg;
	    break;
	case 'd':
	    dfile = optarg;
	    break;
	case 'n':
	    nflag++;
	    break;
	case 'w':
	    wflag++;
	    break;
	case 'u':
	    flags |= DO_USR;
	    break;
	case 'm':
	    flags |= (DO_GRP|DO_MEM);
	    break;
	case 'g':
	    flags |= DO_GRP;
	    break;
	case 's':
	    flags |= DO_SYS;
	    break;
	case 'x':
	    flags |= DO_OTR;
	    break;
	default:
	    fprintf(stderr,
		    "Usage: pt_util [options] [-d data] [-p prdb]\n");
	    fputs("  Options:\n", stderr);
	    fputs("    -w  Update prdb with contents of data file\n", stderr);
	    fputs("    -u  Display users\n", stderr);
	    fputs("    -g  Display groups\n", stderr);
	    fputs("    -m  Display group members\n", stderr);
	    fputs("    -n  Follow name hash chains (not id hashes)\n", stderr);
	    fputs("    -s  Display only system (Moira) data\n", stderr);
	    fputs("    -x  Display extra users/groups\n", stderr);
	    exit(1);
	}
    }
    if ((dbase_fd = open(pfile, wflag ? O_RDWR : O_RDONLY, 0600)) < 0) {
	fprintf(stderr, "pt_util: cannot open %s: %s\n",
		pfile, strerror(errno));
	exit (1);
    }
    if (read(dbase_fd, buffer, HDRSIZE) < 0) {
	fprintf(stderr, "pt_util: error reading %s: %s\n",
		pfile, strerror(errno));
	exit (1);
    }

    if (dfile) {
	if ((dfp = fopen(dfile, wflag ? "r" : "w")) == 0) {
	    fprintf(stderr, "pt_util: error opening %s: %s\n",
		    dfile, strerror(errno));
	    exit(1);
	}
    } else
	dfp = (wflag ? stdin : stdout);

    uh = (struct ubik_hdr *)buffer;
    if (ntohl(uh->magic) != UBIK_MAGIC)
	fprintf(stderr, "pt_util: %s: Bad UBIK_MAGIC. Is %x should be %x\n",
		pfile, ntohl(uh->magic), UBIK_MAGIC);
    memcpy(&uv, &uh->version, sizeof(struct ubik_version));
    if (wflag && uv.epoch==0 && uv.counter==0) {
	uv.epoch=2; /* a ubik version of 0 or 1 has special meaning */
	memcpy(&uh->version, &uv, sizeof(struct ubik_version));
	lseek(dbase_fd, 0, SEEK_SET);
	if (write(dbase_fd, buffer, HDRSIZE) < 0) {
	    fprintf(stderr, "pt_util: error writing ubik version to %s: %s\n",
		    pfile, strerror(errno));
	    exit (1);
	}
    }
    fprintf(stderr, "Ubik Version is: %d.%d\n",
	    uv.epoch, uv.counter);
    if (read(dbase_fd, &prh, sizeof(struct prheader)) < 0) {
	fprintf(stderr, "pt_util: error reading %s: %s\n",
		pfile, strerror(errno));
	exit (1);
    }

    Initdb();
    initialize_pt_error_table();

    if (wflag) {
	struct usr_list *u;

	while(fgets(buffer, sizeof(buffer), dfp)) {
	    int id, oid, cid, flags, quota, uid;
	    char name[PR_MAXNAMELEN], mem[PR_MAXNAMELEN];

	    if (isspace(*buffer)) {
		sscanf(buffer, "%s %d", mem, &uid);

		for (u=usr_head; u; u=u->next)
		    if (u->uid && u->uid==uid) break;
		if (u) {
		    /* Add user - deferred because it is probably foreign */
		    u->uid = 0;
		    if (FindByID(0, uid))
			code = PRIDEXIST;
		    else {
			if (!code && (flags&(PRGRP|PRQUOTA))==(PRGRP|PRQUOTA)){
			    gentry.ngroups++;
			    code = pr_WriteEntry(0,0,gpos,&gentry);
			    if (code)
				fprintf(stderr, "Error setting group count on %s: %s\n",
					name, error_message(code));
			}
			code = CreateEntry
			    (0, u->name, &uid, 1/*idflag*/, 1/*gflag*/,
			     SYSADMINID/*oid*/, SYSADMINID/*cid*/);
		    }
		    if (code)
			fprintf(stderr, "Error while creating %s: %s\n",
				u->name, error_message(code));
		    continue;
		}
		/* Add user to group */
		if (id==ANYUSERID || id==AUTHUSERID || uid==ANONYMOUSID) {
		    code = PRPERM;
		} else if ((upos=FindByID(0,uid)) && (gpos=FindByID(0,id))) {
		    code = pr_ReadEntry(0,0,upos,&uentry);
		    if (!code) code = pr_ReadEntry(0,0,gpos,&gentry);
		    if (!code) code = AddToEntry (0, &gentry, gpos, uid);
		    if (!code) code = AddToEntry (0, &uentry, upos, id);
		} else
		    code = PRNOENT;

		if (code)
		    fprintf(stderr, "Error while adding %s to %s: %s\n",
			    mem, name, error_message(code));
	    } else {
		sscanf(buffer, "%s %d/%d %d %d %d",
		       name, &flags, &quota, &id, &oid, &cid);

		str = malloc(strlen(name) + 1);
		if (!str)
		  {
		    fprintf(stderr, "malloc failed!");
		    exit(1);
		  }
		strunvis(str, name);
		strncpy(name, str, PR_MAXNAMELEN);
		name[PR_MAXNAMELEN] = '\0';
		if (FindByID(0, id))
		    code = PRIDEXIST;
		else
		    code = CreateEntry(0, name, &id, 1/*idflag*/,
				       flags&PRGRP, oid, cid);
		if (code == PRBADNAM) {
		    u = (struct usr_list *)malloc(sizeof(struct usr_list));
		    u->next = usr_head;
		    u->uid = id;
		    strcpy(u->name, name);
		    usr_head = u;
		} else
		if (code) {
		    fprintf(stderr, "Error while creating %s: %s\n",
			    name, error_message(code));
		} else if ((flags&PRACCESS) ||
			   (flags&(PRGRP|PRQUOTA))==(PRGRP|PRQUOTA)) {
		    gpos = FindByID(0, id);
		    code = pr_ReadEntry(0,0,gpos,&gentry);
		    if (!code) {
			gentry.flags = flags;
			gentry.ngroups = quota;
			code = pr_WriteEntry(0,0,gpos,&gentry);
		    }
		    if (code)
			fprintf(stderr,"Error while setting flags on %s: %s\n",
				name, error_message(code));
		}
	    }
	}
	for (u=usr_head; u; u=u->next)
	    if (u->uid)
		fprintf(stderr, "Error while creating %s: %s\n",
			u->name, error_message(PRBADNAM));
    } else {
	for (i = 0; i < HASHSIZE; i++) {
	    upos = nflag ? ntohl(prh.nameHash[i]) : ntohl(prh.idHash[i]);
	    while (upos)
		upos = display_entry(upos);
	}
	if (flags & DO_GRP)
	    display_groups();
    }

    lseek (dbase_fd, 0, L_SET);		/* rewind to beginning of file */
    if (read(dbase_fd, buffer, HDRSIZE) < 0) {
	fprintf(stderr, "pt_util: error reading %s: %s\n",
		pfile, strerror(errno));
	exit (1);
    }
    uh = (struct ubik_hdr *)buffer;
    if ((uh->version.epoch != uv.epoch) ||
	(uh->version.counter != uv.counter)) {
	fprintf(stderr, "pt_util: Ubik Version number changed during execution.\n");
	fprintf(stderr, "Old Version = %d.%d, new version = %d.%d\n",
		uv.epoch, uv.counter, uh->version.epoch,
		uh->version.counter);
    }
    close (dbase_fd);
    exit (0);
}

int display_entry(int offset)
{
    register int i;

    lseek (dbase_fd, offset+HDRSIZE, L_SET);
    read(dbase_fd, &pre, sizeof(struct prentry));

    fix_pre(&pre);

    if ((pre.flags & PRFREE) == 0) {
	if (pre.flags & PRGRP) {
	    if (flags & DO_GRP)
		add_group(pre.id);
	} else {
	    if (print_id(pre.id) && (flags&DO_USR))
		fprintf(dfp, FMT_BASE,
			pre.name, pre.flags, pre.ngroups,
			pre.id, pre.owner, pre.creator);
	    checkin(&pre);
	}
    }
    return(nflag ? pre.nextName: pre.nextID);
}

void add_group(long id)
{
    struct grp_list *g;
    register long i;

    i = grp_count++ % 1024;
    if (i == 0) {
	g = (struct grp_list *)malloc(sizeof(struct grp_list));
	g->next = grp_head;
	grp_head = g;
    }
    g = grp_head;
    g->groups[i] = id;
}

void display_groups(void)
{
    register int i, id;
    struct grp_list *g;

    g = grp_head;
    while (grp_count--) {
	i = grp_count%1024;
	id = g->groups[i];
	display_group(id);
	if (i==0) {
	    grp_head = g->next;
	    free(g);
	    g = grp_head;
	}
    }
}

void display_group(int id)
{
    register int i, offset;
    int print_grp = 0;

    offset = ntohl(prh.idHash[IDHash(id)]);
    while (offset) {
	lseek(dbase_fd, offset+HDRSIZE, L_SET);
	if (read(dbase_fd, &pre, sizeof(struct prentry)) < 0) {
	    fprintf(stderr, "pt_util: read i/o error: %s\n",
		    strerror(errno));
	    exit (1);
	}
	fix_pre(&pre);
	if (pre.id == id)
	    break;
	offset = pre.nextID;
    }

    if (print_id(id)) {
	fprintf(dfp, FMT_BASE,
		pre.name, pre.flags, pre.ngroups,
		pre.id, pre.owner, pre.creator);
	print_grp = 1;
    }

    if ((flags&DO_MEM) == 0)
	return;

    for (i=0; i<PRSIZE; i++) {
	if ((id=pre.entries[i]) == 0)
	    break;
	if (id==PRBADID) continue;
	if (print_id(id) || print_grp==1) {
	    if (print_grp==0) {
		fprintf(dfp, FMT_BASE,
			pre.name, pre.flags, pre.ngroups,
			pre.id, pre.owner, pre.creator);
		print_grp = 2;
	    }
	    fprintf(dfp, FMT_MEM, id_to_name(id), id);
	}
    }
    if (i == PRSIZE) {
	offset = pre.next;
	while (offset) {
	    lseek(dbase_fd, offset+HDRSIZE, L_SET);
	    read(dbase_fd, &prco, sizeof(struct contentry));
	    prco.next = ntohl(prco.next);
	    for (i = 0; i < COSIZE; i++) {
		prco.entries[i] = ntohl(prco.entries[i]);
		if ((id=prco.entries[i]) == 0)
		    break;
		if (id==PRBADID) continue;
		if (print_id(id) || print_grp==1) {
		    if (print_grp==0) {
			fprintf(dfp, FMT_BASE,
				pre.name, pre.flags, pre.ngroups,
				pre.id, pre.owner, pre.creator);
			print_grp = 2;
		    }
		    fprintf(dfp, FMT_MEM, id_to_name(id), id);
		}
	    }
	    if ((i == COSIZE) && prco.next)
		offset = prco.next;
	    else offset = 0;
	}
    }
}

void fix_pre(struct prentry *pre)
{
    register int i;
    char *str = malloc(4 * strlen(pre->name) + 1);

    if (!str)
      {
	fprintf(stderr, "malloc failed in fix_pre()!");
	exit(1);
      }
    strvis(str, pre->name, VIS_WHITE);
    if (strlen(str) > PR_MAXNAMELEN) 
      {
	fprintf(stderr, "encoding greater than PR_MAXNAMELEN!\n");
	fprintf(stderr, "groupname %s will not be encoded!\n", pre->name);
      }
    else 
      {
	strncpy(pre->name, str, PR_MAXNAMELEN);
	pre->name[PR_MAXNAMELEN] = '\0';
      }
    pre->flags = ntohl(pre->flags);
    pre->id = ntohl(pre->id);
    pre->cellid = ntohl(pre->cellid);
    pre->next = ntohl(pre->next);
    pre->nextID = ntohl(pre->nextID);
    pre->nextName = ntohl(pre->nextName);
    pre->owner = ntohl(pre->owner);
    pre->creator = ntohl(pre->creator);
    pre->ngroups = ntohl(pre->ngroups);
    pre->nusers = ntohl(pre->nusers);
    pre->count = ntohl(pre->count);
    pre->instance = ntohl(pre->instance);
    pre->owned = ntohl(pre->owned);
    pre->nextOwned = ntohl(pre->nextOwned);
    pre->parent = ntohl(pre->parent);
    pre->sibling = ntohl(pre->sibling);
    pre->child = ntohl(pre->child);
    for (i = 0; i < PRSIZE; i++) {
	pre->entries[i] = ntohl(pre->entries[i]);
    }
}

char *id_to_name(int id)
{
    register int offset;
    static struct prentry pre;
    char *name;

    name = check_core(id);
    if (name) return(name);
    offset = ntohl(prh.idHash[IDHash(id)]);
    while (offset) {
	lseek(dbase_fd, offset+HDRSIZE, L_SET);
	if (read(dbase_fd, &pre, sizeof(struct prentry)) < 0) {
	    fprintf(stderr, "pt_util: read i/o error: %s\n",
		    strerror(errno));
	    exit (1);
	}
	pre.id = ntohl(pre.id);
	if (pre.id == id) {
	    name = checkin(&pre);
	    return(name);
	}
	offset = ntohl(pre.nextID);
    }
    return 0;
}

char *checkin(struct prentry *pre)
{
    struct hash_entry *he, *last;
    register int id;

    id = pre->id;
    last = (struct hash_entry *)0;
    he = hat[IDHash(id)];
    while (he) {
	if (id == he->h_id) return(he->h_name);
	last = he;
	he = he->next;
    }
    he = (struct hash_entry *)malloc(sizeof(struct hash_entry));
    if (he == 0) {
	fprintf(stderr, "pt_util: No Memory for internal hash table.\n");
	exit (1);
    }
    he->h_id = id;
    he->next = (struct hash_entry *)0;
    strncpy(he->h_name, pre->name, PR_MAXNAMELEN);
    if (last == (struct hash_entry *)0) hat[IDHash(id)] = he;
    else last->next = he;
    return(he->h_name);
}

char *check_core(register int id)
{
    struct hash_entry *he;
    he = hat[IDHash(id)];
    while (he) {
	if (id == he->h_id) return(he->h_name);
	he = he->next;
    }
    return 0;
}

/* returns 1 if the id value should be printered, otherwise 0 */
int print_id(int id)
{
  /* process system (Moira) users */
  if (flags & DO_SYS)
    {
      /* hard-coded list of IDs in the system id space that aren't actually
       * system users.  Skip them here.  Mmm, legacies.
       */
      switch (id)
	{
	case -65541:
	case -65542:
	case -65544:
	case -65546:
	case -65548:
	case -65549:
	case -65551:
	case -65557:
	case -65563:
	case -65574:
	case -65576:
	case -65578:
	case -65579:
	case -65582:
	case -97536:
	case -97537:
	case -97538:
	case -98766:
	case -98775:
	case -98781:
	case -98782:
	case -98783:
	case -98784:
	case -98785:
	case -98786:
	case -98787:
	case -98788:
	case -98789:
	case -98792:
	case -98813:
	case -98815:
	case -98816:
	case -98818:
	case -98819:
	case -98820:
	case -98821:
	case -98828:
	case -98829:
	case -98830:
	case -98835:
	case -98836:
	case -98837:
	case -98841:
	case -98842:
	case -98844:
	case -98845:
	case -98846:
	case -98847:
	case -98848:
	case -98849:
	case -98851:
	case -98854:
	case -98855:
	case -98856:
	case -98857:
	case -98859:
	case -98867:
	case -98868:
	case -98869:
	case -98870:
	case -98871:
	case -98873:
	case -98878:
	case -98879:
	case -98883:
	case -98884:
	case -98885:
	case -98888:
	case -98891:
	case -98898:
	case -98903:
	case -98905:
	case -98914:
	case -98918:
	case -98919:
	case -98920:
	case -98923:
	case -98927:
	case -98941:
	case -98942:
	case -98945:
	case -98948:
	case -98949:
	case -98951:
	case -98952:
	case -98953:
	case -98956:
	case -98957:
	case -98960:
	case -98961:
	case -98963:
	case -98966:
	case -98994:
	case -98996:
	case -98998:
	case -99000:
	case -99001:
	case -99002:
	case -99003:
	case -99004:
	case -99005:
	case -99006:
	case -99007:
	case -99008:
	case -99009:
	case -99010:
	case -99011:
	case -99012:
	case -99013:
	case -99014:
	case -99015:
	case -99016:
	case -99017:
	case -99018:
	case -99019:
	case -99023:
	case -99029:
	case -99030:
	case -99042:
	case -99048:
	case -99056:
	case -99057:
	case -99058:
	case -99059:
	case -99063:
	case -99064:
	case -99076:
	case -99079:
	case -99090:
	case -99091:
	case -99105:
	case -99106:
	case -99113:
	case -99114:
	case -99115:
	case -99116:
	case -99118:
	case -99120:
	case -99121:
	case -99129:
	case -99130:
	case -99131:
	case -99133:
	case -99146:
	case -99150:
	case -99153:
	case -99154:
	case -99193:
	case -99194:
	case -99200:
	case -99205:
	case -99211:
	case -99214:
	case -99218:
	case -99220:
	case -99223:
	case -99224:
	case -99225:
	case -99233:
	case -99234:
	case -99236:
	case -99237:
	case -99242:
	case -99245:
	case -99250:
	case -99252:
	case -99253:
	case -99254:
	case -99255:
	case -99259:
	case -99260:
	case -99263:
	case -99264:
	case -99267:
	case -99284:
	case -99287:
	case -99289:
	case -99295:
	case -99297:
	case -99307:
	case -99308:
	case -99309:
	case -99310:
	case -99311:
	case -99315:
	case -99317:
	case -99327:
	case -99334:
	case -99344:
	case -99348:
	case -99349:
	case -99354:
	case -99366:
	case -99367:
	case -99370:
	case -99371:
	case -99372:
	case -99373:
	case -99377:
	case -99378:
	case -99379:
	case 65613:
	case 65678:
	case 65770:
	case 65896:
	case 65971:
	case 65987:
	case 66011:
	case 66114:
	case 66142:
	case 67145:
	case 67299:
	case 67393:
	case 67669:
	case 67712:
	case 67903:
	case 68276:
	case 68421:
	case 68467:
	case 69005:
	case 69143:
	case 69234:
	case 69611:
	case 70141:
	case 70481:
	case 71468:
	case 71559:
	case 71689:
	case 72203:
	case 72775:
	case 72799:
	case 73319:
	case 73348:
	case 73578:
	case 73642:
	case 73656:
	case 74424:
	case 74610:
	case 75067:
	case 75169:
	case 75197:
	case 75332:
	case 75717:
	case 76261:
	case 76322:
	case 76341:
	case 76529:
	case 76546:
	case 76747:
	case 76804:
	case 77353:
	case 77409:
	case 77800:
	case 78720:
	case 80148:
	case 80176:
	case 80422:
	case 80533:
	case 80856:
	case 81342:
	case 82058:
	case 82279:
	case 82304:
	case 82605:
	case 82611:
	case 84255:
	case 84476:
	case 85005:
	case 85309:
	case 85426:
	case 85530:
	case 87306:
	case 88377:
	case 89960:
	case 90782:
	case 92388:
	case 92990:
	case 94724:
	case 95044:
	case 95057:
	case 95969:
	case 96863:
	case 97354:
	case 97538:
	case 97540:
	case 97542:
	case 97544:
	case 97545:
	case 97546:
	case 97547:
	case 97556:
	case 97559:
	case 97560:
	case 97570:
	case 99070:
	case 99071:
	case 99072:
	case 101061:
	case 101502:
	case 102042:
	case 103500:
	case 106026:
	case 119873:
	case 127811:
	case 128401:
	case 128906:
	case 130756:
	case 130781:
	case 132835:
	case 137739:
	case 139937:
	case 140868:
	case 141877:
	case 147840:
	case 156318:
	case 164606:
	case 164607:
	case 164608:
	case 196291:
	case 196317:
	case 203275:
	case 230142:
	case 230143:
	case 261827:
	case 261853:
	  return 0;
	  break;
	}
      if (id > -262145 && id < 262145)
	return 1;
      else
	return 0;
    }
  /* process non-system (not in Moira) users */
  else if (flags & DO_OTR)
    {
      /* hard-coded list of IDs in the system id space that aren't actually
       * system users.  Print them here.  Mmm, legacies.
       */
      switch (id)
	{
	case -65541:
	case -65542:
	case -65544:
	case -65546:
	case -65548:
	case -65549:
	case -65551:
	case -65557:
	case -65563:
	case -65574:
	case -65576:
	case -65578:
	case -65579:
	case -65582:
	case -97536:
	case -97537:
	case -97538:
	case -98766:
	case -98775:
	case -98781:
	case -98782:
	case -98783:
	case -98784:
	case -98785:
	case -98786:
	case -98787:
	case -98788:
	case -98789:
	case -98792:
	case -98813:
	case -98815:
	case -98816:
	case -98818:
	case -98819:
	case -98820:
	case -98821:
	case -98828:
	case -98829:
	case -98830:
	case -98835:
	case -98836:
	case -98837:
	case -98841:
	case -98842:
	case -98844:
	case -98845:
	case -98846:
	case -98847:
	case -98848:
	case -98849:
	case -98851:
	case -98854:
	case -98855:
	case -98856:
	case -98857:
	case -98859:
	case -98867:
	case -98868:
	case -98869:
	case -98870:
	case -98871:
	case -98873:
	case -98878:
	case -98879:
	case -98883:
	case -98884:
	case -98885:
	case -98888:
	case -98891:
	case -98898:
	case -98903:
	case -98905:
	case -98914:
	case -98918:
	case -98919:
	case -98920:
	case -98923:
	case -98927:
	case -98941:
	case -98942:
	case -98945:
	case -98948:
	case -98949:
	case -98951:
	case -98952:
	case -98953:
	case -98956:
	case -98957:
	case -98960:
	case -98961:
	case -98963:
	case -98966:
	case -98994:
	case -98996:
	case -98998:
	case -99000:
	case -99001:
	case -99002:
	case -99003:
	case -99004:
	case -99005:
	case -99006:
	case -99007:
	case -99008:
	case -99009:
	case -99010:
	case -99011:
	case -99012:
	case -99013:
	case -99014:
	case -99015:
	case -99016:
	case -99017:
	case -99018:
	case -99019:
	case -99023:
	case -99029:
	case -99030:
	case -99042:
	case -99048:
	case -99056:
	case -99057:
	case -99058:
	case -99059:
	case -99063:
	case -99064:
	case -99076:
	case -99079:
	case -99090:
	case -99091:
	case -99105:
	case -99106:
	case -99113:
	case -99114:
	case -99115:
	case -99116:
	case -99118:
	case -99120:
	case -99121:
	case -99129:
	case -99130:
	case -99131:
	case -99133:
	case -99146:
	case -99150:
	case -99153:
	case -99154:
	case -99193:
	case -99194:
	case -99200:
	case -99205:
	case -99211:
	case -99214:
	case -99218:
	case -99220:
	case -99223:
	case -99224:
	case -99225:
	case -99233:
	case -99234:
	case -99236:
	case -99237:
	case -99242:
	case -99245:
	case -99250:
	case -99252:
	case -99253:
	case -99254:
	case -99255:
	case -99259:
	case -99260:
	case -99263:
	case -99264:
	case -99267:
	case -99284:
	case -99287:
	case -99289:
	case -99295:
	case -99297:
	case -99307:
	case -99308:
	case -99309:
	case -99310:
	case -99311:
	case -99315:
	case -99317:
	case -99327:
	case -99334:
	case -99344:
	case -99348:
	case -99349:
	case -99354:
	case -99366:
	case -99367:
	case -99370:
	case -99371:
	case -99372:
	case -99373:
	case -99377:
	case -99378:
	case -99379:
	case 65613:
	case 65678:
	case 65770:
	case 65896:
	case 65971:
	case 65987:
	case 66011:
	case 66114:
	case 66142:
	case 67145:
	case 67299:
	case 67393:
	case 67669:
	case 67712:
	case 67903:
	case 68276:
	case 68421:
	case 68467:
	case 69005:
	case 69143:
	case 69234:
	case 69611:
	case 70141:
	case 70481:
	case 71468:
	case 71559:
	case 71689:
	case 72203:
	case 72775:
	case 72799:
	case 73319:
	case 73348:
	case 73578:
	case 73642:
	case 73656:
	case 74424:
	case 74610:
	case 75067:
	case 75169:
	case 75197:
	case 75332:
	case 75717:
	case 76261:
	case 76322:
	case 76341:
	case 76529:
	case 76546:
	case 76747:
	case 76804:
	case 77353:
	case 77409:
	case 77800:
	case 78720:
	case 80148:
	case 80176:
	case 80422:
	case 80533:
	case 80856:
	case 81342:
	case 82058:
	case 82279:
	case 82304:
	case 82605:
	case 82611:
	case 84255:
	case 84476:
	case 85005:
	case 85309:
	case 85426:
	case 85530:
	case 87306:
	case 88377:
	case 89960:
	case 90782:
	case 92388:
	case 92990:
	case 94724:
	case 95044:
	case 95057:
	case 95969:
	case 96863:
	case 97354:
	case 97538:
	case 97540:
	case 97542:
	case 97544:
	case 97545:
	case 97546:
	case 97547:
	case 97556:
	case 97559:
	case 97560:
	case 97570:
	case 99070:
	case 99071:
	case 99072:
	case 101061:
	case 101502:
	case 102042:
	case 103500:
	case 106026:
	case 119873:
	case 127811:
	case 128401:
	case 128906:
	case 130756:
	case 130781:
	case 132835:
	case 137739:
	case 139937:
	case 140868:
	case 141877:
	case 147840:
	case 156318:
	case 164606:
	case 164607:
	case 164608:
	case 196291:
	case 196317:
	case 203275:
	case 230142:
	case 230143:
	case 261827:
	case 261853:
	  return 1;
	  break;
	}
      if (id < -262144 || id > 262144)
	return 1;
      else
	return 0;
    }
  /* neither flag set, don't do anything */
  else
    return 0;
}
