/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/afssync/ubik.c,v 1.1 1989-09-23 18:48:18 mar Exp $ */

#include <sys/types.h>
#include <lock.h>
#define UBIK_INTERNALS
#include <ubik.h>
#include <rx/xdr.h>
#include "print.h"
#include "prserver.h"


struct ubik_dbase *dbase;

long ubik_BeginTrans()
{
    return(0);
}

long ubik_SetLock()
{
    return(0);
}

long ubik_AbortTrans()
{
    return(0);
}

long ubik_GetVersion(dummy, ver)
int dummy;
struct ubik_version *ver;
{
    bzero(ver, sizeof(struct ubik_version));
    return(0);
}

long ubik_EndTrans()
{
    return(0);
}


extern int dbase_fd;

long ubik_Seek(tt, afd, pos)
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

long ubik_Write(tt, buf, len)
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

long ubik_Read(tt, buf, len)
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


char *prdir = "/dev/null";

afsconf_GetNoAuthFlag()
{
    return(1);
}


struct prheader cheader;
