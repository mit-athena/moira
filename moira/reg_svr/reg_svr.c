/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/reg_svr.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/reg_svr.c,v 1.1 1987-07-31 15:48:13 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 * 	Server for user registration with SMS and Kerberos.
 *
 * 	This program is a client of the SMS server and the Kerberos
 * 	admin_server, and is a server for the userreg program.
 * 
 *	$Log: not supported by cvs2svn $
 */

#ifndef lint
static char *rcsid_reg_svr_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/reg_svr.c,v 1.1 1987-07-31 15:48:13 wesommer Exp $";
#endif lint

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <des.h>
#include <errno.h>
#include <ctype.h>
#include "ureg_err.h"
#include "ureg_proto.h"
#include "../../include/sms.h"
#include <strings.h>

extern void abort();

struct msg {
    u_long version;
    u_long request;
    char *first;
    char *last;
    char *sealed;
    int sealed_len;
};

static char retval[BUFSIZ];

#if defined(vax) || defined(ibm032) || defined(sun)
#define MAXINT 0x7fffffff
#else
    Hey turkey!  What's the biggest integer on this machine?
#endif

main()
{
    struct sockaddr_in sin;
    struct servent *sp;
    int s;
    int status;
    int addrlen, len;
    char buf[BUFSIZ];
    extern int errno;
    u_long seqno;
    struct msg message;
    
    init_ureg_err_tbl();
    
    status = sms_connect();
    if (status != 0) {
	com_err("reg_svr", status, "");
	exit(1);
    }
    status = sms_auth();
    if (status != 0) {
	com_err("reg_svr", status, "");
	exit(1);
    }
    
    sp = getservbyname("sms_ureg", "udp");
    if (sp == NULL) {
	fprintf(stderr, "Unknown service sms_ureg/udp\n");
	exit(1);
    }
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
	perror("socket");
	exit(1);
    }
    bzero((char *)&sin, sizeof(sin));

    sin.sin_family = AF_INET;
    sin.sin_port = sp->s_port;
    sin.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, &sin, sizeof(sin)) < 0) {
	perror("bind");
	exit(1);
    }
    
    for (;;) {
	addrlen = sizeof(sin);
	bzero(retval, BUFSIZ);
	len = recvfrom(s, buf, BUFSIZ, 0, &sin, &addrlen);
	if (len < 0) {
	    perror("recvfrom");
	    if (errno == EINTR) continue;
	    
	    exit(1);
	}
	/* Parse a request packet */
	status = parse_pkt(buf, len, &seqno, &message);
	if (status != 0) {
	    len = BUFSIZ;
	    format_pkt(buf, &len, seqno, status, (char *)NULL);
	    (void) sendto(s, buf, len, 0, &sin, addrlen);
	    continue;
	}
	/* do action */
	switch((int)message.request) {
	case UREG_VERIFY_USER:
	    status = verify_user(&message);
	    break;
	case UREG_RESERVE_LOGIN:
	    status = reserve_user(&message);
	    break;
	case UREG_SET_PASSWORD:
	    status = set_password(&message);
	    break;
	    
	default:
	    status = UREG_UNKNOWN_REQUEST;
	    break;
	}
	len = BUFSIZ;
	format_pkt(buf, &len, seqno, status, retval);
	
	sendto(s, buf, len, 0, &sin, addrlen);
    }
}

set_password(message)
    struct msg *message;
{
    /* validate, as with verify, that user is who he claims to be */
    /* validate that state is equal to '1' */
    /* send set password request to kerberos admin_server */
    /* reflect reply to client */
    return 0;
}
    
int got_one;
int reg_status;
char *mit_id;
char *reg_misc;
int reg_misc_len;
#define min(a,b) ((a)>(b)?(b):(a))
    
