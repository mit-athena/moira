/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/hesiod.qc,v 1.20 1989-09-08 15:15:14 mar Exp $
 *
 * This generates the zone files necessary to load a hesiod server.
 * The following zones are generated: passwd, uid, pobox, group,
 * grplist, gid, filsys, cluster, pcap, sloc, service.
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
#include <ctype.h>


char hesiod_dir[64];

#define min(x,y)	((x) < (y) ? (x) : (y))
struct hash *machines = NULL;
struct hash *users = NULL;
char *whoami = "hesiod.gen";

struct grp {
    struct grp *next;
    int id;
};
struct user {
    char name[9];
    struct grp *lists;
};

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
    initialize_sms_error_table ();
    sprintf(hesiod_dir, "%s/hesiod", DCM_DIR);

##  ingres sms
##  set lockmode session where level = table

    changed = do_passwd();
    changed += do_filsys();
    changed += do_cluster();
    changed += do_printcap();
    changed += do_palladium();
    changed += do_sloc();
    changed += do_service();
    changed += do_groups();

##  exit

    if (!changed) {
	fprintf(stderr, "No files updated.\n");
	if (argc == 2 && stat(argv[1], &sb) == 0)
	  exit(SMS_NO_CHANGE);
    }

    if (argc == 2) {
	fprintf(stderr, "Building tar file.\n");
	sprintf(cmd, "cd %s; tar cf %s .", hesiod_dir, argv[1]);
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
    critical_alert("DCM", "Hesiod build encountered INGRES ERROR %d", *num);
    exit(ingres_errno);
}


get_mach()
##{
##  int id;
##  char name[33];

    if (machines)
      return;

    machines = create_hash(1000);
##  retrieve (name = machine.#name, id = machine.mach_id) {
	hash_store(machines, id, strsave(strtrim(name)));
##  }
##}


do_passwd()
##{
    FILE *pout, *uout, *bout;
    char poutf[64], uoutf[64], poutft[64], uoutft[64], boutf[64], boutft[64];
    struct stat psb, usb, bsb;
    time_t ftime;
    struct user *u;
    char *mach;
##  char login[9], shell[33], fullname[33], oa[17], op[13], hp[17], *filetime;
##  char nn[17], ptype[9];
##  int uid, flag1, flag2, id, pid;

    sprintf(poutf, "%s/passwd.db", hesiod_dir);
    sprintf(uoutf, "%s/uid.db", hesiod_dir);
    sprintf(boutf, "%s/pobox.db", hesiod_dir);

    if (stat(poutf, &psb) == 0 && stat(uoutf, &usb) == 0 &&
	stat(boutf, &bsb) == 0) {
	ftime = min(min(psb.st_mtime, usb.st_mtime), bsb.st_mtime);
	filetime = ingres_date_and_time(ftime);
##	retrieve (flag1 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "users"
##	retrieve (flag2 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "machine"
	if (flag1 < 0 && flag2 < 0) {
	    fprintf(stderr, "Files passwd.db, uid.db, and pobox.db do not need to be rebuilt.\n");
	    return(0);
      }
    }

    sprintf(poutft, "%s~", poutf);
    pout = fopen(poutft, "w");
    if (!pout) {
	perror("cannot open passwd.db~ for write");
	exit(SMS_OCONFIG);
    }
    sprintf(uoutft, "%s~", uoutf);
    uout = fopen(uoutft, "w");
    if (!uout) {
	perror("cannot open uid.db~ for write");
	exit(SMS_OCONFIG);
    }
    sprintf(boutft, "%s~", boutf);
    bout = fopen(boutft, "w");
    if (!bout) {
	perror("cannot open pobox.db for write");
	exit(SMS_OCONFIG);
    }

    fprintf(stderr, "Building passwd.db, uid.db, and pobox.db\n");
    get_mach();

    users = create_hash(10000);
