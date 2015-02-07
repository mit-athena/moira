/* $Id: mrclient.h 4160 2014-04-22 15:51:03Z zacheiss $
 *
 * Copyright (C) 1999 by the Massachusetts Institute of Technology
 *
 */

#ifndef _mrclient_h_
#define _mrclient_h_

#define MRCL_SUCCESS		0
#define MRCL_FAIL		1
#define MRCL_REJECT		2
#define MRCL_WARN               3
#define MRCL_ENOMEM		4
#define MRCL_MOIRA_ERROR	5
#define MRCL_AUTH_ERROR		6

/* It is important that M_USER < M_LIST < M_STRING */
#define MRCL_M_ANY           0
#define MRCL_M_USER          1
#define MRCL_M_LIST          2
#define MRCL_M_STRING        3
#define MRCL_M_KERBEROS      4
#define MRCL_M_MACHINE       5
#define MRCL_M_NONE          6

struct mrcl_ace_type {
  int type;
  char *name, *tag;
};

int mrcl_connect(char *server, char *client, int version, int auth);
char *mrcl_krb_user(void);
char *partial_canonicalize_hostname(char *s);

int mrcl_validate_pobox_smtp(char *user, char *address, char **ret);

struct mrcl_ace_type *mrcl_parse_member(char *s);

char *mrcl_get_message(void);
void mrcl_com_err(char *whoami);

#endif /* _mrclient_h_ */
