/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/admin_call.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/admin_call.c,v 1.8 1988-09-13 15:50:39 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 *	Utility functions for communication with the Kerberos admin_server
 *
 * 	Original version written by Jeffery I. Schiller, January 1987
 *	Completely gutted and rewritten by Bill Sommerfeld, August 1987
 *
 */

#ifndef lint
static char *rcsid_admin_call_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/admin_call.c,v 1.8 1988-09-13 15:50:39 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <netdb.h>
#include <strings.h>
#include <ctype.h>
#include <stdio.h>

#include "admin_err.h"
#include "admin_server.h"
#include "prot.h"
#include "krb.h"

extern int krb_err_base;	/* Offset between com_err and kerberos codes */
extern int errno;		/* System call error numbers */

extern long gethostid();

static u_char *strapp();		/* string append function */

static int inited = 0;		/* are we initialized? */
static int admin_fd = -1;	/* socket to talk to admin_server. */

static struct sockaddr_in admin_addr; /* address of admin server. */
static struct sockaddr_in my_addr;    /* address bound to admin_fd. */
static int my_addr_len;		/* size of above address. */

static char krbrlm[REALM_SZ];	/* Local kerberos realm */
static char krbhost[BUFSIZ];	/* Name of server for local realm */
char admin_errmsg[BUFSIZ]; /* Server error message */

/*
 * Initialize socket, etc. to use to talk to admin_server.
 */

int admin_call_init()
{
    register int status = 0;
    
    if (!inited) {
	struct hostent *hp;	/* host to talk to */
	struct servent *sp;	/* service to talk to */

	init_kadm_err_tbl();
	if (status = get_krbrlm(krbrlm, 1)) {
	    status += krb_err_base;
	    goto punt;
	}

	/*
	 * Locate server.
	 */

	if (status = get_krbhst(krbhost, krbrlm, 1)) {
	    status += krb_err_base;
	    goto punt;
	}
	hp = gethostbyname(krbhost);
	if (!hp) {
	    status = ADMIN_UNKNOWN_HOST;
	    goto punt;
	}
	sp = getservbyname("atest3", "udp");
	if (!sp) {
	    status = ADMIN_UNKNOWN_SERVICE;
	    goto punt;
	}
	bzero((char *)&admin_addr, sizeof(admin_addr));
	admin_addr.sin_family = hp->h_addrtype;
	bcopy((char *)hp->h_addr, (char *)&admin_addr.sin_addr, hp->h_length);
	admin_addr.sin_port = sp->s_port;

	/* lowercase & truncate hostname becuase it will be used as an
	 * instance name.
	 */
        {
	    char *s;
	    for (s = krbhost; *s && *s != '.'; s++)
		if (isupper(*s))
		    *s = tolower(*s);
	    *s = 0;
	}
	inited = 1;
    }
    return 0;
    
punt:
    (void) close(admin_fd);
    admin_fd = -1;
    return status;
}

/*
 * Build and transmit request to admin_server, and wait for
 * response from server.  Returns a standard error code.
 */

