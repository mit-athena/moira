/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/tst.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/tst.c,v 1.1 1991-05-07 16:40:16 mar Exp $
 *
 *	Copyright (C) 1991 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <ctype.h>
#include <moira.h>
#include <ss/ss.h>
#include "mr_err_array.h"

extern ss_execute_line();
extern print_reply();
extern CompData();
extern int ss;
extern int errno;
extern int count;
extern int recursion;  

char *DataBuf;
char *ErrorBuf;

test_test (argc, argv)
int argc;
char *argv[];
{
  FILE *inp, *outp;
  char *cp, *index();
  int LineNum;
  int status;
  int NumArgs;
  char *ValArgs[50];
  char lastcmd[BUFSIZ], input[BUFSIZ], cmd[BUFSIZ];

  DataBuf =  (char *)malloc (2222);
  ErrorBuf = (char *)malloc (30);

  if (recursion > 8) {
    ss_perror(ss, 0, "too many levels deep in script/test files\n");
    return;
  }
  
  if (argc < 2) {
    ss_perror(ss, 0, "Usage: test input_file [ output_file ]");
    return;
  }
  
  inp = fopen(argv[1], "r");
  if (inp == NULL) {
    ss_perror(ss, 0, "Cannot open input file %s", argv[1]);
    return;
  }
  
  if (argc == 3) {
    outp = fopen(argv[2], "a");
    if (!outp) {
      ss_perror(ss, errno, "Unable to open output for %s\n", argv[2]);   
      return;}}
  else outp = stdout;

  *lastcmd = '\0';

  recursion++;
  
  for(LineNum = 0;;LineNum++) {
    *DataBuf = '\0';
    *ErrorBuf = '\0';
    if (fgets(input, BUFSIZ, inp) == NULL) {
      if (lastcmd[0] != '\0') {
	strcpy (input, lastcmd);
	lastcmd[0] = '\0';}
      else break;}
    if ((cp = index(input, '\n')) != (char *)NULL)
      *cp = 0;
    if (input[0] == 0) continue;

    if (input[0] == '%') {
      for (cp = &input[1]; *cp && isspace(*cp); cp++);
      strcat(DataBuf, "Comment: ");
      strcat(DataBuf, cp);
      strcat(DataBuf, "\n");
      continue;
    }

    if (input[0] == '>') {     /* Load in a Comparison String */
      if (lastcmd[0] == '\0') {
	fprintf(outp, 
	   "\nERROR IN LINE %d: Comparison String Without Comparable Command\n", 
	   LineNum);
	fprintf(outp, "%s\n", input);
	*DataBuf = '\0';
	continue;}
      else { /* Parse and Execute command with compare */
	sprintf (cmd, "COMPARE_%s", lastcmd);
	bzero((char *)ValArgs, sizeof(ValArgs));
	Partial_parse_string(0, cmd, &NumArgs, ValArgs); 
	ValArgs[NumArgs] = (char *)malloc(sizeof(char) * (1+strlen(input)));
	strcpy(ValArgs[NumArgs], input);
	status = ss_execute_command(ss, ValArgs); 
	lastcmd[0] = '\0';

/* Dump errors and data if necessary */
	if (!strcmp(ErrorBuf, "Malformed Comparison String0")) {
	  fprintf(outp, "\nERROR IN LINE %d: %s\n", LineNum, ErrorBuf);
	  fprintf(outp, "%s\n", input);
	  continue;}
	else if (*ErrorBuf) { /* Data Error */
	  fprintf(outp, "\nERROR IN LINE %d: %s\n", LineNum, ErrorBuf);
	  fprintf(outp, "%s\n", DataBuf);
	  continue;}
	else continue;  /* Command Checks */
      }}

/* It wasn't a Comparison line, so clear the stack */
    if (lastcmd[0] != '\0') {  /* Run an old command w/o a comparison string */
      ss_execute_line(ss, lastcmd, &status);
      if (status == SS_ET_COMMAND_NOT_FOUND) 
	printf("Bad command: %s\n", input);
      *lastcmd = '\0';}

/* Push command on the stack if it's comparable (currently only queries are) */
    if (!(strncasecmp (input, "qy ", 3) && strncasecmp (input, "query ", 6))) {
      /* Delay this command in case there's a comparison line following it */
      strcpy (lastcmd, input);
      continue;}

/* Non-comparible command; execute immediately */
    ss_execute_line(ss, input, &status);
    if (status == SS_ET_COMMAND_NOT_FOUND) {
      printf("Bad command: %s\n", input);
    }
  }
  
  recursion--;
  
  fclose(inp);
  if (argc > 2) 
    close(outp);
}

/**********************************/

