/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dbck/phase4.qc,v 1.1 1988-09-25 22:02:34 mar Exp $
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include "dbck.h"


count_boxes(id, u, boxes)
int id;
struct user *u;
struct hash *boxes;
{
    int i;

    if (u->potype == 'P') {
	if (i = (int) hash_lookup(boxes, u->pobox_id))
	  hash_store(boxes, u->pobox_id, i+1);
	else
	  printf("User %s(%s) has pobox on non-POP server %d\n",
		 u->fullname, u->login, u->pobox_id);
    }
}


##check_box_counts(id, cnt, counts)
##int id, cnt;
struct hash *counts;
##{
##  int newval, rowcount, error;

    newval = (int) hash_lookup(counts, id);
    if (newval != cnt - 1) {
	printf("Count wrong on POBox machine %s; is %d in db, counted %d\n",
	       ((struct machine *) hash_lookup(machines, id))->name,
	       newval, cnt - 1);
	if (single_fix("Update", 1)) {
##	    range of s is serverhosts
##    	    replace s (value1 = newval) where
##		s.service = "POP" and s.mach_id = id
##	    inquire_equel(rowcount = "rowcount", error = "errorno")
	    if (error != 0)
	      printf("Error = %d\n", error);
	    if (rowcount > 0)
	      printf("%d entr%s fixed\n", rowcount, rowcount==1?"y":"ies");
	    else
	      printf("Not fixed\n");
	}
    }
##}


##check_nfs_counts(id, n, hint)
##int id, hint;
struct nfsphys *n;
##{
##  int val, rowcount;

    val = n->count;
    if (n->allocated != val) {
	printf("Count wrong on NFSphys %s:%s; is %d in db, counted %d\n",
	       ((struct machine *) hash_lookup(machines, n->mach_id))->name,
	       n->dir, n->allocated, val);
	if (single_fix("Update", 1)) {
##	    replace nfsphys (allocated = val) where nfsphys.nfsphys_id = id
##	    inquire_equel(rowcount = "rowcount")
	    if (rowcount > 0)
	      printf("%d entr%s fixed\n", rowcount, rowcount==1?"y":"ies");
	    else
	      printf("Not fixed\n");
	}
    }
##}


phase4()
##{
    struct hash *boxes, *counts;
##  int id, cnt;

    printf("Phase 4 - Checking counts\n");

    dprintf("Doing POBoxes...\n");
    boxes = create_hash(10);
    counts = create_hash(10);
##  retrieve (id = serverhosts.mach_id, cnt = serverhosts.value1)
##	where serverhosts.service = "POP" {
      hash_store(boxes, id, 1);
      hash_store(counts, id, cnt);
##  }
    hash_step(users, count_boxes, boxes);
    hash_step(boxes, check_box_counts, counts);

    dprintf("Doing NFSphys...\n");
    hash_step(nfsphys, check_nfs_counts, 0);
##}


count_only_setup()
##{
##  int id, status, id2, id3;
##  char name[33], last[17], first[17], buf[257];
    struct save_queue *sq;
    struct user *u;
    struct nfsphys *n;
    struct machine *m;

    dprintf("Loading users...\n");
    users = create_hash(10000);
##  range of u is users
##  retrieve (id = u.users_id, name = u.login, last = u.#last,
##	      first = u.#first, status = u.#status, buf = u.potype,
##	      id2 = u.pop_id, id3 = u.box_id) 
##    where u.potype != "NONE" {
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
	hash_store(users, id, u);
##  }

    dprintf("Loading machines...\n");
    machines = create_hash(1000);
##  range of m is machine
##  retrieve (id = m.mach_id, name = m.#name) {
    	m = (struct machine *) malloc(sizeof(struct machine));
	strcpy(m->name, strtrim(name));
	m->mach_id = id;
	hash_store(machines, id, m);
##  }

    dprintf("Loading nfsphys...\n");
    nfsphys = create_hash(500);
##  retrieve (id = nfsphys.nfsphys_id, name = nfsphys.dir,
##	      id2 = nfsphys.mach_id, id3 = nfsphys.allocated) {
    	n = (struct nfsphys *) malloc(sizeof(struct nfsphys));
	strcpy(n->dir, strtrim(name));
	n->mach_id = id2;
	n->nfsphys_id = id;
	n->allocated = id3;
	n->count = 0;
	hash_store(nfsphys, id, n);
##  }

    dprintf("Counting quotas...\n");
##  retrieve (id = nfsquota.phys_id, id2 = nfsquota.quota) {
        if (n = (struct nfsphys  *) hash_lookup(nfsphys, id)) {
	    n->count += id2;
	}
##  }
##}
