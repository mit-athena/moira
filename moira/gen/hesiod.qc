/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/hesiod.qc,v 1.2 1988-06-20 12:52:46 mar Exp $
 *
 * This generates the zone files necessary to load a hesiod server.
 * The following zones are generated:
 */

#include <stdio.h>
#include <sms.h>
#include <sms_app.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#define HESIOD_DIR	"/u1/sms/dcm/hesiod"

#define min(x,y)	((x) < (y) ? (x) : (y))

char *malloc(), *strsave();
char *ingres_date_and_time(), *ingres_time(), *ingres_date();

main(argc, argv)
int argc;
char **argv;
{
    char cmd[64];
    struct stat sb;
    int changed = 0;

    if (argc > 2) {
	fprintf(stderr, "usage: %s [outfile]\n", argv[0]);
	exit(-1);
    }

##  ingres sms
##  set lockmode session where readlock = nolock

    changed = do_passwd();
    changed += do_pobox();
    changed += do_groups();
    changed += do_filsys();
    changed += do_cluster();
    changed += do_printers();
    changed += do_services();

##  exit

    if (!changed) {
	fprintf(stderr, "No files updated.\n");
	if (argc == 2 && stat(argv[1], &sb) == 0)
	  exit(0);
    }

    if (argc == 2) {
	sprintf(cmd, "cd %s; tar cf %s .", HESIOD_DIR, argv[1]);
	exit(system(cmd));
    }

    exit(0);
}


do_passwd()
##{
    FILE *pout, *uout;
    char poutf[64], uoutf[64];
    struct stat psb, usb;
    time_t ftime;
##  char login[9], shell[33], fullname[33], oa[17], op[13], hp[17], *filetime;
##  int uid, flag;

    sprintf(poutf, "%s/passwd.db", HESIOD_DIR);
    sprintf(uoutf, "%s/uid.db", HESIOD_DIR);

    if (stat(poutf, &psb) == 0 && stat(uoutf, &usb) == 0) {
	ftime = min(psb.st_mtime, usb.st_mtime);
	filetime = ingres_date_and_time(ftime);
##	retrieve (flag = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "users"
	if (flag < 0) {
	    fprintf(stderr, "Files passwd.db and uid.db do not need to be rebuilt.\n");
	    return(0);
      }
    }

    pout = fopen(poutf, "w");
    if (!pout) {
	perror("cannot open passwd.db for write");
	exit(-1);
    }
    uout = fopen(uoutf, "w");
    if (!uout) {
	perror("cannot open uid.db for write");
	exit(-1);
    }

    fprintf(stderr, "Building passwd.db and uid.db\n");

##  range of u is users
##  retrieve (login = u.#login, uid = u.#uid, shell = u.#shell,
##	      fullname = u.#fullname, oa = u.office_addr,
##	      op = u.office_phone, hp = u.home_phone)
##	where u.status != 0 sort by #login {
	    trim(login);
	    trim(fullname);
	    trim(oa);
	    trim(op);
	    trim(hp);
	    trim(shell);
    	    fprintf(pout, "%s.passwd\tHS UNSPECA \"%s:*:%d:101:%s,%s,%s,%s:/mit/%s:%s\"\n",
		    login, login, uid, fullname, oa, op, hp, login, shell);
	    fprintf(uout, "%d.uid\tHS CNAME %s.passwd\n", uid, login);
##  }

    if (fclose(pout) || fclose(uout)) {
	fprintf(stderr, "Unsuccessful file close of passwd.db or uid.db\n");
	exit(-1);
    }
    return(1);
##}


do_pobox()
##{
    FILE *out;
    char outf[64];
    struct stat sb;
    time_t ftime;
##  char login[9], mach[33], *filetime;
##  int flag1, flag2;

    sprintf(outf, "%s/pobox.db", HESIOD_DIR);

    if (stat(outf, &sb) == 0) {
	ftime = sb.st_mtime;
	filetime = ingres_date_and_time(ftime);
##	retrieve (flag1 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "users"
##	retrieve (flag2 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "machine"
	if (flag1 < 0 && flag2 < 0) {
	    fprintf(stderr,"File pobox.db does not need to be rebuilt.\n");
	    return(0);
      }
    }

    out = fopen(outf, "w");
    if (!out) {
	perror("cannot open pobox.db for write");
	exit(-1);
    }

    fprintf(stderr, "Building pobox.db\n");

