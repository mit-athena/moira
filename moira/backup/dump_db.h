/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/dump_db.h,v 1.2 1989-12-04 12:52:53 mar Exp $ */

#define SEP_CHAR	('|')

#define dump_nl(f) { if (putc('\n', f) < 0) wpunt(); }
#define dump_sep(f) { if (putc(SEP_CHAR, f) < 0) wpunt(); }
			   
