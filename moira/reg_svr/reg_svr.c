/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/reg_svr.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/reg_svr.c,v 1.5 1987-09-10 22:18:32 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 * 	Server for user registration with SMS and Kerberos.
 *
 * 	This program is a client of the SMS server and the Kerberos
 * 	admin_server, and is a server for the userreg program.
 * 
 *	$Log: not supported by cvs2svn $
 * Revision 1.4  87/09/04  23:33:19  wesommer
 * Deleted test scaffolding (second oops.)
 * 
 * Revision 1.3  87/09/03  03:05:18  wesommer
 * Version used for userreg tests.
 * 
 * Revision 1.2  87/08/22  18:39:45  wesommer
 * User registration server.
 * 
 * Revision 1.1  87/07/31  15:48:13  wesommer
 * Initial revision
 * 
 */

#ifndef lint
static char *rcsid_reg_svr_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/reg_svr.c,v 1.5 1987-09-10 22:18:32 wesommer Exp $";
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
#include "admin_server.h"
#include "admin_err.h" 
#include <strings.h>

extern void abort();
extern char *strdup();
extern char *malloc();
extern int krb_err_base;
extern char admin_errmsg[];

long now;
		       
struct msg {
    u_long version;
    u_long request;
    char *first;
    char *last;
    char *sealed;
    int sealed_len;
};

