/*
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/reg_svr.c,v $
 *      $Author: danw $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/reg_svr.c,v 1.47 1998-01-06 20:40:08 danw Exp $
 *
 *      Copyright (C) 1987, 1988 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 *      Server for user registration with Moira and Kerberos.
 *
 *      This program is a client of the Kerberos admin_server and a
 *      server for the userreg program.  It is not a client of the
 *      Moira server as it is linked with libmoiraglue which bypasses
 *      the network protocol.
 */

#ifndef lint
static char *rcsid_reg_svr_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/reg_svr.c,v 1.47 1998-01-06 20:40:08 danw Exp $";
#endif lint

#include <mit-copyright.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/file.h>
#include <krb.h>
#include <des.h>
#include <kadm.h>
#include <kadm_err.h>
#include <krb_err.h>
#include <errno.h>
#include <com_err.h>
#include "moira.h"
#include "moira_site.h"
#include "reg_svr.h"

extern char admin_errmsg[];

void reg_com_err_hook();

int main(int argc, char *argv[])
{
  struct msg message;		/* Storage for parsed packet */
  int status = SUCCESS;		/* Error status */
  char retval[BUFSIZ];		/* Buffer to hold return message for client */

  void req_initialize();	/* Initialize request layer */
  void get_request();		/* Get a request */
  void report();		/* Respond to a request */

  /* Initialize */
  whoami = argv[0];

  /* Error messages sent one line at a time */
  setvbuf(stderr, NULL, _IOLBF, BUFSIZ);
  setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
  set_com_err_hook(reg_com_err_hook);

  /* Initialize com_err error tables */
  init_ureg_err_tbl();
  init_krb_err_tbl();
  init_kadm_err_tbl();
  initialize_gdss_error_table();

  /* Set the name of our kerberos ticket file */
  krb_set_tkt_string("/tmp/tkt_ureg");

  /* Connect to the Moira server */
  if ((status = mr_connect(MOIRA_SERVER)) != MR_SUCCESS)
    {
      com_err(whoami, status, " on mr_connect");
      exit(1);
    }

  /* Authorize, telling the server who you are */
  if ((status = mr_auth(whoami)) != MR_SUCCESS)
    {
      com_err(whoami, status, " on mr_auth");
      exit(1);
    }

  journal = fopen(REGJOURNAL, "a");
  if (!journal)
    {
      com_err(whoami, errno, " while opening journal file");
      exit(1);
    }

  /* Allow request layer to initialize */
  req_initialize();

  /* Sit around waiting for requests from the client. */
  for (;;)
    {
      get_request(&message);

      switch (message.request)
	{
	case UREG_VERIFY_USER:
	  status = verify_user(&message, retval);
	  break;
	case UREG_RESERVE_LOGIN:
	  status = reserve_user(&message, retval);
	  break;
	case UREG_SET_PASSWORD:
	case UREG_GET_KRB:
	  status = set_password(&message, retval);
	  break;
	case UREG_SET_IDENT:
	  status = set_identity(&message, retval);
	  break;
	case UREG_GET_SECURE:
	  status = get_secure(&message, retval);
	  break;
	case UREG_SET_SECURE:
	  status = set_secure(&message, retval);
	  break;
	default:
	  status = UREG_UNKNOWN_REQUEST;
	  critical_alert(FAIL_INST, "Unknown request %d from userreg.",
			 message.request);
	  break;
	}

      /* Report what happened to client */
      report(status, retval);
    }
}

/* This routine makes sure that the ID from the database matches
   the ID sent accross in the packet.  The information in the packet
   was created in the following way:

   The database used to contain encrypted IDs.  Now we don't encrypt
   them in the database, although there are still some encrypted IDs
   there.

   The plain text ID number was encrypted via EncryptID() resulting
   in the form that would appear in the Moira database.  This is
   concatenated to the plain text ID so that the ID string contains plain
   text ID followed by a null followed by the encrypted ID.  Other
   information such as the username or password is appended.  The whole
   thing is then DES encrypted using the encrypted ID as the source of
   the key.

   This routine tries each ID in the database that belongs
   to someone with this user's first and last name and tries to
   decrypt the packet with this information.  If it succeeds, it returns
   zero and initializes all the fields of the formatted packet structure
   that depend on the encrypted information. */

