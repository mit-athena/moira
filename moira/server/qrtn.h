/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/qrtn.h,v 1.1 1992-12-30 16:55:40 mar Exp $ 
 *  
 *  Used by the SQL query routines of the Moira server.
 */

#define MR_CDUMMY_LEN  256
#define MR_STMTBUF_LEN 1024

/* Requires having already done EXEC SQL INCLUDE sqlda */
typedef IISQLDA_TYPE(MR_SQLDA,MR_SQLDA_T,QMAXARGS);

extern MR_SQLDA_T *SQLDA;
extern int mr_sig_length;
extern char stmt_buf[];

extern void ingerr();

/* eof:qtrn.h */
