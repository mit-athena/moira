/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dbck/dbck.h,v 1.2 1988-09-26 14:41:01 mar Exp $
 *
 * Declarations for SMS database consistancy checker
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#include <sms_app.h>
#include <mit-copyright.h>

#define MODE_ASK 1
#define MODE_NO 2
#define MODE_PREEN 3
#define MODE_YES 4

extern int debug, mode, fast, dcmenable, warn;
extern struct hash *users, *machines, *clusters, *lists;
extern struct hash *filesys, *nfsphys, *strings;

#define MAX_ID_VALUE 32765
#define MIN_ID_VALUE 100

#define dprintf		if (debug) printf

struct user {
    char login[9];
    char potype;
    char *fullname;
    int status;
    int users_id;
    int pobox_id;
};

struct machine {
    char name[33];
    int mach_id;
    int clucount;
};

struct cluster {
    char name[33];
    int clu_id;
};

struct list {
    char name[33];
    char acl_type;
    int list_id;
    int  acl_id;
    int members;
};

struct string {
    char *name;
    int string_id;
    int refc;
};

struct filesys {
    char name[33];
    char dir[33];
    char type;
    int filsys_id;
    int mach_id;
    int owner;
    int owners;
    int phys_id;
};

struct nfsphys {
    char dir[33];
    int mach_id;
    int nfsphys_id;
    int allocated;
    int count;
};
