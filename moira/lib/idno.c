/* $Id: idno.c,v 1.7 1998-02-08 19:31:17 danw Exp $
 *
 * Routines to deal with MIT IDs
 *
 * Copyright (C) 1988-1998 by the Massachusetts Institute of Technology.
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>

#include <ctype.h>
#include <string.h>

#ifdef USE_CRYPT_H
#include <crypt.h>
#else
#include <unistd.h>
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

