/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/reg_stubs.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/reg_stubs.c,v 1.11 1990-02-28 12:13:09 mar Exp $
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#ifndef lint
static char *rcsid_reg_stubs_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/reg_stubs.c,v 1.11 1990-02-28 12:13:09 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <des.h>
#include <errno.h>
#include <sms.h>
#include <sms_app.h>
#include "ureg_err.h"
#include "ureg_proto.h"
#include <strings.h>

static int reg_sock = -1;
static int seq_no = 0;
extern errno;
#define UNKNOWN_HOST -1
#define UNKNOWN_SERVICE -2

ureg_init()
{
    struct servent *sp;
    char *host, **p;
    struct hostent *hp;
    struct sockaddr_in sin;
    extern char *getenv(), **hes_resolve();
    
    initialize_ureg_error_table();
    
    seq_no = getpid();

    host = NULL;
    host = getenv("REGSERVER");
#ifdef HESIOD
    if (!host || (strlen(host) == 0)) {
	p = hes_resolve("registration", "sloc");
	if (p) host = *p;
    }
#endif HESIOD
    if (!host || (strlen(host) == 0)) {
	host = strsave(SMS_SERVER);
	*index(host, ':') = 0;
    }
    hp = gethostbyname(host);
    if (hp == NULL) return UNKNOWN_HOST;

    sp = getservbyname("sms_ureg", "udp");

    if (sp == NULL) return UNKNOWN_SERVICE;
    
    (void) close(reg_sock);
    reg_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (reg_sock < 0) return errno;

    bzero((char *)&sin, sizeof(sin));
    sin.sin_port = sp->s_port;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, sizeof(struct in_addr));
    sin.sin_family = AF_INET;

    if (connect(reg_sock, &sin, sizeof(sin)) < 0)
	return errno;
    return 0;
}

int
verify_user(first, last, idnumber, hashidnumber, login)
    char *first, *last, *idnumber, *hashidnumber, *login;
{
    char buf[1024];
    int version = ntohl((u_long)1);
    int call = ntohl((u_long)UREG_VERIFY_USER);
    des_cblock key;
    des_key_schedule ks;
    register char *bp = buf;
    register int len;
    char crypt_src[1024];
    
    bcopy((char *)&version, bp, sizeof(int));
    bp += sizeof(int);
    seq_no++;
    bcopy((char *)&seq_no, bp, sizeof(int));

    bp += sizeof(int);

    bcopy((char *)&call, bp, sizeof(int));

    bp += sizeof(int);

    (void) strcpy(bp, first);
    bp += strlen(bp)+1;

    (void) strcpy(bp, last);
    bp += strlen(bp)+1;

    len = strlen(idnumber) + 1;
    bcopy(idnumber, crypt_src, len);

    bcopy(hashidnumber, crypt_src+len, 13);

    des_string_to_key(hashidnumber, key);
    des_key_sched(key, ks);
    des_pcbc_encrypt(crypt_src, bp, len+14, ks, key, 1);
    bp += len+14+8;
    len = bp - buf;
    return do_call(buf, len, seq_no, login);
}

grab_login(first, last, idnumber, hashidnumber, login)
    char *first, *last, *idnumber, *hashidnumber, *login;
{
    char buf[1024];
    int version = ntohl((u_long)1);
    int call = ntohl((u_long)UREG_RESERVE_LOGIN);
    des_cblock key;
    des_key_schedule ks;
    register char *bp = buf;
    register int len;
    
    char crypt_src[1024];
    char *cbp;
    
    bcopy((char *)&version, bp, sizeof(int));
    bp += sizeof(int);
    seq_no++;
    bcopy((char *)&seq_no, bp, sizeof(int));

    bp += sizeof(int);

    bcopy((char *)&call, bp, sizeof(int));

    bp += sizeof(int);

    (void) strcpy(bp, first);
    bp += strlen(bp)+1;

    (void) strcpy(bp, last);
    bp += strlen(bp)+1;

    len = strlen(idnumber) + 1;
    cbp = crypt_src;
    
    bcopy(idnumber, crypt_src, len);
    cbp += len;
    
    bcopy(hashidnumber, cbp, 14);
    cbp += 14;
    
    len = strlen(login) + 1;
    bcopy(login, cbp, len);
    cbp += len;

    len = cbp - crypt_src;
    des_string_to_key(hashidnumber, key);
    des_key_sched(key, ks);
    des_pcbc_encrypt(crypt_src, bp, len, ks, key, 1);
#ifdef notdef    
    for (i = 0; i < len; i++) {
	printf("%02.2x ", (unsigned char)bp[i]);
    }
    printf("\n");
#endif notdef
    len = ((len + 7) >> 3) << 3;
    bp += len;
    
    len = bp - buf;
    return do_call(buf, len, seq_no, 0);

}

