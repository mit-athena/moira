/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/util.c,v 1.1 1988-06-20 12:51:49 mar Exp $
 *
 * Utility routines used by the SMS extraction programs.
 */


#include <stdio.h>
#include <sys/time.h>

char *malloc();


/* Trim trailing spaces from a string by replacing one of them with a null.
 */

trim(s)
register char *s;
{
    register char *p;

    for (p = s; *s; s++)
      if (*s != ' ')
	p = s;
    if (p != s) {
	if (*p == ' ')
	  *p = 0;
	else
	  p[1] = 0;
    }
}


/* return a "saved" copy of the string */

char *strsave(s)
register char *s;
{
    register char *r;

    r = malloc(strlen(s) + 1);
    strcpy(r, s);
    return(r);
}



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

