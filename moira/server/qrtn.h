/* $HeadURL$ $Id$
 *
 *  Used by the SQL query routines of the Moira server.
 */

#define MR_STMTBUF_LEN (MAX_FIELD_WIDTH * 2)
extern char stmt_buf[MR_STMTBUF_LEN];

void dbmserr(void);

/* eof:qtrn.h */