set_password(first, last, idnumber, hashidnumber, password)
    char *first, *last, *idnumber, *hashidnumber, *password;
{
    char buf[1024];
    int version = ntohl((u_long)1);
    int call = ntohl((u_long)UREG_SET_PASSWORD);
    des_cblock key;
    des_key_schedule ks;
    register char *bp = buf;
    register int len;
    
    char crypt_src[1024];
    char *cbp;
    
    bcopy((char *)&version, bp, sizeof(int));
    bp += sizeof(int);
    seq_no++;
    bcopy((char *)&seq_no, bp, sizeof(int));

    bp += sizeof(int);

    bcopy((char *)&call, bp, sizeof(int));

    bp += sizeof(int);

    (void) strcpy(bp, first);
    bp += strlen(bp)+1;

    (void) strcpy(bp, last);
    bp += strlen(bp)+1;

    len = strlen(idnumber) + 1;
    cbp = crypt_src;
    
    bcopy(idnumber, crypt_src, len);
    cbp += len;
    
    bcopy(hashidnumber, cbp, 14);
    cbp += 14;
    
    len = strlen(password) + 1;
    bcopy(password, cbp, len);
    cbp += len;

    len = cbp - crypt_src;
    des_string_to_key(hashidnumber, key);
    des_key_sched(key, ks);
    des_pcbc_encrypt(crypt_src, bp, len, ks, key, 1);
#ifdef notdef    
    for (i = 0; i < len; i++) {
	printf("%02.2x ", (unsigned char)bp[i]);
    }
    printf("\n");
#endif notdef
    len = ((len + 7) >> 3) << 3;
    bp += len;
    
    len = bp - buf;
    return do_call(buf, len, seq_no, 0);

}

static do_call(buf, len, seq_no, login)
    char *buf;
    char *login;
    int seq_no;
    int len;
{
    struct timeval timeout;
    char ibuf[1024];
    fd_set set;
    
    int retry = 0;

    do {
	if (write(reg_sock, buf, len) != len) return errno;

	FD_ZERO(&set);
	FD_SET(reg_sock, &set);
	timeout.tv_sec = 30;
	timeout.tv_usec = 0;
	do {
	    int rtn;
	    struct sockaddr_in sin;
	    int addrlen = sizeof(sin);
	    int vno;
	    int sno;
	    int stat;
	    
	    rtn = select(reg_sock+1, &set, (fd_set *)0, (fd_set *)0, &timeout);
	    if (rtn == 0)
		break;
	    else if (rtn < 0) return errno;
	
	    len = recvfrom(reg_sock, ibuf, BUFSIZ, 0, &sin, &addrlen);
	    if (len < 0) return errno;
	    if (len < 12) return UREG_BROKEN_PACKET;
	    bcopy(ibuf, (char *)&vno, sizeof(long));
	    vno = ntohl((u_long)vno);
	    if (vno != 1) continue;
	    bcopy(ibuf + 4, (char *)&sno, sizeof(long));

	    if (sno != seq_no) continue;

	    bcopy(ibuf + 8, (char *)&stat, sizeof(long));
	    stat = ntohl((u_long)stat);
	    if (login && len > 12) {
		bcopy(ibuf+12, login, len-12);
		login[len-12] = '\0';
	    } else if (login)
		*login = '\0';
	    return stat;
	} while (1);
    } while (++retry < 10);
    return ETIMEDOUT;
}    
