/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dbck/dbck.h,v 1.8 1997-01-29 23:13:05 danw Exp $
 *
 * Declarations for Moira database consistancy checker
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#include <moira_site.h>
#include <mit-copyright.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

#define MODE_ASK 1
#define MODE_NO 2
#define MODE_PREEN 3
#define MODE_YES 4

extern int debug, mode, fast, dcmenable, warn;
extern struct hash *users, *machines, *clusters, *lists;
extern struct hash *filesys, *nfsphys, *strings, *subnets, *string_dups;

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
  int comment;
  int modby;
  int fmodby;
  int pmodby;
  int sigwho;
};

struct machine {
  char name[33];
  char owner_type;
  int owner_id;
  int snet_id;
  int mach_id;
  int clucount;
  int acomment;
  int ocomment;
  int creator;
  int modby;
};

struct subnet {
  char name[33];
  char owner_type;
  int owner_id;
  int snet_id;
  int modby;
};

struct cluster {
    char name[33];
    int clu_id;
    int modby;
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
    char dir[81];
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