int parse_encrypted(struct msg *message, struct db_data *data)
{
  des_cblock key;		/* The key for DES en/decryption */
  des_key_schedule sched;	/* En/decryption schedule */
  static char decrypt[BUFSIZ];	/* Buffer to hold decrypted information */
  long decrypt_len;		/* Length of decypted ID information */
  char recrypt[14];		/* Buffer to hold re-encrypted information */
  static char hashid[14];	/* Buffer to hold one-way encrypted ID */
  char idnumber[BUFSIZ];	/* Buffer to hold plain-text ID */
  char *temp;			/* A temporary string pointer */
  int len;			/* Keeps track of length left in packet */
  int status = SUCCESS;		/* Error status */

  /* Make the decrypted information length the same as the encrypted
     information length.  Both are integral multples of eight bytes
     because of the DES encryption routines. */
  decrypt_len = message->encrypted_len;

  /* Get key from the possibly one-way encrypted ID in the Moira database */
  if (data->mit_id[0] >= '0' && data->mit_id[0] <= '9')
    {
      char buf[32];

      EncryptID(buf, data->mit_id, message->first, message->last);
      des_string_to_key(buf, key);
    }
  else
    des_string_to_key(data->mit_id, key);

  /* Get schedule from key */
  des_key_sched(key, sched);
  /* Decrypt information from packet using this key.  Since decrypt_len
     is an integral multiple of eight bytes, it will probably be null-
     padded. */
  des_pcbc_encrypt(message->encrypted, decrypt, decrypt_len,
		   sched, key, DES_DECRYPT);

  /* Extract the plain text and encrypted ID fields from the decrypted
     packet information. */
  /* Since the decrypted information starts with the plain-text ID
     followed by a null, if the decryption worked, this will only
     copy the plain text part of the decrypted information.  It is
     important that strncpy be used because if we are not using the
     correct key, there is no guarantee that a null will occur
     anywhere in the string. */
  strncpy(idnumber, decrypt, decrypt_len);
  /* Check that the idnumber of a mismatched decryption doesn't overflow
   * the buffer. */
  if (strlen(idnumber) != 9)
    return FAILURE;

  /* Point temp to the end of the plain text ID number. */
  temp = decrypt + strlen(idnumber) + 1;
  /* Find out how much more packet there is. */
  len = message->encrypted_len - (temp - decrypt);
  /* Copy the next CRYPT_LEN bytes of the decrypted information into
     hashid if there are CRYPT_LEN more bytes to copy.  There will be
     if we have the right key. */
  strncpy(hashid, temp, min(len, CRYPT_LEN));
  /* Point temp to the end of the encrypted ID field */
  temp += strlen(hashid) + 1;
  /* Find out how much more room there is. */
  len = message->encrypted_len - (temp - decrypt);

  /* Now compare encrypted ID and clear text ID for a match. */
  if (strcmp(hashid, data->mit_id) &&
      strcmp(idnumber, data->mit_id))
    status = FAILURE;

  if (status == SUCCESS)
    {
      /* We made it.  Now we can finish initializing message. */
      /* Point leftover to whatever is left over! */
      message->leftover = temp;
      message->leftover_len = len;
      /* Since we know we have the right user, fill in the information
	 from the Moira database. */
      message->db.reg_status = data->reg_status;
      strncpy(message->db.uid, data->uid, sizeof(message->db.uid));
      strncpy(message->db.mit_id, data->mit_id, sizeof(message->db.mit_id));
      strncpy(message->db.login, data->login, sizeof(message->db.login));
    }

    return status;
}

/* This function is called by mr_query after each tuple found.  It is
   used by find_user to cache information about each user found.  */
int db_callproc(int argc, char **argv, struct save_queue *queue)
{
  struct db_data *data;	/* Structure to store the information in */
  int status = SUCCESS;	/* Error status */

  if (argc != U_END)
    {
      critical_alert(FAIL_INST, "Wrong number of arguments returned "
		     "from get_user_account_by_name.");
      status = MR_ABORT;
    }
  else
    {
      /* extract the needed information from the results of the Moira query */
      data = malloc(sizeof(struct db_data));
      data->reg_status = atoi(argv[U_STATE]);
      strncpy(data->login, argv[U_NAME], sizeof(data->login));
      strncpy(data->mit_id, argv[U_MITID], sizeof(data->mit_id));
      strncpy(data->uid, argv[U_UID], sizeof(data->uid));
      sq_save_data(queue, data);
    }

  return status;
}