##  range of u is users
##  retrieve (login = u.#login, uid = u.#uid, shell = u.#shell,
##	      fullname = u.#fullname, nn = u.nickname, oa = u.office_addr,
##	      op = u.office_phone, hp = u.home_phone, id = u.users_id,
##	      pid = u.pop_id, ptype = u.potype)
##	where u.status = 1 {
	    strtrim(login);
	    dequote(fullname);
	    dequote(nn);
	    dequote(oa);
	    dequote(op);
	    dequote(hp);
	    dequote(shell);
	    u = (struct user *) malloc(sizeof(struct user));
	    strcpy(u->name, login);
	    u->lists = NULL;
	    hash_store(users, id, u);
    	    fprintf(pout, "%s.passwd\tHS UNSPECA \"%s:*:%d:101:%s,%s,%s,%s,%s:/mit/%s:%s\"\n",
		    login, login, uid, fullname, nn, oa, op, hp, login, shell);
	    fprintf(uout, "%d.uid\tHS CNAME %s.passwd\n", uid, login);
	    if (ptype[0] == 'P' && (mach = hash_lookup(machines, pid))) {
		fprintf(bout, "%s.pobox\tHS UNSPECA \"POP %s %s\"\n",
			login, mach, login);
	    }
##  }

    if (fclose(pout) || fclose(uout) || fclose(bout)) {
	fprintf(stderr, "Unsuccessful file close of passwd.db, uid.db, or pobox.db\n");
	exit(SMS_CCONFIG);
    }
    fix_file(poutf);
    fix_file(uoutf);
    fix_file(boutf);
    return(1);
##}


/************************************************************************
 * WARNING: this routine mallocs a large amount of memory which it never frees.
 * This is considered OK only because main has been arranged to call this
 * last, so the process will exit shortly after we return.
 */

do_groups()
##{
    FILE *iout, *gout, *lout;
    char ioutf[64], goutf[64], loutf[64], buf[2048], *l;
    struct hash *groups;
    register struct bucket *b, **p;
    struct grp *g;
    struct user *u;
    struct stat isb, gsb, lsb;
    time_t ftime;
    register struct save_queue *sq;
    struct save_queue *sq_create();
    register int i;
##  char name[33], *filetime;
##  int gid, id, lid, flag1, flag2, flag3, maxid;

    /* open files */
    sprintf(ioutf, "%s/gid.db", hesiod_dir);
    sprintf(goutf, "%s/group.db", hesiod_dir);
    sprintf(loutf, "%s/grplist.db", hesiod_dir);

    if (stat(ioutf, &isb) == 0 && stat(goutf, &gsb) == 0 && stat(loutf, &lsb) == 0) {
	ftime = min(isb.st_mtime, min(gsb.st_mtime, lsb.st_mtime));
	filetime = ingres_date_and_time(ftime);
##	retrieve (flag1 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "users"
##	retrieve (flag2 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "list"
##	retrieve (flag3 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "imembers"
	if (flag1 < 0 && flag2 < 0 && flag3 < 0) {
	    fprintf(stderr, "Files gid.db, group.db and grplist.db do not need to be rebuilt.\n");
	    return(0);
      }
    }

    sprintf(buf, "%s~", ioutf);
    iout = fopen(buf, "w");
    if (!iout) {
	perror("cannot open gid.db for write");
	exit(SMS_OCONFIG);
    }
    sprintf(buf, "%s~", goutf);
    gout = fopen(buf, "w");
    if (!gout) {
	perror("cannot open group.db for write");
	exit(SMS_OCONFIG);
    }
    sprintf(buf, "%s~", loutf);
    lout = fopen(buf, "w");
    if (!lout) {
	perror("cannot open grplist.db for write");
	exit(SMS_OCONFIG);
    }

    fprintf(stderr, "Building gid.db, group.db, and grplist.db\n");

    /* make space for group list */
    groups = create_hash(15000);

    /* retrieve simple groups */
##  range of l is list
##  range of m is imembers
##  begin transaction
    /* get lock records */
##  retrieve (name = l.modtime) where l.list_id = 0
##  retrieve (name = users.modtime) where users.users_id = 0

##  retrieve (name = l.#name, gid = l.#gid, lid = l.list_id)
##	where l.group != 0 and l.active != 0 {
	    strtrim(name);
	    sprintf(buf, "%s:%d", name, gid);
	    hash_store(groups, lid, strsave(buf));
	    fprintf(iout, "%d.gid\tHS CNAME %s.group\n", gid, name);
	    fprintf(gout, "%s.group\tHS UNSPECA \"%s:*:%d:\"\n",
		    name, name, gid);
