/* $Id$
 *
 * Put a name into Moira-canonical form
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>

#include <ctype.h>
#include <string.h>

RCSID("$HeadURL$ $Id$");

#define LAST_LEN		100
#define FIRST_LEN		100

void FixName(char *ilnm, char *ifnm, char *last, char *first, char *middle)
{
  int ends_jr = 0, ends_sr = 0;
  int ends_iii = 0, ends_iv = 0, ends_ii = 0, ends_v = 0;

  uppercase(ilnm);
  uppercase(ifnm);

  /* Last name ... */

  TrimTrailingSpace(ilnm);
  LookForJrAndIII(ilnm, &ends_jr, &ends_sr,
		  &ends_ii, &ends_iii, &ends_iv, &ends_v);
  LookForSt(ilnm);
  LookForO(ilnm);
  FixCase(ilnm);
  strncpy(last, ilnm, LAST_LEN);

  /* First name  & middle initial ... */

  TrimTrailingSpace(ifnm);
  LookForJrAndIII(ifnm, &ends_jr, &ends_sr,
		  &ends_ii, &ends_iii, &ends_iv, &ends_v);

  GetMidInit(ifnm, middle);

  FixCase(ifnm);
  strncpy(first, ifnm, FIRST_LEN);
}

void FixCase(unsigned char *p)
{
  int up;	/* Should next letter be uppercase */
  int pos;	/* Position within word */

  for (up = 1, pos = 1; *p; p++, pos++)
    {
      if (!up && isupper(*p))
	*p = tolower(*p);
      else if (up && islower(*p))
	*p = toupper(*p);

      if (isalpha(*p))		/* If letter, next letter should be lower */
	up = 0;
      else if (isspace(*p))	/* If space, next letter should be upper */
	{
	  pos = 0;
	  up = 1;
	}
      else if (*p == '\'')	/* If ', next letter should be upper only */
	up = (pos == 2);	/* if the ' is the 2nd char in the name */
      else if (*p >= 0x80)      /* If the high bit is set, don't touch it. */
	up = 0;
      else
	up = 1;			/* If other punctuation (eg, -), upper */
    }
}

void LookForJrAndIII(char *nm, int *pends_jr, int *pends_sr, int *pends_ii,
		     int *pends_iii, int *pends_iv, int *pends_v)
{
  int len = strlen(nm);

  if (len >= 4 && !strcmp(nm + len - 3, " JR"))
    {
      *pends_jr = 1;
      nm[len - 3] = '\0';
    }
  else if (len >= 4 && !strcmp(nm + len - 3, " SR"))
    {
      *pends_sr = 1;
      nm[len - 3] = '\0';
    }
  else if (len >= 4 && !strcmp(nm + len - 3, " IV"))
    {
      *pends_iv = 1;
      nm[len - 3] = '\0';
    }
  else if (len >= 5 && !strcmp(nm + len - 4, " JR."))
    {
      *pends_jr = 1;
      nm[len - 4] = '\0';
    }
  else if (len >= 5 && !strcmp(nm + len - 4, " III"))
    {
      *pends_iii = 1;
      nm[len - 4] = '\0';
    }
  else if (len >= 4 && !strcmp(nm + len - 3, " II"))
    {
      *pends_ii = 1;
      nm[len - 3] = '\0';
    }
  else if (len >= 3 && !strcmp(nm + len - 2, " V"))
    {
      *pends_v = 1;
      nm[len - 2] = '\0';
    }
}

/* XXX no way to avoid possible buffer overrun here since we don't know
   how long nm is and we're trying to make it one longer */
void LookForSt(char *nm)		/* ST PIERRE, etc. */
{
  char temp[256];

  if (!strcmp(nm, "ST "))
    {
      strcpy(temp, nm + 3);
      strcpy(nm, "ST. ");
      strcat(nm, temp);
    }
}

void LookForO(char *nm)		/* O BRIEN, etc. */
{
  if (!strcmp(nm, "O ") && isalpha(nm[2]))
    nm[1] = '\'';
}

void TrimTrailingSpace(char *ip)
{
  char *p;
  for (p = ip + strlen(ip) - 1; p >= ip && isspace(*p); p--)
    *p = '\0';
}

void GetMidInit(char *nm, char *mi)
{
  while (*nm && !isspace(*nm))
    nm++;
  if (*nm)
    *nm++ = '\0';
  while (*nm && isspace(*nm))
    nm++;
  if (*nm)
    *mi++ = *nm;
  *mi = '\0';
}

/*	Function Name: RemoveHyphens
 *	Description: Removes all hyphens from the string passed to it.
 *	Arguments: str - the string to remove the hyphens from
 *	Returns: none
 */

void RemoveHyphens(char *str)
{
  char *hyphen;

  while ((hyphen = strchr(str, '-')))
    strcpy(hyphen, hyphen + 1);
}