/* This routine verifies that a user is allowed to register by finding
   him/her in the Moira database.  It returns the status of the Moira
   query that it calls. */
int find_user(struct msg *message)
{
#define GUBN_ARGS 2		/* Arguements needed by get_user_by_name */
  char *q_name;			/* Name of query */
  int q_argc;			/* Number of arguments for query */
  char *q_argv[GUBN_ARGS];	/* Arguments to query */
  int status = SUCCESS;		/* Query return status */

  struct save_queue *queue;	/* Queue to hold Moira data */
  struct db_data *data;		/* Structure for data for one tuple */
  short verified = FALSE;	/* Have we verified the user? */

  /* Zero the mit_id field in the formatted packet structure.  This
     being zeroed means that no user was found. */
  memset(message->db.mit_id, 0, sizeof(message->db.mit_id));

  if (status == SUCCESS)
    {
      /* Get ready to make an Moira query */
      q_name = "get_user_account_by_name";
      q_argc = GUBN_ARGS;	/* #defined in this routine */
      q_argv[0] = message->first;
      q_argv[1] = message->last;

      /* Create queue to hold information */
      queue = sq_create();

      /* Do it */
      status = mr_query(q_name, q_argc, q_argv, db_callproc, (char *)queue);

      if (status == MR_SUCCESS)
	{
	  /* Traverse the list, freeing data as we go.  If sq_get_data()
	     returns zero if there is no more data on the queue. */
	  while (sq_get_data(queue, &data))
	    {
	      if (!verified)
		/* parse_encrypted returns zero on success */
		verified = (parse_encrypted(message, data) == SUCCESS);
	      free(data);
	    }
	}

      /* Destroy the queue */
      sq_destroy(queue);
    }

  return status;
}

/* This routine determines whether a user is in the databse and returns
   his state so that other routines can figure out whether he is the
   correct state for various transactions. */
int verify_user(struct msg *message, char *retval)
{
  int status = SUCCESS;	/* Return status */

  /* Log that we are about to veryify user */
  com_err(whoami, 0, "verifying user %s %s", message->first, message->last);

  /* Figure out what user (if any) can be found based on the
     encrypted information in the packet.  (See the comment on
     parse_encrypted().) */

  status = find_user(message);

  /* If Moira coudn't find the user */
  if (status == MR_NO_MATCH)
    status = UREG_USER_NOT_FOUND;
  else if (status == MR_SUCCESS)
    {
      /* If the information sent over in the packet did not point to a
	 valid user, the mit_id field in the formatted packet structure
	 will be empty. */
      if (message->db.mit_id[0] == '\0')
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
	    case US_ENROLLED:
	      status = UREG_ENROLLED;
	      break;
	    case US_ENROLL_NOT_ALLOWED:
	      status = UREG_ENROLL_NOT_ALLOWED;
	      break;
	    case US_HALF_ENROLLED:
	      status = UREG_HALF_ENROLLED;
	      break;
	    default:
	      status = UREG_MISC_ERROR;
	      critical_alert(FAIL_INST, "Bad user state %d for login %s.",
			     message->db.reg_status, message->db.login);
	      break;
	    }
	  /* Set retval to the login name so that the client can use
	     it in the error message it will give the user. */
	  strcpy(retval, message->db.login);
	}
    }

  if (status)
    com_err(whoami, status, " returned from verify_user");
  else
    com_err(whoami, 0, "User verified");

  return status;
}

int ureg_kadm_init(void)
{
  unsigned int status = SUCCESS;	 /* Return status */
  static char krbrealm[REALM_SZ];	 /* kerberos realm name */
  static char hostbuf[BUFSIZ], *host;	 /* local hostname in principal fmt */
  static int inited = 0;
  char *p;

  if (!inited)
    {
      inited++;
      memset(krbrealm, 0, sizeof(krbrealm));
      if (status = krb_get_lrealm(krbrealm, 1))
	{
	  status += krb_err_base;
	  com_err(whoami, status, " fetching kerberos realm");
	  exit(1);
	}
      if (gethostname(hostbuf, sizeof(hostbuf)) < 0)
	com_err(whoami, errno, "getting local hostname");
      host = canonicalize_hostname(strsave(hostbuf));
      for (p = host; *p && *p != '.'; p++)
	{
	  if (isupper(*p))
	    *p = tolower(*p);
	}
      *p = 0;
    }

  /* Get keys for interacting with Kerberos admin server. */
  /* principal, instance, realm, service, service instance, life, file */
  if (status = krb_get_svc_in_tkt(MOIRA_SNAME, host, krbrealm, PWSERV_NAME,
				  KADM_SINST, 1, KEYFILE))
    status += krb_err_base;

  if (status != SUCCESS)
    com_err(whoami, status, " while get admin tickets");
  else
    {
      if ((status = kadm_init_link(PWSERV_NAME, KADM_SINST, krbrealm)) !=
	  KADM_SUCCESS)
	com_err(whoami, status, " while initializing kadmin connection");
    }

  return status;
}

