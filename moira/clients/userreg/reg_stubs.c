/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/reg_stubs.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/reg_stubs.c,v 1.20 1992-06-01 15:45:29 mar Exp $
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#ifndef lint
static char *rcsid_reg_stubs_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/userreg/reg_stubs.c,v 1.20 1992-06-01 15:45:29 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#ifdef _AIX_SOURCE
#include <sys/select.h>
#endif
#include <netinet/in.h>
#include <netdb.h>
#include <des.h>
#include <krb.h>
#include <krb_err.h>
#include <errno.h>
#include <moira.h>
#include <moira_site.h>
#include "ureg_err.h"
#include "ureg_proto.h"
#include <strings.h>
#include <ctype.h>

static int reg_sock = -1;
static int seq_no = 0;
static char *host;
extern errno;
#define UNKNOWN_HOST -1
#define UNKNOWN_SERVICE -2

#ifndef FD_SET
#define FD_ZERO(p)  ((p)->fds_bits[0] = 0)
#define FD_SET(n, p)   ((p)->fds_bits[0] |= (1 << (n)))
#define FD_ISSET(n, p)   ((p)->fds_bits[0] & (1 << (n)))
#endif /* FD_SET */

ureg_init()
{
    struct servent *sp;
    char **p, *s;
    struct hostent *hp;
    struct sockaddr_in sin;
    extern char *getenv(), **hes_resolve();
    
    initialize_ureg_error_table();
    initialize_krb_error_table(); 
    initialize_sms_error_table();
    initialize_kadm_error_table();
   
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
	host = strsave(MOIRA_SERVER);
	s = index(host, ':');
	if (s) *s = 0;
    }
    hp = gethostbyname(host);
    host = strsave(hp->h_name);
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
    des_pcbc_encrypt(crypt_src, bp, len+13, ks, key, DES_ENCRYPT);
    bp += len+14+8;
    len = bp - buf;
    return do_call(buf, len, seq_no, login);
}

do_operation(first, last, idnumber, hashidnumber, data, opcode)
    char *first, *last, *idnumber, *hashidnumber, *data;
    u_long opcode;
{
    char buf[1024];
    int version = ntohl((u_long)1);
    int call = ntohl(opcode);
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
    
    len = strlen(data) + 1;
    bcopy(data, cbp, len);
    cbp += len;

    len = cbp - crypt_src;
    des_string_to_key(hashidnumber, key);
    des_key_sched(key, ks);
    des_pcbc_encrypt(crypt_src, bp, len, ks, key, 1);
    len = ((len + 7) >> 3) << 3;
    bp += len;
    
    len = bp - buf;
    return do_call(buf, len, seq_no, 0);

}

grab_login(first, last, idnumber, hashidnumber, login)
    char *first, *last, *idnumber, *hashidnumber, *login;
{
    return(do_operation(first, last, idnumber, hashidnumber, login,
			UREG_RESERVE_LOGIN));
}

enroll_login(first, last, idnumber, hashidnumber, login)
    char *first, *last, *idnumber, *hashidnumber, *login;
{
    return(do_operation(first, last, idnumber, hashidnumber, login,
			UREG_SET_IDENT));
}

set_password(first, last, idnumber, hashidnumber, password)
    char *first, *last, *idnumber, *hashidnumber, *password;
{
    return(do_operation(first, last, idnumber, hashidnumber, password,
			UREG_SET_PASSWORD));
}

get_krb(first, last, idnumber, hashidnumber, password)
    char *first, *last, *idnumber, *hashidnumber, *password;
{
    return(do_operation(first, last, idnumber, hashidnumber, password,
			UREG_GET_KRB));
}