int validate_idno(message, db_mit_id, first, last)
    struct msg *message;
    char *db_mit_id;
    char *first, *last;
{
    C_Block key;
    Key_schedule sched;
    char decrypt[BUFSIZ];
    char recrypt[14];
    static char hashid[14];
    char idnumber[BUFSIZ];
    char *temp;
    int len;

    int i;
#ifdef notdef
    for (i = 0; i < message->sealed_len; i++) {
	printf("%02x ", (unsigned char)message->sealed[i]);
    }
    printf("\n");
#endif notdef
    mit_id = 0;
    
    string_to_key(db_mit_id, key);
    key_sched(key, sched);
    pcbc_encrypt(message->sealed, decrypt, message->sealed_len, sched, key, 0);

#ifdef notdef
    for (i = 0; i < message->sealed_len; i++) {
	printf("%02x ", (unsigned char)decrypt[i]);
    }
    printf("\n");
    for (i = 0; i < message->sealed_len; i++) {
	if (isprint(decrypt[i])) 
	    printf("%c  ", (unsigned char)decrypt[i]);
	else printf(".  ");
    }
    printf("\n");
#endif notdef
    (void) strncpy(idnumber, decrypt, message->sealed_len);
    temp = decrypt + strlen(idnumber) + 1;
    len = message->sealed_len - (temp - decrypt);
    
    (void) strncpy(hashid, temp, min(len, 14));
    temp += strlen(hashid) + 1;
    len = message->sealed_len - (temp - decrypt);

    if (strcmp(hashid, db_mit_id)) return 1;
    encrypt_mitid(recrypt, idnumber, first, last);
    if (strcmp(recrypt, db_mit_id)) return 1;

    reg_misc = temp;
    reg_misc_len = len;
    mit_id = hashid;
    
    return 0;
}

vfy_callbk(argc, argv, p_message)
    int argc;			/* Should sanity check this.. */
    char **argv;
    char *p_message;
{
    struct msg *message = (struct msg *)p_message;
    char *db_mit_id;
    char *firstname, *lastname;
    int status;
    
    printf("Callback: %s %s %s\n", argv[8], argv[5], argv[4]);
    
    if (got_one) return 0;
    reg_status = 0;
    
    db_mit_id = argv[8];
    firstname = argv[5];
    lastname = argv[4];

    status = validate_idno(message, db_mit_id, firstname, lastname);
    if (status) return 0; /* Nope; decryption failed */
    
    if (atoi(argv[7]) == 1) {
	reg_status = UREG_ALREADY_REGISTERED;
	(void) strcpy(retval, argv[0]);
    }
    got_one = 1;
    return 0;
}    

encrypt_mitid(buf, idnumber, first, last)
    char *buf, *idnumber, *first, *last;
{
    char salt[3];
    extern char *crypt();
    
#define _tolower(c) ((c)|0x60)

    salt[0] = _tolower(last[0]);
    salt[1] = _tolower(first[0]);
    salt[2] = 0;
    
    (void) strcpy(buf, crypt(&idnumber[2], salt));
}

int verify_user(message)
    struct msg *message;

{
    char *argv[3];
    int status;
    
    printf("verify_user\n");
    argv[0] = "get_user_by_first_and_last";
    argv[1] = message->first;
    argv[2] = message->last;
    got_one = 0;
    
    status = sms_query_internal(3, argv, vfy_callbk, (char *)message);
    
    if (status == SMS_NO_MATCH) status = UREG_USER_NOT_FOUND;
    if (!got_one && !status)
	status = UREG_USER_NOT_FOUND;
    
    if (status != 0) goto punt;

    if (reg_status != 0) status = reg_status;
    
punt:
    return status;
}

reserve_user(message)
    struct msg *message;
{
    char *argv[3];
    int status;
    int i;
    char *login;
    
    printf("reserve_user\n");

    argv[0] = "gufl";		/* get_user_by_first_and_last */
    argv[1] = message->first;
    argv[2] = message->last;
    got_one = 0;

    status = sms_query_internal(3, argv, vfy_callbk, (char *)message);

    if (status == SMS_NO_MATCH) status = UREG_USER_NOT_FOUND;
    if (!got_one && !status)
	status = UREG_USER_NOT_FOUND;

    if (status != 0) goto punt;
    if (reg_status != 0) {
	status = reg_status;
	goto punt;
    }