/* This routine is a null callback that should be used for queries that
   do not return tuples.  If it ever gets called, something is wrong. */
int null_callproc(int argc, char *argv[], char *message)
{
  critical_alert(FAIL_INST, "Something returned from an update query.");
  return FAILURE;
}

/*
 * This routine reserves a principal in kerberos by setting up a
 * principal with a random initial key.
 */
int reserve_krb(char *login)
{
  int status = SUCCESS;
  Kadm_vals new;
  des_cblock key;
  u_long *lkey = (u_long *)key;

  if ((status = ureg_kadm_init()) == SUCCESS)
    {
      memset(&new, 0, sizeof(new));
      SET_FIELD(KADM_DESKEY, new.fields);
      SET_FIELD(KADM_NAME, new.fields);

      des_random_key(key);
      new.key_low = htonl(lkey[0]);
      new.key_high = htonl(lkey[1]);
      strcpy(new.name, login);

      com_err(whoami, 0, "Creating kerberos principal for %s", login);
      status = kadm_add(&new);
      if (status != KADM_SUCCESS)
	com_err(whoami, status, " while reserving principal");

      memset(&new, 0, sizeof(new));
    }

  dest_tkt();

  return status;
}

/*
 * This routine reserves a principal in kerberos by setting up a
 * principal with a random initial key.
 */
int setpass_krb(char *login, char *password)
{
  int status = SUCCESS;
  Kadm_vals new;
  des_cblock key;
  u_long *lkey = (u_long *)key;

  if ((status = ureg_kadm_init()) == SUCCESS)
    {
      memset(&new, 0, sizeof(new));
      SET_FIELD(KADM_DESKEY, new.fields);
      SET_FIELD(KADM_NAME, new.fields);

      des_string_to_key(password, key);
      new.key_low = htonl(lkey[0]);
      new.key_high = htonl(lkey[1]);
      strcpy(new.name, login);

      com_err(whoami, 0, "Setting password for %s", login);
      /* First arguement is not used if user has modify privileges */
      if ((status = kadm_mod(&new, &new)) != KADM_SUCCESS)
	{
	  if (status == KADM_NOENTRY)
	    {
	      com_err(whoami, 0, "kerberos principal doesn't exist; creating");
	      if ((status = kadm_add(&new)) != KADM_SUCCESS)
		com_err(whoami, status, " while creating kerberos principal");
	    }
	  else
	    com_err(whoami, status, " while setting password");
	}
    }

  dest_tkt();
  return status;
}

