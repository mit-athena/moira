/* $Id: moira.h,v 1.21 1998-05-26 18:14:03 danw Exp $
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 *
 */

#ifndef _moira_h_
#define _moira_h_

/* return values from queries (and error codes) */

#include <com_err.h>
#include "mr_et.h"
#include "krb_et.h"
#include "gdss_et.h"
#include "ureg_err.h"
#define MR_SUCCESS 0		/* Query was successful */

#define MR_VERSION_1 1		/* Version in use from 7/87 to 4/88 */
#define MR_VERSION_2 2		/* After 4/88, new query lib */

/* return values for Moira server calls, used by clients */

#define MR_CONT 0		/* Everything okay, continue sending values. */
#define MR_ABORT -1		/* Something went wrong don't send anymore
				   values. */

#define MOIRA_SNAME	"moira"	/* Kerberos service key to use */

/* Protocol operations */
#define MR_NOOP 0
#define MR_AUTH 1
#define MR_SHUTDOWN 2
#define MR_QUERY 3
#define MR_ACCESS 4
#define MR_DO_UPDATE 5
#define MR_MOTD 6
#define MR_MAX_PROC 6

/* values used in NFS physical flags */
#define MR_FS_STUDENT	0x0001
#define	MR_FS_FACULTY	0x0002
#define MR_FS_STAFF	0x0004
#define MR_FS_MISC	0x0008
#define MR_FS_GROUPQUOTA 0x0010

/* magic values to pass for list and user queries */
#define UNIQUE_GID	"create unique GID"
#define UNIQUE_UID	"create unique UID"
#define UNIQUE_LOGIN	"create unique login ID"

/* Structure used by Save Queue routines (for temporary storage of data) */
struct save_queue
{
  struct save_queue *q_next;
  struct save_queue *q_prev;
  struct save_queue *q_lastget;
  void *q_data;
};

/* Hash table declarations */
struct bucket {
  struct bucket *next;
  int key;
  void *data;
};
struct hash {
  int size;
  struct bucket **data;
};

/* prototypes from critical.c */
void critical_alert(char *instance, char *msg, ...);
void send_zgram(char *inst, char *msg);

/* prototypes from fixhost.c */
char *canonicalize_hostname(char *s);

/* prototypes from fixname.c */
void FixName(char *ilnm, char *ifnm, char *last, char *first, char *middle);
void FixCase(char *p);
void LookForJrAndIII(char *nm, int *pends_jr, int *pends_sr,
		     int *pends_ii, int *pends_iii,
		     int *pends_iv, int *pends_v);
void LookForSt(char *nm);
void LookForO(char *nm);
void TrimTrailingSpace(char *ip);
void GetMidInit(char *nm, char *mi);

/* prototypes from gdss_convert.c */
int gdss2et(int code);

/* prototypes from hash.c */
struct hash *create_hash(int size);
void *hash_lookup(struct hash *h, int key);
int hash_update(struct hash *h, int key, void *value);
int hash_store(struct hash *h, int key, void *value);
void hash_search(struct hash *h, void *value, void (*callback)(int));
void hash_step(struct hash *h, void (*callback)(int, void *, void *),
	       void *hint);
void hash_destroy(struct hash *h);

/* prototypes from idno.c */
void RemoveHyphens(char *str);
void EncryptID(char *sbuf, char *idnumber, char *first, char *last);

/* prototypes from kname_unparse.c */
char *kname_unparse(char *p, char *i, char *r);

/* prototypes from nfsparttype.c */
char *parse_filesys_type(char *fs_type_name);
char *format_filesys_type(char *fs_status);

/* prototypes from sq.c */
struct save_queue *sq_create(void);
int sq_save_data(struct save_queue *sq, void *data);
int sq_save_args(int argc, char *argv[], struct save_queue *sq);
int sq_save_unique_data(struct save_queue *sq, void *data);
int sq_save_unique_string(struct save_queue *sq, char *data);
/* in sq_get_data and sq_remove_data, the `data' arg should be a
   pointer to a pointer */
int sq_get_data(struct save_queue *sq, void *data);
int sq_remove_data(struct save_queue *sq, void *data);
int sq_empty(struct save_queue *sq);
void sq_destroy(struct save_queue *sq);

/* prototypes from strs.c */
char *strtrim(char *s);
char *uppercase(char *s);
char *lowercase(char *s);

/* mr_ functions */
int mr_access(char *handle, int argc, char **argv);
int mr_auth(char *prog);
int mr_connect(char *server);
int mr_disconnect(void);
int mr_do_update(void);
int mr_host(char *host, int size);
int mr_motd(char **motd);
int mr_noop(void);
int mr_query(char *handle, int argc, char **argv,
	     int (*callback)(int, char **, void *), void *callarg);

/* error-name backward compatibility */
#define MR_INGRES_ERR		MR_DBMS_ERR
#define MR_INGRES_SOFTFAIL	MR_DBMS_SOFTFAIL


#ifdef __GNUC__
#if __GNUC_MINOR__ < 7
#define __attribute__(x)
#endif
#else /* ! __GNUC __ */
#define __attribute__(x)
#endif

#define RCSID(id) static char *rcsid __attribute__ ((__unused__)) = id

#endif /* _moira_h_ */		/* Do Not Add Anything after this line. */
