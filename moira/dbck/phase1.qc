/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dbck/phase1.qc,v 1.2 1988-09-26 14:07:09 mar Exp $
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include "dbck.h"

static char phase1_qc_rcsid[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dbck/phase1.qc,v 1.2 1988-09-26 14:07:09 mar Exp $";


show_user_id(u)
struct user *u;
{
    printf("User %s (%s, status %d) has duplicate ID\n",
	   u->login, u->fullname, u->status);
    return(0);
}

handle_duplicate_logins(sq)
struct save_queue *sq;
{
    struct user *u, *uu, *tmp;

    uu = (struct user *)0;
    while (sq_get_data(sq, &u)) {
	if (!strcmp(u->login, uu->login)) {
	    if (uu->status == 1 || u->status == 0) {
		tmp = u;
		u = uu;
		uu = tmp;
	    }
	    printf("User %s (%s, status %d) and\n",
		   u->login, u->fullname, u->status);
	    printf("User %s (%s, status %d) have duplicate logins\n",
		   uu->login, uu->fullname, uu->status);
	    if (!strcmp(u->fullname, uu->fullname) &&
		single_fix("Delete the second one")) {
		single_delete("users", "users_id", uu->users_id);
	    } else if (single_fix("Unregister the second one"))
##	      {
##    		int id = uu->users_id, rowcount;

##		replace users (login = "#"+text(users.uid), status = 0)
##		    where users.users_id = id
##		inquire_equel(rowcount = "rowcount")
    		if (rowcount > 0)
		  printf("%d entr%s fixed\n", rowcount, rowcount==1?"y":"ies");
		else
		  printf("Not fixed\n");
		modified("users");
##	    }
	} else {
	    uu = u;
	}
    }
}

fix_user_id(u)
struct user *u;
{
    u->users_id = generic_fix_id("users", "users_id", "login",
				 u->users_id, u->login);
}


cant_fix(id)
int id;
{
    printf("Sorry, don't know how to fix that\n");
}

show_mach_id(m)
struct machine *m;
{
    printf("Machine %s has duplicate ID %d\n", m->name, m->mach_id);
    return(0);
}

show_mach_name(m)
struct machine *m;
{
    printf("Machine %s (%d) has duplicate name\n", m->name, m->mach_id);
    return(0);
}

fix_mach_id(m)
struct machine *m;
{
    m->mach_id = generic_fix_id("machine", "mach_id", "name",
				m->mach_id, m->name);
}

show_clu_id(c)
struct cluster *c;
{
    printf("Cluster %s has duplicate ID %d\n", c->name, c->clu_id);
    return(0);
}

show_clu_name(c)
struct cluster *c;
{
    printf("Cluster %s (%d) has duplicate name\n", c->name, c->clu_id);
    return(0);
}

fix_clu_id(c)
struct cluster *c;
{
    c->clu_id = generic_fix_id("cluster", "clu_id", "name", c->clu_id, c->name);
}

show_list_id(l)
struct list *l;
{
    printf("List %s has duplicate ID %d\n", l->name, l->list_id);
    return(0);
}

show_list_name(l)
struct list *l;
{
    printf("List %s (%d) has duplicate name\n", l->name, l->list_id);
    return(0);
}

fix_list_id(l)
struct list *l;
{
    l->list_id = generic_fix_id("list", "list_id", "name", l->list_id, l->name);
}

show_fs_id(f)
struct filesys *f;
{
    printf("Filesys %s has duplicate ID %d\n", f->name, f->filsys_id);
    return(0);
}

fix_fs_id(f)
struct filesys *f;
{
    f->filsys_id = generic_fix_id("filesys", "filsys_id", "label",
				  f->filsys_id, f->name);
}


show_np_id(n)
struct nfsphys *n;
{
    printf("NfsPhys %s:%s has duplicate ID %d\n",
	   ((struct machine *)hash_lookup(machines, n->mach_id))->name,
	   n->dir, n->nfsphys_id);
    return(0);
}

fix_np_id(n)
struct nfsphys *n;
{
    n->nfsphys_id = generic_fix_id("nfsphys", "nfsphys_id", "dir",
				   n->nfsphys_id, n->dir);
}

show_str_id(s)
struct string *s;
{
    printf("String %s has duplicate ID %d\n", s->name, s->string_id);
    return(0);
}


phase1()
##{
##  char name[33], name1[33], last[17], first[17], buf[257];
##  int id, id2, id3, aid, aid2, status;
    struct save_queue *sq;
    struct user *u;
    struct machine *m;
    struct list *l;
    struct cluster *c;
    struct string *s;
    struct filesys *f;
    struct nfsphys *n;

    printf("Phase 1 - Looking for duplicates\n");

    dprintf("Loading users...\n");
    sq = sq_create();
    users = create_hash(10000);
##  range of u is users
##  retrieve (id = u.users_id, name = u.login, last = u.#last,
##	      first = u.#first, status = u.#status, buf = u.potype,
##	      id2 = u.pop_id, id3 = u.box_id) {
    	u = (struct user *) malloc(sizeof(struct user));
	strcpy(u->login, strtrim(name));
	u->potype = buf[0];
	sprintf(buf, "%s, %s", strtrim(last), strtrim(first));
	u->fullname = strsave(buf);
	u->status = status;
	u->users_id = id;
	switch (u->potype) {
	case 'P':
	    u->pobox_id = id2;
	    break;
	case 'S':
	    u->pobox_id = id3;
	    break;
	default:
	    u->pobox_id = 0;
	}
	if (hash_store(users, id, u)) {
	    sq_save_data(sq, hash_lookup(users, id));
	    sq_save_data(sq, u);
	}
##  }
    generic_fix(sq, show_user_id, "Change ID", fix_user_id, 0);

    if (!fast) {
	sq = sq_create();
##	retrieve (id = u.users_id)
##	    where u.login = users.login and u.tid != users.tid {
	  sq_save_data(sq, hash_lookup(users, id));
##	}
	handle_duplicate_logins(sq);
    }

    dprintf("Loading machines...\n");
    machines = create_hash(1000);
    sq = sq_create();
##  range of m is machine
##  retrieve (id = m.mach_id, name = m.#name) {
    	m = (struct machine *) malloc(sizeof(struct machine));
	strcpy(m->name, strtrim(name));
	m->mach_id = id;
	m->clucount = 0;
	if (hash_store(machines, id, m)) {
	    sq_save_data(sq, hash_lookup(machines, id));
	    sq_save_data(sq, m);
	}
##  }
    generic_fix(sq, show_mach_id, "Change ID", fix_mach_id, 0);

    if (!fast) {
	sq = sq_create();
##	retrieve (id = m.mach_id)
##	    where m.#name = machine.#name and m.tid != machine.tid {
	  sq_save_data(sq, hash_lookup(machines, id));
##	}
	generic_fix(sq, show_mach_name, "Change name", cant_fix, 0);
    }	

    dprintf("Loading clusters...\n");
    sq = sq_create();
    clusters = create_hash(100);
##  range of c is cluster
##  retrieve (id = cluster.clu_id, name = cluster.#name) {
    	c = (struct cluster *) malloc(sizeof(struct cluster));
	strcpy(c->name, strtrim(name));
	c->clu_id = id;
	if (hash_store(clusters, id, c)) {
	    sq_save_data(sq, hash_lookup(clusters, id));
	    sq_save_data(sq, c);
	}
##  }
    generic_fix(sq, show_clu_id, "Change ID", fix_clu_id, 0);

    if (!fast) {
	sq = sq_create();
##	retrieve (id = c.clu_id)
##	    where c.#name = cluster.#name and c.tid != cluster.tid {
    	  sq_save_data(sq, hash_lookup(clusters, id));
##	}
	generic_fix(sq, show_clu_name, "Change name", cant_fix, 0);
    }

    dprintf("Loading lists...\n");
    sq = sq_create();
    lists = create_hash(10000);
##  range of l is list
##  retrieve (id = l.list_id, name = l.#name,
##	      aid = l.acl_id, buf = l.acl_type) {
    	l = (struct list *) malloc(sizeof(struct list));
	strcpy(l->name, strtrim(name));
	l->acl_type = buf[0];
	l->acl_id = aid;
	l->list_id = id;
	l->members = 0;
	if (hash_store(lists, id, l)) {
	    sq_save_data(sq, hash_lookup(lists, id));
	    sq_save_data(sq, l);
	}
##  }
    generic_fix(sq, show_list_id, "Change ID", fix_list_id, 0);

    if (!fast) {
	sq = sq_create();
##	retrieve (id = l.list_id)
##	    where l.#name = list.#name and l.tid != list.tid {
	  sq_save_data(sq, hash_lookup(lists, id));
##	}
	generic_fix(sq, show_list_name, "Change name", cant_fix, 0);
    }

    dprintf("Loading filesys...\n");
    sq = sq_create();
    filesys = create_hash(10000);
##  retrieve (id = filesys.filsys_id, name = filesys.label, aid = filesys.owner,
##	      aid2 = filesys.owners, id2 = filesys.phys_id,
##	      id3 = filesys.mach_id, buf = filesys.type,
##	      name1 = filesys.#name) {
    	f = (struct filesys *) malloc(sizeof(struct filesys));
	strcpy(f->name, strtrim(name));
	strcpy(f->dir, strtrim(name1));
	f->filsys_id = id;
	f->owner = aid;
	f->owners = aid2;
	f->phys_id = id2;
	f->mach_id = id3;
	f->type = buf[0];
	if (hash_store(filesys, id, f)) {
	    sq_save_data(sq, hash_lookup(filesys, id));
	    sq_save_data(sq, f);
	}
##  }
    generic_fix(sq, show_fs_id, "Change ID", fix_fs_id, 0);

    dprintf("Loading nfsphys...\n");
    sq = sq_create();
    nfsphys = create_hash(500);
##  retrieve (id = nfsphys.nfsphys_id, name = nfsphys.dir,
##	      id2 = nfsphys.mach_id, id3 = nfsphys.allocated) {
    	n = (struct nfsphys *) malloc(sizeof(struct nfsphys));
	strcpy(n->dir, strtrim(name));
	n->mach_id = id2;
	n->nfsphys_id = id;
	n->allocated = id3;
	n->count = 0;
	if (hash_store(nfsphys, id, n)) {
	    sq_save_data(sq, hash_lookup(nfsphys, id));
	    sq_save_data(sq, n);
	}
##  }
    generic_fix(sq, show_np_id, "Change ID", fix_np_id, 0);

    dprintf("Loading strings...\n");
    sq = sq_create();
    strings = create_hash(5000);
##  range of s is strings
##  retrieve (id = s.string_id, buf = s.string) {
    	s = (struct string *) malloc(sizeof(struct string));
	s->name = strsave(strtrim(buf));
	s->string_id = id;
	s->refc = 0;
	if (hash_store(strings, id, s)) {
	    sq_save_data(sq, hash_lookup(strings, id));
	    sq_save_data(sq, s);
	}
##  }
    generic_delete(sq, show_str_id, "strings", "string_id", 0);

    if (!fast) {
##	retrieve (id = s.string_id, buf = s.string)
##	    where s.string = strings.string and s.tid != strings.tid {
	  printf("String %s(%d) is a duplicate!\n", strtrim(buf), id);
##	}
    }
##}

