/* $Id: ubik.c,v 1.3 1992-06-07 04:19:28 probe Exp $ */

#include <sys/types.h>
#include <netinet/in.h>

#include <lock.h>
#define UBIK_INTERNALS
#include <ubik.h>
#include <rx/xdr.h>
#include "print.h"
#include "prserver.h"

extern int dbase_fd;
struct ubik_dbase *dbase;

int ubik_ServerInit()
{
    return(0);
}

int ubik_BeginTrans()
{
    static int init=0;
    struct ubik_hdr thdr;

    if (!init) {
	thdr.version.epoch = htonl(0);
	thdr.version.counter = htonl(0);
	thdr.magic = htonl(UBIK_MAGIC);
	thdr.size = htonl(HDRSIZE);
	lseek(dbase_fd, 0, 0);
	write(dbase_fd, &thdr, sizeof(thdr));
	fsync(dbase_fd);
	init = 1;
    }
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
