/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/nfs.qc,v 1.11 1989-08-08 13:07:23 qjb Exp $
 *
 * This generates the files necessary to load an nfs server.
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <stdio.h>
#include <sms.h>
#include <sms_app.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#undef NFS_DIR
#define NFS_DIR	"/u1/sms/dcm/nfs"

#define min(x,y)	((x) < (y) ? (x) : (y))

char *whoami = "nfs.gen";
char *malloc(), *strsave();
char *ingres_date_and_time(), *ingres_time(), *ingres_date();

main(argc, argv)
int argc;
char **argv;
{
    char cmd[64];
    struct stat sb;
    int changed = 0;
    int ingerr();

    if (argc > 2) {
	fprintf(stderr, "usage: %s [outfile]\n", argv[0]);
	exit(SMS_ARGS);
    }

    IIseterr(ingerr);
##  ingres sms
##  set lockmode session where level = table

    changed = do_nfs();

##  exit

    if (!changed) {
	fprintf(stderr, "No files updated.\n");
	if (argc == 2 && stat(argv[1], &sb) == 0)
	  exit(SMS_NO_CHANGE);
    }

    if (argc == 2) {
	sprintf(cmd, "cd %s; cp /u1/sms/nfs/* .; tar cf %s .", NFS_DIR, argv[1]);
	if (system(cmd))
	  exit(SMS_TAR_FAIL);
    }

    exit(SMS_SUCCESS);
}


/*
 * ingerr: (supposedly) called when Ingres indicates an error.
 * I have not yet been able to get this to work to intercept a
 * database open error.
 */
#define INGRES_DEADLOCK 4700

static int ingerr(num)
    int *num;
{
    char buf[256];
    int ingres_errno;

    switch (*num) {
    case INGRES_DEADLOCK:
	ingres_errno = SMS_DEADLOCK;
	break;
    default:
	ingres_errno = SMS_INGRES_ERR;
    }
    com_err(whoami, SMS_INGRES_ERR, " code %d\n", *num);
    critical_alert("DCM", "NFS build encountered INGRES ERROR %d", *num);
    exit(ingres_errno);
}


/* Generate the files.  Returns zero if nothing changed, non-zero otherwise. */

int do_nfs()
##{
##  char machname[33], listname[33];
    struct save_queue *machs, *lists;
    int changed;

    machs = sq_create();
    lists = sq_create();
##  range of s is serverhosts
##  retrieve (machname = trim(machine.name), listname = trim(s.value3))
##	where machine.mach_id = s.mach_id and s.enable != 0 {
      sq_save_unique_string(machs, strsave(machname));
      sq_save_unique_string(lists, strsave(listname));
##  }

    changed = do_lists(lists);
    changed += do_machs(machs);
    return(changed);
##}


/* Make all of the credentials lists that will be needed.  Returns 0 if
 * no files were actually changed */

int do_lists(lists)
struct save_queue *lists;
##{
    struct save_queue *sq, *members;
    char file[64], *u;
    struct hash *users, *do_everyone();
    struct stat sb;
    FILE *fd;
##  char *listname, *lsname, lname[33], uname[9], *filetime;
##  int uid, id, flag1, flag2, flag3, flag4;

    sprintf(file, "%s/list-", NFS_DIR);
    if (stat(file, &sb) == 0) {
	filetime = ingres_date_and_time(sb.st_mtime);
##	retrieve (flag1 = int4(interval("min", tblstats.modtime - filetime)))
##		where tblstats.table = "users"
##	retrieve (flag2 = int4(interval("min", tblstats.modtime - filetime)))
##		where tblstats.table = "list"
##	retrieve (flag3 = int4(interval("min", tblstats.modtime - filetime)))
##		where tblstats.table = "members"
##	retrieve (flag4 = int4(interval("min", tblstats.modtime - filetime)))
##		where tblstats.table = "serverhosts"
	if (flag1 < 0 && flag2 < 0 && flag3 < 0 && flag4 < 0) {
	    fprintf(stderr, "The lists do not need to be rebuilt.\n");
	    return(0);
	}
    }

