/*
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/reg_svr.c,v $
 *      $Author: qjb $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/reg_svr.c,v 1.15 1988-08-07 16:17:34 qjb Exp $
 *
 *      Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *      Server for user registration with SMS and Kerberos.
 *
 *      This program is a client of the Kerberos admin_server and a
 *      server for the userreg program.  It is not a client of the
 *      SMS server as it is linked with libsmsglue which bypasses
 *      the network protocol.
 */

#ifndef lint
static char *rcsid_reg_svr_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/reg_svr.c,v 1.15 1988-08-07 16:17:34 qjb Exp $";
#endif lint

#include "reg_svr.h"
#include "admin_server.h"
#include "admin_err.h"

extern int krb_err_base;
extern char admin_errmsg[];

static char krbhst[BUFSIZ];	/* kerberos server name */
static char krbrealm[REALM_SZ];	/* kerberos realm name */

main(argc,argv)
  int argc;
  char *argv[];
{
    struct msg message;		/* Storage for parsed packet */
    int status = SUCCESS;	/* Error status */
    char retval[BUFSIZ];	/* Buffer to hold return message for client */
    
    void req_initialize();	/* Initialize request layer */
    void get_request();		/* Get a request */
    void report();		/* Respond to a request */

    /* Initialize */
    whoami = argv[0];
    
    /* Use com_err or output to stderr for all log messages. */    
#ifdef DEBUG
    fprintf(stderr,"*** Debugging messages enabled. ***\n");
#endif DEBUG
    
    /* Error messages sent one line at a time */
    setlinebuf(stderr);
    
    /* Initialize user registration error table for com_err */
    init_ureg_err_tbl();
    
    /* Connect to the SMS server */
    if ((status = sms_connect()) != SMS_SUCCESS) 
    {
	com_err(whoami, status, " on connect");
	exit(1);
    }
    
    /* Authorize, telling the server who you are */
    if ((status = sms_auth(whoami)) != SMS_SUCCESS) 
    {
	com_err(whoami, status, " on auth");
	exit(1);
    }
    
    if (status = get_krbrlm(krbrealm, 1)) {
	status += krb_err_base;
	com_err(whoami, status, " fetching kerberos realm");
	exit(1);
    }
    
    if (status = get_krbhst(krbhst, krbrealm, 1)) {
	status += krb_err_base;
	com_err(whoami, status, " fetching kerberos hostname");
	exit(1);
    } else {
	char *s;
	for (s = krbhst; *s && *s != '.'; s++)
	    if (isupper(*s))
		*s = tolower(*s);
	*s = 0;
    }
    
    /* Allow request layer to initialize */
    req_initialize();
    
    /* Sit around waiting for requests from the client. */
    for (;;) 
    {
	get_request(&message);
	
	switch((int)message.request) 
	{
	  case UREG_VERIFY_USER:
	    status = verify_user(&message,retval);
	    break;
	  case UREG_RESERVE_LOGIN:
	    status = reserve_user(&message,retval);
	    break;
	  case UREG_SET_PASSWORD:
	    status = set_password(&message,retval);
	    break;
	    
	  default:
	    status = UREG_UNKNOWN_REQUEST;
	    critical_alert(FAIL_INST,"Unknown request %d from userreg.",
			   message.request);
	    break;
	}
	
	/* Report what happened to client */
	report(status, retval);
    }
}

/* This is necessary so that this server can know where to put its
   tickets. */
char *tkt_string()
{
    return("/tmp/tkt_ureg");
}

int parse_encrypted(message,data)
  struct msg *message;		/* Formatted packet */
  struct db_data *data;		/* Data from the SMS database */
