/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dbck/phase2.qc,v 1.9 1990-03-19 18:44:07 mar Exp $
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <stdio.h>
#include <moira.h>
#include "dbck.h"

static char phase2_qc_rcsid[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dbck/phase2.qc,v 1.9 1990-03-19 18:44:07 mar Exp $";


show_mcm_mach(id)
int id;
##{
##  int iid = id, found = 1;
##  char name[33];

##  retrieve (name = cluster.#name) where cluster.clu_id = mcmap.clu_id
##	and mcmap.mach_id = iid {
      strtrim(name);
      found = 0;
      printf("Cluster %s, non-existant machine %d in cluster map\n", name, id);
##  }
    return(found);
##}

show_mcm_clu(id)
int id;
##{
##  int iid = id, found = 1;
##  char name[33];

##  retrieve (name = machine.#name) where machine.mach_id = mcmap.mach_id
##	and mcmap.clu_id = iid {
      strtrim(name);
      found = 0;
      printf("Machine %s, non-existant cluster %d in cluster map\n", name, id);
##  }
    return(found);
##}

pobox_check(id, u, hint)
int id;
struct user *u;
int hint;
{
    switch (u->potype) {
    case 'P':
	if (!hash_lookup(machines, u->pobox_id)) {
	    printf("User %s(%s) has P.O.Box on non-existant machine %d\n",
		   u->login, u->fullname, u->pobox_id);
	    if (single_fix("Delete", 0)) {
		remove_pobox(u->users_id);
		u->potype = 'N';
	    }
	}
	break;
    case 'S':
	if (!string_check(u->pobox_id)) {
	    printf("User %s(%s) has P.O.Box with non-existant string %d\n",
		   u->login, u->fullname, u->pobox_id);
	    if (single_fix("Delete", 0)) {
		remove_pobox(u->users_id);
		u->potype = 'N';
	    }
	}
	break;
    default:
	;
    }
}


remove_pobox(id)
int id;
##{
##  int rowcount, iid = id;
##  replace users (potype = "NONE") where users.users_id = iid
##  inquire_equel(rowcount = "rowcount")
    if (rowcount > 0)
      printf("%d entr%s removed\n", rowcount, rowcount==1?"y":"ies");
    else
      printf("Not removed\n");
    modified("users");
##}

show_svc(id)
int id;
##{
##  int iid = id, found = 1;
##  char label[17], data[33];

##  retrieve (label = svc.serv_label, data = svc.serv_cluster) 
##	where svc.clu_id = iid {
      strtrim(label);
      strtrim(data);
      found = 0;
      printf("Cluster data [%s] %s for non-existant cluster %d\n",
	     label, data, id);
##  }
    return(found);
##}

list_check(id, l, hint)
int id;
struct list *l;
int hint;
{
    switch (l->acl_type) {
    case 'L':
	if (!hash_lookup(lists, l->acl_id)) {
	    printf("List %s has bad LIST acl %d\n", l->name, l->acl_id);
	    if (single_fix("Patch", 1)) {
		fix_list_acl(l->list_id);
	    }
	}
	break;
    case 'U':
	if (!hash_lookup(users, l->acl_id)) {
	    printf("List %s has bad USER acl %d\n", l->name, l->acl_id);
	    if (single_fix("Patch", 1)) {
		fix_list_acl(l->list_id);
	    }
	}
	break;
    }
}

fix_list_acl(id)
int id;
##{
##  int rowcount, iid = id;
##  replace list (acl_id = iid, acl_type = "LIST") where list.list_id = iid
##  inquire_equel(rowcount = "rowcount")
    if (rowcount > 0)
      printf("%d entr%s fixed\n", rowcount, rowcount==1?"y":"ies");
    else
      printf("Not fixed\n");
    modified("list");
##}


show_member_list(id)
int id;
##{
##  int mid, iid = id, found = 1;
##  char mtype[9], *name = "";

##  retrieve (mtype = imembers.member_type, mid = imembers.member_id)
##	where imembers.list_id = iid and imembers.direct = 1 {
      strtrim(mtype);
      found = 0;
      if (mtype[0] == 'L')
	name = ((struct list *) hash_lookup(lists, mid))->name;
      else if (mtype[0] == 'U')
	name = ((struct user *) hash_lookup(users, mid))->login;
      else if (mtype[0] == 'S' || mtype[0] == 'K')
	name = ((struct string *) hash_lookup(strings, mid))->name;
      printf("Non-existant list %d has member %s %s\n", iid, mtype, name);
##  }
    return(found);
##}

show_mem_user(id)
int id;
##{
##  int lid, iid = id, found = 1;
##  char name[33];

##  retrieve (lid = imembers.list_id)
##	where imembers.member_id = iid and imembers.member_type = "USER" and
##	      imembers.direct = 1 {
      found = 0;
      printf("List %s has non-existant user member, id %d\n",
	     ((struct list *)hash_lookup(lists, lid))->name, iid);
##  }
    return(found);
##}

show_mem_list(id)
int id;
##{
##  int lid, iid = id, found = 1;
##  char name[33];

##  retrieve (lid = imembers.list_id)
##	where imembers.member_id = iid and imembers.member_type = "LIST" and
##	      imembers.direct = 1 {
      found = 0;
      printf("List %s has non-existant list member, id %d\n",
	     ((struct list *)hash_lookup(lists, lid))->name, iid);
##  }
    return(found);
##}

show_mem_str(id)
int id;
##{
##  int lid, iid = id, found = 1;
##  char name[33];

##  retrieve (lid = imembers.list_id)
##	where imembers.member_id = iid and imembers.member_type = "STRING" and
##	      imembers.direct = 1 {
      found = 0;
      printf("List %s has non-existant string member, id %d\n",
	     ((struct list *)hash_lookup(lists, lid))->name, iid);
##  }
    return(found);
##}


show_mem_krb(id)
int id;
##{
##  int lid, iid = id, found = 1;
##  char name[33];

##  retrieve (lid = imembers.list_id)
##	where imembers.member_id = iid and imembers.member_type = "KERBEROS" and
##	      imembers.direct = 1 {
      found = 0;
      printf("List %s has non-existant kerberos member, id %d\n",
	     ((struct list *)hash_lookup(lists, lid))->name, iid);
##  }
    return(found);
##}


##del_mem_user(id)
##int id;
##{
##  int rowcount;

##  delete imembers where imembers.member_type = "USER" and
##	imembers.member_id = id and imembers.direct = 1
##  inquire_equel(rowcount = "rowcount");
    if (rowcount > 0)
      printf("%d entr%s deleted\n", rowcount, rowcount==1?"y":"ies");
    else
      printf("Not deleted\n");
    modified("imembers");
##}

##del_mem_list(id)
##int id;
##{
##  int rowcount;

##  delete imembers where imembers.member_type = "LIST" and
##	imembers.member_id = id and imembers.direct = 1
##  inquire_equel(rowcount = "rowcount");
    if (rowcount > 0)
      printf("%d entr%s deleted\n", rowcount, rowcount==1?"y":"ies");
    else
      printf("Not deleted\n");
    modified("imembers");
##}

##del_mem_str(id)
##int id;
##{
##  int rowcount;

##  delete imembers where imembers.member_type = "STRING" and
##	imembers.member_id = id and imembers.direct = 1
##  inquire_equel(rowcount = "rowcount");
    if (rowcount > 0)
      printf("%d entr%s deleted\n", rowcount, rowcount==1?"y":"ies");
    else
      printf("Not deleted\n");
    modified("imembers");
##}


##del_mem_krb(id)
##int id;
##{
##  int rowcount;

##  delete imembers where imembers.member_type = "KERBEROS" and
##	imembers.member_id = id and imembers.direct = 1
##  inquire_equel(rowcount = "rowcount");
    if (rowcount > 0)
      printf("%d entr%s deleted\n", rowcount, rowcount==1?"y":"ies");
    else
      printf("Not deleted\n");
    modified("imembers");
##}


##show_sh(id)
##int id;
##{
##  char name[33];
    int found = 1;

##  retrieve (name = serverhosts.service) where serverhosts.mach_id = id {
      found = 0;
      printf("ServerHost entry for service %s non-existant host %d\n",
	     name, id);
##  }
    return(found);
##}

##del_sh_mach(id)
##int id;
##{
##  int rowcount;

##  delete serverhosts where serverhosts.mach_id = id
##  inquire_equel(rowcount = "rowcount");
    if (rowcount > 0)
      printf("%d entr%s deleted\n", rowcount, rowcount==1?"y":"ies");
    else
      printf("Not deleted\n");
    modified("serverhosts");
##}


static int fnchecklen;

fsmatch(id, n, f)
int id;
struct nfsphys *n;
struct filesys *f;
{
    if (n->mach_id == f->mach_id &&
	!strncmp(f->dir, n->dir, strlen(n->dir)) &&
	strlen(n->dir) > fnchecklen) {
	f->phys_id = id;
	fnchecklen = strlen(n->dir);
    }
}


##check_fs(id, f, hint)
##int id;
register struct filesys *f;
int hint;
##{
##  int id1, id2, id3, rowcount;
##  char *dir;
    struct nfsphys *n;
    struct machine *m;

    if (f->type == 'N' || f->type == 'R') {
	if (!hash_lookup(machines, f->mach_id)) {
	    printf("Filesys %s with bad machine %d\n", f->name, f->mach_id);
	    if (single_fix("Fix", 0)) {
##		replace filesys (mach_id = 0) where filesys.filsys_id = id
##		inquire_equel(rowcount = "rowcount")
		if (rowcount > 0)
		  printf("%d entr%s fixed\n",rowcount, rowcount==1?"y":"ies");
		else
		  printf("Not fixed\n");
		modified("filesys");
		f->mach_id = 0;
	    }
	}
    }
    if (f->type == 'N' || f->type == 'R' || f->type == 'A') {
	if (!hash_lookup(users, f->owner)) {
	    printf("Filesys %s with bad owning user %d\n", f->name, f->owner);
	    if (single_fix("Fix", 1)) {
		zero_fix("filesys", "owner", "filsys_id", f->filsys_id);
		f->owner = 0;
	    }
	}
	if (!hash_lookup(lists, f->owners)) {
	    printf("Filesys %s with bad owning group %d\n", f->name, f->owners);
	    if (single_fix("Fix", 1)) {
		zero_fix("filesys", "owners", "filsys_id", f->filsys_id);
		f->owners = 0;
	    }
	}
    }

    if (f->type == 'N') {
	if (!hash_lookup(nfsphys, f->phys_id)) {
	    m = (struct machine *)hash_lookup(machines, f->mach_id);
	    printf("Filesys %s with bad phys_id %d\n", f->name, f->phys_id);
	    if (single_fix("Fix", 1)) {
		fnchecklen = 0;
		hash_step(nfsphys, fsmatch, f);
		if (fnchecklen != 0) {
    		  id1 = f->phys_id;
		  id2 = f->filsys_id;
		  id3 = f->mach_id;
##		  replace filesys (phys_id = id1) where filesys.filsys_id = id2
##		  inquire_equel(rowcount = "rowcount")
		  if (rowcount > 0)
		    printf("%d entr%s fixed\n",rowcount, rowcount==1?"y":"ies");
		  else
		    printf("Not fixed\n");
		  modified("filesys");
		} else {
		    printf("No NFSphys exsits for %s:%s\n", m->name, f->dir);
		    if (single_fix("Create", 0)) {
			dir = f->dir;
			id1 = f->phys_id;
			id2 = f->filsys_id;
			id3 = f->mach_id;
			if (set_next_object_id("nfsphys_id", "nfsphys") !=
				MR_SUCCESS) {
			    printf("Unable to assign unique ID\n");
			    return;
			}
##			retrieve (id1 = values.value)
##			  where values.name = "nfsphys_id"
##			inquire_equel(rowcount = "rowcount")
			if (rowcount != 1) {
			    printf("Unable to retrieve unique ID\n");
			    return;
			}
##			append nfsphys (nfsphys_id = id1, mach_id = id3,
##					device = "???", #dir = dir, status = 0,
##					allocated = 0, size = 0,
##					modtime = "now", modby = 0,
##					modwith = "dbck")
##			inquire_equel(rowcount = "rowcount")
			if (rowcount > 0)
			  printf("%d entr%s created\n", rowcount,
				 rowcount==1?"y":"ies");
			else
			  printf("Not created\n");
			modified("nfsphys");
			n = (struct nfsphys *)malloc(sizeof(struct nfsphys));
			if (n == NULL)
			  out_of_mem("storing new nfsphys");
			strcpy(n->dir, dir);
			n->mach_id = id3;
			n->nfsphys_id = id1;
			n->allocated = 0;
			n->count = 0;
			hash_store(nfsphys, id1, n);
##			replace filesys (phys_id = id1)
##			  where filesys.filsys_id = id2
##			inquire_equel(rowcount = "rowcount")
			if (rowcount > 0)
			  printf("%d filesys entr%s fixed\n", rowcount,
				 rowcount==1?"y":"ies");
			else
			  printf("Not fixed\n");
			modified("filesys");
		    }
	        }
	    }
	}
    }
##}


check_nfsphys(id, n, hint)
int id;
struct nfsphys *n;
int hint;
{
    if (!hash_lookup(machines, n->mach_id)) {
	printf("NFSphys %d(%s) on non-existant machine %d\n",
	       id, n->dir, n->mach_id);
	if (single_fix("Delete", 0))
	  single_delete("nfsphys", "nfsphys_id", id);
    }
}

##show_fsg_missing(id)
##int id;
##{
##  int id1, found = 1;
    struct filesys *f;

##  retrieve (id1 = fsgroup.filsys_id) where fsgroup.group_id = id {
      found = 0;
      if (f = (struct filesys *) hash_lookup(filesys, id1))
	printf("Missing fsgroup %d has member filesystem %s\n", id, f->name);
      else
	printf("Missing fsgroup %d has member filesystem %d\n", id, id1);
##  }
    return(found);
##}

show_fsg_type(f)
struct filesys *f;
{
    char *t;

    switch (f->type) {
    case 'N':
	t = "NFS";
	break;
    case 'R':
	t = "RVD";
	break;
    case 'A':
	t = "AFS";
	break;
    case 'E':
	t = "ERR";
	break;
    case 'F':
	t = "FSGROUP";
	break;
    default:
	t = "???";
    }
    printf("FSGroup %s has type %s instead of FSGROUP\n", f->name, t);
    return(0);
}

fix_fsg_type(f)
struct filesys *f;
##{
##  int rowcount, id = f->filsys_id;

##  replace filesys (type = "FSGROUP") where filesys.filsys_id = id
##  inquire_equel(rowcount = "rowcount")
    if (rowcount > 0)
      printf("%d entr%s fixed\n", rowcount, rowcount==1?"y":"ies");
    else
      printf("Not fixed\n");
    modified("filesys");
##}

##show_fsg_nomember(id)
##int id;
##{
##  int id1, found = 1;
    struct filesys *f;

##  retrieve (id1 = fsgroup.group_id) where fsgroup.filsys_id = id {
      found = 0;
      if (f = (struct filesys *) hash_lookup(filesys, id1))
	printf("FSGroup %s has missing member %d\n", f->name, id);
      else
	printf("FSGroup %d has missing member %d\n", id1, id);
##  }
    return(found);
##}

##show_quota_nouser(id)
##int id;
##{
##  int id1, found = 1;

##  retrieve (id1 = nfsquota.filsys_id) where nfsquota.users_id = id {
      found = 0;
      printf("NFSquota on fs %d for non-existant user %d\n", id1, id);
##  }
    return(found);
##}

fix_quota_nouser(id)
{
    single_delete("nfsquota", "users_id", id);
}

##show_quota_nofs(id)
##int id;
##{
##  int id1, found = 1;
    struct user *u;

##  retrieve (id1 = nfsquota.users_id) where nfsquota.filsys_id = id {
      u = (struct user *)hash_lookup(users, id1);
      found = 0;
      printf("NFSquota for user %s on non-existant filesys %d\n", u->login, id);
##  }
    return(found);
##}

fix_quota_nofs(id)
{
    single_delete("nfsquota", "filsys_id", id);
}

##show_quota_wrongpid(id)
##int id;
##{
##  int id1, found = 1;
    struct user *u;
    struct filesys *f;

    f = (struct filesys *)hash_lookup(filesys, id);
##  retrieve (id1 = nfsquota.users_id) where nfsquota.filsys_id = id {
      u = (struct user *)hash_lookup(users, id1);
      found = 0;
      printf("NFSquota for user %s on filesys %s has wrong phys_id %d\n",
	     u->login, f->name, id);
##  }
    return(found);
##}

##fix_quota_physid(id)
##int id;
##{
##  int rowcount, id1;

    id1 = ((struct filesys *)hash_lookup(filesys, id))->phys_id;
##  replace nfsquota (phys_id = id1) where nfsquota.filsys_id = id and
##	nfsquota.phys_id != id1	
##  inquire_equel(rowcount = "rowcount")
    if (rowcount > 0)
      printf("%d entr%s fixed\n",rowcount, rowcount==1?"y":"ies");
    else
      printf("Not fixed\n");
    modified("nfsquota");
##}

##show_srv_user(id)
##int id;
##{
##  char name[33];
    int found = 1;

##  retrieve (name = s.#name) where s.acl_type = "USER" and s.acl_id = id {
	strtrim(name);
	printf("Service %s has acl non-existant user %d\n", name, id);
	found = 0;
##  }
    return(found);
##}

##show_srv_list(id)
##int id;
##{
##  char name[33];
    int found = 1;

##  retrieve (name = s.#name) where s.acl_type = "LIST" and s.acl_id = id {
	strtrim(name);
	printf("Service %s has acl non-existant list %d\n", name, id);
	found = 0;
##  }
    return(found);
##}

##zero_srv_user(id)
##int id;
##{
##  int rowcount;

##  replace servers (acl_id = 0) where servers.acl_id = id and
##	servers.acl_type = "USER"
##  inquire_equel(rowcount = "rowcount")
    if (rowcount > 0)
      printf("%d entr%s fixed\n", rowcount, rowcount==1?"y":"ies");
    else
      printf("Not fixed\n");
    modified("servers");
##}

##zero_srv_list(id)
##int id;
##{
##  int rowcount;

##  replace servers (acl_id = 0) where servers.acl_id = id and
##	servers.acl_type = "LIST"
##  inquire_equel(rowcount = "rowcount")
    if (rowcount > 0)
      printf("%d entr%s fixed\n", rowcount, rowcount==1?"y":"ies");
    else
      printf("Not fixed\n");
    modified("servers");
##}


##show_krb_usr(id)
##int id;
##{
##  int found = 1, id1;
    struct string *s;
    char *ss;

##  retrieve (id1 = krbmap.string_id) where krbmap.users_id = id {
	if (s = ((struct string *)hash_lookup(strings, id1)))
	  ss = s->name;
	else
	  ss = "[unknown]";
	found = 0;
	printf("Kerberos map for non-existant user %d to principal %s\n",
	       id, s);
##  }
    return(found);
##}


##show_krb_str(id)
##int id;
##{
##  int found = 1, id1;
    struct user *u;
    char *s;

##  retrieve (id1 = krbmap.users_id) where krbmap.string_id = id {
	if (u = ((struct user *)hash_lookup(users, id1)))
	  s = u->login;
	else
	  s = "[???]";
	found = 0;
	printf("Kerberos map for user %s (%d) to non-existant string %d\n",
	       s, id1, id);
##  }
    return(found);
##}


##show_pdm_mach(id)
##int id;
##{
##  char name[33];
    int found = 1;

##  retrieve (name = palladium.#name) where palladium.mach_id = id {
	strtrim(name);
        printf("Palladium server/supervisor %s is on non-existant machine %d\n",
	       name, id);
	found = 0;
##  }
    return(found);
##}


phase2()
##{
##  int id1, id2, id3, id4, id5;
##  char type[9], name[33];
    struct save_queue *sq, *sq1, *sq2, *sq3, *sq4, *sq5;
    struct filesys *f;
    struct list *l;
    struct nfsphys *n;
    struct machine *m;

    printf("Phase 2 - Checking references\n");

    dprintf("Checking users...\n");
    hash_step(users, pobox_check, NULL);

    dprintf("Checking mcmap...\n");
    sq1 = sq_create();
    sq2 = sq_create();
##  retrieve (id1 = mcmap.mach_id, id2 = mcmap.clu_id) {
	if (!(m = (struct machine *)hash_lookup(machines, id1)))
	  sq_save_unique_data(sq1, id1);
	if (!hash_lookup(clusters, id2))
	  sq_save_unique_data(sq2, id2);
	if (m) m->clucount++;
##  }
    generic_delete(sq1, show_mcm_mach, "mcmap", "mach_id", 1);
    generic_delete(sq2, show_mcm_clu, "mcmap", "clu_id", 1);

    dprintf("Checking service clusters...\n");
    sq1 = sq_create();
##  retrieve (id1 = svc.clu_id) {
    	if (!hash_lookup(clusters, id1))
	  sq_save_unique_data(sq1, id1);
##  }
    generic_delete(sq1, show_svc, "svc", "clu_id", 1);

    dprintf("Checking lists...\n");
    hash_step(lists, list_check, NULL);

    dprintf("Checking members...\n");
    sq1 = sq_create();
    sq2 = sq_create();
    sq3 = sq_create();
    sq4 = sq_create();
    sq5 = sq_create();
##  range of m is imembers
##  retrieve (id1 = m.list_id, type = m.member_type, id2 = m.member_id,
##	      id3 = m.ref_count, id4 = m.direct) {
	if ((l = (struct list *) hash_lookup(lists, id1)) == NULL)
	  sq_save_unique_data(sq1, id1);
	else if (type[0] == 'U' && !hash_lookup(users, id2))
	  sq_save_unique_data(sq2, id2);
	else if (type[0] == 'L' && !hash_lookup(lists, id2))
	  sq_save_unique_data(sq3, id2);
	else if (type[0] == 'S' && !string_check(id2))
	  sq_save_unique_data(sq4, id2);
	else if (type[0] == 'K' && !string_check(id2))
	  sq_save_unique_data(sq5, id2);
	else
	  l->members++;
##  }
    generic_delete(sq1, show_member_list, "imembers", "list_id", 1);
    generic_fix(sq2, show_mem_user, "Delete", del_mem_user, 1);
    generic_fix(sq3, show_mem_list, "Delete", del_mem_list, 1);
    generic_fix(sq4, show_mem_str, "Delete", del_mem_str, 1);
    generic_fix(sq5, show_mem_krb, "Delete", del_mem_krb, 1);

    dprintf("Checking servers...\n");
    sq1 = sq_create();
    sq2 = sq_create();
##  range of s is servers
##  retrieve (name = s.#name, type = s.acl_type, id1 = s.acl_id) {
	strtrim(type);
	if (!strcmp(type, "USER") && !hash_lookup(users, id1)) {
	    sq_save_data(sq1, id1);
	} else if (!strcmp(type, "LIST") && !hash_lookup(lists, id1)) {
	    sq_save_data(sq2, id1);
	}
##  }
    generic_fix(sq1, show_srv_user, "Fix", zero_srv_user, 1);
    generic_fix(sq2, show_srv_list, "Fix", zero_srv_list, 1);

    dprintf("Checking servershosts...\n");
    sq = sq_create();
##  retrieve (id1 = serverhosts.mach_id) {
    	if (!hash_lookup(machines, id1))
	  sq_save_data(sq, id1);
##  }
    generic_fix(sq, show_sh, "Delete", del_sh_mach, 0);

    dprintf("Checking nfsphys...\n");
    hash_step(nfsphys, check_nfsphys, NULL);

    dprintf("Checking filesys...\n");
    hash_step(filesys, check_fs, NULL);

    dprintf("Checking filesystem groups...\n");
    sq1 = sq_create();
    sq2 = sq_create();
    sq3 = sq_create();
##  retrieve (id1 = fsgroup.group_id, id2 = fsgroup.filsys_id) {
    	if (!(f = (struct filesys *) hash_lookup(filesys, id1)))
	  sq_save_data(sq1, id1);
	if (f->type  != 'F')
	  sq_save_data(sq2, f);
    	if (!hash_lookup(filesys, id2))
	  sq_save_data(sq3, id2);
##  }
    generic_delete(sq1, show_fsg_missing, "fsgroup", "group_id", 0);
    generic_fix(sq2, show_fsg_type, "Change to \"FSTYPE\"", fix_fsg_type, 0);
    generic_delete(sq3, show_fsg_nomember, "fsgroup", "filsys_id", 1);

    dprintf("Checking nfsquota...\n");
    sq1 = sq_create();
    sq2 = sq_create();
    sq3 = sq_create();
##  retrieve (id1 = nfsquota.users_id, id2 = nfsquota.filsys_id,
##	      id3 = nfsquota.phys_id, id4 = nfsquota.quota) {
	if (id1 != 0 && !hash_lookup(users, id1))
	  sq_save_data(sq1, id1);
	else if (!(f = (struct filesys *) hash_lookup(filesys, id2)))
	  sq_save_data(sq2, id2);
	else if (id3 != f->phys_id ||
	    ((n = (struct nfsphys*) hash_lookup(nfsphys, id3)) ==
	    (struct nfsphys *)NULL))
	  sq_save_data(sq3, id2);
	else
	  n->count += id4;
##  }
    generic_fix(sq1, show_quota_nouser, "Delete", fix_quota_nouser, 1);
    generic_fix(sq2, show_quota_nofs, "Delete", fix_quota_nofs, 0);
    generic_fix(sq3, show_quota_wrongpid, "Fix", fix_quota_physid, 1);

    dprintf("Not checking zephyr.\n");

    dprintf("Checking hostaccess...\n");
##  range of h is hostaccess
##  retrieve (id1 = h.mach_id, type = h.acl_type, id2 = h.acl_id) {
	strtrim(type);
	if (!hash_lookup(machines, id1)) {
	    printf("Hostaccess for non-existant host %d\n", id1);
	    printf("Not fixing this error\n");
	}
	if (!strcmp(type, "USER") && !hash_lookup(users, id2)) {
	    printf("Hostaccess for %d is non-existant user %d\n", id1, id2);
	    printf("Not fixing this error\n");
	} else if (!strcmp(type, "LIST") && !hash_lookup(lists, id2)) {
	    printf("Hostaccess for %d is non-existant list %d\n", id1, id2);
	    printf("Not fixing this error\n");
	}
##  }

    dprintf("Checking palladium...\n");
    sq1 = sq_create();
##  range of p is palladium
##  retrieve (id1 = p.mach_id) {
	if (!hash_lookup(machines, id1)) {
	    sq_save_unique_data(sq1, id1);
	}
##  }
    generic_delete(sq1, show_pdm_mach, "palladium", "mach_id", 1);

    dprintf("Checking krbmap...\n");
    sq1 = sq_create();
    sq2 = sq_create();
##  range of k is krbmap
##  retrieve (id1 = k.users_id, id2 = k.string_id) {
    	if (!hash_lookup(users, id1))
	  sq_save_unique_data(sq1, id1);
	if (!string_check(id2))
	  sq_save_unique_data(sq2, id2);
##  }
    generic_delete(sq1, show_krb_usr, "krbmap", "users_id", 1);
    generic_delete(sq2, show_krb_str, "krbmap", "string_id", 1);

    dprintf("Checking capacls...\n");
##  retrieve (id1 = capacls.list_id, name = capacls.tag) {
	if (!hash_lookup(lists, id1)) {
	    printf("Capacl for %s is non-existant list %d\n", name, id1);
	    printf("Not fixing this error\n");
	}
##  }

##}

