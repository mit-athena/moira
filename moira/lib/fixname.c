/* $Id: fixname.c,v 1.13 1998-02-08 20:37:51 danw Exp $
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

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/fixname.c,v 1.13 1998-02-08 20:37:51 danw Exp $");

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

void FixCase(char *p)
{
  int cflag;	/* convert to lcase, unless at start or following */
		/* a space or punctuation mark (e.g., '-') */

  for (cflag = 0; *p; p++)
    {
      if (cflag && isupper(*p))
	*p = tolower(*p);
      else if (!cflag && islower(*p))
	*p = toupper(*p);
      if (isalpha(*p))
	cflag = 1;
      else
	cflag = 0;
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
