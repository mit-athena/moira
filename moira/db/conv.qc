/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/db/conv.qc,v 1.1 1988-09-13 13:37:06 mar Exp $ */

/*  (c) Copyright 1988 by the Massachusetts Institute of Technology. */
/*  For copying and distribution information, please see the file */
/*  <mit-copyright.h>. */


#include <mit-copyright.h>
#include <stdio.h>
#include <sys/file.h>
#include <ctype.h>

char prefix[256];

main(argc, argv)
int argc;
char **argv;
{
##  char *db;

    if (argc == 2)
      db = argv[1];
    else
      db = "sms";

    printf("Prefix of backup to restore: ");
    fflush(stdout);
    if (gets(prefix) == NULL) {
	return 1;
    }

    printf("Opening database...");
    fflush(stdout);
##  ingres db
    printf("done\n");

    do_users();
    do_finger();
    do_mach();
    do_clu();
    do_mcm();
    do_cld();
    do_servers();
    do_serverhosts();
    do_list();
    do_maillists();
    do_groups();
    do_members();
    do_strings();
    do_pobox();
    do_nfsphys();
    do_filesys();
    do_nfsquota();
    do_services();
    do_printcap();
    do_alias();
    do_values();

    printf("All done.\n");
##  exit
}

char field_chars[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,  /* ^@ - ^O */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* ^P - ^_ */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* SP - /  */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,  /* 0  - ?  */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* @  - O  */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* P  - _  */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* `  - o  */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* p  - ^? */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
};


int parse_int(f)
    register FILE *f;
{
    register int c;
    register int val = 0;
    register int sign = 1;
    while ((c = getc(f)) != EOF && field_chars[c] == 0) {
	if (c == '-') sign = -1;
	else if (isdigit(c)) {
	    val *= 10;
	    val += (c - '0');
	} else (void) fprintf(stderr,"non-digit in numeric field\n");
    }
    (void) ungetc(c, f);
    return(val * sign);
}

void parse_str(f, buf, len)
    register FILE *f;
    register char *buf;
    register int len;		/* incl trailing NULL */
{
    register int c;

    while ((c = getc(f)) != EOF && field_chars[c] == 0 && len > 0) {
	if (c == '\\') {
	    c = getc(f);
	    if (isdigit(c)) {
		/* Expect three-digit octal number.. */
		register int c1, c2;
		c1 = getc(f);
		c2 = getc(f);
		if (!isdigit(c1) || !isdigit(c2)) 
		    punt("Broken \\###");
		/* Convert to ASCII code: */
		*buf++ =  (((c-'0')<<6) + ((c1-'0')<<3) + c2-'0');
	    } else if (c == '\\' || c == ':') {
		*buf++ = c;
		--len;
	    } else punt ("Broken '\\'");
	} else {
	    *buf++ = c;
	    --len;
	}
    }
    if (c == EOF)
	return;
    
    if (c != EOF && c != ':' && c != '\n') {
	fprintf(stderr, "Field too wide, truncated\n");
	while ((c = getc(f)) != EOF && c != ':' && c != '\n');
	(void) ungetc(c, f);
    } else {
	*buf++ = 0;
	(void) ungetc(c, f);
    }
}
    
void parse_sep(f)
    FILE *f;
{
    if (getc(f) != ':') punt("Expected colon");
}
void parse_nl(f)
    FILE *f;
{
    if (getc(f) != '\n') punt("Expected newline");
}


FILE *open_file(suffix)
    char *suffix;
{
    char name[BUFSIZ];
    int fd;
    FILE *f;
    
    sprintf(name, "%s/%s", prefix, suffix);

    fd = open(name, O_RDONLY, 0);
    if (fd < 0) {
	punt(name);
    }
    f = fdopen(fd, "r");
    if (f == NULL) {
	fprintf(stderr, "fdopen of ");
	punt(name);
    }
    fprintf(stderr, "Working on %s\n", name);
    return(f);
}

punt(s)
char *s;
{
    printf("exiting: %s\n", s);
    exit(1);
}

static int count;
static int interval;

start_counter(x)
int x;
{
    count = 0;
    interval = x;
}

inc_count()
{
    if (count++ % interval == 0) {
	printf("\r%d...", count - 1);
	fflush(stdout);
    }
}

end_counter()
{
    printf("\r%d items processed.\n", count);
}
