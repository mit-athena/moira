/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/nfs.qc,v 1.1 1988-06-27 13:13:24 mar Exp $
 *
 * This generates the files necessary to load an nfs server.
 */

#include <stdio.h>
#include <sms.h>
#include <sms_app.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#define NFS_DIR	"/u1/sms/dcm/nfs"

#define min(x,y)	((x) < (y) ? (x) : (y))

char *malloc(), *strsave();
char *ingres_date_and_time(), *ingres_time(), *ingres_date();

main(argc, argv)
int argc;
char **argv;
{
    char cmd[64];
    struct stat sb;

    if (argc > 2) {
	fprintf(stderr, "usage: %s [outfile]\n", argv[0]);
	exit(SMS_ARGS);
    }

##  ingres sms
##  set lockmode session where readlock = nolock

    do_nfs();

##  exit

    if (argc == 2) {
	sprintf(cmd, "cd %s; tar cf %s .", NFS_DIR, argv[1]);
	if (system(cmd))
	  exit(SMS_TAR_FAIL);
    }

    exit(SMS_SUCCESS);
}


do_nfs()
##{
##  char machname[33], listname[33];
    struct save_queue *machs, *lists;

    machs = sq_create();
    lists = sq_create();
##  range of s is serverhosts
##  retrieve (machname = trim(machine.name), listname = trim(s.value3))
##	where machine.mach_id = s.mach_id and s.enable != 0 {
      sq_save_unique_string(machs, strsave(machname));
      sq_save_unique_string(lists, strsave(listname));
##  }

    do_lists(lists);
    do_machs(machs);
##}


do_lists(lists)
struct save_queue *lists;
##{
    struct save_queue *sq;
    char file[64], **users, **do_everyone();
    FILE *fd;
##  char *listname, *lsname, lname[33], uname[9];
##  int uid, id;

    users = do_everyone();

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
	while (sq_get_data(sq, &lsname)) {
##	    repeat retrieve (lname = trim(l.name))
##		where m.list_id = l1.list_id and l1.name = @lsname and
##		      m.member_type = "LIST" and m.member_id = l.list_id {
	      sq_save_unique_string(sq, lname);
##	    }
##	    repeat retrieve (id = m.member_id)
##		where m.list_id = l1.list_id and l1.name = @lsname and
##		      m.member_type = "USER" {
	      if (users[id])
		fprintf(fd, "%s\n", users[id]);
##	    }
	}
	sq_destroy(sq);
	if (fclose(fd)) {
	    fprintf(stderr, "error closing %s\n", file);
	    exit(SMS_CCONFIG);
	}
    }
    sq_destroy(lists);
##}


char **do_everyone()
##{
    char **groups, **users, buf[BUFSIZ];
##  char name[33];
##  int gid, id, lid, maxid, uid;
    FILE *fd;
    int first;
    struct save_queue *sq;

    sprintf(buf, "%s/list-", NFS_DIR);
    fd = fopen(buf, "w");
    if (!fd) {
	fprintf(stderr, "cannot open %s for output\n", buf);
	exit(SMS_OCONFIG);
    }

    /* make space for group list */
##  range of l is list
##  retrieve (maxid = max(l.#list_id))
    groups = (char **)malloc((maxid + 1) * sizeof(char *));
    if (groups == NULL) {
	fprintf(stderr, "unable to malloc space for groups\n");
	exit(SMS_NO_MEM);
    }
    bzero(groups, (maxid + 1) * sizeof(char *));

    /* retrieve simple groups */
##  retrieve (gid = l.#gid, lid = l.list_id)
##	where l.group != 0 {
	    sprintf(buf, ":%d", gid);
	    groups[lid] = strsave(buf);
##  }

    /* get special cases: lists that aren't groups themselves but are
     * members of groups. */
    sq = sq_create();
##  range of m is members
##  retrieve (gid = list.#gid, lid = l.list_id)
##	where l.group = 0 and m.member_type = "LIST" and
##		m.member_id = l.list_id and m.list_id = list.list_id and
##		list.group != 0 {
    	if (groups[lid]) {
	    sprintf(buf, "%s:%d", groups[lid], gid);
	    free(groups[lid]);
	    groups[lid] = strsave(buf);
	} else {
	    sprintf(buf, ":%d", gid);
	    groups[lid] = strsave(buf);
	}
    	sq_save_data(sq, lid);
##  }
    while (sq_get_data(sq, &id)) {
##	repeat retrieve (lid = l.list_id)
##	    where l.group = 0 and m.member_type = "LIST" and
##		m.member_id = l.list_id and m.list_id = @id {
	    if (groups[lid]) {
		sprintf(buf, "%s%s", groups[lid], groups[id]);
		groups[lid] = strsave(buf);
	    } else {
		groups[lid] = strsave(groups[id]);
	    }
	    sq_save_unique_data(sq, lid);
##	}
    }
    sq_destroy(sq);

##  range of u is users
##  retrieve (maxid = max(u.#users_id))
    users = (char **)malloc((maxid + 1) * sizeof(char *));
    if (users == NULL) {
	fprintf(stderr, "unable to malloc space for groups\n");
	exit(SMS_NO_MEM);
    }
    bzero(users, (maxid + 1) * sizeof(char *));

    /* now do grplists */
    sq = sq_create();
##  retrieve (id = u.users_id) where u.status != 0 {
	sq_save_data(sq, id);
##  }
    while (sq_get_data(sq, &id)) {
##	repeat retrieve (name = u.login, uid = u.#uid) where u.users_id = @id
	trim(name);
	sprintf(buf, "%s:%d", name, uid);
##	repeat retrieve (lid = m.list_id)
##		where m.member_type = "USER" and m.member_id = @id {
	    if (groups[lid])
	      strcat(buf, groups[lid]);
##	}
	users[id] = strsave(buf);
	fprintf(fd, "%s\n", buf);
    }
    fclose(fd);
    sq_destroy(sq);
    for (id = 0; id < maxid; id++)
      if (groups[id])
	free(groups[id]);
    free(groups);
    return(users);
##}


do_machs(machs)
struct save_queue *machs;
##{
##  char *machname, listname[33], dev[33], *device, dir[64];
##  int uid, quota, id, gid;
    char file[64], f1[64], f2[64], *cp, *index();
    int prevuid, quotasum;
    FILE *fd;
    struct save_queue *sq;

##  range of s is serverhosts
##  range of m is machine
##  range of n is nfsphys
##  range of q is nfsquota
##  range of f is filesys
##  range of u is users
##  range of l is list
    while (sq_get_data(machs, &machname)) {
##	repeat retrieve (listname = s.value3)
##	    where s.mach_id = m.mach_id and m.name = @machname
	trim(listname);
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
##	    repeat retrieve (dir = trim(f.#name), uid = u.#uid, gid = l.#gid)
##		where f.phys_id = @id and f.owner = u.users_id and 
##		      f.owners = l.list_id and f.createflg != 0 {
	      fprintf(fd, "%s %d %d\n", dir, uid, gid);
##	    }
	    if (fclose(fd)) {
		fprintf(stderr, "error closing %s", file);
		exit(SMS_CCONFIG);
	    }
	}
	sq_destroy(sq);
    }
##}
