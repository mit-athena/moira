/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qrtn.h,v 1.6 2000-01-28 00:03:36 danw Exp $
 *
 *  Used by the SQL query routines of the Moira server.
 */

#define MR_STMTBUF_LEN 1024
extern char stmt_buf[MR_STMTBUF_LEN];

void dbmserr(void);

/* eof:qtrn.h */
