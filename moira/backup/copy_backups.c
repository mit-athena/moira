/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/copy_backups.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/copy_backups.c,v 1.1 1987-08-22 17:03:18 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 */

#ifndef lint
static char *rcsid_copy_backups_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/copy_backups.c,v 1.1 1987-08-22 17:03:18 wesommer Exp $";
#endif lint
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include "update.h"

char *whoami;
char host[BUFSIZ];
char buf[BUFSIZ];

static struct update_desc info = {
     42,                        /* last_time_tried */
     1,                         /* success */
     12,                        /* interval */
     "backup",			/* service_name */
     "ZEUS.MIT.EDU",		/* host_name */
     "/tmp/frobnicate",         /* target_path */
     2,                         /* override */
     1,                         /* enable */
     "/dev/null"
                                /* instructions */
};

extern char *error_message();
#include <sys/dir.h>

main(argc,argv)
    int argc;
    char **argv;
{
    int rc;
    DIR *pd, *sd;
    struct direct *pde, *sde;
    
    whoami = rindex(argv[0], '/');
    if (whoami)
        whoami++;
    else
        whoami = argv[0];
    whoami = argv[0];
    if (chdir ("/u3/sms_backup") < 0) {
	perror("can't change to /u3/sms_backup");
	exit(1);
    }
    
    pd = opendir(".");
    if (pd == NULL) {
	perror("can't open sms_backup directory");
	exit(1);
    }

    while ( (pde = readdir(pd)) != NULL ) {
	char *dir_name = pde->d_name;
	printf("Directory: %s\n", dir_name);

	if (dir_name[0] == '.') continue; /* ignore hidden files */
	
	if (chdir(dir_name) < 0) {
	    perror(dir_name);
	    continue;
	}
	sd = opendir (".");
	if (sd == NULL) {
	    perror("Can't open .");
	    goto dotdot;
	}
	while ( (sde = readdir(sd)) != NULL ) {
	    if (sde->d_name[0] == '.') continue;
	    
	    sprintf(buf, "/site/sms/sms_backup/%s/%s", dir_name, sde->d_name);
	    printf("Updating: %s\n", buf);
	    info.target_path = buf;
	    rc = sms_update_server(&info, sde->d_name);
	    if (rc) printf("return code: %s\n", error_message(rc));
	}
	closedir(sd);
	
    dotdot:
	chdir("..");
    }
    closedir(pd);
}

/*
 * Local Variables:
 * mode: c
 * c-indent-level: 4
 * c-continued-statement-offset: 4
 * c-brace-offset: -4
 * c-argdecl-indent: 4
 * c-label-offset: -4
 * End:
 */
