/* $HeadURL$ $Id$
 *
 * Declarations for Moira database consistancy checker
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira_site.h>
#include <moira_schema.h>

#define MODE_ASK 1
#define MODE_NO 2
#define MODE_PREEN 3
#define MODE_YES 4

extern int debug, mode, fast, dcmenable, warn;
extern struct hash *users, *machines, *clusters, *lists, *printservers;
extern struct hash *filesys, *nfsphys, *strings, *subnets, *string_dups;
extern struct hash *containers;

#define MAX_ID_VALUE 131072
#define MIN_ID_VALUE 100

#define dprintf if (debug) printf

struct user {
  char login[USERS_LOGIN_SIZE];
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
  char sponsor_type;
  int sponsor_id;
};

struct machine {
  char name[MACHINE_NAME_SIZE];
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
  char name[SUBNET_NAME_SIZE];
  char owner_type;
  int owner_id;
  int snet_id;
  int modby;
};

struct cluster {
  char name[CLUSTERS_NAME_SIZE];
  int clu_id;
  int modby;
};

struct list {
  char name[LIST_NAME_SIZE];
  char acl_type;
  char memacl_type;
  int list_id;
  int acl_id;
  int memacl_id;
  int modby;
  int members;
};

struct string {
  char *name;
  int string_id;
  int refc;
};

struct filesys {
  char name[FILESYS_LABEL_SIZE];
  char dir[FILESYS_NAME_SIZE];
  char type;
  int filsys_id;
  int mach_id;
  int owner;
  int owners;
  int phys_id;
};

struct nfsphys {
  char dir[NFSPHYS_DIR_SIZE];
  int mach_id;
  int nfsphys_id;
  unsigned long long allocated;
  int modby;
  unsigned long long count;
};

struct printserver {
  int mach_id;
  int printer_types;
  char owner_type;
  int owner_id;
  int lpc_acl;
  int modby;
};

struct container {
  char name[CONTAINERS_NAME_SIZE];
  int cnt_id;
  int list_id;
  char acl_type;
  int acl_id;
  char memacl_type;
  int memacl_id;
  int modby;
};

void dbmserr(void);
void out_of_mem(char *msg);
void cleanup(void);

void modified(char *table);

void phase1(void);
void phase2(void);
void phase3(void);
void phase4(void);
void count_only_setup(void);

/* prototypes from fix.pc */
void generic_delete(struct save_queue *sq, int (*pfunc)(void *), char *table,
		    char *idfield, int preen);
void single_delete(char *table, char *idfield, int id);
void zero_fix(char *tbl, char *zrfield, char *idfield, int id);
int single_fix(char *msg, int preen);
void generic_fix(struct save_queue *sq, int (*pfunc)(void *), char *msg,
		 void (*ffunc)(void *), int preen);
int prompt(char *msg);
int set_next_object_id(char *object, char *tablename);
int generic_fix_id(char *tbl, char *idfield, char *txtfield,
		   int oldid, char *name);

/* prototypes from phase3.pc */
struct string *string_check(int id);
