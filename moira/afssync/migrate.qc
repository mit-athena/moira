/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/migrate.qc,v 1.1 1992-06-16 11:31:43 mar Exp $ */

#include <stdio.h>
#include <ctype.h>


main(argc, argv)
int argc;
char **argv;
{
    int debug = 0;
    FILE *input = stdin;
    char buf[1024], junk[129];
##  int cell, quota, cnt, mach, oquota, id, who;
##  char name[81], type[9], volume[81], path[129], quotas[33];
##  char oname[129],otype[9];
    char **arg = argv, *first_indir(), *second_indir(), *course_indir();

    cnt = 0;

    while (++arg - argv < argc) {
	if (**arg == '-') {
	    switch ((*arg)[1]) {
	    case 'd':
		debug = atoi((*arg)[2] ? *arg+2 : *++arg);
		break;
	    default:
		fprintf(stderr, "Usage: %s [-d level] [inputfile]\n",
			argv[0]);
		exit(1);
	    }
	} else {
	    input = fopen(*arg, "r");
	    if (input == NULL) {
		fprintf(stderr, "Unable to open in put file %s\n", *arg);
		exit(1);
	    }
	}
    }

    if (debug > 4)
      setenv("ING_SET", "set printqry");

    if (debug) { printf("Opening database..."); fflush(stdout);}
##  ingres sms
    if (debug) printf("done.\n");

##  range of f is filesys
##  range of m is machine
##  range of q is #quota
##  retrieve (cell = machine.mach_id) where machine.#name="ATHENA.MIT.EDU"
##  retrieve (who = users.users_id) where users.login="root"
##  begin transaction

    while (!feof(input)) {
	fgets(buf, sizeof(buf), input);
	if (sscanf(buf, "%s %s %s %s %s %s %s %s %s %s %s %s",
		   name, type, junk, junk, junk, junk,
		   quotas, junk, junk, junk, junk, junk) < 12) {
	    fprintf(stderr, "unable to parse line: %s\n", buf);
	    continue;
	}
	quota = atoi(quotas);
	if (!strcmp(type, "HOMEDIR")) {
	    sprintf(path, "/afs/athena.mit.edu/user/%s/%s/%s",
		    first_indir(name), second_indir(name), name);
	} else if (!strcmp(type, "COURSE")) {
	    sprintf(path, "/afs/athena.mit.edu/course/%s/%s",
		    course_indir(name), name);
	} else if (!strcmp(type, "ACTIVITY")) {
	    sprintf(path, "/afs/athena.mit.edu/activity/%s/%s",
		    first_indir(name), name);
	} else if (!strcmp(type, "APROJ")) {
	    sprintf(path, "/afs/athena.mit.edu/astaff/project/%s",
		    name);
	} else if (!strcmp(type, "PROJECT")) {
	    sprintf(path, "/afs/athena.mit.edu/project/%s",
		    name);
	} else if (!strcmp(type, "AREF")) {
	    sprintf(path, "/afs/athena.mit.edu/astaff/reference/%s",
		    name);
	} else if (!strcmp(type, "REF")) {
	    sprintf(path, "/afs/athena.mit.edu/reference/%s",
		    name);
	} else if (!strcmp(type, "CONTRIB")) {
	    sprintf(path, "/afs/athena.mit.edu/contrib/%s",
		    name);
	} else if (!strcmp(type, "SW")) {
	    sprintf(path, "/afs/athena.mit.edu/software/%s",
		    name);
	} else if (!strcmp(type, "SYSTEM")) {
	    sprintf(path, "/afs/athena.mit.edu/system/%s",
		    name);
	} else {
	    fprintf(stderr, "Don't know how to deal with type %s\n", type);
	    continue;
	}
##	repeat retrieve (otype = f.#type, mach = f.mach_id, oname = f.#name)
##		where f.label = @name {
			strtrim(oname);
			printf("changing FS %s (%s, %s on %d) to (AFS, %s)\n",
			       name, type, oname, mach, path);
##	}
##	repeat replace f (phys_id=0, #type="AFS", mach_id=cell,
##			      #name=@path, modtime=date("now"),
##			      modby=who, modwith="migrate")
##  		where f.label = @name
##	repeat retrieve (type = q.#type, id=q.entity_id, oquota=q.#quota)
##		where q.filsys_id = f.filsys_id and f.label=@name {
			printf("removing quota (%d (%s %d) on %s)\n",
			       oquota, type, id, name);
##	}
##	repeat delete q where q.filsys_id = f.filsys_id and f.label=@name
	printf("adding quota of %d on %s\n", quota, name);
##	repeat append #quota (filsys_id = f.filsys_id, #type="ANY",
##			 entity_id=0, phys_id=0,
##			 #quota=@quota, modtime=date("now"),
##			 modby=who, modwith="migrate")
##  		where f.label= @name
	cnt++;
    }

    if (cnt > 0) {
##	replace tblstats (updates = tblstats.updates + cnt,
##			    modtime =  date("now"))
##	    where tblstats.tbl = "filesys"
##	replace tblstats (updates = tblstats.updates + cnt,
##			    modtime =  date("now"))
##	    where tblstats.tbl = "quota"
    }

##  end transaction
    if (debug) printf("Done.\n");
    exit(0);
}


char *first_indir(name)
char *name;
{
    static char buf[10];

    if (islower(name[0]))
      sprintf(buf, "%c", name[0]);
    else
      sprintf(buf, "other");
    return(buf);
}

char *second_indir(name)
char *name;
{
    static char buf[10];

    if (islower(name[1]))
      sprintf(buf, "%c", name[1]);
    else
      sprintf(buf, "other");
    return(buf);
}

char *course_indir(name)
char *name;
{
    static char buf[10];
    char *src, *dst;

    dst = buf;
    for (src = name; *src && isdigit(*src); src++)
      *dst++ = *src;

    if  (dst == buf)
      strcpy(buf, "other");
    else
      *dst = 0;

    return(buf);
}