##  range of u is users
##  range of m is machine
##  retrieve (login = u.#login, mach = m.#name)
##	where u.potype = "POP" and m.mach_id = u.pop_id {
	    trim(login);
	    trim(mach);
	    fprintf(out, "%s.pobox\tHS UNSPECA \"POP %s %s\"\n",
		    login, mach, login);
##  }

    if (fclose(out)) {
	fprintf(stderr, "Unsuccessful close of pobox.db\n");
	exit(-1);
    }
    return(1);
##}


do_groups()
##{
    FILE *iout, *gout, *lout;
    char ioutf[64], goutf[64], loutf[64], buf[256];
    char **groups;
    struct stat isb, gsb, lsb;
    time_t ftime;
    struct save_queue *sq, *sq_create();
    register int first;
##  char name[33], *filetime;
##  int gid, id, lid, flag1, flag2, flag3, maxid;

    /* open files */
    sprintf(ioutf, "%s/gid.db", HESIOD_DIR);
    sprintf(goutf, "%s/group.db", HESIOD_DIR);
    sprintf(loutf, "%s/grplist.db", HESIOD_DIR);

    if (stat(ioutf, &isb) == 0 && stat(goutf, &gsb) == 0 && stat(loutf, &lsb) == 0) {
	ftime = min(isb.st_mtime, min(gsb.st_mtime, lsb.st_mtime));
	filetime = ingres_date_and_time(ftime);
##	retrieve (flag1 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "users"
##	retrieve (flag2 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "list"
##	retrieve (flag3 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "members"
	if (flag1 < 0 && flag2 < 0 && flag3 < 0) {
	    fprintf(stderr, "Files gid.db, group.db and grplist.db do not need to be rebuilt.\n");
	    return(0);
      }
    }

    iout = fopen(ioutf, "w");
    if (!iout) {
	perror("cannot open gid.db for write");
	exit(-1);
    }
    gout = fopen(goutf, "w");
    if (!gout) {
	perror("cannot open group.db for write");
	exit(-1);
    }
    lout = fopen(loutf, "w");
    if (!lout) {
	perror("cannot open grplist.db for write");
	exit(-1);
    }

    fprintf(stderr, "Building gid.db, group.db, and grplist.db\n");

    /* make space for group list */
##  range of l is list
##  retrieve (maxid = max(l.#list_id))
    groups = (char **)malloc((maxid + 1) * sizeof(char *));
    if (groups == NULL) {
	fprintf(stderr, "unable to malloc space for groups\n");
	exit(1);
    }
    bzero(groups, (maxid + 1) * sizeof(char *));

    /* retrieve simple groups */
##  retrieve (name = l.#name, gid = l.#gid, lid = l.list_id)
##	where l.group != 0 {
	    trim(name);
	    sprintf(buf, "%s:%d", name, gid);
	    groups[lid] = strsave(buf);
	    fprintf(iout, "%d.gid\tHS CNAME %s.group\n", gid, name);
	    fprintf(gout, "%s.group\tHS UNSPECA \"%s:*:%d:\"\n",
		    name, name, gid);
##  }

    /* get special cases: lists that aren't groups themselves but are
     * members of groups. */
    sq = sq_create();
##  range of m is members
##  retrieve (name = l.#name, gid = l.#gid, lid = l.list_id)
##	where l.group = 0 and m.member_type = "LIST" and
##		m.member_id = l.list_id and m.list_id = list.list_id and
##		list.group != 0 {
	trim(name);
	sprintf(buf, "%s:%d", name, gid);
	groups[lid] = strsave(buf);
    	sq_save_data(sq, lid);
##  }
    while (sq_get_data(sq, &id)) {
##	repeat retrieve (name = l.#name, gid = l.#gid, lid = l.list_id)
##	    where l.group = 0 and m.member_type = "LIST" and
##		m.member_id = l.list_id and m.list_id = @id {
	    trim(name);
	    sprintf(buf, "%s:%d", name, gid);
	    groups[lid] = strsave(buf);
	    sq_save_unique_data(sq, lid);
##	}
    }
    sq_destroy(sq);

    /* now do grplists */
    sq = sq_create();
