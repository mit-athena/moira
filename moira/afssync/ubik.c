/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/ubik.c,v 1.2 1990-09-21 15:22:14 mar Exp $ */

#include <sys/types.h>
#include <lock.h>
#define UBIK_INTERNALS
#include <ubik.h>
#include <rx/xdr.h>
#include "print.h"
#include "prserver.h"


struct ubik_dbase *dbase;

int ubik_ServerInit()
{
    return(0);
}

int ubik_BeginTrans()
{
    return(0);
}

int ubik_BeginTransReadAny()
{
    return(0);
}

int ubik_AbortTrans()
{
    return(0);
}

int ubik_EndTrans()
{
    return(0);
}

int ubik_Tell()
{
    return(0);
}

int ubik_Truncate()
{
    return(0);
}

long ubik_SetLock()
{
    return(0);
}

int ubik_WaitVersion()
{
    return(0);
}

int ubik_CacheUpdate()
{
    return(0);
}

int panic(a, b, c, d)
char *a, *b, *c, *d;
{
    printf(a, b, c, d);
    abort();
    printf("BACK FROM ABORT\n");    /* shouldn't come back from floating pt exception */
    exit(1);    /* never know, though */
}

int ubik_GetVersion(dummy, ver)
int dummy;
struct ubik_version *ver;
{
    bzero(ver, sizeof(struct ubik_version));
    return(0);
}


extern int dbase_fd;

int ubik_Seek(tt, afd, pos)
struct ubik_trans *tt;
long afd;
long pos;
{
    if (lseek(dbase_fd, pos+HDRSIZE, 0) < 0) {
	perror("ubik_Seek");
	return(-1);
    }
    return(0);
}

int ubik_Write(tt, buf, len)
struct ubik_trans *tt;
char *buf;
long len;
{
    int status;

    status = write(dbase_fd, buf, len);
    if (status < len) {
	perror("ubik_Write");
	return(1);
    }
    return(0);
}

int ubik_Read(tt, buf, len)
struct ubik_trans *tt;
char *buf;
long len;
{
    int status;
    
    status = read(dbase_fd, buf, len);
    if (status < 0) {
	perror("ubik_Read");
	return(1);
    }
    if (status < len)
      bzero(&buf[status], len - status);
    return(0);
}


/* Global declarations from ubik.c */
long ubik_quorum=0;
struct ubik_dbase *ubik_dbase=0;
struct ubik_stats ubik_stats;
long ubik_host;
long ubik_epochTime = 0;
long urecovery_state = 0;

struct rx_securityClass *ubik_sc[3];


/* Other declarations */

afsconf_GetNoAuthFlag()
{
    return(1);
}


char *prdir = "/dev/null";
struct prheader cheader;
int pr_realmNameLen;
char *pr_realmName;
