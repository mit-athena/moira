/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qrtn.h,v 1.5 1998-02-23 19:24:35 danw Exp $
 *
 *  Used by the SQL query routines of the Moira server.
 */

#define MR_STMTBUF_LEN 1024

extern int mr_sig_length;
extern char stmt_buf[MR_STMTBUF_LEN];

void dbmserr(void);

/* eof:qtrn.h */