int reserve_user(struct msg *message, char *retval)
{
  int q_argc;			/* Number of arguments to query */
  char *q_argv[3];		/* Arguments to Moira query */
  char *q_name;			/* Name of Moira query */
  int status = SUCCESS;		/* General purpose error status */
  char fstype_buf[7];		/* Buffer to hold fs_type, a 16 bit number */
  char *login;			/* The login name the user wants */
  int i;			/* A counter */

  /* Log that we are about to reserve a user. */
  com_err(whoami, 0, "reserving user %s %s", message->first, message->last);

  /* Check to make sure that we can verify this user. */
  if ((status = verify_user(message, retval)) == SUCCESS)
    {
      /* Get the requested login name from leftover packet information. */
      login = message->leftover;

      /* Check the login name for validity.  The login name is currently
	 is allowed to contain lowercase letters in any position and
	 and numbers and underscore characters in any position but the
	 first. */
      if ((strlen(login) < MIN_UNAME) || (strlen(login) > MAX_UNAME))
	status = UREG_INVALID_UNAME;
    }
  if (status == SUCCESS)
    if ((login[0] == '_') || isdigit(login[0]))
      status = UREG_INVALID_UNAME;

  if (status == SUCCESS)
    {
      for (i = 0; i < strlen(login); i++)
	{
	  if (!islower(login[i]) && !isdigit(login[i]) &&
	      (login[i] != '_'))
	    {
	      status = UREG_INVALID_UNAME;
	      break;
	    }
	}
    }
  if (status == SUCCESS)
    {
      /* Now that we have a valid user with a valid login... */

      /* First, try to reserve the user in Moira. */
      sprintf(fstype_buf, "%d", MR_FS_STUDENT);
      q_name = "register_user";
      q_argv[0] = message->db.uid;
      q_argv[1] = login;
      q_argv[2] = fstype_buf;
      q_argc = 3;
      status = mr_query(q_name, q_argc, q_argv, null_callproc, NULL);
      switch (status)
	{
	case MR_SUCCESS:
	  status = SUCCESS;
	  break;
	case MR_IN_USE:
	  status = UREG_LOGIN_USED;
	  break;
	case MR_DEADLOCK:
	  status = UREG_MISC_ERROR;
	  break;
	default:
	  critical_alert(FAIL_INST, "%s returned from register_user.",
			 error_message(status));
	  status = UREG_MISC_ERROR;
	  break;
	}
    }

  if (status == SUCCESS)
    {
      /*
       * Moira login was successfully created; try to reserve kerberos
       * principal.
       *
       * If this routine fails, store the login in the retval so
       * that it can be used in the client-side error message.
       */
      if ((status = reserve_krb(login)) != SUCCESS)
	strcpy(retval, login);
    }

  if (status)
    com_err(whoami, status, " returned from reserve_user");
  else
    com_err(whoami, 0, "User reserved");

  return status;
}

/* This routine updates a user's registration status to fully
   registered. */
int set_final_status(struct msg *message)
{
  char *login;
  char *q_name;			/* Name of Moira query */
  int q_argc;			/* Number of arguments for Moira query */
  char *q_argv[2];		/* Arguments to get user by uid */
  char state[7];		/* Can hold a 16 bit integer */
  int status;			/* Error status */

  if (message->request == UREG_SET_PASSWORD)
    sprintf(state, "%d", US_REGISTERED);
  else if (message->db.reg_status == US_NO_LOGIN_YET)
    sprintf(state, "%d", US_ENROLLED);
  else
    sprintf(state, "%d", US_ENROLL_NOT_ALLOWED);

  login = message->db.login;
  com_err(whoami, 0, "Setting final status for %s to %s", login, state);

  q_name = "update_user_status";
  q_argc = 2;
  q_argv[0] = login;
  q_argv[1] = state;
  if ((status = mr_query(q_name, q_argc, q_argv, null_callproc, NULL))
      != MR_SUCCESS)
    {
      if (status == MR_DEADLOCK)
	status = UREG_MISC_ERROR;
      else
	critical_alert(FAIL_INST, "%s returned from update_user_status.",
		       error_message(status));
    }
  if (status)
    com_err(whoami, status, " returned from set_final_status");
  else
    com_err(whoami, 0, "Final status set");
  return status;
}


/* This routine is used to set the initial password for the new user. */
int set_password(struct msg *message, char *retval)
{
  int status = SUCCESS;	/* Return status */
  char *passwd;		/* User's password */

  com_err(whoami, 0, "setting password %s %s", message->first, message->last);

  status = verify_user(message, retval);

  /* Don't set the password unless the registration status of the user
     is that he exists and has no password. */
  if (status == SUCCESS)
    status = UREG_NO_LOGIN_YET;
  if ((message->request == UREG_SET_PASSWORD &&
       status == UREG_NO_PASSWD_YET) ||
      (message->request == UREG_GET_KRB &&
       status == UREG_HALF_ENROLLED))
    {
      /* User is in proper state for this transaction. */

      passwd = message->leftover;

      /* Set password. */
      if ((status = setpass_krb(message->db.login, passwd)) != SUCCESS)
	/* If failure, allow login name to be used in client error message */
	strcpy(retval, message->db.login);
      else
	/* Otherwise, mark user as finished. */
	status = set_final_status(message);
    }

  if (status)
    com_err(whoami, status, " returned from set_passwd");
  else
    com_err(whoami, 0, "Password set");

  return status;
}