test_query_compare(argc, argv)
     int argc;
     char **argv;
{ /* argv = "COMPARE_qy" + args to query + compstring */
  int Qstatus = 0;      /* Status returned from the query */
  char *CompTo[5];
  int NumWordsComp=0, i;

  if (argc < 2) { 
    ss_perror(ss, 0, "Usage: query handle [ args ... ]");
    return;
  }
/* Execute query with a comparison string */
  count = 0;

  /* Parse comp string into '>', char, ErrMsg, NumEntries, and Data */
  bzero((char *)CompTo, sizeof(CompTo));
  Partial_parse_string (4, argv[argc-1], &NumWordsComp, CompTo);
  if (NumWordsComp < 3) { /* Too few args in comparison string */
        strcpy(ErrorBuf, "Malformed Comparison String1");}
  else {

    for (i=0;i<argc;i++) {
      printf("argv[%d] = %s\n", i, argv[i]);}
    printf("CompTo[3] = %s\n\n", CompTo[3]);


    Qstatus = mr_query(argv[1], argc-3, argv+2, CompData,
		       (char *)(&CompTo[3]));

/* Check the number of tuples returned */
    if (*CompTo[2] == '<') {
      if (isdigit(*(CompTo[2] + 1))) {
	if (atoi(CompTo[2] + 1) <= count) 
	  strcat(ErrorBuf, "\nToo many tuples returned");}
      else if (*ErrorBuf == '\0') 
	strcpy(ErrorBuf, "Malformed Comparison String2");}
    else if (*CompTo[2] == '>') {
      if (isdigit(*(CompTo[2] + 1))) {
	if (atoi(CompTo[2] + 1) >= count)
	  strcat(ErrorBuf, "\nToo few tuples returned");}
      else if (*ErrorBuf == '\0') 
	strcpy(ErrorBuf, "Malformed Comparison String3");}
    else if (isdigit(*(CompTo[2]))) {
      if (atoi(CompTo[2]) != count)
	strcat(ErrorBuf, "\nWrong number tuples returned");}
    else if (strcmp(CompTo[2], "*")) 
      if (*ErrorBuf == '\0')
	strcpy(ErrorBuf, "Malformed Comparison String4");
    
    /* Check return status */
    if (!Comp_mr_err_table(Qstatus, CompTo[1]))
      {
	strcat(ErrorBuf, "\nRet Status Error, returns: ");
	if (Qstatus)
	  strcat(ErrorBuf, MR_ERR_ARRAY[Qstatus - ERROR_TABLE_BASE_sms]);
	else 
	  strcat(ErrorBuf, "SUCCESS");}
    
  }}

/********************************************/

int Comp_mr_err_table (ErrNum, ErrName)
int ErrNum;
char *ErrName;

/* Returns 1 if ErrNum = the string in ErrName in MR_ERR_ARRAY, else 0 */

{
if (!ErrNum && ((!strcasecmp(ErrName, "SUCCESS"))  
		 || (!strcasecmp(ErrName, "S")))) 
  return(1);
else if (!ErrNum) return (0);
else if (ErrNum >= ERROR_TABLE_BASE_sms) 
  return (!strcmp (MR_ERR_ARRAY[ErrNum - ERROR_TABLE_BASE_sms], ErrName));
else return (0);
}

/********************************************/

int NumWords (Str)
char *Str;
{
int Count;
int CharIndex;

for (CharIndex = 0, Count = 0;*(Str + CharIndex);) {
  if (isspace(*(Str + CharIndex)))
    for (;isspace(*(Str + CharIndex));CharIndex++);
  else if (*(Str + CharIndex) && !isspace(*(Str + CharIndex)))
    for (Count++;(*(Str + CharIndex) && !isspace(*(Str + CharIndex)));
         CharIndex++);
}
return(Count);
}

/**********************/

/*
 * Partial_parse_string(MaxWords, Str, argc_ptr, argv_ptr)
 *
 * Function:
 *      Parses line, dividing at whitespace, into tokens, returns
 *      the "argc" and "argv" values, up to MaxWords-1 tokens.  Remaining
 *      tokens after MaxWords-1 are all returned as one set in the final
 *      slot of "argv".  If MaxWords = 0, the number of tokens is not limited.
 * Arguments:
 *      MaxWords (int)
 *              Maximum number of tokens/strings to return
 *      Str (char *)
 *              Pointer to text string to be parsed.
 *      argc_ptr (int *)
 *              Where to put the "argc" (number of tokens) value.
 *      argv_ptr (char *[])
 *              Series of pointers to parsed tokens
 */

Partial_parse_string (MaxWords, Str, argc_ptr, argv_ptr)
int MaxWords;
char *Str;
int *argc_ptr;
char *argv_ptr[];

{
char Buf[BUFSIZ];
int NumTokens;
int CharIndex;
int i;

for (CharIndex = 0, NumTokens = 0;*(Str + CharIndex) 
     && ((NumTokens < MaxWords-1) || !MaxWords);) {
  if (isspace(*(Str + CharIndex)))
    for (;isspace(*(Str + CharIndex));CharIndex++);
  else if (*(Str + CharIndex) && !isspace(*(Str + CharIndex)))
    for (NumTokens++, i=0;(*(Str + CharIndex) && !isspace(*(Str + CharIndex)));
         CharIndex++, i++) Buf[i] = *(Str + CharIndex);
  Buf[i] = '\0';
  argv_ptr[NumTokens-1] = (char *)malloc(sizeof(char) * (strlen(Buf) + 1));
  strcpy(argv_ptr[NumTokens-1], Buf);
}
*argc_ptr = NumTokens;
if (NumTokens = MaxWords) {
  argv_ptr[NumTokens-1]=
    (char *) malloc(sizeof(char) * (1 + strlen(Str + CharIndex)));
  strcpy (argv_ptr[NumTokens-1], (Str + CharIndex));}

}

