/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/nfsparttype.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/nfsparttype.c,v 1.5 1993-10-22 14:19:56 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 */

#ifndef lint
static char *rcsid_nfsparttype_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/nfsparttype.c,v 1.5 1993-10-22 14:19:56 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include <moira.h>
#include <stdio.h>
#include <string.h>
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
    { MR_FS_STUDENT, "Student" },
    { MR_FS_FACULTY, "Faculty" },
    { MR_FS_STAFF, "Staff" },
    { MR_FS_MISC, "Other" },
    { MR_FS_GROUPQUOTA, "GroupQuota" },
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
	char *t = strchr (cp, ',');
	if (t) {
	    memcpy(temp, cp, t-cp);
	    temp[t-cp]='\0';
	    cp = t + 1; /* one after the comma */
	} else {
	    (void) strcpy(temp, cp);
	    cp = NULL;
	}

	t = strtrim(temp);	/* nuke leading and trailing whitespace */

	for (pp = fs_names; pp->type; pp++) {
	    if (strcasecmp(pp->name, t) == 0) {
		flags |= pp->type;
		break;
	    }
	}
    } while (cp);
    (void) sprintf(temp, "%d", flags);
    return strsave(temp);
}