##  begin transaction
    /* get locks */
##  retrieve (lname = list.modtime) where list.list_id = 0
##  retrieve (lname = users.modtime) where users.users_id = 0

    /* build the list of everyone, and store it in a file whose name
     * corresponds to the empty list.
     */
    users = do_everyone();

    fprintf(stderr, "Building specific lists\n");
    /* now do each of the lists used by an NFS server */
##  range of l is list
##  range of l1 is list
##  range of m is members
##  range of u is users
    while (sq_get_data(lists, &listname)) {
	if (strlen(listname) == 0)
	  continue;
	sprintf(file, "%s/list-%s", NFS_DIR, listname);
	fd = fopen(file, "w");
	if (!fd) {
	    fprintf(stderr, "cannot open %s for output\n", file);
	    exit(SMS_OCONFIG);
	}
	sq = sq_create();
	sq_save_data(sq, listname);
	members = sq_create();
	while (sq_get_data(sq, &lsname)) {
##	    repeat retrieve (lname = trim(l.name))
##		where m.list_id = l1.list_id and l1.name = @lsname and
##		      m.member_type = "LIST" and m.member_id = l.list_id {
	      sq_save_unique_string(sq, strsave(lname));
##	    }
##	    repeat retrieve (id = m.member_id)
##		where m.list_id = l1.list_id and l1.name = @lsname and
##		      m.member_type = "USER" {
	      if (u = hash_lookup(users, id)) {
		  sq_save_unique_data(members, u);
	      }
##	    }
	}
	while (sq_get_data(members, &lsname))
	  fprintf(fd, "%s\n", lsname);
/* test to see if this speeds things up; don't free anything!
	sq_destroy(members);
	sq_destroy(sq);
 */	if (fclose(fd)) {
	    fprintf(stderr, "error closing %s\n", file);
	    exit(SMS_CCONFIG);
	}
    }
/* don't free here either
    sq_destroy(lists);
 */
##  end transaction
    return(1);
##}


/*  Build the list of everybody. */
struct grp {
    struct grp *next;
    int id;
};
struct user {
    char name[9];
    int uid;
    struct grp *lists;
};

struct hash *do_everyone()
##{
    char buf[BUFSIZ], *l;
    struct hash *groups, *users;
    struct user *u;
    struct grp *g;
    struct bucket *b, **p;
##  char name[33];
##  int gid, id, lid, maxid, uid;
    FILE *fd;
    int i;
    struct save_queue *sq;

    fprintf(stderr, "Building the list of everybody\n");
    sprintf(buf, "%s/list-", NFS_DIR);
    fd = fopen(buf, "w");
    if (!fd) {
	fprintf(stderr, "cannot open %s for output\n", buf);
	exit(SMS_OCONFIG);
    }

    /* make space for group list */
    groups = create_hash(15000);

    /* retrieve simple groups */
##  range of l is list
##  retrieve (gid = l.#gid, lid = l.list_id)
##	where l.group != 0 and l.active != 0 {
      sprintf(buf, ":%d", gid);
      hash_store(groups, lid, strsave(buf));
##  }

    /* get special cases: lists that aren't groups themselves but are
     * members of groups. */
    sq = sq_create();
##  range of m is members
##  retrieve (gid = list.#gid, lid = l.list_id)
##	where l.active != 0 and list.active != 0 and m.member_type = "LIST" and
##		m.member_id = l.list_id and m.list_id = list.list_id and
##		list.group != 0 {
      if (l = hash_lookup(groups, lid)) {
	  sprintf(buf, "%s:%d", l, gid);
	  free(l);
      } else {
	  sprintf(buf, ":%d", gid);
      }
      hash_store(groups, lid, strsave(buf));
      sq_save_data(sq, lid);