##  }

    fflush(iout);
    fflush(gout);

    /* now do grplists */
    if (users == NULL) {
	users = create_hash(10000);
##	range of u is users
##	retrieve (id = u.users_id, name = u.login) where u.status = 1 {
	  u = (struct user *) malloc(sizeof(struct user));
	  strcpy(u->name, strtrim(name));
	  u->lists = NULL;
	  hash_store(users, id, u);
##	}
    }

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
##  end transaction

    for (p = &(users->data[users->size - 1]); p >= users->data; p--) {
	for (b = *p; b; b = b->next) {
	    i = 0;
	    sprintf(buf, "%s.grplist\tHS UNSPECA \"",
		    ((struct user *)b->data)->name);
	    for (g = ((struct user *)b->data)->lists; g; g = g->next)
	      if (l = hash_lookup(groups, g->id)) {
		  i++;
		  strcat(buf, l);
		  if (g->next)
		    strcat(buf, ":");
	      }
	    strcat(buf, "\"\n");
	    if (i > 0)
		fputs(buf, lout);
	}
    }

#ifdef notdef
/* This is commented out because it takes on the order of 30 minutes to 
 * run.  Instead, we never free the memory, but the program will exit
 * shortly anyway.
 */
    sq_destroy(sq);
    {
	register char **p;
	for (p = &groups[maxid-1]; p >= groups; p--)
	  if (*p)
	    free(*p);
    }
    free(groups);
#endif

    if (fclose(iout) || fclose(gout) || fclose(lout)) {
	fprintf(stderr, "Unsuccessful close of gid.db, group.db, or grplist.db\n");
	exit(SMS_CCONFIG);
    }
    fix_file(ioutf);
    fix_file(goutf);
    fix_file(loutf);
    return(1);
##}


do_filsys()
##{
    FILE *out;
    char outf[64], outft[64], *mach, machbuf[128], *group;
    register char *p;
    struct stat sb;
    time_t ftime;
    struct save_queue *sq, *sq_create();
##  char name[33], type[9], loc[81], access[2], mount[33], trans[257];
##  char *filetime, comments[65];
##  int flag1, flag2, flag3, flag4, id, fid;
    char *index();

    sprintf(outf, "%s/filsys.db", hesiod_dir);

    if (stat(outf, &sb) == 0) {
	ftime = sb.st_mtime;
	filetime = ingres_date_and_time(ftime);
##	retrieve (flag1 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "filesys"
##	retrieve (flag2 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "machine"
##	retrieve (flag3 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "alias"
##	retrieve (flag4 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "fsgroup"
	if (flag1 < 0 && flag2 < 0 && flag3 < 0 && flag4 < 0) {
	    fprintf(stderr, "File filsys.db does not need to be rebuilt.\n");
	    return(0);
      }
    }

    sprintf(outft, "%s~", outf);
    out = fopen(outft, "w");
    if (!out) {
	perror("cannot open filsys.db for write");
	exit(SMS_OCONFIG);
    }

    fprintf(stderr, "Building filsys.db\n");
    get_mach();
    sq = sq_create();

##  range of f is filesys
##  retrieve (name = f.label, type = f.#type, loc = f.#name, id = f.mach_id,
##	      access = f.#access, mount = f.#mount, comments = f.#comments,
##	      fid = f.filsys_id) {
	    strtrim(name);
	    strtrim(type);
	    if (!strcmp(type, "NFS") || !strcmp(type, "RVD")) {
		strtrim(loc);
		strtrim(access);
		strtrim(mount);
		if (mach = hash_lookup(machines, id)) {
		    for (p = machbuf; *mach && *mach != '.'; mach++) {
			if (isupper(*mach))
			  *p++ = tolower(*mach);
			else
			  *p++ = *mach;
			*p = 0;
		    }
		    fprintf(out, "%s.filsys\tHS UNSPECA \"%s %s %s %s %s\"\n",
			    name, type, loc, machbuf, access, mount);
		}
	    } else if (!strcmp(type, "AFS")) {
		strtrim(loc);
		strtrim(access);
		strtrim(mount);
		fprintf(out, "%s.filsys\tHS UNSPECA \"AFS %s %s %s\"\n",
			name, loc, access, mount);
	    } else if (!strcmp(type, "ERR")) {
		strtrim(comments);
		fprintf(out, "%s.filsys\tHS UNSPECA \"ERR %s\"\n",
			name, comments);
	    } else if (!strcmp(type, "FSGROUP")) {
		sprintf(trans, "%s:%d", name, fid);
		sq_save_data(sq, strsave(trans));
	    }
##  }

