/* $Id: mrclient.h 3956 2010-01-05 20:56:56Z zacheiss $
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

int mrcl_connect(char *server, char *client, int version, int auth);
char *mrcl_krb_user(void);
char *partial_canonicalize_hostname(char *s);

int mrcl_validate_pobox_smtp(char *user, char *address, char **ret);

#endif /* _mrclient_h_ */