##  range of u is users
##  retrieve (id = u.users_id) where u.status != 0 {
	sq_save_data(sq, id);
##  }
    while (sq_get_data(sq, &id)) {
##	repeat retrieve (name = u.login) where u.users_id = @id
	trim(name);
	first = 1;
##	repeat retrieve (lid = m.list_id)
##		where m.member_type = "USER" and m.member_id = @id {
	    if (groups[lid]) {
		if (first)
		  fprintf(lout, "%s.grplist\tHS UNSPECA \"%s",name,groups[lid]);
		else
		  fprintf(lout, ":%s", groups[lid]);
		first = 0;
	    }
##	}
	if (!first)
	  fprintf(lout, "\"\n");
    }
    sq_destroy(sq);
    for (id = 0; id < maxid; id++)
      if (groups[id])
	free(groups[id]);
    free(groups);

    if (fclose(iout) || fclose(gout) || fclose(lout)) {
	fprintf(stderr, "Unsuccessful close of gid.db, group.db, or grplist.db\n");
	exit(-1);
    }
    return(1);
##}


do_filsys()
##{
    FILE *out;
    char outf[64];
    struct stat sb;
    time_t ftime;
##  char name[33], type[9], loc[33], mach[33], access[2], mount[33], trans[257];
##  char *filetime;
##  int flag1, flag2, flag3;

    sprintf(outf, "%s/filsys.db", HESIOD_DIR);

    if (stat(outf, &sb) == 0) {
	ftime = sb.st_mtime;
	filetime = ingres_date_and_time(ftime);
##	retrieve (flag1 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "filesys"
##	retrieve (flag2 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "machine"
##	retrieve (flag3 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "alias"
	if (flag1 < 0 && flag2 < 0 && flag3 < 0) {
	    fprintf(stderr, "File filsys.db does not need to be rebuilt.\n");
	    return(0);
      }
    }


    out = fopen(outf, "w");
    if (!out) {
	perror("cannot open filsys.db for write");
	exit(-1);
    }

    fprintf(stderr, "Building filsys.db\n");

##  range of f is filesys
##  range of m is machine
##  retrieve (name = f.label, type = f.#type, loc = f.#name, mach = m.#name,
##	      access = f.#access, mount = f.#mount)
##	where m.mach_id = f.mach_id {
	    trim(name);
	    trim(type);
	    trim(loc);
	    trim(mach);
	    trim(access);
	    trim(mount);
	    fprintf(out, "%s.filsys\tHS UNSPECA \"%s %s %s %s %s\"\n",
		    name, type, loc, mach, access, mount);
##  }

##  range of a is alias
##  retrieve (name = a.#name, trans = a.#trans) where a.#type = "FILESYS" {
    	trim(name);
	trim(trans);
    	fprintf(out, "%s.filsys\tHS CNAME %s.filsys\n", name, trans);
##  }

    if (fclose(out)) {
	fprintf(stderr, "Unsuccessful close of filsys.db\n");
	exit(-1);
    }
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
    char outf[64];
    struct stat sb;
    time_t ftime;
##  int flag1, flag2, flag3, flag4, maxmach, maxclu, mid, cid, id;
##  char name[33], label[17], data[33], mach[33], *filetime;
    set_mask **machs, *ms, *ps;

    sprintf(outf, "%s/cluster.db", HESIOD_DIR);

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

    out = fopen(outf, "w");
    if (!out) {
	perror("cannot open cluster.db for write");
	exit(-1);
    }

    fprintf(stderr, "Building cluster.db\n");

##  range of c is cluster
##  retrieve (maxclu = max(c.clu_id))
    setsize = howmany(maxclu, NSETBITS);
##  range of m is machine
##  retrieve (maxmach = max(m.mach_id))
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
	fprintf(out, "smsinternal-%d.cluster", mid);
	for (cid = 1; cid < maxclu; cid++) {
	    if (SET_ISSET(cid, ms)) {
##		repeat retrieve (label = d.serv_label, data = d.serv_cluster)
##		    where d.clu_id = @cid {
		    trim(label);
		    trim(data);
		    fprintf(out, "\tHS UNSPECA \"%s %s\"\n", label, data);
##		}
	    }
	}

