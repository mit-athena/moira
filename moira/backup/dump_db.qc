/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/dump_db.qc,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/dump_db.qc,v 1.1 1987-07-11 19:51:05 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 * 	This program dumps the SMS database to a series of output files
 * which can be later read back into SMS in the event of a crash.
 * 
 *	$Log: not supported by cvs2svn $
 *
 */

#ifndef lint
static char *rcsid_dump_db_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/backup/dump_db.qc,v 1.1 1987-07-11 19:51:05 wesommer Exp $";
#endif lint

#include <stdio.h>
#include <sys/file.h>

FILE *open_file();

main(argc, argv)
    int argc;
    char **argv;
{
    char *prefix;
    
    if (argc != 2) {
	fprintf(stderr, "Usage: smsbackup prefix\n");
	exit(1);
    }
    prefix = argv[1];

##  ingres sms 
    
    do_backups(prefix);
    
##  exit
    exit(0);
}

FILE *open_file(prefix, suffix)
    char *prefix, *suffix;
{
    char name[BUFSIZ];
    int fd;
    FILE *f;
    
    strcpy(name, prefix);
    strcat(name, suffix);

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

dump_int(f, n)
    FILE *f;
    int n;
{
    char buf[1024];
    sprintf(buf, "%d", n);
    dump_str(f, buf);
}

dump_nl(f)
{
    dump_str(f, "\n");
}

/*
 * Should check that we're not printing non-printing characters or
 * ':' characters here (??? recovery if we do??)
 */

dump_str(f, str)
    FILE *f;
    char *str;
{
    register int len = strlen(str);

    if (fwrite(str, 1, len, f) != len) punt("short write");
}


punt(msg)
	char *msg;
{
	perror(msg);
##	exit
	exit(1);
}

safe_close(stream)
	FILE *stream;
{
	if (fflush(stream) == EOF)
		punt("Unable to fflush");
	if (fsync(fileno(stream)) != 0) 
		punt("Unable to fsync");
	fclose(stream);
}	

/*
 * Local Variables:
 * mode: c
 * c-indent-level: 4
 * c-continued-statement-offset: 4
 * c-brace-offset: -4
 * c-argdecl-indent: 4
 * c-label-offset: -4
 * End:
 */
