/* $Id: mrclient-internal.h,v 1.2 2008-08-22 17:49:11 zacheiss Exp $
 *
 * Copyright (C) 1999 by the Massachusetts Institute of Technology
 *
 */

#ifndef _mrclient_internal_h_
#define _mrclient_internal_h_

void mrcl_set_message(char *, ...);
void mrcl_clear_message(void);

enum { MAILTYPE_ERROR, MAILTYPE_IMAP, MAILTYPE_EXCHANGE, MAILTYPE_POP, 
       MAILTYPE_LOCAL, MAILTYPE_MAILHUB, MAILTYPE_SMTP };
int mailtype(char *machine);

#endif /* _mrclient_internal_h_ */
