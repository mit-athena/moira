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
#include <strings.h>
#include <sys/file.h>
#include <lock.h>
#include <netinet/in.h>
#define UBIK_INTERNALS
#include <ubik.h>
#include <rx/xdr.h>
#include <rx/rx.h>
#include <afs/ptint.h>
#include <afs/ptserver.h>
extern char *optarg;
extern int optind;
extern int errno;
extern char *sys_errlist[];
struct prheader prh;
struct prentry pre;
struct ubik_version uv;
char buffer[1024];
int grpflag, nmflg, cflag;

main(argc, argv)
int argc;
char **argv;
{
    int cc;
    int fd, offset;
    register int i;
    char file[512];
    register struct ubik_hdr *uh;
    int didit;
    strcpy(file, "/usr/afs/db/prdb.DB0");
    while ((cc = getopt(argc, argv, "f:gnc")) != EOF) {
	switch (cc) {
	case 'f':
	    strncpy (file, optarg, sizeof(file));
	    break;
	case 'g':
	    grpflag++;
	    break;
	case 'c':
	    cflag++;
	    break;
	case 'n':
	    nmflg++;		/* Use the Name hash chain */
	    break;
	default:
	    fprintf(stderr, "ptdump: -%c: unknown option\n", cc);
	    break;
	}
    }
    if (cflag && !grpflag) {
	fprintf(stderr, "ptdump: -c requires -g flag.\n");
	exit (1);
    }
    if ((fd = open(file, O_RDONLY, 0600)) < 0) {
	fprintf(stderr, "ptdump: cannot open %s: %s\n",
		file, sys_errlist[errno]);
	exit (1);
    }
    if (read(fd, buffer, HDRSIZE) < 0) {
	fprintf(stderr, "ptdump: error reading %s: %s\n",
		file, sys_errlist[errno]);
	exit (1);
    }
    uh = (struct ubik_hdr *)buffer;
    if (ntohl(uh->magic) != UBIK_MAGIC)
	fprintf(stderr, "ptdump: %s: Bad UBIK_MAGIC. Is %x should be %x\n",
		file, ntohl(uh->magic), UBIK_MAGIC);
    bcopy(&uh->version, &uv, sizeof(struct ubik_version));
    fprintf(stderr, "Ubik Version is: %D.%d\n",
	    uv.epoch, uv.counter);
    if (read(fd, &prh, sizeof(struct prheader)) < 0) {
	fprintf(stderr, "ptdump: error reading %s: %s\n",
		file, sys_errlist[errno]);
	exit (1);
    }
    for (i = 0; i < HASHSIZE; i++) {
	offset = nmflg ? ntohl(prh.nameHash[i]) : ntohl(prh.idHash[i]);
	didit = 0;
	while (offset)
	    offset = display_entry(offset, fd, &didit);
    }
    lseek (fd, 0, L_SET);		/* rewind to beginning of file */
    if (read(fd, buffer, HDRSIZE) < 0) {
	fprintf(stderr, "ptdump: error reading %s: %s\n",
		file, sys_errlist[errno]);
	exit (1);
    }
    uh = (struct ubik_hdr *)buffer;
    if ((uh->version.epoch != uv.epoch) ||
	(uh->version.counter != uv.counter)) {
	fprintf(stderr, "ptdump: Ubik Version number changed during execution.\n");
	fprintf(stderr, "Old Version = %D.%d, new version = %D.%d\n",
		uv.epoch, uv.counter, uh->version.epoch,
		uh->version.counter);
    }
    close (fd);
    exit (0);
}
int display_entry (offset, fd, didit)
int offset, fd;
int *didit;
{
    void display_useful_groups();
    char *checkin();
    register int i;
    lseek (fd, offset+HDRSIZE, L_SET);
    read(fd, &pre, sizeof(struct prentry));
    pre.flags = ntohl(pre.flags);
    pre.id = ntohl(pre.id);
    pre.cellid = ntohl(pre.cellid);
    pre.next = ntohl(pre.next);
    pre.nextID = ntohl(pre.nextID);
    pre.nextName = ntohl(pre.nextName);
    pre.owner = ntohl(pre.owner);
    pre.creator = ntohl(pre.creator);
    pre.ngroups = ntohl(pre.ngroups);
    pre.nusers = ntohl(pre.nusers);
    pre.count = ntohl(pre.count);
    pre.instance = ntohl(pre.instance);
    pre.owned = ntohl(pre.owned);
    pre.nextOwned = ntohl(pre.nextOwned);
    pre.parent = ntohl(pre.parent);
    pre.sibling = ntohl(pre.sibling);
    pre.child = ntohl(pre.child);
    for (i = 0; i < PRSIZE; i++) {
	pre.entries[i] = ntohl(pre.entries[i]);
    }
    if ((pre.flags & PRFREE) == 0) { /* A normal user */
	if (cflag) (void) checkin(&pre);
	if (((pre.flags & PRGRP) && grpflag) ||
	    (((pre.flags & PRGRP) == 0) && !grpflag)) {
	    if (!*didit && !cflag) {
		*didit = 1;
		printf("==========\n");
	    }
	    if (!cflag)
		printf("Name: %s ID: %D\n", pre.name, pre.id);
	    else display_useful_groups(&pre, fd);
	}
    }
    return(nmflg ? pre.nextName : pre.nextID);
}
static struct contentry prco;
void display_useful_groups(pre, fd)
register struct prentry *pre;
int fd;
{
    register int i;
    register int offset;
    char *id_to_name();
    if (pre->entries[0] == 0) return;
    printf("Group: %s\n", pre->name);
    for (i = 0; i < PRSIZE; i++) {
	if (pre->entries[i] == 0) break;
	printf("   Member:  %s\n", id_to_name(pre->entries[i], fd));
    }
    if (i == PRSIZE) {
	offset = pre->next;
	while (offset) {
	    lseek(fd, offset+HDRSIZE, L_SET);
	    read(fd, &prco, sizeof(struct contentry));
	    prco.next = ntohl(prco.next);
	    for (i = 0; i < COSIZE; i++) {
		prco.entries[i] = ntohl(prco.entries[i]);
		if (prco.entries[i] == 0) break;
		printf("   Member(co):  %s\n", id_to_name(prco.entries[i], fd));
	    }
	    if ((i == COSIZE) && prco.next)
		offset = prco.next;
	    else offset = 0;
	}
    }
}
char *id_to_name(id, fd)
int id;
int fd;
{
    register int offset;
    struct prentry pre;
    char *name;
    char *check_core();
    char *checkin();
    long IDHash();
    name = check_core(id);
    if (name != NULL) return(name);
    offset = ntohl(prh.idHash[IDHash(id)]);
    if (offset == NULL) return("NOT FOUND");
    while (offset) {
	lseek(fd, offset+HDRSIZE, L_SET);
	if (read(fd, &pre, sizeof(struct prentry)) < 0) {
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
    return("NOT FOUND");
}
struct hash_entry {
    char h_name[PR_MAXNAMELEN];
    int h_id;
    struct hash_entry *next;
};
struct hash_entry *hat[HASHSIZE];
char *checkin(pre)
struct prentry *pre;
{
    struct hash_entry *he, *last;
    register int id;
    long IDHash();
    id = pre->id;
    last = (struct hash_entry *)0;
    he = hat[IDHash(id)];
    while (he) {
	if (id == he->h_id) return(he->h_name);
	last = he;
	he = he->next;
    }
    he = (struct hash_entry *)malloc(sizeof(struct hash_entry));
    if (he == NULL) {
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
    long IDHash();
    he = hat[IDHash(id)];
    while (he) {
	if (id == he->h_id) return(he->h_name);
	he = he->next;
    }
    return(NULL);
}

long IDHash(x)
long x;
{
    /* returns hash bucket for x */
    return ((abs(x)) % HASHSIZE);
}
