/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/hesiod.qc,v 1.25 1990-03-19 19:05:56 mar Exp $
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
#include <moira.h>
#include <moira_site.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <ctype.h>


#ifdef ATHENA
#define HTYPE "UNSPECA"
#else
#define HTYPE "TXT"
#endif

char hesiod_dir[64];

#define min(x,y)	((x) < (y) ? (x) : (y))
struct hash *machines = NULL;
struct hash *users = NULL;
char *whoami = "hesiod.gen";

struct grp {
    struct grp *next;
    char *lid;
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
	exit(MR_ARGS);
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
	  exit(MR_NO_CHANGE);
    }

    if (argc == 2) {
	fprintf(stderr, "Building tar file.\n");
	sprintf(cmd, "cd %s; tar cf %s .", hesiod_dir, argv[1]);
	if (system(cmd))
	  exit(MR_TAR_FAIL);
    }

    exit(MR_SUCCESS);
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
    int ingres_errno;

    switch (*num) {
    case INGRES_DEADLOCK:
	ingres_errno = MR_DEADLOCK;
	break;
    default:
	ingres_errno = MR_INGRES_ERR;
    }
    com_err(whoami, MR_INGRES_ERR, " code %d\n", *num);
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
	exit(MR_OCONFIG);
    }
    sprintf(uoutft, "%s~", uoutf);
    uout = fopen(uoutft, "w");
    if (!uout) {
	perror("cannot open uid.db~ for write");
	exit(MR_OCONFIG);
    }
    sprintf(boutft, "%s~", boutf);
    bout = fopen(boutft, "w");
    if (!bout) {
	perror("cannot open pobox.db for write");
	exit(MR_OCONFIG);
    }

    fprintf(stderr, "Building passwd.db, uid.db, and pobox.db\n");
    get_mach();

    users = create_hash(12001);
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
    	    fprintf(pout, "%s.passwd\tHS %s \"%s:*:%d:101:%s,%s,%s,%s,%s:/mit/%s:%s\"\n",
		    login, HTYPE, login, uid, fullname, nn, oa, op, hp, login, shell);
	    fprintf(uout, "%d.uid\tHS CNAME %s.passwd\n", uid, login);
	    if (ptype[0] == 'P' && (mach = hash_lookup(machines, pid))) {
		fprintf(bout, "%s.pobox\tHS %s \"POP %s %s\"\n",
			login, HTYPE, mach, login);
	    }
##  }

    if (fclose(pout) || fclose(uout) || fclose(bout)) {
	fprintf(stderr, "Unsuccessful file close of passwd.db, uid.db, or pobox.db\n");
	exit(MR_CCONFIG);
    }
    fix_file(poutf);
    fix_file(uoutf);
    fix_file(boutf);
    return(1);
##}


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
##  char name[33], *filetime;
##  int gid, id, lid, flag1, flag2, flag3;

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
	exit(MR_OCONFIG);
    }
    sprintf(buf, "%s~", goutf);
    gout = fopen(buf, "w");
    if (!gout) {
	perror("cannot open group.db for write");
	exit(MR_OCONFIG);
    }
    sprintf(buf, "%s~", loutf);
    lout = fopen(buf, "w");
    if (!lout) {
	perror("cannot open grplist.db for write");
	exit(MR_OCONFIG);
    }

    fprintf(stderr, "Building gid.db, group.db, and grplist.db\n");

    /* make space for group list */
    groups = create_hash(15001);

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
	    fprintf(gout, "%s.group\tHS %s \"%s:*:%d:\"\n",
		    name, HTYPE, name, gid);