int getuserinfo(int argc, char **argv, char **qargv)
{
  int status = SUCCESS;
  int  i;

  if (argc != U_END)
    {
      critical_alert(FAIL_INST,
		     "Wrong number of args returned from get_user_by_uid");
      status = MR_ABORT;
    }
  else
    {
      qargv[U_NAME] = strsave(argv[U_NAME]);
      for (i = 1; i < U_MODTIME; i++)
	qargv[i + 1] = strsave(argv[i]);
      qargv[U_MODTIME + 1] = NULL;
    }
  return status;
}


int set_identity(struct msg *message, char *retval)
{
  int q_argc;			/* Number of arguments to query */
  char *q_argv[U_END];		/* Arguments to Moira query */
  char *q_name;			/* Name of Moira query */
  int status = SUCCESS;		/* General purpose error status */
  char fstype_buf[7];		/* Buffer to hold fs_type, a 16 bit number */
  char *login;			/* The login name the user wants */
  int i;			/* A counter */

  /* Log that we are about to reserve a user. */
  com_err(whoami, 0, "setting identity %s %s",
	  message->first, message->last);

  /* Check to make sure that we can verify this user. */
  status = verify_user(message, retval);
  if (status == SUCCESS || status == UREG_NOT_ALLOWED)
    {
      status = SUCCESS;
      /* Get the requested login name from leftover packet information. */
      login = message->leftover;

      /* Check the login name for validity.  The login name is currently
	 is allowed to contain lowercase letters in any position and
	 and numbers and underscore characters in any position but the
	 first. */
      if ((strlen(login) < MIN_UNAME) || (strlen(login) > MAX_UNAME))
	status = UREG_INVALID_UNAME;
    }
  if (status == SUCCESS)
    {
      if ((login[0] == '_') || isdigit(login[0]))
	status = UREG_INVALID_UNAME;
    }
  if (status == SUCCESS)
    {
      for (i = 0; i < strlen(login); i++)
	{
	  if (!islower(login[i]) && !isdigit(login[i]) &&
	      (login[i] != '_'))
	    {
	      status = UREG_INVALID_UNAME;
	      break;
	    }
	}
    }
  if (status == SUCCESS)
    {
      /* Now that we have a valid user with a valid login... */

      q_argv[0] = message->db.uid;
      status = mr_query("get_user_account_by_uid", 1, q_argv,
			getuserinfo, (char *)q_argv);
      if (status != SUCCESS)
	{
	  com_err(whoami, status, " while getting user info");
	  return status;
	}
      q_argv[U_NAME + 1] = login;
      q_argv[U_STATE + 1] = "7";
      q_argv[U_SIGNATURE + 1] = "";
      status = mr_query("update_user_account", U_MODTIME + 1, q_argv,
			null_callproc, NULL);
      switch (status)
	{
	case MR_SUCCESS:
	  status = SUCCESS;
	  break;
	case MR_IN_USE:
	case MR_NOT_UNIQUE:
	  status = UREG_LOGIN_USED;
	  break;
	case MR_DEADLOCK:
	  status = UREG_MISC_ERROR;
	  break;
	default:
	  critical_alert(FAIL_INST, "%s returned from update_user_account.",
			 error_message(status));
	  status = UREG_MISC_ERROR;
	  break;
	}
    }
  if (status == SUCCESS)
    {
      /* Moira login was successfully created; try to reserve kerberos
	 principal. */
      /* If this routine fails, store the login in the retval so
	 that it can be used in the client-side error message. */
      if ((status = reserve_krb(login)) != SUCCESS)
	strcpy(retval, login);
    }

  if (status)
    com_err(whoami, status, " returned from set_identity");
  else
    com_err(whoami, 0, "Identity set");

  return status;
}


void reg_com_err_hook(char *whoami, int code, char *fmt, caddr_t pvar)
{
  if (whoami)
    {
      fputs(whoami, stderr);
      fputs(": ", stderr);
    }
  if (code)
    fputs(error_message(code), stderr);
  if (fmt)
    _doprnt(fmt, pvar, stderr);
  putc('\n', stderr);
  fflush(stderr);
}


/* Find out if someone's secure instance password is set.
 * Returns UREG_ALREADY_REGISTERED if set, SUCCESS (0) if not.
 */

