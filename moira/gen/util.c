/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/util.c,v 1.5 1990-03-19 19:07:45 mar Exp $
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


/* ingres_date_and_time: passed a unix time_t, returns a string that ingres
 * can parse to obtain that time.
 */

static char *month_name[] = {
    "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct",
    "nov", "dec"
  };


char *ingres_date(), *ingres_time();

char *ingres_date_and_time(l)
long l;
{
	char *ans = NULL, *date, *time;
      
	if ((date = ingres_date(l)) && (time = ingres_time(l))) {
		char buf[BUFSIZ];
		sprintf(buf, "%s %s", date, time);
		ans = strsave(buf);
	}
	if (date)
		free(date);
	if (time)
		free(time);
	return ans;
}

char *ingres_time(t)
	long t;
{
	struct tm *tm;

	if (t == (long) 0)
		(void) time(&t);

	if ((tm = localtime(&t)) == (struct tm *) NULL) {
		return NULL;
	} else {
		char buf[BUFSIZ];

		sprintf(buf, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min,
			tm->tm_sec);
		return strsave(buf);
	}
}

char *ingres_date(t)
	long t;
{
	struct tm *tm;

	if (t == (long) 0)
		(void) time(&t);

	if ((tm = localtime(&t)) == (struct tm *) NULL) {
		return NULL;
	} else {
		char buf[BUFSIZ];

		sprintf(buf, "%02d-%3.3s-%04d", tm->tm_mday,
			month_name[tm->tm_mon], 1900 + tm->tm_year);
		return strsave(buf);
	}
}


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
}
