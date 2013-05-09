/* $Id$
 *
 * Copyright (C) 1987-1999 by the Massachusetts Institute of Technology
 *
 */

struct entry {
  char *id;
  char *last;
  char *first;
  char *middle;
  char *type;

  char *name;
  char *dept;
  char *haddr;
  char *hphone;
  char *oaddr;
  char *ophone;

  char *xtitle;
  char *xaddress;
  char *xphone1;
  char *xphone2;

  /* Only used by student load */
  char *reg_type;

  char *affiliation_basic;
  char *affiliation_detailed;
};

void fixphone(char *phone);
void fixaddress(char *address);

void process_entry(struct entry *e, int secure);
void newuser(struct entry *e, int secure);

int set_next_users_id(void);
int set_next_uid(void);

void sqlexit(void);
void dbmserr(char *where, int what);

#define SQL_DUPLICATE -2112
#define sqlfail() (sqlca.sqlcode && sqlca.sqlcode != 1403)