##  }

    fflush(iout);
    fflush(gout);

    /* now do grplists */
    if (users == NULL) {
	users = create_hash(12001);
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
      if (((l = hash_lookup(groups, lid)) != NULL) &&
	  (u = (struct user *) hash_lookup(users, id))) {
	  g = (struct grp *) malloc(sizeof(struct grp));
	  g->next = u->lists;
	  u->lists = g;
	  g->lid = l;
      }
##  }
##  end transaction

    for (p = &(users->data[users->size - 1]); p >= users->data; p--) {
	for (b = *p; b; b = b->next) {
	    if ((g = ((struct user *)b->data)->lists) == NULL)
	      continue;
	    fprintf(lout, "%s.grplist\tHS %s \"",
		    ((struct user *)b->data)->name, HTYPE);
	    for (; g; g = g->next) {
		fputs(g->lid, lout);
		if (g->next)
		  putc(':', lout);
	    }
	    fputs("\"\n", lout);
	}
    }

    if (fclose(iout) || fclose(gout) || fclose(lout)) {
	fprintf(stderr, "Unsuccessful close of gid.db, group.db, or grplist.db\n");
	exit(MR_CCONFIG);
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
	exit(MR_OCONFIG);
    }

    fprintf(stderr, "Building filsys.db\n");
    get_mach();
    sq = sq_create();

##  range of f is filesys
##  retrieve (name = f.label, type = f.#type, loc = f.#name, id = f.mach_id,
##	      access = f.#access, mount = f.#mount, comments = f.#comments,
##	      fid = f.filsys_id) {
	    strtrim(type);
	    if (!strcmp(type, "NFS") || !strcmp(type, "RVD")) {
		if (mach = hash_lookup(machines, id)) {
		    for (p = machbuf; *mach && *mach != '.'; mach++) {
			if (isupper(*mach))
			  *p++ = tolower(*mach);
			else
			  *p++ = *mach;
		    }
		    *p = 0;
		    fprintf(out, "%s.filsys\tHS %s \"%s %s %s %s %s\"\n",
			    strtrim(name), HTYPE, type, strtrim(loc), machbuf,
			    strtrim(access), strtrim(mount));
		}
	    } else if (!strcmp(type, "AFS")) {
		fprintf(out, "%s.filsys\tHS %s \"AFS %s %s %s\"\n",
			strtrim(name), HTYPE, strtrim(loc), strtrim(access),
			strtrim(mount));
	    } else if (!strcmp(type, "ERR")) {
		fprintf(out, "%s.filsys\tHS %s \"ERR %s\"\n",
			name, HTYPE, strtrim(comments));
	    } else if (!strcmp(type, "FSGROUP")) {
		sprintf(trans, "%s:%d", strtrim(name), fid);
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
		if (mach = hash_lookup(machines, id)) {
		    for (p = machbuf; *mach && *mach != '.'; mach++) {
			if (isupper(*mach))
			  *p++ = tolower(*mach);
			else
			  *p++ = *mach;
		    }
		    *p = 0;
		    fprintf(out, "%s.filsys\tHS %s \"%s %s %s %s %s\"\n",
			    group, HTYPE, type, strtrim(loc), machbuf,
			    strtrim(access), strtrim(mount));
		}
	    } else if (!strcmp(type, "AFS")) {
		fprintf(out, "%s.filsys\tHS %s \"AFS %s %s %s\"\n",
			group, HTYPE, strtrim(loc), strtrim(access),
			strtrim(mount));
	    } else if (!strcmp(type, "ERR")) {
		fprintf(out, "%s.filsys\tHS %s \"ERR %s\"\n",
			group, HTYPE, strtrim(comments));
    	    }
##	}
	free(group);
    }
    sq_destroy(sq);

##  range of a is alias
##  retrieve (name = a.#name, trans = a.#trans) where a.#type = "FILESYS" {
    	fprintf(out, "%s.filsys\tHS CNAME %s.filsys\n",
		strtrim(name), strtrim(trans));
