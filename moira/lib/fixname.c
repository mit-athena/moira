/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/fixname.c,v $
 *	$Author: danw $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/fixname.c,v 1.9 1998-01-05 19:53:02 danw Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 */

#ifndef lint
static char *rcsid_fixname_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/lib/fixname.c,v 1.9 1998-01-05 19:53:02 danw Exp $";
#endif

#include <mit-copyright.h>
#include <string.h>
#include <ctype.h>

#define LAST_LEN		100
#define FIRST_LEN		100

void FixName(char *ilnm, char *ifnm, char *last, char *first, char *middle)
{
  int ends_jr = 0, ends_iii = 0, ends_iv = 0, ends_ii = 0, ends_v = 0;

  uppercase(ilnm);
  uppercase(ifnm);

  /* Last name ... */

  TrimTrailingSpace(ilnm);
  LookForJrAndIII(ilnm, &ends_jr, &ends_ii, &ends_iii, &ends_iv, &ends_v);
  LookForSt(ilnm);
  LookForO(ilnm);
  FixCase(ilnm);
  strncpy(last, ilnm, LAST_LEN);

  /* First name  & middle initial ... */

  TrimTrailingSpace(ifnm);
  LookForJrAndIII(ifnm, &ends_jr, &ends_ii, &ends_iii, &ends_iv, &ends_v);

  GetMidInit(ifnm, middle);

  FixCase(ifnm);
#ifdef notdef
  /* okay, finish up first name */
  AppendJrOrIII(ifnm, &ends_jr, &ends_ii, &ends_iii, &ends_iv, &ends_v);
#endif
  strncpy(first, ifnm, FIRST_LEN);
}

FixCase(register char *p)
{
  register int cflag;	/* convert to lcase, unless at start or following */
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

LookForJrAndIII(register char *nm, register int *pends_jr, int *pends_ii,
		register int *pends_iii, register int *pends_iv,
		int *pends_v)
{
  register int len = strlen(nm);

  if (len >= 4 && !strcmp(nm + len - 3, " JR"))
    {
      *pends_jr = 1;
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

LookForSt(register char *nm)		/* ST PIERRE, etc. */
{
  char temp[256];

  if (!strcmp(nm, "ST "))
    {
      strcpy(temp, nm + 3);
      strcpy(nm, "ST. ");
      strcat(nm, temp);
    }
}

LookForO(register char *nm)		/* O BRIEN, etc. */
{
  if (!strcmp(nm, "O ") && isalpha(nm[2]))
    nm[1] = '\'';
}

TrimTrailingSpace(register char *ip)
{
  register char *p;
  for (p = ip + strlen(ip) - 1; p >= ip && isspace(*p); p--)
    *p = '\0';
}

GetMidInit(register char *nm, register char *mi)
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