##  range of g is fsgroup
    while (sq_get_data(sq, &group)) {
	fid = atoi(index(group, ':')+1);
	*index(group, ':') = 0;
##      retrieve (type = f.#type, loc = f.#name, id = f.mach_id,
##		  access = f.#access, mount = f.#mount, comments = f.#comments,
##		  name = f.#label, trans = g.#key)
##	      where f.filsys_id = g.filsys_id and g.group_id = fid
##	      sort by #trans, #name {
	    strtrim(type);
	    if (!strcmp(type, "NFS") || !strcmp(type, "RVD")) {
		strtrim(loc);
		strtrim(access);
		strtrim(mount);
		if (mach = hash_lookup(machines, id)) {
		    for (p = machbuf; *mach && *mach != '.'; mach++) {
			if (isupper(*mach))
			  *p++ = tolower(*mach);
			else
			  *p++ = *mach;
			*p = 0;
		    }
		    fprintf(out, "%s.filsys\tHS UNSPECA \"%s %s %s %s %s\"\n",
			    group, type, loc, machbuf, access, mount);
		}
	    } else if (!strcmp(type, "AFS")) {
		strtrim(loc);
		strtrim(access);
		strtrim(mount);
		fprintf(out, "%s.filsys\tHS UNSPECA \"AFS %s %s %s\"\n",
			group, loc, access, mount);
	    } else if (!strcmp(type, "ERR")) {
		strtrim(comments);
		fprintf(out, "%s.filsys\tHS UNSPECA \"ERR %s\"\n",
			group, comments);
    	    }
##	}
	free(group);
    }
    sq_destroy(sq);

##  range of a is alias
##  retrieve (name = a.#name, trans = a.#trans) where a.#type = "FILESYS" {
    	strtrim(name);
	strtrim(trans);
    	fprintf(out, "%s.filsys\tHS CNAME %s.filsys\n", name, trans);
##  }

    if (fclose(out)) {
	fprintf(stderr, "Unsuccessful close of filsys.db\n");
	exit(SMS_CCONFIG);
    }
    fix_file(outf);
    return(1);
##}


/*
 * Modified from sys/types.h:
 */
int setsize;	/* = howmany(setbits, NSETBITS) */

typedef long	set_mask;
#define NSETBITS	(sizeof(set_mask) * NBBY)	/* bits per mask */
#ifndef howmany
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#endif

#define	SET_SET(n, p)	((p)[(n)/NSETBITS] |=  (1 << ((n) % NSETBITS)))
#define	SET_CLR(n, p)	((p)[(n)/NSETBITS] &= ~(1 << ((n) % NSETBITS)))
#define	SET_ISSET(n, p)	((p)[(n)/NSETBITS] &   (1 << ((n) % NSETBITS)))
#define SET_CREATE()	((set_mask *)malloc(setsize * sizeof(set_mask)))
#define SET_ZERO(p)	bzero((char *)(p), setsize * sizeof(set_mask))
#define SET_CMP(p1, p2) (bcmp((p1), (p2), setsize * sizeof(set_mask)))


do_cluster()
##{
    FILE *out;
    char outf[64], outft[64], *mach, machbuf[33], *p;
    struct stat sb;
    time_t ftime;
##  int flag1, flag2, flag3, flag4, maxmach, maxclu, mid, cid, id;
##  char name[33], label[17], data[33], *filetime;
    set_mask **machs, *ms, *ps;

    sprintf(outf, "%s/cluster.db", hesiod_dir);

    if (stat(outf, &sb) == 0) {
	ftime = sb.st_mtime;
	filetime = ingres_date_and_time(ftime);
##	retrieve (flag1 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "cluster"
##	retrieve (flag2 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "machine"
##	retrieve (flag3 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "mcmap"
##	retrieve (flag4 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "svc"
	if (flag1 < 0 && flag2 < 0 && flag3 < 0 && flag4 < 0) {
	    fprintf(stderr, "File cluster.db does not need to be rebuilt.\n");
	    return(0);
      }
    }

    sprintf(outft, "%s~", outf);
    out = fopen(outft, "w");
    if (!out) {
	perror("cannot open cluster.db for write");
	exit(SMS_OCONFIG);
    }

    fprintf(stderr, "Building cluster.db\n");
    get_mach();

##  range of c is cluster
##  begin transaction
##  retrieve (maxclu = max(c.clu_id))
    maxclu++;
    setsize = howmany(maxclu, NSETBITS);