static char retval[BUFSIZ];

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
    extern char *whoami;
    int i;
    
    setlinebuf(stderr);
    whoami = "reg_svr";
    
    init_ureg_err_tbl();

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

    status = sms_connect();
    if (status != 0) {
	com_err("reg_svr", status, " on connect");
	exit(1);
    }
    status = sms_auth();
    if (status != 0) {
	com_err("reg_svr", status, " on auth");
	exit(1);
    }
    
    for (;;) {
	com_err("reg_svr", 0, "Ready for next request");
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

int got_one;
int reg_status;
char *mit_id;
char *reg_misc;
int reg_misc_len;
int user_id;

#define min(a,b) ((a)>(b)?(b):(a))
    
int validate_idno(message, db_mit_id, first, last)
    struct msg *message;
    char *db_mit_id;
    char *first, *last;
{
    C_Block key;
    Key_schedule sched;
    static char decrypt[BUFSIZ];
    char recrypt[14];
    static char hashid[14];
    char idnumber[BUFSIZ];
    char *temp;
    int len;

    int i;
    mit_id = 0;
    
    string_to_key(db_mit_id, key);
    key_sched(key, sched);
    pcbc_encrypt(message->sealed, decrypt, message->sealed_len, sched, key, 0);

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

static int status_in_db;

vfy_callbk(argc, argv, p_message)
    int argc;			/* Should sanity check this.. */
    char **argv;
    char *p_message;
{
    struct msg *message = (struct msg *)p_message;
    char *db_mit_id;
    char *firstname, *lastname;
    int status;
    
    if (got_one) return 0;
    reg_status = 0;
    
    db_mit_id = argv[8];
    firstname = argv[5];
    lastname = argv[4];

    status = validate_idno(message, db_mit_id, firstname, lastname);
    if (status) return 0; /* Nope; decryption failed */

    status_in_db = atoi(argv[7]);
    reg_status = status_in_db;    

    if (status_in_db != 0) {
	(void) strcpy(retval, argv[0]);
    }
    user_id = atoi(argv[1]);
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
    
    com_err("reg_svr", 0, " verify_user %s %s\n",
	    message->first, message->last);
    argv[0] = "get_user_by_first_and_last";
    argv[1] = message->first;
    argv[2] = message->last;
    got_one = 0;
    
    status = sms_query_internal(3, argv, vfy_callbk, (char *)message);
    
    if (status == SMS_NO_MATCH) status = UREG_USER_NOT_FOUND;
    if (!got_one && !status)
	status = UREG_USER_NOT_FOUND;
    
    if (status != 0) goto punt;

    if (reg_status == 1) status = UREG_ALREADY_REGISTERED;
    if (reg_status == 2) status = UREG_NO_PASSWD_YET;
    
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
    char uid_buf[20];
    
    com_err("reg_svr", 0, " reserve_user %s %s\n",
	    message->first, message->last);

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
	status = UREG_ALREADY_REGISTERED;
	goto punt;
    }
    /*
     * He's made it past this phase already.
     */
    if (status_in_db == 2) {
	status = 0;
	goto punt;
    }

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
    /* get keys */
    status = get_svc_in_tkt("register", "sms", "ATHENA.MIT.EDU",
			    "changepw", "kerberos",
			    1, "/etc/srvtab");
    if (status) {
	    status += krb_err_base;
	    goto punt;
    }
    
    /* send set password request to kerberos admin_server */
    (void) sprintf(uid_buf, "%013d", user_id); /* 13 chars of placebo */
					       /* for backwards-compat. */
    
    status = admin_call(ADMIN_ADD_NEW_KEY_ATTR, login, "", 
			"", uid_buf);

    if (status) {
	    if (status == ADMIN_SERVER_ERROR) {
		    printf("Server error: %s\n", admin_errmsg);
		    
		    if (strcmp(admin_errmsg,
				"Principal already in kerberos database.") ==0)
			    status = UREG_LOGIN_USED;
	    }
	    goto punt;
    }

    dest_tkt();
    /* If valid: */
    
    /* Set login name */
    status = set_login(login, mit_id);

    
    if (status) {
	com_err("set_login", status, 0);
	goto punt;
    }
    /* choose post office */
    
    status = choose_pobox(login);
    if (status) {
	com_err("choose_pobox", status, 0);
	goto punt;
    }
    /* create group */
    
    status = create_group(login);
    if (status == SMS_LIST) status = UREG_LOGIN_USED;
    
    if (status) {
	com_err("create_group", status, 0);
	goto punt;
    }
    /* set quota entry, create filsys */
    
    status = alloc_filsys(login, SMS_FS_STUDENT, 0, 0);
    if (status == SMS_FILESYS_EXISTS) status = UREG_LOGIN_USED;
    if (status) {
	com_err("alloc_filsys", status, 0);
	goto punt;
    }
    /* set filsys and status in SMS database */
    
    status = set_status_filsys(reg_misc, mit_id);
    if (status) {
	com_err("set_filsys", status, 0);
	goto punt;
    }
punt:
    dest_tkt();

    com_err("reg_svr", status, " returned from reserve_user");
    return status;
}

set_password(message)
    struct msg *message;
{
    char *argv[3];
    int status;
    char uid_buf[10];
    
    com_err("reg_svr", 0, " set_password %s %s\n",
	    message->first, message->last);

    /* validate that user is who he claims to be */

    argv[0] = "get_user_by_first_and_last";
    argv[1] = message->first;
    argv[2] = message->last;
    got_one = 0;
    
    status = sms_query_internal(3, argv, vfy_callbk, (char *)message);
    
    if (status == SMS_NO_MATCH) status = UREG_USER_NOT_FOUND;
    if (!got_one && !status)
	status = UREG_USER_NOT_FOUND;
    
    if (status != 0) goto punt;

    /* validate that state is equal to '2' (login, but no password) */

    if (reg_status != 2) {
	status = UREG_NO_LOGIN_YET;
	goto punt;
    }

    /* get keys */
    status = get_svc_in_tkt("register", "sms", "ATHENA.MIT.EDU",
			    "changepw", "kerberos",
			    1, "/etc/srvtab");
    if (status) {
	    status += krb_err_base;
	    goto punt;
    }

    (void) sprintf(uid_buf, "%013d", user_id); /* 13 chars of placebo */
					       /* for backwards-compat. */
    /* send set password request to kerberos admin_server */
    status = admin_call(ADMIN_ADD_NEW_KEY_ATTR, retval, "", 
			reg_misc, uid_buf);

    if (status) goto punt;
    dest_tkt();
    
    status = set_final_status(retval, mit_id);

    /* reflect reply to client */
punt:
    dest_tkt();
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
    char *retv[13];
    
    argv[0] = "get_user_by_mitid";
    argv[1] = idnumber;

    for (i=0; i<13; i++) {
	retv[i] = 0;
    }

    status = sms_query_internal(2, argv, store_user, (char *)(retv+1));
    if (status) return status;

    retv[0] = retv[1];
    retv[1] = username;
    if (retv[4]) free(retv[4]);
    retv[4] = "null";		/* No such filesystem */
    
    printf("Update_user(%s, %s)\n", retv[0], retv[1]);
    
    status = sms_query("update_user", 12, retv, abort, 0);
    retv[1] = 0;
    retv[4] = 0;
    
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
    char *retv[13];
    
    argv[0] = "get_user_by_mitid";
    argv[1] = idnumber;

    for (i=0; i<13; i++) {
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
/*
 * Set the status and filsys of user with username "uname" and filesys filsys.
 */

set_final_status(username, idnumber)
    char *username;
    char *idnumber;
{
    char *argv[2];
    int status, i;
    char *retv[13];
    
    argv[0] = "get_user_by_mitid";
    argv[1] = idnumber;

    for (i=0; i<13; i++) {
	retv[i] = 0;
    }

    status = sms_query_internal(2, argv, store_user, (char *)(retv+1));
    if (status) return status;

    retv[0] = retv[1];

    free(retv[8]);
    retv[8] = "1";
    
    printf("Update_user(%s, %s)\n", retv[0], retv[1]);
    
    status = sms_query("update_user", 12, retv, abort, 0);
    retv[8] = 0;
    for (i=1; i<12; i++) {
	if (retv[i]) free(retv[i]);
	retv[i] = 0;
    }
    return status;
}    

create_group(login)
    char *login;
{
    int status;
    static char *cr[] = {
	"add_user_group",
	0,
    };
    
    cr[1] = login;
    
    return sms_query_internal(2, cr, abort, 0);
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

