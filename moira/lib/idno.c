/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/idno.c,v 1.5 1998-01-05 19:53:04 danw Exp $
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
#include <unistd.h>
#ifdef USE_CRYPT_H
#include <crypt.h>
#endif

/*	Function Name: RemoveHyphens
 *	Description: Removes all hyphens from the string passed to it.
 *	Arguments: str - the string to remove the hyphes from
 *	Returns: none
 */

void RemoveHyphens(char *str)
{
  char *hyphen;

  while ((hyphen = strchr(str, '-')))
    strcpy(hyphen, hyphen + 1);
}


/*	Function Name: EncryptMITID
 *	Description: Encrypts an mit ID number.
 *	Arguments: sbuf - the buffer to return the encrypted number in.
 *                 idnumber - the id number (string).
 *                 first, last - name of the person.
 *	Returns: none.
 */

void EncryptID(char *sbuf, char *idnumber, char *first, char *last)
{
  char salt[3];

  RemoveHyphens(idnumber);
  salt[0] = tolower(last[0]);
  salt[1] = tolower(first[0]);
  salt[2] = '\0';

  strcpy(sbuf, crypt(&idnumber[2], salt));
}