/* The handles the operations for secure passwords.
 * To find out if a user has a secure instance, the newpasswd
 * field is ignored (but must be a valid char *)
 * and the opcode = UREG_GET_SECURE need to be specified (but the
 * other strings must be valid char*'s).  This will return
 * UREG_ALREADY_REGISTERED if it is set, or SUCCESS if not.
 * To set the password, fill in the rest of the fields, and
 * use opcode = UREG_SET_SECURE.  This returns SUCCESS or any number
 * of failure codes.
 */

do_secure_operation(login, idnumber, passwd, newpasswd, opcode)
    char *login, *idnumber, *passwd, *newpasswd;
    u_long opcode;
{
    char buf[1500], data[128], tktstring[128];
    int version = ntohl((u_long)1);
    int call = ntohl(opcode);
    char inst[INST_SZ], hosti[INST_SZ];
    char *bp = buf, *src, *dst, *realm;
    int len, status, i;
    KTEXT_ST cred;
    CREDENTIALS creds;
    Key_schedule keys;
    char *krb_get_phost(), *krb_realmofhost();
    
    bcopy((char *)&version, bp, sizeof(int));
    bp += sizeof(int);
    seq_no++;
    bcopy((char *)&seq_no, bp, sizeof(int));

    bp += sizeof(int);

    bcopy((char *)&call, bp, sizeof(int));

    bp += sizeof(int);

    /* put the login name in the firstname field */
    (void) strcpy(bp, login);
    bp += strlen(bp)+1;

    /* the old lastname field */
    (void) strcpy(bp, "");
    bp += strlen(bp)+1;

    /* don't overwrite existing ticket file */
    (void) sprintf(tktstring, "/tmp/tkt_cpw_%d",getpid());
    krb_set_tkt_string(tktstring);

    /* get realm and canonizalized hostname of server */
    realm = krb_realmofhost(host);
    for (src = host, dst = hosti; *src && *src != '.'; src++)
      if (isupper(*src))
	*dst++ = tolower(*src);
      else
      	*dst++ = *src;
    *dst = 0;
    inst[0] = 0;
    inst[0] = 0;

    /* get changepw tickets.  We use this service because it's the
     * only one that guarantees we used the password rather than a
     * ticket granting ticket.
     */
    status = krb_get_pw_in_tkt(login, inst, realm,
			       "changepw", hosti, 5, passwd);
    if (status) return (status + krb_err_base);

    status = krb_mk_req(&cred, "changepw", hosti, realm, 0);
    if (status) return (status + krb_err_base);

    /* round up to word boundry */
    bp = (char *)((((u_long)bp)+3)&0xfffffffc);

    /* put the ticket in the packet */
    len = cred.length;
    cred.length = htonl(cred.length);
    bcopy(&(cred), bp, sizeof(int)+len);
#ifdef DEBUG
    com_err("test", 0, "Cred: length %d", len);
    for (i = 0; i < len; i += 16)
      com_err("test", 0, " %02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x",
	      cred.dat[i+0], cred.dat[i+1], cred.dat[i+2], cred.dat[i+3],
	      cred.dat[i+4], cred.dat[i+5], cred.dat[i+6], cred.dat[i+7],
	      cred.dat[i+8], cred.dat[i+9], cred.dat[i+10], cred.dat[i+11],
	      cred.dat[i+12], cred.dat[i+13], cred.dat[i+14], cred.dat[i+15]);
#endif /* DEBUG */
    bp += sizeof(int) + len;

    /* encrypt the data in the session key */
    sprintf(data, "%s,%s", idnumber, newpasswd);
    len = strlen(data);
    len = ((len + 7) >> 3) << 3;

    status = krb_get_cred("changepw", hosti, realm, &creds);
    if (status) {
	bzero(data, strlen(data));
	return (status + krb_err_base);
    }
    dest_tkt();

    des_key_sched(creds.session, keys);
    des_pcbc_encrypt(data, bp + sizeof(int), len, keys, creds.session, 1);
    *((int *)bp) = htonl(len);
    bzero(data, strlen(data));

    bp += len + sizeof(int);
    
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
