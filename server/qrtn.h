/* $HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/server/qrtn.h $ $Id: qrtn.h 3956 2010-01-05 20:56:56Z zacheiss $
 *
 *  Used by the SQL query routines of the Moira server.
 */

#define MR_STMTBUF_LEN 1024
extern char stmt_buf[MR_STMTBUF_LEN];

void dbmserr(void);

/* eof:qtrn.h */
