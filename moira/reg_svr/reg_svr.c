/*
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/reg_svr.c,v $
 *      $Author: qjb $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/reg_svr.c,v 1.9 1988-07-26 14:50:40 qjb Exp $
 *
 *      Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *      Server for user registration with SMS and Kerberos.
 *
 *      This program is a client of the SMS server and the Kerberos
 *      admin_server, and is a server for the userreg program.
 * 
 *      $Log: not supported by cvs2svn $
 * Revision 1.8  88/07/20  15:39:25  mar
 * find realm at runtime; don't use hard-coded one
 * 
 * Revision 1.7  88/02/08  15:08:15  mar
 * Moved header file locations
 * 
 * Revision 1.6  87/09/21  15:19:11  wesommer
 * Allow numbers, _, and . as legal characters in the username.
 * 
 * Revision 1.5  87/09/10  22:18:32  wesommer
 * Clean up output format.
 * 
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
static char *rcsid_reg_svr_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/reg_svr.c,v 1.9 1988-07-26 14:50:40 qjb Exp $";
#endif lint

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <krb.h>
#include <des.h>
#include <errno.h>
#include <ctype.h>
#include "ureg_err.h"
#include "ureg_proto.h"
#include "sms.h"
#include "admin_server.h"
#include "admin_err.h"
#include <strings.h>

#define WHOAMI "reg_svr"	/* Name of program for SMS logging */
#define CUR_SMS_VERSION SMS_VERSION_2 	  /* SMS version for this program */


extern int abort();
extern char *strdup();
extern char *malloc();
extern int krb_err_base;
extern char admin_errmsg[];

extern int errno;		/* Unix error number */

/* This is the structure used to store the packet information */
/* Get this.  The register client gets the MIT id of the registering user
   in plain text, encrypts it with one-way password encryption,
   concatenates that to the plain text id, and then des encrypts the
   whole thing using the password encrypted id as the key!  The result
   goes in enc_mitid. */
struct msg 
{    
    u_long version;		/* SMS version */
    u_long request;		/* Request */
    char *first;		/* First name */
    char *last;			/* Last name */
    char *enc_mitid;		/* See comment above */
    int enc_mitid_len;		/* Length of enc_mitid */
};

static char errmsg[BUFSIZ];