/* This routine makes sure that the ID from the database matches
   the ID sent accross in the packet.  The information in the packet
   was created in the following way:

   The plain text ID number was encrypted via EncryptID() resulting
   in the form that would appear in the SMS database.  This is
   concatinated to the plain text ID so that the ID string contains plain
   text ID followed by a null followed by the encrypted ID.  Other
   information such as the username or password is appended.  The whole
   thing is then DES encrypted using the encrypted ID as the source of
   the key.

   This routine tries each encrypted ID in the database that belongs
   to someone with this user's first and last name and tries to 
   decrypt the packet with this information.  If it succeeds, it returns 
   zero and initializes all the fields of the formatted packet structure
   that depend on the encrypted information. */
{
    C_Block key;		/* The key for DES en/decryption */
    Key_schedule sched;		/* En/decryption schedule */
    static char decrypt[BUFSIZ];   /* Buffer to hold decrypted information */
    long decrypt_len;		/* Length of decypted ID information */
    char recrypt[14];		/* Buffer to hold re-encrypted information */
    static char hashid[14];	/* Buffer to hold one-way encrypted ID */
    char idnumber[BUFSIZ];	/* Buffer to hold plain-text ID */
    char *temp;			/* A temporary string pointer */
    int len;			/* Keeps track of length left in packet */
    int status = SUCCESS;	/* Error status */
    
#ifdef DEBUG
    com_err(whoami,0,"Entering parse_encrypted");
#endif

    /* Make the decrypted information length the same as the encrypted
       information length.  Both are integral multples of eight bytes 
       because of the DES encryption routines. */
    decrypt_len = (long)message->encrypted_len;
    
    /* Get key from the one-way encrypted ID in the SMS database */
    string_to_key(data->mit_id, key);
    /* Get schedule from key */
    key_sched(key, sched);
    /* Decrypt information from packet using this key.  Since decrypt_len
       is an integral multiple of eight bytes, it will probably be null-
       padded. */
    pcbc_encrypt(message->encrypted,decrypt, decrypt_len, sched, key, DECRYPT);
    
    /* Extract the plain text and encrypted ID fields from the decrypted
       packet information. */
    /* Since the decrypted information starts with the plain-text ID
       followed by a null, if the decryption worked, this will only 
       copy the plain text part of the decrypted information.  It is
       important that strncpy be used because if we are not using the
       correct key, there is no guarantee that a null will occur
       anywhere in the string. */
    (void) strncpy(idnumber,decrypt,(int)decrypt_len);
    /* Point temp to the end of the plain text ID number. */
    temp = decrypt + strlen(idnumber) + 1;
    /* Find out how much more packet there is. */
    len = message->encrypted_len - (temp - decrypt);
    /* Copy the next CRYPT_LEN bytes of the decrypted information into 
       hashid if there are CRYPT_LEN more bytes to copy.  There will be
       if we have the right key. */
    (void) strncpy(hashid, temp, min(len, CRYPT_LEN));
    /* Point temp to the end of the encrypted ID field */
    temp += strlen(hashid) + 1;
    /* Find out how much more room there is. */
    len = message->encrypted_len - (temp - decrypt);
    
    /* Now compare encrypted ID's don't match. */
    if (strcmp(hashid, data->mit_id)) status = FAILURE;
    if (status == SUCCESS)
    {
	EncryptID(recrypt, idnumber, message->first, message->last);
	/* Now compare encrypted plain text to ID from database. */
	if (strcmp(recrypt, data->mit_id)) status = FAILURE;
    }
    
    if (status == SUCCESS)
    {
	/* We made it.  Now we can finish initializing message. */
	/* Point leftover to whatever is left over! */
	message->leftover = temp;
	message->leftover_len = len;
	/* Since we know we have the right user, fill in the information 
	   from the SMS database. */
	message->db.reg_status = data->reg_status;
	(void) strncpy(message->db.uid,data->uid, sizeof(message->db.uid));
	(void) strncpy(message->db.mit_id,data->mit_id, 
		       sizeof(message->db.mit_id));
	(void) strncpy(message->db.login,data->login, sizeof(message->db.login));
    }
    
#ifdef DEBUG
    if (status)
	com_err(whoami,status," parse_encrypted failed.");
    else
	com_err(whoami,status,"parse_encrypted succeeded.");
#endif

    return status;
}

int db_callproc(argc,argv,queue)
  int argc;			/* Number of arguments returned by SMS */
  char *argv[];			/* Arguments returned by SMS */
  struct save_queue *queue;	/* Queue to save information in */