##  range of m is machine
##  retrieve (maxmach = max(m.mach_id))
    maxmach++;
    machs = (set_mask **)malloc((maxmach + 1) * sizeof(set_mask **));
    bzero(machs, (maxmach + 1) * sizeof(int));

##  range of p is mcmap
##  retrieve (mid = p.mach_id, cid = p.clu_id) {
        if (!(ms = machs[mid])) {
	    ms = machs[mid] = SET_CREATE();
	    SET_ZERO(ms);
	}
	SET_SET(cid, ms);
##  }

##  range of d is svc
    for (mid = 1; mid < maxmach; mid++) {
	if (!machs[mid])
	  continue;
	ms = machs[mid];
	for (cid = 1; cid < maxclu; cid++) {
	    if (SET_ISSET(cid, ms)) {
##		repeat retrieve (label = d.serv_label, data = d.serv_cluster)
##		    where d.clu_id = @cid {
		    strtrim(label);
		    strtrim(data);
		    fprintf(out,
			    "smsinternal-%d.cluster\tHS UNSPECA \"%s %s\"\n",
			    mid, label, data);
##		}
	    }
	}

	if (mach = hash_lookup(machines, mid)) {
	    for (p = machbuf; *mach && *mach != '.'; mach++)
	      *p++ = *mach;
	    *p = 0;
	    fprintf(out, "%s.cluster\tHS CNAME smsinternal-%d.cluster\n",
		    machbuf, mid);
	}
	for (id = mid + 1; id < maxmach; id++) {
	    if ((ps = machs[id]) && !SET_CMP(ms, ps)) {
		free(ps);
		machs[id] = NULL;
		if (mach = hash_lookup(machines, id)) {
		    for (p = machbuf; *mach && *mach != '.'; mach++)
		      *p++ = *mach;
		    *p = 0;
		    fprintf(out,"%s.cluster\tHS CNAME smsinternal-%d.cluster\n",
			    machbuf, mid);
		}
	    }
	}
	free(ms);
	machs[mid] = NULL;
    }

##  retrieve (name = c.#name, label = d.serv_label, data = d.serv_cluster)
##	where c.clu_id = d.clu_id {
	    strtrim(name);
	    strtrim(label);
	    strtrim(data);
	    fprintf(out, "%s.cluster\tHS UNSPECA \"%s %s\"\n",
		    name, label, data);
##  }
##  end transaction
    free(machs);

    if (fclose(out)) {
	fprintf(stderr, "Unsuccessful close of cluster.db\n");
	exit(SMS_CCONFIG);
    }
    fix_file(outf);
    return(1);
##}


do_printcap()
##{
    FILE *out;
    char outf[64], outft[64];
    struct stat sb;
    time_t ftime;
##  char name[17], rp[17], sd[33], rm[33], *filetime;
##  int flag;

    sprintf(outf, "%s/printcap.db", hesiod_dir);

    if (stat(outf, &sb) == 0) {
	ftime = sb.st_mtime;
	filetime = ingres_date_and_time(ftime);
##	retrieve (flag = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "printcap"
	if (flag < 0) {
	    fprintf(stderr, "File printcap.db does not need to be rebuilt.\n");
	    return(0);
      }
    }

    sprintf(outft, "%s~", outf);
    out = fopen(outft, "w");
    if (!out) {
	perror("cannot open printcap.db for write");
	exit(SMS_OCONFIG);
    }

    fprintf(stderr, "Building printcap.db\n");

##  range of p is printcap
##  retrieve (name = p.#name, rp = p.#rp, sd = p.dir, rm = machine.#name)
##    where machine.mach_id = p.mach_id {
	strtrim(name);
	strtrim(rp);
	strtrim(sd);
	strtrim(rm);
	fprintf(out, "%s.pcap\tHS UNSPECA \"%s:rp=%s:rm=%s:sd=%s\"\n",
		name, name, rp, rm, sd);
##  }

    if (fclose(out)) {
	fprintf(stderr, "Unsuccessful close of pcap.db\n");
	exit(SMS_CCONFIG);
    }
    fix_file(outf);
    return(1);
##}


do_palladium()
##{
    FILE *out;
    char outf[64], outft[64];
    struct stat sb;
    time_t ftime;
