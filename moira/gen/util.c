/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/util.c,v 1.6 1990-08-17 18:32:02 mar Exp $
 *
 * Utility routines used by the MOIRA extraction programs.
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */


#include <mit-copyright.h>
#include <stdio.h>
#include <sys/time.h>
#include <moira.h>
#include <moira_site.h>


fix_file(targetfile)
char *targetfile;
{
    char oldfile[64], filename[64];

    sprintf(oldfile, "%s.old", targetfile);
    sprintf(filename, "%s~", targetfile);
    if (rename(targetfile, oldfile) == 0) {
	if (rename(filename, targetfile) < 0) {
	    rename(oldfile, targetfile);
	    perror("Unable to install new file (rename failed)\n");
	    fprintf(stderr, "Filename = %s\n", targetfile);
	    exit(MR_CCONFIG);
	}
    } else {
	if (rename(filename, targetfile) < 0) {
	    perror("Unable to rename old file\n");
	    fprintf(stderr, "Filename = %s\n", targetfile);
	    exit(MR_CCONFIG);
	}
    }
    unlink(oldfile);
}


char *dequote(s)
register char *s;
{
    char *last = s;

    while (*s) {
	if (*s == '"')
	  *s = '\'';
	else if (*s != ' ')
	  last = s;
	s++;
    }
    if (*last == ' ')
      *last = '\0';
    else
      *(++last) = '\0';
    return(s);
}
