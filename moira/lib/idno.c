/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/idno.c,v 1.4 1997-01-29 23:24:10 danw Exp $
 *
 * Routines to encrypt ID's
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <string.h>
#include <ctype.h>
#ifdef POSIX
#include <unistd.h>
#endif
#ifdef USE_CRYPT_H
#include <crypt.h>
#endif

/*	Function Name: RemoveHyphens
 *	Description: Removes all hyphens from the string passed to it.
 *	Arguments: str - the string to remove the hyphes from
 *	Returns: none
 */

void
RemoveHyphens(str)
char *str;
{
    char *hyphen;

    while ((hyphen = strchr(str, '-')) != (char *)0)
	(void) strcpy(hyphen, hyphen + 1);
}


/*	Function Name: EncryptMITID
 *	Description: Encrypts an mit ID number. 
 *	Arguments: sbuf - the buffer to return the encrypted number in.
 *                 idnumber - the id number (string).
 *                 first, last - name of the person.
 *	Returns: none.
 */

void
EncryptID(sbuf, idnumber, first, last)
char *sbuf, *idnumber, *first, *last;
{
    char salt[3];

    RemoveHyphens(idnumber);
    salt[0] = tolower(last[0]);
    salt[1] = tolower(first[0]);
    salt[2] = 0;

    (void) strcpy(sbuf, crypt(&idnumber[2], salt));
}

