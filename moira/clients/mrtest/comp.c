/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/comp.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/comp.c,v 1.2 1993-10-22 16:25:40 mar Exp $
 *
 *	Copyright (C) 1991 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 */

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <moira.h>

extern char *DataBuf;
extern char *ErrorBuf;
extern int count;

CompData (argc, argv, CompStr)
int argc;        /* # of fields outputted by Moira  */
char **argv;     /* Array of strings outputted by Moira */
char **CompStr;  /* Pointer to Comparison String -- Data section only */
/*  CompData uses the global variables: */
/*      char *DataBuf;   */
/*      char *ErrorBuf;  */
/*      int  count;      */
{
int Field = 0; /* Field number being compared  */
int Len, Len2;   /* Length of a comparison field */
int i;     /* index for printing */

if (*ErrorBuf == '\0') {    /* Don't process if there's a data error already */

  for (;(**CompStr) && isspace(**CompStr); (*CompStr)++); /*whitespace*/
  if ((**CompStr != '{') && (**CompStr != '\0')) 
    strcpy (ErrorBuf, "Malformed Comparison String");
  else if (**CompStr != '\0') {
    for ((*CompStr)++; ((**CompStr != '}') && (**CompStr != '\0') 
			&& (Field < argc));) {
      for (;**CompStr && isspace(**CompStr); (*CompStr)++);  /* Whitespace */
      if (**CompStr == '*') {   /* Wildcard -- field automatically checks */
	(*CompStr)++;}
      else if (**CompStr == '"') {  /* Compare a field */
	(*CompStr)++;
	Len = (int)(strchr(*CompStr, '"')) - (int)(*CompStr);
	if (Len < 0) {
	  strcpy (ErrorBuf, "Malformed Comparison String");
	  break;}
	else if ((strncmp (*CompStr, argv[Field], Len)) ||
		 (strlen(argv[Field]) != Len)) {   /* Data Error */
	  strcpy (ErrorBuf, "Data Error");
	  break;}
	*CompStr = (char *)strchr(*CompStr, '"');
	(*CompStr)++;}
      else {
	Len  = (int)(strchr(*CompStr, ' ')) - (int)(*CompStr);
	Len2 = (int)(strchr(*CompStr, '}')) - (int)(*CompStr);
	if ((Len < 0) || ((Len2 >= 0) && (Len2 < Len)))
	  Len = Len2;
	if (Len < 0) 
	  strcpy (ErrorBuf, "Malformed Comparison String");
	else {
	  if ((strncmp (*CompStr, argv[Field], Len)) ||
	      (strlen(argv[Field]) != Len)) {   /* Data Error */
	    strcpy (ErrorBuf, "Data Error");
	    break;}
	  *CompStr = (char *)(Len + (int)(*CompStr));}}
      Field++;}
    for (;**CompStr && isspace(**CompStr); (*CompStr)++);  /* Whitespace */
    if (*ErrorBuf == '\0') {
      if (**CompStr == '\0')
	strcpy (ErrorBuf, "Malformed Comparison String");
      else if ((**CompStr != '}') || (Field < argc)) 
	strcpy (ErrorBuf, "Data Error");}
    (*CompStr)++;
  }
  if (!strcmp(ErrorBuf, "Data Error")) strcat (DataBuf, "*****");
}
else {
  strcat (DataBuf, "     ");}

if (strcmp(ErrorBuf, "Data Error")) strcat (DataBuf, "     ");

/* Write Query output to data buffer */
for (i = 0; i < argc; i++) {
  if (i != 0) strcat (DataBuf, ", ");
  strcat (DataBuf, argv[i]);
}
strcat (DataBuf, "\n");
count++;
return(MR_CONT);
}
      
	    
	  