##  }

    while (sq_get_data(sq, &id)) {
##	repeat retrieve (lid = l.list_id) where l.active != 0 and
##		m.member_type = "LIST" and m.member_id = l.list_id and
##		m.list_id = @id {
	  if (l = hash_lookup(groups, lid)) {
	      sprintf(buf, "%s%s", l, hash_lookup(groups, id));
	      free(l);
	      l = strsave(buf);
	  } else {
	      l = hash_lookup(groups, id);
	  }
	  hash_store(groups, lid, l);
	  sq_save_unique_data(sq, lid);
##      }
    }
    sq_destroy(sq);

    /* now do grplists */
    users = create_hash(10000);
##  range of u is users
##  retrieve (id = u.users_id, name = u.login, uid = u.#uid)
##	 where u.status = 1 {
      u = (struct user *) malloc(sizeof(struct user));
      strcpy(u->name, strtrim(name));
      u->uid = uid;
      u->lists = NULL;
      hash_store(users, id, u);
##  }

##  repeat retrieve (lid = m.list_id, id = m.member_id)
##	where m.member_type = "USER" {
      if ((u = (struct user *) hash_lookup(users, id)) &&
	  (hash_lookup(groups, lid) != NULL)) {
	  g = (struct grp *) malloc(sizeof(struct grp));
	  g->next = u->lists;
	  u->lists = g;
	  g->id = lid;
      }
##  }

    for (p = &(users->data[users->size - 1]); p >= users->data; p--) {
	for (b = *p; b; b = b->next) {
	    i = 0;
	    u = (struct user *)b->data;
	    sprintf(buf, "%s:%d", u->name, u->uid);
	    for (g = u->lists; g; g = g->next)
	      if (l = hash_lookup(groups, g->id)) {
		  i++;
		  strcat(buf, l);
	      }
	    /* should free stuff here... */
	    if (i > 1) {
		remove_duplicate_groups(buf);
	    }
	    b->data = strsave(buf);
	    fprintf(fd, "%s\n", buf);
	}
    }

    fclose(fd);
    free(groups);
    return(users);
##}

#define MAXGROUPS 256

remove_duplicate_groups(p)
register char *p;
{
    register char *pp;
    char *index();
    register int *mp;
    int memory[MAXGROUPS], i, *mg = &memory[MAXGROUPS-1];

    bzero(memory, MAXGROUPS * sizeof(int));
    p = index(p, ':');
    p++;
    while (p = index(p, ':')) {
	i = atoi(++p);
	pp = p;
	for (mp = memory; *mp && mp < mg; mp++)
	  if (*mp == i) {
	      if (p = index(p, ':'))
		strcpy(pp, ++p);
	      else {
		  *(--pp) = 0;
	      }
	      p = pp - 2;
	      break;
	  }
	*mp = i;
    }
}


/* Now do each of the servers, linking the credentials list file and 
 * compiling the quota and dirs files.
 */

int do_machs(machs)
struct save_queue *machs;
##{
##  char *machname, listname[33], dev[33], *device, dir[64], fstype[9];
##  char *filetime;
##  int uid, quota, id, gid, flag1, flag2, flag3, flag4;
    char file[64], f1[64], f2[64], *cp, *index();
    int prevuid, quotasum;
    FILE *fd;
    struct stat sb;
    struct save_queue *sq;

/*
    sprintf(file, "%s/list-", NFS_DIR);
    if (stat(file, &sb) == 0) {
	filetime = ingres_date_and_time(sb.st_mtime);
 #	retrieve (flag1 = int4(interval("min", tblstats.modtime - filetime)))
 #		where tblstats.table = "serverhosts"
 #	retrieve (flag2 = int4(interval("min", tblstats.modtime - filetime)))
 #		where tblstats.table = "filesys"
 #	retrieve (flag3 = int4(interval("min", tblstats.modtime - filetime)))
 #		where tblstats.table = "nfsquota"
 #	retrieve (flag4 = int4(interval("min", tblstats.modtime - filetime)))
 #		where tblstats.table = "nfsphys"
	if (flag1 < 0 && flag2 < 0 && flag3 < 0 && flag4 < 0) {
	    fprintf(stderr, "The machine files do not need to be rebuilt.\n");
	    return(0);
	}
    }
*/

    fprintf(stderr, "Building machine files\n");