int get_secure(struct msg *message, char *retval)
{
  int status;
  char *argv[U_END];

  com_err(whoami, 0, "checking status of secure password for %s",
	  message->first);
  argv[0] = message->first;
  status = mr_query("get_user_account_by_login", 1, argv, getuserinfo,
		    (char *)argv);
  if (status != SUCCESS)
    {
      com_err(whoami, status, " while getting user info");
      return status;
    }
  if (atoi(argv[U_SECURE + 1]))
    return UREG_ALREADY_REGISTERED;
  return SUCCESS;
}


/* Set someone's secure instance password. */

int set_secure(struct msg *message, char *retval)
{
  int status, i;
  char *argv[U_END], hostbuf[256], *bp, *p, buf[512], *passwd, *id;
  KTEXT_ST creds;
  AUTH_DAT auth;
  C_Block key;
  Key_schedule keys;
  Kadm_vals kv;
  u_long *lkey = (u_long *)key;
  struct timeval now;
  static int inited = 0;
  static char *host;
  extern char *krb_get_phost(char *);

  if (!inited)
    {
      inited++;
      if (gethostname(hostbuf, sizeof(hostbuf)) < 0)
	com_err(whoami, errno, "getting local hostname");
      host = strsave(krb_get_phost(hostbuf));
    }

  com_err(whoami, 0, "setting secure passwd for %s", message->first);
  argv[0] = message->first;
  status = mr_query("get_user_account_by_login", 1, argv, getuserinfo,
		    (char *)argv);
  if (status != SUCCESS)
    {
      com_err(whoami, status, " while getting user info");
      return status;
    }
  if (atoi(argv[U_SECURE + 1]))
    {
      com_err(whoami, UREG_ALREADY_REGISTERED, "in set_secure()");
      return UREG_ALREADY_REGISTERED;
    }

  bp = message->encrypted;
  /* round up to word boundary */
  bp = (char *)((((u_long)bp + 3) >> 2) << 2);

  creds.length = ntohl(*((int *)bp));
  bp += sizeof(int);
  memcpy(creds.dat, bp, creds.length);
  creds.mbz = 0;
  bp += creds.length;

  status = krb_rd_req(&creds, "changepw", host, cur_req_sender(), &auth, "");
  if (status)
    {
      status += krb_err_base;
      com_err(whoami, status, " verifying credentials in set_secure()");
      return status;
    }

  message->leftover_len = ntohl(*((int *)(bp)));
  bp += sizeof(int);
  message->leftover = bp;

  des_key_sched(auth.session, keys);
  des_pcbc_encrypt(message->leftover, buf, message->leftover_len,
		   keys, auth.session, 0);

  id = buf;
  passwd = strchr(buf, ',');
  *passwd++ = 0;

  if (strcmp(id, argv[U_MITID + 1]))
    {
      char buf[32];

      EncryptID(buf, id, argv[U_FIRST + 1], argv[U_LAST + 1]);
      if (strcmp(buf, argv[U_MITID + 1]))
	{
	  status = UREG_USER_NOT_FOUND;
	  com_err(whoami, status, "IDs mismatch: %s (%s), %s", id, buf,
		  argv[U_MITID + 1]);
	  return status;
	}
    }

  /* now do actual password setting stuff */

  if ((status = ureg_kadm_init()) != SUCCESS)
    {
      com_err(whoami, status, "initing kadm stuff");
      return status;
    }

  memset(&kv, 0, sizeof(kv));
  SET_FIELD(KADM_DESKEY, kv.fields);
  SET_FIELD(KADM_NAME, kv.fields);
  SET_FIELD(KADM_INST, kv.fields);
  des_string_to_key(passwd, key);
  kv.key_low = htonl(lkey[0]);
  kv.key_high = htonl(lkey[1]);
  strcpy(kv.name, message->first);
  strcpy(kv.instance, "extra");

  if ((status = kadm_add(&kv)) != KADM_SUCCESS)
    {
      com_err(whoami, status, " while creating kerberos principal");
      return status;
    }

  argv[0] = message->first;
  argv[1] = buf;
  gettimeofday(&now, NULL);
  sprintf(buf, "%d", now.tv_sec);
  status = mr_query("update_user_security_status", 2, argv, getuserinfo,
		    (char *)argv);
  if (status != SUCCESS)
    {
      com_err(whoami, status, " while updating user status");
      return status;
    }
  return SUCCESS;
}
