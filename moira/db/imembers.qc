
#include <stdio.h>
#include <sms.h>

struct mem {
    int parent;
    int child;
    short ref;
    char direct;
    char ctype;
};


main()
{
    struct save_queue *sq, *load_members();

##  ingres sms
    printf("Loading LIST members...\n");
    sq = load_members();
    printf("Computing LIST memberships...\n");
    do_members(sq);
    printf("Computing full memberships...\n");
    do_all_members(sq);
    printf("Done\n");
    exit(0);
}

struct save_queue *load_members()
##{
    register struct mem *m;
    struct mem *new_mem();
    struct save_queue *sq;
##  int lid, mid;
##  char mtype[9];

    sq = sq_create();
##  range of m is members
##  retrieve (lid = m.list_id, mid = m.member_id, mtype = m.member_type) {
    	m = new_mem();
	m->parent = lid;
	m->child = mid;
	m->ref = 0;
	m->direct = 1;
	m->ctype = mtype[0];
	sq_save_data(sq, m);
##  }
    return(sq);
##}

struct mem *new_mem()
{
    static int count = 0;
    static struct mem *m = NULL;
    struct mem *ret;

    if (m == NULL || count <= 0) {
	m = malloc(100 * sizeof(struct mem));
	if (m == NULL)
	  printf("Out of memory!\n");
	count = 100;
    }
    ret = m++;
    count--;
    return(ret);
}


#define MAXLINK 100

do_members(sq)
struct save_queue *sq;
##{
    struct mem *m;
##  int cid, pid, id, exists;
    int descendants[MAXLINK], ancestors[MAXLINK];
    int dcount, acount, a, d;

##  range of m is imembers
    while (sq_get_data(sq, &m)) {
	if (m->ctype != 'L')
	  continue;
	cid = m->child;
	pid = m->parent;
	dcount = 0;
	descendants[dcount++] = cid;
##	repeat retrieve (id = m.member_id) 
##	    where m.list_id = @cid {
	    descendants[dcount++] = id;
##	}
	acount = 0;
	ancestors[acount++] = pid;
##	repeat retrieve (id = m.list_id)
##	    where m.member_id = @pid {
	    ancestors[acount++] = id;
##	}
	if (dcount >= MAXLINK | acount >= MAXLINK)
	  printf("too many links working on %d -> %d\n", cid, pid);
	for (d = 0; d < dcount; d++) {
	    for (a = 0; a < acount; a++) {
		if ((cid = descendants[d]) == (pid = ancestors[a]))
		  printf("LOOP! on %d\n", cid);
##		repeat retrieve (exists = any(m.ref_count where m.list_id = @pid
##					      and m.member_id = @cid 
##					      and m.member_type = "LIST"))
		if (exists) {
		    if (a == 0 && d == 0)
##		      replace m (ref_count = m.ref_count+1, direct = 1)
##			where m.list_id = pid and m.member_id = cid
		    else
##		      replace m (ref_count = m.ref_count+1)
##			where m.list_id = pid and m.member_id = cid
		} else {
		    if (a == 0 && d == 0)
##		      append imembers (list_id=pid, member_id = cid, direct = 1,
##				       member_type="LIST", ref_count = 1)
		    else
##		      append imembers (list_id=pid, member_id = cid,
##				       member_type="LIST", ref_count = 1)
		}
	    }
	}
    }
##}



do_all_members(sq)
struct save_queue *sq;
##{
    struct mem *mem;
    register struct mem *m;
##  int cid, pid, id, exists;
##  char *ctype;
    int descendants[MAXLINK], ancestors[MAXLINK];
    int dcount, acount, a, d;

##  range of m is imembers
    sq->q_lastget = 0;			/* rewind queue */
    while (sq_get_data(sq, &mem)) {
	m = mem;
	switch (m->ctype) {
	case 'L':
	    continue;
	case 'S':
	    ctype = "STRING";
	    break;
	case 'U':
	    ctype = "USER";
	    break;
	case 'K':
	    ctype = "KERBEROS";
	    break;
	default:
	    printf("bad type %c on member %d\n", m->ctype, m->child);
	}

	cid = m->child;
	pid = m->parent;
	acount = 0;
	ancestors[acount++] = pid;
##	repeat retrieve (id = m.list_id)
##	    where m.member_id = @pid and m.member_type = "LIST" {
	    ancestors[acount++] = id;
##	}
	if (acount >= MAXLINK)
	  printf("too many links working on %d -> %d\n", cid, pid);
	for (a = 0; a < acount; a++) {
	    pid = ancestors[a];
##	    repeat retrieve (exists = any(m.ref_count where m.list_id = @pid
##					  and m.member_id = @cid 
##					  and m.member_type = @ctype))
  	    if (exists) {
		if (a == 0)
##		    replace m (ref_count = m.ref_count+1, direct = 1)
##			where m.list_id = pid and m.member_id = cid
		else
##		    replace m (ref_count = m.ref_count+1)
##			where m.list_id = pid and m.member_id = cid
	    } else {
		if (a == 0)
##		    append imembers (list_id=pid, member_id = cid, direct = 1,
##				     member_type=ctype, ref_count = 1)
		else
##		    append imembers (list_id=pid, member_id = cid,
##				     member_type=ctype, ref_count = 1)
	    }
	}
    }
##}