##  char name[33], rm[33], trans[129], *filetime;
##  int flag, flag1, ident;

    sprintf(outf, "%s/palladium.db", hesiod_dir);

    if (stat(outf, &sb) == 0) {
	ftime = sb.st_mtime;
	filetime = ingres_date_and_time(ftime);
##	retrieve (flag = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "palladium"
##	retrieve (flag1 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "alias"
	if (flag < 0 && flag1 < 0) {
	    fprintf(stderr, "File palladium.db does not need to be rebuilt.\n");
	    return(0);
      }
    }

    sprintf(outft, "%s~", outf);
    out = fopen(outft, "w");
    if (!out) {
	perror("cannot open palladium.db for write");
	exit(SMS_OCONFIG);
    }

    fprintf(stderr, "Building palladium.db\n");

##  range of p is palladium
##  retrieve (name = p.#name, ident = p.#ident, rm = machine.#name)
##    where machine.mach_id = p.mach_id {
	strtrim(name);
	strtrim(rm);
	fprintf(out,
		"%s.palladium\tHS UNSPECA \"%s %d %s interface directory\"\n",
		name, rm, ident, name);
##  }
##  range of a is alias
##  retrieve (name = a.#name, trans = a.#trans) where a.#type = "PALLADIUM" {
    	strtrim(name);
	strtrim(trans);
	fprintf(out, "%s.palladium\tHS UNSPECA \"%s\"\n", name, trans);
##  }

    if (fclose(out)) {
	fprintf(stderr, "Unsuccessful close of palladium.db\n");
	exit(SMS_CCONFIG);
    }
    fix_file(outf);
    return(1);
##}


do_sloc()
##{
    FILE *out, *old, *new;
    char outf[64], outft[64], *mach;
    struct stat sb;
    time_t ftime;
    register int c;
##  char service[17], *filetime;
##  int port, flag1, flag2, id;

    sprintf(outf, "%s/sloc.db", hesiod_dir);

    if (stat(outf, &sb) == 0) {
	ftime = sb.st_mtime;
	filetime = ingres_date_and_time(ftime);
##	retrieve (flag1 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "serverhosts"
##	retrieve (flag2 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "machine"
	if (flag1 < 0 && flag2 < 0) {
	    fprintf(stderr, "File sloc.db does not need to be rebuilt.\n");
	    return(0);
      }
    }

    sprintf(outft, "%s~", outf);
    out = fopen(outft, "w");
    if (!out) {
	perror("cannot open sloc.db for write");
	exit(SMS_OCONFIG);
    }

    fprintf(stderr, "Building sloc.db\n");
    get_mach();

##  range of s is serverhosts
##  retrieve (service = s.#service, id = s.mach_id)
##	sort by #service {
	    strtrim(service);
	    if (mach = hash_lookup(machines, id))
	      fprintf(out, "%s.sloc\tHS UNSPECA %s\n", service, mach);
##  }

    if (fclose(out)) {
	fprintf(stderr, "Unsuccessful close of sloc.db\n");
	exit(SMS_CCONFIG);
    }

    fix_file(outf);
    return(1);
##}

do_service()
##{
    FILE *out;
    char outf[64], outft[64];
    struct stat sb;
    time_t ftime;
##  char mach[33], service[33], protocol[9], altserv[129], *filetime;
##  int port, flag1, flag2;

    sprintf(outf, "%s/service.db", hesiod_dir);

    if (stat(outf, &sb) == 0) {
	ftime = sb.st_mtime;
	filetime = ingres_date_and_time(ftime);
##	retrieve (flag1 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "services"
	if (flag1 < 0) {
	    fprintf(stderr, "File service.db does not need to be rebuilt.\n");
	    return(0);
      }
    }

    sprintf(outft, "%s~", outf);
    out = fopen(outft, "w");
    if (!out) {
	perror("cannot open service.db for write");
	exit(SMS_OCONFIG);
    }

    fprintf(stderr, "Building service.db\n");

##  range of s is services
##  retrieve (service = s.name, protocol = lowercase(s.#protocol),
##	      port = s.#port) {
	strtrim(service);
	strtrim(protocol);
	fprintf(out, "%s.service\tHS UNSPECA \"%s %s %d\"\n",
		service, service, protocol, port);
##  }

##  range of a is alias
##  retrieve (service = a.name, altserv = a.trans) where a.type = "SERVICE" {
	strtrim(service);
	strtrim(altserv);
	fprintf(out, "%s.service\tHS CNAME %s.service\n", service, altserv);
##    }

    if (fclose(out)) {
	fprintf(stderr, "Unsuccessful close of service.db\n");
	exit(SMS_CCONFIG);
    }
    fix_file(outf);
    return(1);
##}
