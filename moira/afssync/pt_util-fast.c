/*
 *
 * ptdump: Program to dump the AFS protection server database
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

#define IDHash(x) (abs(x) % HASHSIZE)
#define print_id(x) ( ((flags&DO_SYS)==0 && (x<-32767 || x>97536)) || \
		      ((flags&DO_OTR)==0 && (x>-32768 && x<97537)))

extern char *optarg;
extern int optind;
extern int errno;
extern char *sys_errlist[];
#define strerror(a) sys_errlist[a]

int display_entry();
void add_group();
void display_groups();
void display_group();
void fix_pre();
char *checkin();
char *check_core();
char *id_to_name();

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

main(argc, argv)
int argc;
char **argv;
{
    register int i;
    register long code;
    long cc, upos, gpos;
    struct prentry uentry, gentry;
    struct ubik_hdr *uh;
    char *dfile = 0;
    char *pfile = "/usr/afs/db/prdb.DB0";
    int need_gwrite = 0;
    
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
		    "Usage: ptdump [options] [-d data] [-p prdb]\n");
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
	fprintf(stderr, "ptdump: cannot open %s: %s\n",
		pfile, sys_errlist[errno]);
	exit (1);
    }
    if (read(dbase_fd, buffer, HDRSIZE) < 0) {
	fprintf(stderr, "ptdump: error reading %s: %s\n",
		pfile, sys_errlist[errno]);
	exit (1);
    }

    if (dfile) {
	if ((dfp = fopen(dfile, wflag ? "r" : "w")) == 0) {
	    fprintf(stderr, "ptdump: error opening %s: %s\n",
		    dfile, sys_errlist[errno]);
	    exit(1);
	}
    } else
	dfp = (wflag ? stdin : stdout);

    uh = (struct ubik_hdr *)buffer;
    if (ntohl(uh->magic) != UBIK_MAGIC)
	fprintf(stderr, "ptdump: %s: Bad UBIK_MAGIC. Is %x should be %x\n",
		pfile, ntohl(uh->magic), UBIK_MAGIC);
    memcpy(&uv, &uh->version, sizeof(struct ubik_version));
    fprintf(stderr, "Ubik Version is: %d.%d\n",
	    ntohl(uv.epoch), ntohl(uv.counter));
    if (read(dbase_fd, &prh, sizeof(struct prheader)) < 0) {
	fprintf(stderr, "ptdump: error reading %s: %s\n",
		pfile, sys_errlist[errno]);
	exit (1);
    }

    Initdb();
    initialize_pt_error_table();

#define FLUSH_GROUP \
    if (need_gwrite) { \
	code = pr_WriteEntry(0,0,gpos,&gentry); \
	if (code) \
	    fprintf(stderr, "Error while writing group %s: %s\n", \
		    gentry.name, error_message(code)); \
	else need_write = 0; \
    }

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
		    if (upos = FindByID(0, uid))
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
		}
		if (!code && !upos) {
		    upos = FindByID(0,uid);
		    if (upos) code = pr_ReadEntry(0,0,upos,&uentry);
		}
		if (!upos || !gpos)
		    code = PRNOENT;

		/* Fast group update */
		if (!code && (gentry.count < PRSIZE)) {
		    gentry.count++;
		    for (i=0; i<PRSIZE; i++) {
			if (gentry.entries[i] == PRBADID) {
			    gentry.entries[i] = uid;
			    break;
			}
		    }
		} else {
		    FLUSH_GROUP;
		    if (!code) code = AddToEntry (0, &gentry, gpos, uid);
		}
		
		if (!code) code = AddToEntry (0, &uentry, upos, id);

		if (code)
		    fprintf(stderr, "Error while adding %s to %s: %s\n",
			    mem, name, error_message(code));
	    } else {
		FLUSH_GROUP;
		sscanf(buffer, "%s %d/%d %d %d %d",
		       name, &flags, &quota, &id, &oid, &cid);

		if (gpos = FindByID(0, id))
		    code = PRIDEXIST;
		else {
		    code = CreateEntry(0, name, &id, 1/*idflag*/,
				       flags&PRGRP, oid, cid);
		    if (!code) gpos = FindByID(0, id);
		}
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
		}
		code = pr_ReadEntry(0,0,gpos,&gentry);
		if (code)
		    fprintf(stderr,"Error while reading group %s: %s\n",
			    name, error_message(code));
		else if ((flags&PRACCESS) ||
			 (flags&(PRGRP|PRQUOTA))==(PRGRP|PRQUOTA)) {
		    gentry.flags = flags;
		    gentry.ngroups = quota;
		    need_gwrite = 1;
		    if (code)
			fprintf(stderr,
				"Error while setting flags on %s: %s\n",
				name, error_message(code));
		}
	    }
	}
	FLUSH_GROUP;
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
	fprintf(stderr, "ptdump: error reading %s: %s\n",
		pfile, sys_errlist[errno]);
	exit (1);
    }
    uh = (struct ubik_hdr *)buffer;
    if ((uh->version.epoch != uv.epoch) ||
	(uh->version.counter != uv.counter)) {
	fprintf(stderr, "ptdump: Ubik Version number changed during execution.\n");
	fprintf(stderr, "Old Version = %d.%d, new version = %d.%d\n",
		ntohl(uv.epoch), ntohl(uv.counter),
		ntohl(uh->version.epoch), ntohl(uh->version.counter));
    }
    close (dbase_fd);
    exit (0);
}

int display_entry (offset)
int offset;
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

void add_group(id)
    long id;
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

void display_groups()
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

void display_group(id)
    int id;
{
    register int i, offset;
    int print_grp = 0;

    offset = ntohl(prh.idHash[IDHash(id)]);
    while (offset) {
	lseek(dbase_fd, offset+HDRSIZE, L_SET);
	if (read(dbase_fd, &pre, sizeof(struct prentry)) < 0) {
	    fprintf(stderr, "ptdump: read i/o error: %s\n",
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

void fix_pre(pre)
    struct prentry *pre;
{
    register int i;
    
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

char *id_to_name(id)
int id;
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
	    fprintf(stderr, "ptdump: read i/o error: %s\n",
		    sys_errlist[errno]);
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

char *checkin(pre)
struct prentry *pre;
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
	fprintf(stderr, "ptdump: No Memory for internal hash table.\n");
	exit (1);
    }
    he->h_id = id;
    he->next = (struct hash_entry *)0;
    strncpy(he->h_name, pre->name, PR_MAXNAMELEN);
    if (last == (struct hash_entry *)0) hat[IDHash(id)] = he;
    else last->next = he;
    return(he->h_name);
}

char *check_core(id)
register int id;
{
    struct hash_entry *he;
    he = hat[IDHash(id)];
    while (he) {
	if (id == he->h_id) return(he->h_name);
	he = he->next;
    }
    return 0;
}