/* This function is called by sms_query after each tuple found.  It is
   used by find_user to cache information about each user found.  */
{
    struct db_data *data;	/* Structure to store the information in */
    int status = SUCCESS;	/* Error status */
    
#ifdef DEBUG
    com_err(whoami,0,"Entering db_callproc.");
#endif

    if (argc != U_END)
    {
	critical_alert
	    (FAIL_INST,
	     "Wrong number of arguments returned from get_user_by_name.");
	status = SMS_ABORT;
    }
    else
    {
	/* extract the needed information from the results of the SMS query */
	data = (struct db_data *)malloc(sizeof(struct db_data));
	data->reg_status = atoi(argv[U_STATE]);
	(void) strncpy(data->login,argv[U_NAME],sizeof(data->login));
	(void) strncpy(data->mit_id,argv[U_MITID],sizeof(data->mit_id));
	(void) strncpy(data->uid,argv[U_UID],sizeof(data->uid));
#ifdef DEBUG
	fprintf(stderr,"Found in database:\n");
	fprintf(stderr,"   Registration status: %d\n",data->reg_status);
	fprintf(stderr,"   login: %s\n",data->login);
	fprintf(stderr,"   MIT ID: %s\n",data->mit_id);
	fprintf(stderr,"   uid: %s\n",data->uid);
#endif
	sq_save_data(queue,data);
    }

    return status;
}
    
int find_user(message)
  struct msg *message;		/* Formatted packet structure */
/* This routine verifies that a user is allowed to register by finding
   him/her in the SMS database.  It returns the status of the SMS
   query that it calls. */
{
#define GUBN_ARGS 2		/* Arguements needed by get_user_by_name */
    char *q_name;		/* Name of query */
    int q_argc;			/* Number of arguments for query */
    char *q_argv[GUBN_ARGS];	/* Arguments to query */
    int status = SUCCESS;	/* Query return status */

    struct save_queue *queue;	/* Queue to hold SMS data */
    struct db_data *data;	/* Structure for data for one tuple */
    short verified = FALSE;	/* Have we verified the user? */

    /* Zero the mit_id field in the formatted packet structure.  This
       being zeroed means that no user was found. */
    bzero(message->db.mit_id,sizeof(message->db.mit_id));
    
    if (status == SUCCESS)
    {
	/* Get ready to make an SMS query */
	q_name = "get_user_by_name";
	q_argc = GUBN_ARGS;	/* #defined in this routine */
	q_argv[0] = message->first;
	q_argv[1] = message->last;
	
	/* Create queue to hold information */
	queue = sq_create();
	
	/* Do it */
	status = sms_query(q_name,q_argc,q_argv,db_callproc,(char *)queue);
	
#ifdef DEBUG
	fprintf(stderr," %d returned by get_user_by_name\n",status);
#endif
	
	if (status == SMS_SUCCESS) 
	{
	    /* Traverse the list, freeing data as we go.  If sq_get_data()
	       returns zero if there is no more data on the queue. */
	    while (sq_get_data(queue,&data))
	    {
		if (!verified)
		    /* parse_encrypted returns zero on success */
		    verified = (parse_encrypted(message,data) == SUCCESS);
		free((char *)data);
	    }
	}

	/* Destroy the queue */
	sq_destroy(queue);
    }
    
#ifdef DEBUG
    fprintf(stderr,"Returned from find_user\n");
    fprintf(stderr,"   MIT ID: %s\n", message->db.mit_id);
    fprintf(stderr,"   Registration status: %d\n",message->db.reg_status);
    fprintf(stderr,"   uid: %s\n",message->db.uid);
    fprintf(stderr,"   login: %s\n",message->db.login);
    fprintf(stderr,"   Status from query: %d\n",status);
#endif DEBGUG

    return status;
}

int verify_user(message,retval)
  struct msg *message;
  char *retval;
  /* This routine determines whether a user is in the databse and returns
     his state so that other routines can figure out whether he is the 
     correct state for various transactions. */
     