    /* Sanity check requested login name. */
    printf("reg_misc_len = %d\n", reg_misc_len);

    for (i = 0; i < reg_misc_len && reg_misc[i]; i++) {
	if (!islower(reg_misc[i])) {
	    status = UREG_INVALID_UNAME;
	    goto punt;
	}
    }
    if (i < 3 || i > 8) {
	status = UREG_INVALID_UNAME;
	goto punt;
    }
    login = reg_misc;
    
    /* Send request to kerberos admin_server for login name */
    /* If valid: */

    /* Set login name */
    set_login(login, mit_id);
    
    /* choose post office */
    choose_pobox(login);

    /* set quota entry, create filsys */
    alloc_filsys(login);

    /* set filsys and status in SMS database */
    set_status_filsys(reg_misc, mit_id);
    
punt:
    printf("reserve_user returning %s\n", error_message(status));
    return status;
}

extern char *malloc();

char *strdup(cp)
    char *cp;
{
    int len = strlen(cp) + 1;
    char *np = malloc(len);
    bcopy(cp, np, len);
    return np;
}

static char *pohost;
static int min_usage;

static po_callbk(argc, argv, argp)
    int argc;
    char **argv;
{
    if (!isdigit(*argv[6])) {
	printf("non-digit value_1 field??\n");
	return 0;	
    }
    
    if (atoi(argv[6]) < min_usage) {
	min_usage = atoi(argv[6]);
	if (pohost) free(pohost);
	pohost = strdup(argv[1]);
    }
	
    return 0;
}

choose_pobox(login)
    char *login;
{
    int status;
    static char *argv[3]={
	"gshi",
	"pop",
	"*"
    };
    static char *apoa[5] = {
	"add_pobox",
	0,
	"pop",
	0,
	0
    }; 
    
    pohost = NULL;
    min_usage = MAXINT;	/* MAXINT */
    status = sms_query_internal(3, argv, po_callbk,(char *)&min_usage);
    if (status != 0) {
	com_err("reg_svr", status, "finding pobox");
	printf("Can't find postoffices\n");
	return status;
    }
    printf("Chose %s\n", pohost);
    apoa[1] = apoa[4] = login;
    apoa[3] = pohost;
    status = sms_query_internal(5, apoa, abort, 0);

    if (status == SMS_EXISTS) status = 0;
    
    if (status != 0) {
	com_err("reg_svr", status, "adding pobox");
    }
    return status;
}


parse_pkt(buf, len, seqnop, messagep)
    char *buf;
    int len;
    u_long *seqnop;
    struct msg *messagep;
{
    if (len < 4) return UREG_BROKEN_PACKET;
    bcopy(buf, (char *)&messagep->version, sizeof(long));
    messagep->version = ntohl(messagep->version);
    if (messagep->version != 1) return UREG_WRONG_VERSION;
    
    buf += 4;
    len -= 4;

    if (len < 4) return UREG_BROKEN_PACKET;
    bcopy(buf, (char *)seqnop, sizeof(long));

    buf += 4;
    len -= 4;
    
    if (len < 4) return UREG_BROKEN_PACKET;
    bcopy(buf, (char *)(&messagep->request), sizeof(long));
    messagep->request = ntohl(messagep->request);
    buf += 4;
    len -= 4;
    
    messagep->first = buf;

    for (; *buf && len > 0; --len, ++buf) continue;
    if (len <= 0) return UREG_BROKEN_PACKET;

    buf++, len--;

    messagep->last = buf;

    for (; *buf && len > 0; --len, ++buf) continue;
    if (len <= 0) return UREG_BROKEN_PACKET;
    
    buf++, len--;

    if (len <= 0) return UREG_BROKEN_PACKET;
    
    messagep->sealed = buf;
    messagep->sealed_len = len;
    
    return 0;
}

