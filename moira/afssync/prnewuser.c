/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/prnewuser.c,v 1.1 1992-05-31 21:14:53 probe Exp $ */

#include <stdio.h>
#include <sys/file.h>
#include <rx/xdr.h>
#include "print.h"
#include "prserver.h"
#include "prerror.h"

int dbase_fd;
extern struct prheader cheader;

main(argc, argv)
int argc;
char **argv;
{
    char *name, *msg;
    long id, status;

    if (argc != 3) {
	fprintf(stderr, "usage: %s loginname uid\n", argv[0]);
	exit(1);
    }
    name = argv[1];
    id = atoi(argv[2]);

    dbase_fd = open("/usr/afs/db/xprdb.DB0", O_RDWR|O_CREAT, 0660);

    status = PR_INewEntry(NULL, name, id, 0);
    if (status != 0) {
	switch (status) {
	case PREXIST:
	    msg = "name already exists";
	    break;
	case PRIDEXIST:
	    msg = "ID already exists";
	    break;
	case PRNOIDS:
	    msg = "no IDs available";
	    break;
	case PRDBFAIL:
	    msg = "database failed";
	    break;
	case PRNOENT:
	    msg = "no space left in database";
	    break;
	case PRPERM:
	    msg = "permission denied";
	    break;
	case PRNOTGROUP:
	    msg = "not a group";
	    break;
	case PRNOTUSER:
	    msg = "not a user";
	    break;
	case PRBADNAM:
	    msg = "bad name";
	    break;
	default:
	    msg = "unknown code";
	    break;
	}
	fprintf(stderr, "Failed: %s (%d)\n", msg, status);
    }
    exit(status);
}