{
    int status = SUCCESS;	/* Return status */

    /* Log that we are about to veryify user */
    com_err(whoami,0,"verify_user %s %s",message->first,message->last);

    /* Figure out what user (if any) can be found based on the
       encrypted information in the packet.  (See the comment on 
       parse_encrypted().) */

    status = find_user(message);

    /* If SMS coudn't find the user */
    if (status == SMS_NO_MATCH) 
	status = UREG_USER_NOT_FOUND;
    else if (status == SMS_SUCCESS)
    {
	/* If the information sent over in the packet did not point to a
	   valid user, the mit_id field in the formatted packet structure
	   will be empty. */
	if (message->db.mit_id[0] == NULL)
	    status = UREG_USER_NOT_FOUND;
	/* If the user was found but the registration has already started,
	   use this as the status */
	else
	{
	    switch (message->db.reg_status)
	    {
	      case US_NO_LOGIN_YET:
		status = SUCCESS;
		break;
	      case US_REGISTERED:
		status = UREG_ALREADY_REGISTERED;
		break;
	      case US_NO_PASSWD:
		status = UREG_NO_PASSWD_YET;
		break;
	      case US_DELETED:
		status = UREG_DELETED;
		break;
	      case US_NOT_ALLOWED:
		status = UREG_NOT_ALLOWED;
		break;

	      default:
		status = UREG_MISC_ERROR;
		critical_alert(FAIL_INST,"Bad user state for login %s.",
			       message->db.login);
		break;
	    }
	    /* Set retval to the login name so that the client can use
	       it in the error message it will give the user. */
	    (void) strcpy(retval,message->db.login);
	}
    }
    
    com_err(whoami,status," returned from verify_user");

    return status;
}
	
int ureg_get_tkt()
{
    int status = SUCCESS;	/* Return status */

    /* Get keys for interacting with Kerberos admin server. */
    /* principal, instance, realm, service, service instance, life, file */
    if (status = get_svc_in_tkt("register", "sms", krbrealm, "changepw", 
				krbhst, 1, KEYFILE))
	status += krb_err_base;

#ifdef DEBUG
    if (status == SUCCESS)
	com_err(whoami,status,"Succeeded in getting tickets.");
    else
	com_err(whoami,status,"Failed to get tickets.");
#endif
    return status;
}

int null_callproc(argc,argv,message)
  int argc;
  char *argv[];
  char *message;
  /* This routine is a null callback that should be used for queries that
     do not return tuples.  If it ever gets called, something is wrong. */
{
    critical_alert(FAIL_INST,"Something returned from an update query.");
    return FAILURE;
}

int do_admin_call(login, passwd, uid)
  char *login;			/* Requested kerberos principal */
  char *passwd;			/* Requested password */
  char *uid;			/* Uid of user who owns this principal */
  /* This routine gets tickets, makes the appropriate call to admin_call,
     and destroys tickets. */
{
    int status;			/* Error status */
    char uid_buf[20];		/* Holds uid for kerberos */

    com_err(whoami,0,"Entering do_admin_call");

    if ((status = ureg_get_tkt()) == SUCCESS)
    {
	/* Try to reserve kerberos principal.  To do this, send a 
	   password request and a null password.  It will only succeed
	   if there is no principal or the principal exists and has no 
	   password. */
	/* 13 chars of placebo for backwards-compatability - the admin
	   server protocol reqires this. */
	bzero(uid_buf,sizeof(uid_buf));
	(void) sprintf(uid_buf, "%13s", uid);
	
	if ((status = admin_call(ADMIN_ADD_NEW_KEY_ATTR, login, 
				 "", passwd, uid_buf)) != KSUCCESS)
	{
	    com_err(whoami,status," server error: %s",admin_errmsg);
	    
	    if (strcmp(admin_errmsg,
		       "Principal already in kerberos database.") == 0)
		status = UREG_KRB_TAKEN;
	    critical_alert(FAIL_INST,"%s is known to Kerberos but not SMS.", 
			   login);
	}
    }
    
    dest_tkt();
    com_err(whoami,status," returned from do_adin_call");
    return status;
}

