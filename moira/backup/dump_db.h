/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/dump_db.h,v 1.3 1998-01-05 19:51:33 danw Exp $ */

#define SEP_CHAR ('|')

#define dump_nl(f) { if (putc('\n', f) < 0) wpunt(); }
#define dump_sep(f) { if (putc(SEP_CHAR, f) < 0) wpunt(); }