int
admin_call(opcode, pname, old_passwd, new_passwd, crypt_passwd)
    int opcode;
    char *pname;
    char *old_passwd;
    char *new_passwd;
    char *crypt_passwd;
{
    int status;
    register u_char *bp;	/* Pointer into buffer. */
    u_char *ep;			/* End of buffer pointer. */
    
    u_char pvt_buf[BUFSIZ];	/* private message plaintext */
    int pvt_len;		/* length of valid data in pvt_buf */

    u_char sealed_buf[BUFSIZ];	/* sealed version of private message */
    int sealed_len;		/* length of valid data in sealed_buf */
    
    u_long checksum;		/* quad_cksum of sealed request. */
    
    C_Block sess_key;		/* Session key. */
    Key_schedule sess_sched;	/* Key schedule for above. */
    
    CREDENTIALS cred;		/* Kerberos credentials. */
    KTEXT_ST authent;		/* Authenticator */
    KTEXT_ST reply;		/* Reply from admin_server */
    MSG_DAT msg_data;		/* Decrypted message */
    int attempts;		/* Number of retransmits so far */

    struct sockaddr rec_addr;	/* Address we got reply from */
    int rec_addr_len;		/* Length of that address */
    int on = 1;			/* ioctl argument */
	
    
    if (!inited) {
	status = admin_call_init();
	if (status) goto bad;
    }

    /*
     * assemble packet:
     *
     * sealed message consists of:
     * version number (one byte).
     * request code (one byte).
     * principal name (null terminated).
     * new password (in the clear, null terminated).
     * old password or instance (null terminated)
     * crypt(new password, seed) (null terminated).
     * an extra null.
     * a '\001' character.
     * This is all sealed inside a private_message, with an
     * authenticator tacked on in front.
     */

    bp = pvt_buf;
    ep = pvt_buf + sizeof(pvt_buf);
    
    *bp++ = PW_SRV_VERSION;
    *bp++ = opcode;
    if ((bp = strapp(bp, (u_char *)pname, ep)) &&
	(bp = strapp(bp, (u_char *)new_passwd, ep)) &&
	(bp = strapp(bp, (u_char *)old_passwd, ep)) &&
	(bp = strapp(bp, (u_char *)crypt_passwd, ep))) {
	*bp++ = '\0';
	*bp++ = '\1';
	pvt_len = bp - pvt_buf;
    } else {
	status = ADMIN_TOO_LONG;
	goto bad;
    }
    
    /*
     * find our session key.
     */

    if (status = krb_get_cred("changepw", krbhost, krbrlm, &cred)) {
	status += krb_err_base;
	goto bad;
    }

    bcopy((char *)cred.session, (char *)sess_key, sizeof(sess_key));
    bzero((char *)cred.session, sizeof(sess_key)); /* lest we remember */

    if(key_sched(sess_key, sess_sched)) {
	status = ADMIN_BAD_KEY;
	goto bad;
    }

    /*
     * Set up socket.
     */

    admin_fd = socket(admin_addr.sin_family, SOCK_DGRAM, 0);
    if (admin_fd < 0) {
	status = errno;
	goto bad;
    }

    bzero((char *)&my_addr, sizeof(my_addr));
    
    my_addr.sin_family = admin_addr.sin_family;
    my_addr.sin_addr.s_addr = gethostid();

    if (bind(admin_fd, &my_addr, sizeof(my_addr)) < 0) {
	status = errno;
	goto bad;
    }

    my_addr_len = sizeof(my_addr);

    if (getsockname(admin_fd, (struct sockaddr *)&my_addr,
		    &my_addr_len) < 0) {
	status = errno;
	goto bad;
    }

    if (ioctl(admin_fd, FIONBIO, (char *)&on) < 0) {
	status = errno;
	goto bad;
    }
	
    /*
     * Encrypt the message using the session key.
     * Since this contains passwords, it must be kept from prying eyes.
     */

    sealed_len = krb_mk_priv(pvt_buf, sealed_buf, pvt_len,
				    sess_sched, sess_key, &my_addr,
				    &admin_addr);
    if (sealed_len < 0) {
	status = ADMIN_CANT_ENCRYPT;
	goto bad;
    }

    /*
     * Checksum the cypher text, to guard against tampering in flight.
     */

    checksum = quad_cksum(sealed_buf, NULL, sealed_len, 0, sess_key);
    
    /*
     * Make an authenticator, so the server can learn the session key
     * and know who we are.
     */

    if (status = krb_mk_req(&authent, "changepw", krbhost, krbrlm,
			   checksum)) {
	status += krb_err_base;
	goto bad;
    }

    /*
     * Add the sealed message to the end of the authenticator.
     */

    if ((authent.length + sealed_len) > MAX_KTXT_LEN) {
	status = ADMIN_TOO_LONG;
	goto bad;
    }

    bcopy(sealed_buf, authent.dat + authent.length, sealed_len);
    authent.length += sealed_len;

    /*
     * send it off, and wait for a reply.
     */

    attempts = 0;
    
    while (attempts++ < RETRY_LIMIT) {
	int active;		/* number of active fds (from select). */
	fd_set set;		/* interesting fd's. */
	struct timeval timeout;	/* timeout on select. */
	
	if (sendto(admin_fd, (char *)authent.dat, authent.length,
		   0, &admin_addr, sizeof(admin_addr)) != authent.length) {
	    status = errno;
	    goto bad;
	}

	FD_ZERO(&set);
	FD_SET(admin_fd, &set);

	timeout.tv_sec = USER_TIMEOUT;
	timeout.tv_usec = 0;
	    
	active = select(admin_fd+1, &set, (fd_set *)0, (fd_set *)0, &timeout);
	if (active < 0) {
	    status = errno;
	    goto bad;
	} else if (active == 0) continue;
	else {
	    reply.length = recvfrom(admin_fd, (char *)reply.dat,
				    sizeof(reply.dat), 0,
				    &rec_addr, &rec_addr_len);
	    if (reply.length < 0) continue;
#ifdef notdef
	    if (!bcmp(&rec_addr, &admin_addr, sizeof(admin_addr)))
		/* the spoofers are out in force.. */
		continue;
#endif notdef
	    break;		/* got one.. */
	}
    }

    if (attempts > RETRY_LIMIT) {
	status = ETIMEDOUT;
	goto bad;
    }

    if (pkt_version((&reply)) != KRB_PROT_VERSION) {
	status = ADMIN_BAD_VERSION;
	goto bad;
    }

    if ((pkt_msg_type((&reply)) & ~1) != AUTH_MSG_PRIVATE) {
	bp = reply.dat;
	if (*bp++ != KRB_PROT_VERSION) {
	    status = ADMIN_BAD_VERSION;
	    goto bad;
	}
	if (*bp++ != AUTH_MSG_ERR_REPLY) {
	    status = ADMIN_UNKNOWN_CODE;
	    goto bad;
	}
	bp += strlen((char *)bp) + 1;	/* Skip name */
	bp += strlen((char *)bp) + 1;	/* Skip instance */
	bp += strlen((char *)bp) + 1;	/* Skip realm */

	/* null-terminate error string */
	reply.dat[reply.length] = '\0'; 

	if (*bp++ == 126) {
	    status = ADMIN_SERVER_ERROR;
	    strcpy(admin_errmsg, bp);
	    goto bad;
	} else {
	    status = ADMIN_UNKNOWN_CODE;
	    goto bad;
	}
    }
    status = krb_rd_priv(reply.dat, reply.length,
			    sess_sched, sess_key,
			    &admin_addr, &my_addr,
			    &msg_data);
    if (status) {
	status += krb_err_base;
	goto bad;
    }
    bp = msg_data.app_data;

    if (*bp++ != PW_SRV_VERSION) {
	status = ADMIN_BAD_VERSION;
	goto bad;
    }
    if (*bp++ != INSTALL_REPLY) {
	status = ADMIN_UNKNOWN_CODE;
	goto bad;
    }

    status = 0;
    /* fall through into cleanup */

bad:
    /*
     * Paranoia: shred all the incriminating evidence.
     * We'll let the caller shred the arguments.
     */
    bzero((char *)sess_key, sizeof(sess_key));
    bzero((char *)sess_sched, sizeof(sess_sched));
    bzero(pvt_buf, sizeof(pvt_buf));
        
    if (admin_fd >= 0) {
	(void) close(admin_fd);
	admin_fd = -1;
    }

    return status;
}

/*
 * Copies characters from source to dest, returning a pointer to the
 * point in dest after the last character copied from source.
 * If this would be greater than end, no characters are copied, and NULL
 * is returned instead.
 */

static u_char *strapp(dest, source, end)
    register u_char *dest, *source, *end;
{
    register int length = strlen(source) + 1;
    if (dest+length > end) return NULL;
    else {
	bcopy(source, dest, length);
	return dest + length;
    }
}

/*
 * Local Variables:
 * mode: c
 * c-indent-level: 4
 * c-continued-statement-offset: 4
 * c-brace-offset: -4
 * c-argdecl-indent: 4
 * c-label-offset: -4
 * End:
 */