##  range of s is serverhosts
##  range of m is machine
##  range of n is nfsphys
##  range of q is nfsquota
##  range of f is filesys
##  range of u is users
##  range of l is list

/* acquire locks on users, lists, machines & filesystems */
##  begin transaction
##  retrieve (listname = u.modtime) where u.users_id = 0
##  retrieve (listname = l.modtime) where l.list_id = 0
##  retrieve (listname = m.modtime) where m.mach_id = 0
##  retrieve (listname = f.modtime) where f.filsys_id = 0

    while (sq_get_data(machs, &machname)) {
##	repeat retrieve (listname = trim(s.value3))
##	    where s.mach_id = m.mach_id and m.name = @machname
	strtrim(machname);
	sprintf(f1, "%s/list-%s", NFS_DIR, listname);
	sprintf(f2, "%s/%s.cred", NFS_DIR, machname);
	unlink(f2); /* ignore errors on this unlink */
	if (link(f1, f2)) {
	    fprintf(stderr, "Cannot link %s to %s\n", f1, f2);
	    exit(SMS_OCONFIG);
	}
	sq = sq_create();
##	repeat retrieve (dev = trim(n.#device))
##	    where n.mach_id = m.mach_id and m.name = @machname {
	  sq_save_data(sq, strsave(dev));
##	}
	while (sq_get_data(sq, &device)) {
##	    repeat retrieve (id = n.nfsphys_id) where n.mach_id = m.mach_id and
##		m.#name = @machname and n.#device = @device
	    while (cp = index(device, '/')) *cp = '@';
	    sprintf(file, "%s/%s.%s.quotas", NFS_DIR, machname, device);
	    fd = fopen(file, "w");
	    if (!fd) {
		fprintf(stderr, "cannot open %s for output\n", file);
		exit(SMS_OCONFIG);
	    }
	    prevuid = -1;
	    quotasum = 0;
##	    repeat retrieve (uid = u.#uid, quota = q.#quota)
##		where q.users_id = u.users_id and q.filsys_id = f.filsys_id and
##		      f.phys_id = @id and u.status != 0 sort by #uid {
    		if (uid != prevuid) {
		    if (quotasum)
		      fprintf(fd, "%d %d\n", prevuid, quotasum);
		    prevuid = uid;
		    quotasum = quota;
		} else {
		    quotasum += quota;
		}
##	    }
	    if (quotasum)
	      fprintf(fd, "%d %d\n", prevuid, quotasum);
	    if (fclose(fd)) {
		fprintf(stderr, "error closing %s", file);
		exit(SMS_CCONFIG);
	    }
	    sprintf(file, "%s/%s.%s.dirs", NFS_DIR, machname, device);
	    fd = fopen(file, "w");
	    if (!fd) {
		fprintf(stderr, "cannot open %s for output\n", file);
		exit(SMS_OCONFIG);
	    }
##	    repeat retrieve (dir = trim(f.#name), fstype = trim(f.lockertype),
##			     uid = u.#uid, gid = l.#gid)
##		where f.phys_id = @id and f.owner = u.users_id and 
##		      f.owners = l.list_id and f.createflg != 0 {
	      fprintf(fd, "%s %d %d %s\n", dir, uid, gid, fstype);
##	    }
	    if (fclose(fd)) {
		fprintf(stderr, "error closing %s", file);
		exit(SMS_CCONFIG);
	    }
	}
	sq_destroy(sq);
    }
##  end transaction
    return(1);
##}
