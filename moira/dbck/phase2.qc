/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dbck/phase2.qc,v 1.1 1988-09-25 21:57:55 mar Exp $
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <stdio.h>
#include <sms.h>
#include "dbck.h"

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
##}


show_member_list(id)
int id;
##{
##  int mid, iid = id, found = 1;
##  char mtype[9], *name;

##  retrieve (mtype = member.member_type, mid = member.member_id)
##	where member.list_id = iid {
      strtrim(mtype);
      found = 0;
      if (mtype[0] == 'L')
	strcpy(name, ((struct list *) hash_lookup(lists, mid))->name);
      else if (mtype[0] == 'U')
	strcpy(name, ((struct user *) hash_lookup(users, mid))->login);
      else if (mtype[0] == 'S')
	strcpy(name, ((struct string *) hash_lookup(strings, mid))->name);
      printf("Non-existant list %d has member %s %s\n", iid, mtype, name);
##  }
    return(found);
##}

show_mem_user(id)
int id;
##{
##  int lid, iid = id, found = 1;
##  char name[33];

##  retrieve (lid = members.list_id)
##	where members.member_id = iid and members.member_type = "USER" {
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

##  retrieve (lid = members.list_id)
##	where members.member_id = iid and members.member_type = "LIST" {
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

##  retrieve (lid = members.list_id)
##	where members.member_id = iid and members.member_type = "STRING" {
      found = 0;
      printf("List %s has non-existant string member, id %d\n",
	     ((struct list *)hash_lookup(lists, lid))->name, iid);
##  }
    return(found);
##}


##del_mem_user(id)
##int id;
##{
##  int rowcount;

##  delete members where members.member_type = "USER" and
##	members.member_id = id
##  inquire_equel(rowcount = "rowcount");
    if (rowcount > 0)
      printf("%d entr%s deleted\n", rowcount, rowcount==1?"y":"ies");
    else
      printf("Not deleted\n");
##}

##del_mem_list(id)
##int id;
##{
##  int rowcount;

##  delete members where members.member_type = "LIST" and
##	members.member_id = id
##  inquire_equel(rowcount = "rowcount");
    if (rowcount > 0)
      printf("%d entr%s deleted\n", rowcount, rowcount==1?"y":"ies");
    else
      printf("Not deleted\n");
##}

##del_mem_str(id)
##int id;
##{
##  int rowcount;

##  delete members where members.member_type = "STRING" and
##	members.member_id = id
##  inquire_equel(rowcount = "rowcount");
    if (rowcount > 0)
      printf("%d entr%s deleted\n", rowcount, rowcount==1?"y":"ies");
    else
      printf("Not deleted\n");
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
		f->mach_id = 0;
	    }
	}
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
		} else {
		    printf("No NFSphys exsits for %s:%s\n", m->name, f->dir);
		    if (single_fix("Create", 0)) {
			dir = f->dir;
			id1 = f->phys_id;
			id2 = f->filsys_id;
			id3 = f->mach_id;
			if (set_next_object_id("nfsphys_id", "nfsphys") !=
				SMS_SUCCESS) {
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
			n = (struct nfsphys *)malloc(sizeof(struct nfsphys));
			strcpy(n->dir, dir);
			n->mach_id = id3;
			n->nfsphys_id = id1;
			n->allocated = 0;
			n->count = 0;
			hash_store(nfsphys, id1, n);
##			replace filesys (phys_id = id1)
##			  where filesys.filsys_id = id
##			inquire_equel(rowcount = "rowcount")
			if (rowcount > 0)
			  printf("%d filesys entr%s fixed\n", rowcount,
				 rowcount==1?"y":"ies");
			else
			  printf("Not fixed\n");
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
##}


phase2()
##{
##  int id1, id2, id3, id4, id5;
##  char type[9], name[33];
    struct save_queue *sq, *sq1, *sq2, *sq3, *sq4;
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
	m->clucount++;
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
##  range of m is members
##  retrieve (id1 = m.list_id, type = m.member_type, id2 = m.member_id) {
	strtrim(type);
	if ((l = (struct list *) hash_lookup(lists, id1)) == NULL)
	  sq_save_unique_data(sq1, id1);
	l->members++;
	if (!strcmp(type, "USER") && !hash_lookup(users, id2))
	  sq_save_unique_data(sq2, id2);
	else if (!strcmp(type, "LIST") && !hash_lookup(lists, id2))
	  sq_save_unique_data(sq3, id2);
	else if (!strcmp(type, "STRING") && !string_check(id2))
	  sq_save_unique_data(sq4, id2);
##  }
    generic_delete(sq1, show_member_list, "members", "list_id", 1);
    generic_fix(sq2, show_mem_user, "Delete", del_mem_user, 1);
    generic_fix(sq3, show_mem_list, "Delete", del_mem_list, 1);
    generic_fix(sq4, show_mem_str, "Delete", del_mem_str, 1);

    dprintf("Checking servers...\n");
##  range of s is servers
##  retrieve (name = s.#name, type = s.acl_type, id1 = s.acl_id) {
	strtrim(type);
	if (!strcmp(type, "USER") && !hash_lookup(users, id1))
	  printf("Service %s has acl non-existant user %d\n", name, id1);
	else if (!strcmp(type, "LIST") && !hash_lookup(lists, id1))
	  printf("Service %s has acl non-existant list %d\n", name, id1);
##  }

    dprintf("Checking servershosts...\n");
    sq = sq_create();
##  retrieve (id1 = serverhosts.mach_id) {
    	if (!hash_lookup(machines, id1))
	  sq_save_data(sq, id1);
##  }
    generic_fix(sq, show_sh, "Delete", del_sh_mach, 1);

    dprintf("Checking nfsphys...\n");
    hash_step(nfsphys, check_nfsphys, NULL);

    dprintf("Checking filesys...\n");
    hash_step(filesys, check_fs, NULL);

    dprintf("Checking nfsquota...\n");
    sq1 = sq_create();
    sq2 = sq_create();
    sq3 = sq_create();
##  retrieve (id1 = nfsquota.users_id, id2 = nfsquota.filsys_id,
##	      id3 = nfsquota.phys_id, id4 = nfsquota.quota) {
	if (!hash_lookup(users, id1))
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
	if (!hash_lookup(machines, id1))
	  printf("Hostaccess for non-existant host %d\n", id1);
	if (!strcmp(type, "USER") && !hash_lookup(users, id2))
	  printf("Hostaccess for %d is non-existant user %d\n", id1, id2);
	else if (!strcmp(type, "LIST") && !hash_lookup(lists, id2))
	  printf("Hostaccess for %d is non-existant list %d\n", id1, id2);
##  }

    dprintf("Checking capacls...\n");
##  retrieve (id1 = capacls.list_id, name = capacls.tag) {
	if (!hash_lookup(lists, id1))
	  printf("Capacl for %s is non-existant list %d\n", name, id1);
##  }

##}