##	repeat retrieve (mach = m.#name) where m.mach_id = @mid
	trim(mach);
	fprintf(out, "%s.cluster\tHS CNAME smmsinternal-%d.cluster\n",
		mach, mid);
	for (id = mid + 1; id < maxmach; id++) {
	    if ((ps = machs[id]) && !SET_CMP(ms, ps)) {
		free(ps);
		machs[id] = NULL;
##		repeat retrieve (mach = m.#name) where m.mach_id = @id
		trim(mach);
		fprintf(out, "%s.cluster\tHS CNAME smmsinternal-%d.cluster\n",
			mach, mid);
	    }
	}
	free(ms);
	machs[mid] = NULL;
    }
    free(machs);

##  retrieve (name = c.#name, label = d.serv_label, data = d.serv_cluster)
##	where c.clu_id = d.clu_id {
	    trim(name);
	    trim(label);
	    trim(data);
	    fprintf(out, "%s.cluster\tHS UNSPECA \"%s %s\"\n",
		    name, label, data);
##  }

    if (fclose(out)) {
	fprintf(stderr, "Unsuccessful close of cluster.db\n");
	exit(-1);
    }
    return(1);
##}


do_printers()
##{
    FILE *out;
    char outf[64];
    struct stat sb;
    time_t ftime;
##  char name[33], pcap[513], *filetime;
##  int flag;

    sprintf(outf, "%s/printcap.db", HESIOD_DIR);

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

    out = fopen(outf, "w");
    if (!out) {
	perror("cannot open printcap.db for write");
	exit(-1);
    }

    fprintf(stderr, "Building printcap.db\n");

##  range of p is printcap
##  retrieve (name = p.#name, pcap = p.#pcap) {
	trim(name);
	trim(pcap);
	fprintf(out, "%s.pcap\tHS UNSPECA \"%s\"\n", name, pcap);
##  }

    if (fclose(out)) {
	fprintf(stderr, "Unsuccessful close of pcap.db\n");
	exit(-1);
    }
    return(1);
##}


do_services()
##{
    FILE *out;
    char outf[64];
    struct stat sb;
    time_t ftime;
##  char mach[33], service[17], protocol[9], *filetime;
##  int port, flag1, flag2;

    sprintf(outf, "%s/sloc.db", HESIOD_DIR);

    if (stat(outf, &sb) == 0) {
	ftime = sb.st_mtime;
	filetime = ingres_date_and_time(ftime);
##	retrieve (flag1 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "serverhosts"
##	retrieve (flag2 = int4(interval("min", tblstats.modtime - filetime)))
##	    where tblstats.table = "machine"
	if (flag1 < 0 && flag2 < 0) {
	    fprintf(stderr, "File sloc.db does not need to be rebuilt.\n");
	    goto next;
      }
    }

    out = fopen(outf, "w");
    if (!out) {
	perror("cannot open sloc.db for write");
	exit(-1);
    }

    fprintf(stderr, "Building sloc.db\n");

##  range of s is serverhosts
##  retrieve (service = s.#service, mach = m.name)
##	where m.mach_id = s.mach_id {
	    trim(service);
	    trim(mach);
	    fprintf(out, "%s.sloc\tHS UNSPECA %s\n", service, mach);
##  }

    if (fclose(out)) {
	fprintf(stderr, "Unsuccessful close of sloc.db\n");
	exit(-1);
    }

 next:
    sprintf(outf, "%s/service.db", HESIOD_DIR);

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

    out = fopen(outf, "w");
    if (!out) {
	perror("cannot open service.db for write");
	exit(-1);
    }

    fprintf(stderr, "Building service.db\n");

##  range of s is services
##  retrieve (service = s.name, protocol = lowercase(s.#protocol),
##	      port = s.#port) {
	trim(service);
	trim(protocol);
	fprintf(out, "%s.service\tHS UNSPECA \"%s %s %d\"\n",
		service, service, protocol, port);
##  }

    if (fclose(out)) {
	fprintf(stderr, "Unsuccessful close of service.db\n");
	exit(-1);
    }
    return(1);
##}