main()
{
    struct servent *sp;		/* Service info from /etc/services */
    int s;			/* Socket descriptor */
    struct sockaddr_in sin;	/* Internet style socket address */
    int addrlen;		/* Size of socket address (sin) */
    char packet[BUFSIZ];	/* Buffer for packet transmission */
    int pktlen;			/* Size of packet */
    u_long seqno;		/* Sequence number for packet transmission */
    struct msg message;		/* Storage for parsed packet */
    int status;			/* General purpose error status */
    
    /* Error messages sent one line at a time */
    setlinebuf(stderr);
    
    /* Initialize user registration error table */
    init_ureg_err_tbl();
    
    /* Get service information from /etc/services */
    if ((sp = getservbyname("sms_ureg", "udp")) == NULL) 
    {
	fprintf(stderr, "Unknown service sms_ureg/udp\n");
	exit(1);
    }
    
    /* Get an internet style datagram socket */
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0) 
    {
	perror("socket");
	exit(1);
    }
    bzero((char *)&sin, sizeof(sin));
    
    sin.sin_family = AF_INET;
    sin.sin_port = sp->s_port;
    sin.sin_addr.s_addr = INADDR_ANY;
    
    /* Bind a name to the socket */
    if (bind(s, &sin, sizeof(sin)) < 0) 
    {
	perror("bind");
	exit(1);
    }
    
    /* Connect to the SMS server */
    if ((status = sms_connect()) != SMS_SUCCESS) 
    {
	com_err(WHOAMI, status, " on connect");
	exit(1);
    }
    
    /* Authorize, telling the server who you are */
    if ((status = sms_auth(WHOAMI)) != SMS_SUCCESS) 
    {
	com_err(WHOAMI, status, " on auth");
	exit(1);
    }
    
    /* Sit around waiting for requests from the client. */
    for (;;) 
    {
	com_err(WHOAMI, 0, "Ready for next request");
	addrlen = sizeof(sin);
	bzero(errmsg, BUFSIZ);
	/* Receive a packet into buf. */
	if ((pktlen = recvfrom(s,packet,sizeof(packet),0,&sin,&addrlen)) < 0) 
	{
	    perror("recvfrom");
	    if (errno == EINTR) continue;
	    exit(1);
	}
	
	/* Parse a request packet */
	if ((status = parse_pkt(packet, pktlen, &seqno, &message)) != 0) 
	{
	    pktlen = sizeof(packet);
	    /* Format packet to send back to the client */
	    format_pkt(packet, &pktlen, seqno, status, (char *)NULL);
	    /* Report the error the the client */
	    (void) sendto(s, packet, pktlen, 0, &sin, addrlen);
	    continue;
	}
	
	/* do action */
	switch((int)message.request) 
	{
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
	
	/* Report what happened to client */
	pktlen = sizeof(packet);
	format_pkt(packet, &pktlen, seqno, status, errmsg);
	sendto(s, packet, pktlen, 0, &sin, addrlen);
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
  struct msg *message;		/* Formatted packet */
  char *db_mit_id;		/* Encrypted MIT ID from SMS database */
  char *first, *last;		/* First and last name for MIT ID encryption */
/* This routine makes sure that the ID from the database matches
   the ID sent accross in the packet.  The information in the packet
   was created in the following way:

   The plain text ID number was encrypted via crypt() resulting in
   the form that would appear in the SMS database.  This is 
   concatinated to the plain text ID so that the ID string contains
   plain text ID followed by a null followed by the encrypted ID.
   The whole thing is then DES encrypted using the encrypted ID as 
   the source of the key.

   This routine tries each encrypted ID in the database that belongs
   to someone with this user's first and last name and tries to 
   decrypt the packet with this information. */
{
    C_Block key;		/* The key for DES en/decryption */
    Key_schedule sched;		/* En/decryption schedule */
    static char decrypt[BUFSIZ];   /* Buffer to hold decrypted information */
    long decrypt_len;		/* Length of decypted ID information */
    char recrypt[14];		/* Buffer to hold re-encrypted information */
    static char hashid[14];	/* Buffer to hold one-way encrypted ID */
    char idnumber[BUFSIZ];	/* Buffer to hold plain-text ID */
    char *temp;			/* A temporary storage buffer */
    int len;			/*  */
    
    mit_id = 0;
    /* Make the decrypted information length the same as the encrypted
       information length.  Both are integral multples of eight bytes 
       because of the DES encryption routines. */
    decrypt_len = (long)message->enc_mitid_len;
    
    /* Get key from the one-way encrypted ID in the SMS database */
    string_to_key(db_mit_id, key);
    /* Get schedule from key */
    key_sched(key, sched);
    /* Decrypt information from packet using this key.  Since decrypt_len
       is an integral multiple of eight bytes, it will probably be null-
       padded. */
    pcbc_encrypt(message->enc_mitid, decrypt, \
		 decrypt_len, sched, key, DECRYPT);
    
    /* Extract the plain text and encrypted ID fields from the decrypted
       packet information. */
    /* Since the decrypted information starts with the plain-text ID
       followed by a null, if the decryption worked, this will only 
       copy the plain text part of the decrypted information.  It is
       important that strncpy be used because if we are not using the
       correct key, there is no guarantee that a null will occur
       anywhere in the string. */
    (void) strncpy(idnumber, decrypt, decrypt_len);
    /* Point temp to the end of the plain text ID number. */
    temp = decrypt + strlen(idnumber) + 1;
    /* Find out how much more room there is. */
    len = message->enc_mitid_len - (temp - decrypt);
    /* Copy the next 14 bytes of the decrypted information into 
       hashid if there are 14 more bytes to copy.  There will be
       if we have the right key. */
    (void) strncpy(hashid, temp, min(len, 14));
    /* Point temp to the end of the encrypted ID field */
    temp += strlen(hashid) + 1;
    /* Find out how much more room there is. */
    len = message->enc_mitid_len - (temp - decrypt);
    
    /* Now compare encrypted ID's returning with an error if they
       don't match. */
    if (strcmp(hashid, db_mit_id)) return 1;
    encrypt_mitid(recrypt, idnumber, first, last);
    /* Now compare encrypted plain text to ID from database. */
    if (strcmp(recrypt, db_mit_id)) return 1;
    
    /* We made it. */
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
    
    if (status_in_db != 0) 
    {
	(void) strcpy(errmsg, argv[0]);
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
    
    com_err(WHOAMI, 0, " verify_user %s %s\n",
	    message->first, message->last);
    argv[0] = "get_user_by_first_and_last";
    argv[1] = message->first;
    argv[2] = message->last;
    got_one = 0;
    
    status = sms_query_internal(3, argv, vfy_callbk, (char *)message);
    
    if (status == SMS_NO_MATCH) status = UREG_USER_NOT_FOUND;
    if (!got_one && !status)
	status = UREG_USER_NOT_FOUND;
    
    if (status != SMS_SUCCESS) goto punt;
    
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
    char realm[REALM_SZ];
    
    com_err(WHOAMI, 0, " reserve_user %s %s\n",
	    message->first, message->last);
    
    argv[0] = "gufl";		/* get_user_by_first_and_last */
    argv[1] = message->first;
    argv[2] = message->last;
    got_one = 0;
    
    status = sms_query_internal(3, argv, vfy_callbk, (char *)message);
    
    if (status == SMS_NO_MATCH) status = UREG_USER_NOT_FOUND;
    if (!got_one && !status)
	status = UREG_USER_NOT_FOUND;
    
    if (status != SMS_SUCCESS) goto punt;
    if (reg_status != 0) 
    {
	status = UREG_ALREADY_REGISTERED;
	goto punt;
    }
    /*
     * He's made it past this phase already.
     */
    if (status_in_db == 2) 
    {
	status = 0;
	goto punt;
    }
    
    for (i = 0; i < reg_misc_len && reg_misc[i]; i++) 
    {
	if (!islower(reg_misc[i]) && !isdigit(reg_misc[i]) &&
	    reg_misc[i] != '_' && reg_misc[i] != '.') 
	{
	    status = UREG_INVALID_UNAME;
	    goto punt;
	}
    }
    if (i < 3 || i > 8) 
    {
	status = UREG_INVALID_UNAME;
	goto punt;
    }
    login = reg_misc;
    
    /* Send request to kerberos admin_server for login name */
    /* get keys */
    if ((status = get_krbrlm(realm, 1)) != KSUCCESS) 
    {
	status += krb_err_base;
	goto punt;
    }
    status = get_svc_in_tkt("register", "sms", realm,
			    "changepw", "kerberos",
			    1, "/etc/srvtab");
    if (status) 
    {
	status += krb_err_base;
	goto punt;
    }
    
    /* send set password request to kerberos admin_server */
    (void) sprintf(uid_buf, "%013d", user_id); /* 13 chars of placebo */
    /* for backwards-compat. */
    
    status = admin_call(ADMIN_ADD_NEW_KEY_ATTR, login, "", 
			"", uid_buf);
    
    if (status) 
    {
	if (status == ADMIN_SERVER_ERROR) 
	{
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
    
    
    if (status) 
    {
	com_err("set_login", status, (char *)0);
	goto punt;
    }
    /* choose post office */
    
    status = choose_pobox(login);
    if (status) 
    {
	com_err("choose_pobox", status, (char *)0);
	goto punt;
    }
    /* create group */
    
    status = create_group(login);
    if (status == SMS_LIST) status = UREG_LOGIN_USED;
    
    if (status) 
    {
	com_err("create_group", status, (char *)0);
	goto punt;
    }
    /* set quota entry, create filsys */
    
    status = alloc_filsys(login, SMS_FS_STUDENT, 0, 0);
    if (status == SMS_FILESYS_EXISTS) status = UREG_LOGIN_USED;
    if (status) 
    {
	com_err("alloc_filsys", status, (char *)0);
	goto punt;
    }
    /* set filsys and status in SMS database */
    
    status = set_status_filsys(reg_misc, mit_id);
    if (status) 
    {
	com_err("set_filsys", status, (char *)0);
	goto punt;
    }
  punt:
    dest_tkt();
    
    com_err(WHOAMI, status, " returned from reserve_user");
    return status;
}

set_password(message)
  struct msg *message;
{
    char *argv[3];
    int status;
    char uid_buf[10];
    char realm[REALM_SZ];
    
    com_err(WHOAMI, 0, " set_password %s %s\n",
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
    
    if (status != SMS_SUCCESS) goto punt;
    
    /* validate that state is equal to '2' (login, but no password) */
    
    if (reg_status != 2) 
    {
	status = UREG_NO_LOGIN_YET;
	goto punt;
    }
    
    /* get keys */
    if ((status = get_krbrlm(realm, 1)) != KSUCCESS) 
    {
	goto punt;
    }
    status = get_svc_in_tkt("register", "sms", realm,
			    "changepw", "kerberos",
			    1, "/etc/srvtab");
    if (status) 
    {
	status += krb_err_base;
	goto punt;
    }
    
    (void) sprintf(uid_buf, "%013d", user_id); /* 13 chars of placebo */
    /* for backwards-compat. */
    /* send set password request to kerberos admin_server */
    /**####**/
    status = admin_call(ADMIN_ADD_NEW_KEY_ATTR, errmsg, "", 
			reg_misc, uid_buf);
    
    if (status) goto punt;
    dest_tkt();
    /**#####**/    
    status = set_final_status(errmsg, mit_id);
    
    /* reflect reply to client */
  punt:
    dest_tkt();
    return status;
}

parse_pkt(packet, pktlen, seqnop, messagep)
  char *packet;
  int pktlen;
  u_long *seqnop;
  struct msg *messagep;
  /* This routine checks a packet and puts the information in it in
       a structure if it is valid. */
{
    if (pktlen < 4) return UREG_BROKEN_PACKET;
    /* Extract the SMS version from the packet */
    bcopy(packet, (char *)&messagep->version, sizeof(long));
    /* Convert byte order from network to host */
    messagep->version = ntohl(messagep->version);
    /* Verify version */
    if (messagep->version != CUR_SMS_VERSION) return UREG_WRONG_VERSION;
    
    packet += 4;
    pktlen -= 4;
    
    if (pktlen < 4) return UREG_BROKEN_PACKET;
    /* Extract the sequence number from the packet */
    bcopy(packet, (char *)seqnop, sizeof(long));
    
    packet += 4;
    pktlen -= 4;
    
    if (pktlen < 4) return UREG_BROKEN_PACKET;
    /* Extract the request from the packet */
    bcopy(packet, (char *)(&messagep->request), sizeof(long));
    messagep->request = ntohl(messagep->request);
    packet += 4;
    pktlen -= 4;
    
    /* Extract first name from the packet */
    messagep->first = packet;
    
    /* Scan forward until null appears in the packet or there
       is no more packet! */
    for (; *packet && pktlen > 0; --pktlen, ++packet) continue;
    if (pktlen <= 0) return UREG_BROKEN_PACKET;
    
    /* Skip over the null */
    packet++, pktlen--;
    
    /* Extract last name from the packet */
    messagep->last = packet;
    
    for (; *packet && pktlen > 0; --pktlen, ++packet) continue;
    if (pktlen <= 0) return UREG_BROKEN_PACKET;
    
    packet++, pktlen--;
    
    if (pktlen <= 0) return UREG_BROKEN_PACKET;
    
    /* Extract MIT id information from packet; see comment on
       struct msg. */
    messagep->enc_mitid = packet;
    messagep->enc_mitid_len = pktlen;
    
    return 0;
}

format_pkt(packet, pktlenp, seqno, status, message)
  char *packet;
  int *pktlenp;
  u_long seqno;
  int status;
  char *message;
  /* This routine prepares a packet to send back to the client. */
{
    u_long vers = htonl((u_long)CUR_SMS_VERSION);
    status = htonl((u_long)status);
    
    /* Put current SMS version into the packet */
    bcopy((char *)&vers, packet, sizeof(long));
    /* Put sequence number into the packet */
    bcopy((char *)&seqno, packet+sizeof(long), sizeof(long));
    /* Put error status into the packet */
    bcopy((char *)&status, packet+ 2*sizeof(long), sizeof(long));
    *pktlenp = sizeof(long) * 3;
    /* Copy the message into the packet */
    (void) strcpy(packet+3*sizeof(long), message);
    (*pktlenp) += strlen(message);
}

store_user(argc, argv, argp)
  int argc;
  char **argv;
    char *argp;
{
    char **retv = (char **) argp;
    int i;
    
    for (i = 0; i < argc; i++) 
    {
	if (retv[i]) 
	{
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
    
    for (i=0; i<13; i++) 
    {
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
    
    for (i=1; i<12; i++) 
    {
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
    
    for (i=0; i<13; i++) 
    {
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
    for (i=1; i<12; i++) 
    {
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
    
    for (i=0; i<13; i++) 
    {
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
    for (i=1; i<12; i++) 
    {
	if (retv[i]) free(retv[i]);
	retv[i] = 0;
    }
    return status;
}    

create_group(login)
  char *login;
{
    int status;
    static char *cr[] = 
    {
	"add_user_group",
	0,
    };
    
    cr[1] = login;
    
    return sms_query_internal(2, cr, abort, 0);
}    

/*
 * Local Variables:
 * mode: c
 * c-argdecl-indent: 2
 * c-brace-offset: -4
 * c-continued-statement-offset: 2
 * c-indent-level: 4
 * c-label-offset: -2
 * End:
 */
