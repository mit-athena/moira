/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/nfs.qc,v 1.17 1990-03-08 23:22:32 mar Exp $
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


#define min(x,y)	((x) < (y) ? (x) : (y))

char *whoami = "nfs.gen";
char *malloc(), *strsave();
char *ingres_date_and_time(), *ingres_time(), *ingres_date();
char nfs_dir[64];

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
    initialize_sms_error_table();
    sprintf(nfs_dir, "%s/nfs", DCM_DIR);

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
	sprintf(cmd, "cd %s; cp %s/nfs/* .; tar cf %s .",
		nfs_dir, SMS_DIR, argv[1]);
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
##  retrieve (machname = machine.name, listname = s.value3)
##	where machine.mach_id = s.mach_id and s.service = "NFS"
##	and s.enable != 0 {
      sq_save_unique_string(machs, strsave(strtrim(machname)));
      sq_save_unique_string(lists, strsave(strtrim(listname)));
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
    char file[64], *u;
    struct hash *users, *do_everyone();
    struct stat sb;
    FILE *fd;
##  char *listname, *lsname, lname[33], uname[9], *filetime;
##  int uid, id, flag1, flag2, flag3, flag4;

    sprintf(file, "%s/list-", nfs_dir);
    if (stat(file, &sb) == 0) {
	filetime = ingres_date_and_time(sb.st_mtime);
##	retrieve (flag1 = int4(interval("min", tblstats.modtime - filetime)))
##		where tblstats.table = "users"
##	retrieve (flag2 = int4(interval("min", tblstats.modtime - filetime)))
##		where tblstats.table = "list"
##	retrieve (flag3 = int4(interval("min", tblstats.modtime - filetime)))
##		where tblstats.table = "imembers"
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
##  range of m is imembers
##  range of u is users
    while (sq_get_data(lists, &listname)) {
	if (strlen(listname) == 0)
	  continue;
	sprintf(file, "%s/list-%s", nfs_dir, listname);
	fd = fopen(file, "w");
	if (!fd) {
	    fprintf(stderr, "cannot open %s for output\n", file);
	    exit(SMS_OCONFIG);
	}
##	repeat retrieve (id = m.member_id)
##	    where m.list_id = l1.list_id and l1.name = @listname and
##	    m.member_type = "USER" {
	    if (u = hash_lookup(users, id))
	      fprintf(fd, "%s\n", u);
##	}
	if (fclose(fd)) {
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
    char *lid;
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
    sprintf(buf, "%s/list-", nfs_dir);
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

##  range of m is imembers
##  retrieve (lid = m.list_id, id = m.member_id) where m.member_type = "USER" {
      if ((u = (struct user *) hash_lookup(users, id)) &&
	  ((l = hash_lookup(groups, lid)) != NULL)) {
	  g = (struct grp *) malloc(sizeof(struct grp));
	  g->next = u->lists;
	  u->lists = g;
	  g->lid = l;
      }
##  }

    for (p = &(users->data[users->size - 1]); p >= users->data; p--) {
	for (b = *p; b; b = b->next) {
	    u = (struct user *)b->data;
	    sprintf(buf, "%s:%d", u->name, u->uid);
	    for (g = u->lists; g; g = g->next)
	      strcat(buf, g->lid);
	    b->data = strsave(buf);
	    fprintf(fd, "%s\n", buf);
	}
    }

    fclose(fd);
    free(groups);
    return(users);
##}


/* Now do each of the servers, linking the credentials list file and 
 * compiling the quota and dirs files.
 */

int do_machs(machs)
struct save_queue *machs;
##{
##  char *machname, listname[33], dev[33], dir[81], fstype[9];
##  int uid, quota, id, gid, flag1, flag2, flag3, flag4;
    char file[64], f1[64], f2[64], *cp, *index();
    int prevuid, quotasum, olddev, oldmach;
    FILE *fd;
    struct hash *machines;

    fprintf(stderr, "Building machine files\n");

##  range of s is serverhosts
##  range of m is machine
##  range of n is nfsphys
##  range of q is nfsquota
##  range of f is filesys
##  range of u is users
##  range of l is list

/* acquire locks on machines & filesystems */
##  begin transaction
##  retrieve (listname = u.modtime) where u.users_id = 0
##  retrieve (listname = m.modtime) where m.mach_id = 0
##  retrieve (listname = f.modtime) where f.filsys_id = 0

    machines = create_hash(100);
    while (sq_get_data(machs, &machname)) {
##	repeat retrieve (listname = s.value3, id = m.mach_id)
##	    where s.mach_id = m.mach_id and m.name = @machname
	strtrim(machname);
	sprintf(f1, "%s/list-%s", nfs_dir, strtrim(listname));
	sprintf(f2, "%s/%s.cred", nfs_dir, machname);
	unlink(f2); /* ignore errors on this unlink */
	if (link(f1, f2)) {
	    fprintf(stderr, "Cannot link %s to %s\n", f1, f2);
	    exit(SMS_OCONFIG);
	}
	hash_store(machines, id, machname);
    }

    olddev = oldmach = -1;
    fd = stdin;
##  retrieve (quota = q.#quota, uid = u.#uid, flag1 = q.phys_id,
##	      dev = n.device, flag2 = n.mach_id)
##	where u.users_id = q.users_id and n.nfsphys_id = q.phys_id and
##	      q.phys_id != 0
##	sort by #flag2, #flag1, #uid {
      if (flag1 != olddev || flag2 != oldmach) {
	  fclose(fd);
	  olddev = flag1;
	  oldmach = flag2;
	  while (cp = index(dev, '/')) *cp = '@';
	  sprintf(file, "%s/%s.%s.quotas", nfs_dir,
		  hash_lookup(machines, flag2), strtrim(dev));
	  fd = fopen(file, "w");
	  if (!fd) {
	      fprintf(stderr, "cannot open %s for output\n", file);
	      exit(SMS_OCONFIG);
	  }
	  prevuid = -1;
	  quotasum = 0;
      }
      if (uid != prevuid) {
	  if (quotasum)
	    fprintf(fd, "%d %d\n", prevuid, quotasum);
	  prevuid = uid;
	  quotasum = quota;
      } else {
	  quotasum += quota;
      }
##  }
    if (quotasum)
      fprintf(fd, "%d %d\n", prevuid, quotasum);
    if (fclose(fd)) {
	fprintf(stderr, "error closing %s", file);
	exit(SMS_CCONFIG);
    }

    olddev = oldmach = -1;
##  retrieve (dir = f.#name, fstype = f.lockertype, uid = u.#uid,
##	      gid = l.#gid, flag1 = f.phys_id, flag2 = f.mach_id,
##	      dev = n.device)
##	where u.users_id = f.owner and l.list_id = f.owners and
##	      f.createflg != 0 and f.phys_id != 0 and f.type = "NFS" and
##	      f.phys_id = n.nfsphys_id
##	sort by #flag2, #flag1 {
      if (flag1 != olddev || flag2 != oldmach) {
	  fclose(fd);
	  olddev = flag1;
	  oldmach = flag2;
	  while (cp = index(dev, '/')) *cp = '@';
	  sprintf(file, "%s/%s.%s.dirs", nfs_dir,
		  hash_lookup(machines, flag2), strtrim(dev));
	  fd = fopen(file, "w");
	  if (!fd) {
	      fprintf(stderr, "cannot open %s for output\n", file);
	      exit(SMS_OCONFIG);
	  }
      }
      fprintf(fd, "%s %d %d %s\n", strtrim(dir), uid, gid, strtrim(fstype));
##  }
    if (fclose(fd)) {
	fprintf(stderr, "error closing %s", file);
	exit(SMS_CCONFIG);
    }
##  end transaction
    return(1);
##}
