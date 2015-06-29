/* $HeadURL: svn+ssh://svn.mit.edu/moira/trunk/moira/server/qrtn.h $ $Id: qrtn.h 4150 2013-12-04 01:45:08Z zacheiss $
 *
 *  Used by the SQL query routines of the Moira server.
 */

#define MR_STMTBUF_LEN (MAX_FIELD_WIDTH * 2)
extern char stmt_buf[MR_STMTBUF_LEN];

void dbmserr(void);

/* eof:qtrn.h */