int reserve_user(message,retval)
  struct msg *message;
  char *retval;
{
    int q_argc;			/* Number of arguments to query */
    char *q_argv[3];		/* Arguments to SMS query */
    char *q_name;		/* Name of SMS query */
    int status = SUCCESS;	/* General purpose error status */
    char fstype_buf[7];		/* Buffer to hold fs_type, a 16 bit number */
    char *login;		/* The login name the user wants */
    register int i;		/* A counter */

    /* Log that we are about to reserve a user. */
    com_err(whoami, 0, "reserve_user %s %s", 
	    message->first, message->last);
    
    /* Check to make sure that we can verify this user. */
    if ((status = verify_user(message,retval)) == SUCCESS)
    {
	/* Get the requested login name from leftover packet information. */
	login = message->leftover;

	/* Check the login name for validity.  The login name is currently
	   is allowed to contain lowercase letters and numbers in any
	   position and underscore characters and periods in any position
	   but the first. */
	if ((strlen(login) < MIN_UNAME) || (strlen(login) > MAX_UNAME))
	    status = UREG_INVALID_UNAME;
    }
    if (status == SUCCESS)
	if ((login[0] == '.') || (login[1] == '_'))
	    status = UREG_INVALID_UNAME;
    if (status == SUCCESS)
    {
	for (i = 0; i < strlen(login); i++)
	    if (!islower(login[i]) && !isdigit(login[i]) &&
		(login[i] != '_') && (login[i] != '.'))
	    {
		status = UREG_INVALID_UNAME;
		break;
	    }
    }
    if (status == SUCCESS)
    {
	/* Now that we have a valid user with a valid login... */

	/* First, try to reserve the user in SMS. */
	(void) sprintf(fstype_buf,"%d",SMS_FS_STUDENT);
	q_name = "register_user";
	q_argv[0] = message->db.uid;
	q_argv[1] = login;
	q_argv[2] = fstype_buf;
	q_argc = 3;
	status = sms_query(q_name,q_argc,q_argv,null_callproc,(char *)0);
	switch (status)
	{
	  case SMS_SUCCESS:
	    status = SUCCESS;
	    break;
	  case SMS_IN_USE:
	    status = UREG_LOGIN_USED;
	    break;
	  default:
	    status = UREG_MISC_ERROR;
	    critical_alert(FAIL_INST,"%s returned from register_user.",
			   error_message(status));
	    break;
	}
    }
    if (status == SUCCESS)
    {
	/* SMS login was successfully created; try to reserve kerberos
	   principal. */
	/* If this routine fails, store the login in the retval so
	   that it can be used in the client-side error message. */
	if ((status = do_admin_call(login, "", message->db.uid)) != SUCCESS)
	    (void) strcpy(retval, login);
    }

    com_err(whoami, status, " returned from reserve_user");
    
    return status;
}

int set_final_status(login)
  char *login;
    /* This routine updates a user's registration status to fully 
       registered. */
{
    char *q_name;		/* Name of SMS query */
    int q_argc;			/* Number of arguments for SMS query */
    char *q_argv[2];		/* Arguments to get user by uid */
    char state[7];		/* Can hold a 16 bit integer */
    int status;			/* Error status */

    com_err(whoami, 0, "Setting final status for %s", login);

    (void) sprintf(state,"%d",US_REGISTERED);
    q_name = "update_user_status";
    q_argc = 2;
    q_argv[0] = login;
    q_argv[1] = state;
    if ((status = sms_query(q_name, q_argc, q_argv, null_callproc,
			    (char *)0)) != SMS_SUCCESS)
	critical_alert(FAIL_INST,"%s returned from update_user_status.",
		       error_message(status));
    
    com_err(whoami,status," returned from set_final_status");
    return status;
}


int set_password(message,retval)
  struct msg *message;
  char *retval;
  /* This routine is used to set the initial password for the new user. */
{
    int status = SUCCESS;	/* Return status */
    char *passwd;		/* User's password */

    com_err(whoami, 0, " set_password %s %s",
	    message->first, message->last);

    status = verify_user(message,retval);

    /* Don't set the password unless the registration status of the user
       is that he exists and has no password. */
    if (status == SUCCESS)
	status = UREG_NO_LOGIN_YET;
    if (status == UREG_NO_PASSWD_YET)
    {
	/* User is in proper state for this transaction. */
	
	passwd = message->leftover;
	
	/* Set password. */
	if ((status = do_admin_call(message->db.login, 
				    passwd, message->db.uid)) != SUCCESS)
	    /* If failure, allow login name to be used in client 
	       error message */
	    (void) strcpy(retval,message->db.login);
	else
	    /* Otherwise, mark user as finished. */
	    status = set_final_status(message->db.login);
    }
    com_err(whoami, status, " returned from set_passwd");
    
    return status;
}
    
/*
 * Local Variables:
 * mode: c
 * c-argdecl-indent: 2
 * c-brace-offset: -4
 * c-continued-statement-offset: 4
 * c-indent-level: 4
 * c-label-offset: -2
 * End:
 */
