/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/nfsparttype.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/nfsparttype.c,v 1.1 1987-09-03 03:13:40 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 */

#ifndef lint
static char *rcsid_nfsparttype_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/nfsparttype.c,v 1.1 1987-09-03 03:13:40 wesommer Exp $";
#endif lint

#include <sms.h>
#include <stdio.h>
#include <strings.h>
#include <ctype.h>

extern char *strsave();
extern char *strtrim();

struct pair {
    int type;
    char *name;
};

/*
 * Table of fs type names.
 */

static struct pair fs_names[] = {
    { SMS_FS_STUDENT, "Student" },
    { SMS_FS_FACULTY, "Faculty/Project" },
    { SMS_FS_FACULTY, "Faculty" },
    { SMS_FS_FACULTY, "Project" },
    { SMS_FS_STAFF, "Staff" },
    { SMS_FS_MISC, "Other" },
    /* Insert new entries before the 0,0 pair */
    { 0, 0 },
};

/*
 * Given a numeric string containing a filesystem status value, return
 * a string indicating what allocation type it is.
 */
char *
format_filesys_type(fs_status)
    char *fs_status;
{
    char buf[BUFSIZ];
    register struct pair *pp;
    
    int n_names = 0;
    int stat = atoi(fs_status);
    
    buf[0] = '\0';

    for (pp = fs_names; pp->type; pp++) {
	if (stat & pp->type) {
	    if (n_names) (void) strcat(buf, ", ");
	    (void) strcat(buf, pp->name);
	    n_names++;
	    stat &= ~pp->type;
	}
    }
    if (stat) {
	char buf1[100];
	
	if (n_names) (void) strcat (buf, ", ");
	(void) sprintf(buf1, "Unknown bits 0x%x", stat);
	(void) strcat (buf, buf1);
    }
    if (!n_names) (void) strcpy(buf, "none");
    return strsave(buf);
}

/*
 * Given a string describing a filesystem allocation type, return the
 * numeric value.
 */
char *
parse_filesys_type(fs_type_name)
    char *fs_type_name;
{
    register struct pair *pp;
    register char *cp = fs_type_name;
    char temp[BUFSIZ];
    int flags = 0;
    
    do {
	/* Copy next component of type to temp */
	char *t = index (cp, ',');
	if (t) {
	    bcopy(cp, temp, t-cp);
	    temp[t-cp]='\0';
	    cp = t + 1; /* one after the comma */
	} else {
	    (void) strcpy(temp, cp);
	    cp = NULL;
	}

	t = strtrim(temp);	/* nuke leading and trailing whitespace */

	for (pp = fs_names; pp->type; pp++) {
	    if (cistrcmp(pp->name, t) == 0) {
		flags |= pp->type;
		break;
	    }
	}
    } while (cp);
    (void) sprintf(temp, "%d", flags);
    return strsave(temp);
}
