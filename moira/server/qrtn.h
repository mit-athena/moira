/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qrtn.h,v 1.2 1997-01-20 18:26:24 danw Exp $ 
 *  
 *  Used by the SQL query routines of the Moira server.
 */

#define MR_CDUMMY_LEN  256
#define MR_STMTBUF_LEN 1024

extern int mr_sig_length;
extern char stmt_buf[MR_STMTBUF_LEN];

extern void dbmserr();

/* eof:qtrn.h */
