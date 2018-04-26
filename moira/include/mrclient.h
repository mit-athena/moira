/* $Id$
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

#define MRCL_MID_ANY           0
#define MRCL_MID_HWADDR        1
#define MRCL_MID_DUID          2
#define MRCL_MID_NONE          3

struct mrcl_ace_type {
  int type;
  char *name, *tag;
};

struct mrcl_identifier_type {
  int type;
  char *value;
};

struct mrcl_netaddr_type {
  char *network;
  char *address;
};

struct mrcl_addropt_type {
  char *address;
  char *opt;
};

int mrcl_connect(char *server, char *client, int version, int auth);
char *mrcl_krb_user(void);
char *partial_canonicalize_hostname(char *s);

int mrcl_validate_pobox_smtp(char *user, char *address, char **ret);
int mrcl_validate_kerberos_member(char *str, char **ret);

struct mrcl_ace_type *mrcl_parse_member(char *s);
struct mrcl_identifier_type *mrcl_parse_mach_identifier(char *s);
struct mrcl_netaddr_type *mrcl_parse_netaddr(char *s);
struct mrcl_addropt_type *mrcl_parse_addropt(char *s);

char *mrcl_get_message(void);
void mrcl_com_err(char *whoami);

#endif /* _mrclient_h_ */
