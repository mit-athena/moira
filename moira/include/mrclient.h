/* $Id: mrclient.h,v 1.1 1999-05-25 21:19:59 danw Exp $
 *
 * Copyright (C) 1999 by the Massachusetts Institute of Technology
 *
 */

#ifndef _mrclient_h_
#define _mrclient_h_

#define MRCL_SUCCESS		0
#define MRCL_FAIL		1
#define MRCL_REJECT		2
#define MRCL_ENOMEM		3
#define MRCL_MOIRA_ERROR	4
#define MRCL_AUTH_ERROR		4

int mrcl_connect(char *server, char *client, int auth);
char *mrcl_krb_user(void);

int mrcl_validate_pobox_smtp(char *user, char *address, char **ret);

#endif /* _mrclient_h_ */
