/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/dump_db.qc,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/dump_db.qc,v 1.7 1992-06-25 15:36:58 mar Exp $
 *
 *	(c) Copyright 1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 * 	This program dumps the SMS database to a series of output files
 * which can be later read back into SMS in the event of a crash.
 * 
 */

#ifndef lint
static char *rcsid_dump_db_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/dump_db.qc,v 1.7 1992-06-25 15:36:58 mar Exp $";
#endif lint

#include <stdio.h>
#include <sys/file.h>
#include <ctype.h>
#include <mit-copyright.h>
#include "dump_db.h"

/* putc without the line buffer hair */

#define putc1(x, p)	(--(p)->_cnt >= 0 ?\
	(int)(*(unsigned char *)(p)->_ptr++ = (x)) :\
		_flsbuf((unsigned char)(x), p))

FILE *open_file();

char act[128];

main(argc, argv)
    int argc;
    char **argv;
{
    char *prefix;
    register int i;
    
    if (argc != 2) {
	fprintf(stderr, "Usage: smsbackup prefix\n");
	exit(1);
    }
    prefix = argv[1];

    bzero(act, 128);

    act[SEP_CHAR]=1;
    act['\\']=1;
    act[127]=2;
    for (i=0; i<' '; i++) act[i]=2;
    
##  ingres sms 
##  set lockmode session where level = table
##  begin transaction

    do_backups(prefix);

##  end transaction    
##  exit
    exit(0);
}

dump_int(f, n)
    FILE *f;
    int n;
{
    char buf[1024];
    (void) sprintf(buf, "%d", n);
    dump_str(f, buf);
}

wpunt()
{
    punt("can't write backup file");
}

dump_str(f, str)
    register FILE *f;
    register char *str;
{
    char  *strtrim();
    register char *ibp = strtrim(str);
    register int c;		/* PCC doesn't put chars in registers.. */
    for (; c = *ibp; ibp++) {
	c = toascii(c);		/* punt 8th bit */
	switch(act[c]) {
	    register int t;
	case 1:
	    if (putc1('\\', f) < 0) wpunt();
	    /* fall thru.. */
	case 0:
	    if (putc1(c, f) < 0) wpunt();
	    
	    break;
	case 2:
	    if (putc1('\\', f) < 0) wpunt();	    
	    t = ((c>>6)&7) + '0';
	    if (putc1(t,f) < 0) wpunt();
	    t = ((c>>3)&7) + '0';
	    if (putc1(t,f) < 0) wpunt();
	    t = (c&7) + '0';
	    if (putc1(t,f) < 0) wpunt();
	    break;
	    
	default:
	    punt("Can't get here");
	}
    }
}

safe_close(stream)
	FILE *stream;
{
	if (fflush(stream) == EOF)
		punt("Unable to fflush");
	if (fsync(fileno(stream)) != 0) 
		punt("Unable to fsync");
	(void) fclose(stream);
}	

FILE *open_file(prefix, suffix)
    char *prefix, *suffix;
{
    char name[BUFSIZ];
    int fd;
    FILE *f;
    
    (void) strcpy(name, prefix);
    (void) strcat(name, suffix);

    fd = open(name, O_CREAT|O_WRONLY|O_EXCL, 0644);
    if (fd < 0) {
	punt(name);
    }
    f = fdopen(fd, "w");
    if (f == NULL) {
	fprintf(stderr, "fdopen of ");
	punt(name);
    }
    fprintf(stderr, "Working on %s\n", name);
    return(f);
}


/*
 * Trim whitespace off both ends of a string.
 */
char *strtrim(save)
    register char *save;
{
    register char *t, *s;

    s = save;
    while (isspace(*s)) s++;
    /* skip to end of string */
    if (*s == '\0') {
	*save = '\0';
	return(save);
    }

    for (t = s; *t; t++) continue; 
    while (t > s) {
	--t;
	if (!isspace(*t)) {
	    t++;
	    break;
	}
    }
    *t = '\0';
    return s;
}