format_pkt(buf, lenp, seqno, status, message)
    char *buf;
    int *lenp;
    u_long seqno;
    int status;
    char *message;
{
    u_long vers = htonl((u_long)1);
    status = htonl((u_long)status);
    
    bcopy((char *)&vers, buf, sizeof(long));
    bcopy((char *)&seqno, buf+sizeof(long), sizeof(long));
    bcopy((char *)&status, buf+ 2*sizeof(long), sizeof(long));
    *lenp = sizeof(long) * 3;
    (void) strcpy(buf+3*sizeof(long), message);
    (*lenp) += strlen(message);
}

store_user(argc, argv, argp)
    int argc;
    char **argv;
    char *argp;
{
    char **retv = (char **) argp;
    int i;
    
    for (i = 0; i < argc; i++) {
	if (retv[i]) {
	    free(retv[i]);
	    retv[i]=0;
	}
	retv[i] = strdup(argv[i]);
    }
    return 0;
}
    

/*
 * Set login name of user with "idnumber" to be "username"
 */

set_login(username, idnumber)
    char *username;
    char *idnumber;
{
    char *argv[2];
    int status, i;
    char *retv[12];
    
    argv[0] = "get_user_by_mitid";
    argv[1] = idnumber;

    for (i=0; i<12; i++) {
	retv[i] = 0;
    }

    status = sms_query_internal(2, argv, store_user, (char *)(retv+1));
    if (status) return status;

    retv[0] = retv[1];
    retv[1] = username;
    
    printf("Update_user(%s, %s)\n", retv[0], retv[1]);
    
    status = sms_query("update_user", 12, retv, abort, 0);
    for (i=1; i<12; i++) {
	if (retv[i]) free(retv[i]);
	retv[i] = 0;
    }
    
    return status;
}    

/*
 * Set the status and filsys of user with username "uname" and filesys filsys.
 */

set_status_filsys(username, idnumber)
    char *username;
    char *idnumber;
{
    char *argv[2];
    int status, i;
    char *retv[12];
    
    argv[0] = "get_user_by_mitid";
    argv[1] = idnumber;

    for (i=0; i<12; i++) {
	retv[i] = 0;
    }

    status = sms_query_internal(2, argv, store_user, (char *)(retv+1));
    if (status) return status;

    retv[0] = retv[1];

    free(retv[4]);
    retv[4] = username;

    free(retv[8]);
    retv[8] = "2";
    
    printf("Update_user(%s, %s)\n", retv[0], retv[1]);
    
    status = sms_query("update_user", 12, retv, abort, 0);
    retv[4] = 0;
    retv[8] = 0;
    for (i=1; i<12; i++) {
	if (retv[i]) free(retv[i]);
	retv[i] = 0;
    }
    return status;
}    

static char *nfs_device;
static char *nfs_dir;
static char *nfs_host;
static int nfs_alloc;

    
static afcb(argc, argv, argp)
    int argc;
    char **argv;
    char *argp;
{
    if ((atoi(argv[3]) & 1) == 0) return 0; /* not free for alloc. */
    
    if (atoi(argv[4]) < nfs_alloc) {
	nfs_alloc = atoi(argv[4]);
	if (nfs_device) free(nfs_device);
	if (nfs_dir) free(nfs_dir);
	if (nfs_host) free(nfs_host);
	nfs_host = strdup(argv[0]);
	nfs_device = strdup(argv[1]);
	nfs_dir = strdup(argv[2]);
    }
    return 0;
}

/*
 * Allocate home filesystem.
 */

alloc_filsys(login)
    char *login;
{
    static char *argv[] = {
	"get_all_nfsphys"
	};
    static char *alocv[] = {
	"add_locker",
	0,
	0,
	0,
	"1024"
	};
    
    int status;
    nfs_alloc = MAXINT;
    
    status = sms_query_internal(1, argv, afcb, 0);
    if (status) {
	com_err("reg_svr", status, "while doing get_all_nfsphys");
	return status;
    }
    alocv[1] = login;
    alocv[2] = nfs_host;
    alocv[3] = nfs_device;
    printf("add_locker(%s, %s, %s)\n", login, nfs_host, nfs_device);
    
    status = sms_query_internal(5, alocv, abort, 0);
    if (status) {
	com_err("reg_svr", status, "while adding locker");
	return status;
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