##  }

    if (fclose(out)) {
	fprintf(stderr, "Unsuccessful close of filsys.db\n");
	exit(MR_CCONFIG);
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

int nbitsset(set)
set_mask *set;
{
    int i, ret;

    ret = 0;
    for (i = 0; i < setsize * NSETBITS; i++)
      if (SET_ISSET(i, set))
	ret++;
    return(ret);
}


do_cluster()
##{
    FILE *out;
    char outf[64], outft[64], *mach, machbuf[33], *p;
    struct stat sb;
    time_t ftime;
##  int flag1, flag2, flag3, flag4, maxmach, maxclu, mid, cid, id;
##  char name[33], label[17], data[33], *filetime;
    set_mask **machs, *ms, *ps;
    int oneclu;

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
	exit(MR_OCONFIG);
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
	if (nbitsset(ms) > 1) {
	    oneclu = 0;
	    for (cid = 1; cid < maxclu; cid++) {
		if (SET_ISSET(cid, ms)) {
##		  repeat retrieve (label = d.serv_label, data = d.serv_cluster)
##		    where d.clu_id = @cid {
		      strtrim(label);
		      strtrim(data);
		      fprintf(out,
			      "mrinternal-%d.cluster\tHS %s \"%s %s\"\n",
			      mid, HTYPE, label, data);
##		  }
		}
	    }
	} else {
	    oneclu = 1;
	    for (cid = 1; cid < maxclu; cid++)
	      if (SET_ISSET(cid, ms)) break;
##	    repeat retrieve (name = c.#name) where c.clu_id = @cid
	    strtrim(name);
	}

	if (mach = hash_lookup(machines, mid)) {
	    for (p = machbuf; *mach && *mach != '.'; mach++)
	      *p++ = *mach;
	    *p = 0;
	    if (oneclu)
	      fprintf(out, "%s.cluster\tHS CNAME %s.cluster\n",
		      machbuf, name);
	    else
	      fprintf(out, "%s.cluster\tHS CNAME mrinternal-%d.cluster\n",
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
		    if (oneclu)
		      fprintf(out, "%s.cluster\tHS CNAME %s.cluster\n",
			      machbuf, name);
		    else
		      fprintf(out,
			      "%s.cluster\tHS CNAME mrinternal-%d.cluster\n",
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
	    fprintf(out, "%s.cluster\tHS %s \"%s %s\"\n",
		    name, HTYPE, label, data);
##  }
##  end transaction
    free(machs);

    if (fclose(out)) {
	fprintf(stderr, "Unsuccessful close of cluster.db\n");
	exit(MR_CCONFIG);
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
	exit(MR_OCONFIG);
    }

    fprintf(stderr, "Building printcap.db\n");

##  range of p is printcap
##  retrieve (name = p.#name, rp = p.#rp, sd = p.dir, rm = machine.#name)
##    where machine.mach_id = p.mach_id {
	strtrim(name);
	strtrim(rp);
	strtrim(sd);
	strtrim(rm);
	fprintf(out, "%s.pcap\tHS %s \"%s:rp=%s:rm=%s:sd=%s\"\n",
		name, HTYPE, name, rp, rm, sd);
##  }

    if (fclose(out)) {
	fprintf(stderr, "Unsuccessful close of pcap.db\n");
	exit(MR_CCONFIG);
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
	exit(MR_OCONFIG);
    }

    fprintf(stderr, "Building palladium.db\n");

##  range of p is palladium
##  retrieve (name = p.#name, ident = p.#ident, rm = machine.#name)
##    where machine.mach_id = p.mach_id {
	strtrim(name);
	strtrim(rm);
	fprintf(out,
		"%s.palladium\tHS %s \"%s %d %s interface directory\"\n",
		name, HTYPE, rm, ident, name);
##  }
##  range of a is alias
##  retrieve (name = a.#name, trans = a.#trans) where a.#type = "PALLADIUM" {
    	strtrim(name);
	strtrim(trans);
	fprintf(out, "%s.palladium\tHS %s \"%s\"\n", name, HTYPE, trans);
##  }

    if (fclose(out)) {
	fprintf(stderr, "Unsuccessful close of palladium.db\n");
	exit(MR_CCONFIG);
    }
    fix_file(outf);
    return(1);
##}


do_sloc()
##{
    FILE *out;
    char outf[64], outft[64], *mach;
    struct stat sb;
    time_t ftime;
##  char service[17], *filetime;
##  int flag1, flag2, id;

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
	exit(MR_OCONFIG);
    }

    fprintf(stderr, "Building sloc.db\n");
    get_mach();

##  range of s is serverhosts
##  retrieve (service = s.#service, id = s.mach_id)
##	sort by #service {
	    strtrim(service);
	    if (mach = hash_lookup(machines, id))
	      fprintf(out, "%s.sloc\tHS %s %s\n", service, HTYPE, mach);
##  }

    if (fclose(out)) {
	fprintf(stderr, "Unsuccessful close of sloc.db\n");
	exit(MR_CCONFIG);
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
##  char service[33], protocol[9], altserv[129], *filetime;
##  int port, flag1;

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
	exit(MR_OCONFIG);
    }

    fprintf(stderr, "Building service.db\n");

##  range of s is services
##  retrieve (service = s.name, protocol = lowercase(s.#protocol),
##	      port = s.#port) {
	strtrim(service);
	strtrim(protocol);
	fprintf(out, "%s.service\tHS %s \"%s %s %d\"\n",
		service, HTYPE, service, protocol, port);
##  }

##  range of a is alias
##  retrieve (service = a.name, altserv = a.trans) where a.type = "SERVICE" {
	strtrim(service);
	strtrim(altserv);
	fprintf(out, "%s.service\tHS CNAME %s.service\n", service, altserv);
##    }

    if (fclose(out)) {
	fprintf(stderr, "Unsuccessful close of service.db\n");
	exit(MR_CCONFIG);
    }
    fix_file(outf);
    return(1);
##}
