/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/moira.h,v $
 *	$Author: danw $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/moira.h,v 1.19 1997-01-20 18:04:14 danw Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 */

#ifndef _moira_h_
#define _moira_h_

/* return values from queries (and error codes) */

#include "mr_et.h"
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
    char *q_data;
};

/* Hash table declarations */
struct bucket {
    struct bucket *next;
    int	key;
    char *data;
};
struct hash {
    int	size;
    struct bucket **data;
};

#ifdef __STDC__
int mr_connect(char *server);
int mr_disconnect();
int mr_host(char *host, int size);
int mr_motd(char **motd);
int mr_auth(char *prog);
int mr_access(char *handle, int argc, char **argv);
int mr_access_internal(int argc, char **argv);
int mr_query(char *handle, int argc, char **argv,
	      int (*callback)(), char *callarg);
int mr_query_internal(int argc, char **argv,
		       int (*callback)(), char *callarg);
int mr_noop();
struct save_queue *sq_create();
struct hash *create_hash(int size);
char *hash_lookup(struct hash *h, int key);
char *strsave(char *s);
char *strtrim(char *s);
char *canonicalize_hostname(char *s);
#else /* !__STDC__ */
int mr_connect();
int mr_disconnect();
int mr_host();
int mr_motd();
int mr_auth();
int mr_access();
int mr_query();
int mr_noop();
struct save_queue *sq_create();
struct hash *create_hash();
char *hash_lookup();
char *strsave();
char *strtrim();
char *canonicalize_hostname();
#endif  /* __STDC__ */

/* error-name backward compatibility */
#define MR_INGRES_ERR		MR_DBMS_ERR
#define MR_INGRES_SOFTFAIL	MR_DBMS_SOFTFAIL

#endif /* _moira_h_ */		/* Do Not Add Anything after this line. */
