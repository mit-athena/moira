/*  (c) Copyright 1988 by the Massachusetts Institute of Technology. */
/*  For copying and distribution information, please see the file */
/*  <mit-copyright.h>. */

#include <mit-copyright.h>
#include <stdio.h>

char *malloc(), *strsave();

struct np {
    struct np *next; /* this must be the first field */
    int	id;
    char *dir;
};
struct np **nfsphys = NULL;

int match_phys(mach, directory)
int mach;
char *directory;
{
    struct np *p, *n;

    if (nfsphys == NULL)
##    {
##	int id, maxmach, mid;
##	char dir[65];

	printf("Building NFSphys table\n");
##	range of n is nfsphys
##	retrieve (maxmach = max(machine.mach_id))
	maxmach++;
	nfsphys = (struct np **) malloc(maxmach * sizeof(struct np *));
	bzero(nfsphys, maxmach * sizeof(struct np *));
##	retrieve (id = n.nfsphys_id, mid = n.mach_id, dir = n.#dir)
##		sort by #dir:d {
	    n = (struct np *) malloc(sizeof(struct np));
	    n->next = NULL;
	    n->id = id;
	    n->dir = strsave(strtrim(dir));
	    p = (struct np *)&nfsphys[mid];
	    while (p->next)
	      p = p->next;
	    p->next = n;
##	}
##    }

    if ((n = nfsphys[mach]) == NULL)
      return(0);
    while (n) {
	if (!strncmp(directory, n->dir, strlen(n->dir)))
	  return(n->id);
	n = n->next;
    }
    return(0);
}


##int match_filsys(mach, device, user)
##int mach;
##char *device;
##int user;
##{
##  int mid, fid;
##  char dev[33];

##  repeat retrieve (fid = f.filsys_id, mid = f.mach_id, dev = n.#device)
##	where f.label = u.login and n.nfsphys_id = f.phys_id and
##		u.users_id = @user
    if (mid == mach && !strcmp(device, strtrim(dev)))
      return(fid);
    fid = 0;
##  repeat retrieve unique (fid = f.filsys_id) where f.mach_id = @mach and
##	f.name = @device
    if (fid == 0) {
##	repeat retrieve unique (fid = f.filsys_id) where f.mach_id = @mach and
##		f.phys_id = n.nfsphys_id and n.#device = @device
    }
    return(fid);
##}
