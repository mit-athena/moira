/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/mrtest.c,v $
 *	$Author: danw $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/mrtest.c,v 1.28 1996-10-29 21:56:10 danw Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 */

#ifndef lint
static char *rcsid_test_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/mrtest.c,v 1.28 1996-10-29 21:56:10 danw Exp $";
#endif lint

#include <mit-copyright.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <moira.h>
#include <com_err.h>

int recursion = 0;
extern int errno;
extern int sending_version_no;
int count, quit=0;
char *whoami;

#define MAXARGS 20

main(argc, argv)
	int argc;
	char **argv;
{	
	int status;
	char cmdbuf[BUFSIZ];
	
	whoami = argv[0];
	
	initialize_sms_error_table();
	initialize_krb_error_table();

	while(!quit) {
		printf("moira:  ");
		fflush(stdout);
		if(!fgets(cmdbuf,BUFSIZ,stdin)) break;
		execute_line(cmdbuf);
	}
	mr_disconnect();
	exit(0);
}

execute_line(cmdbuf)
     char *cmdbuf;
{
  int argc;
  char *argv[MAXARGS];

  argc=parse(cmdbuf, argv);
  if(argc==0) return;
  if(!strcmp(argv[0],"noop"))
    test_noop();
  else if(!strcmp(argv[0],"connect") || !strcmp(argv[0],"c"))
    test_connect(argc, argv);
  else if(!strcmp(argv[0],"disconnect") || !strcmp(argv[0],"d"))
    test_disconnect();
  else if(!strcmp(argv[0],"host"))
    test_host();
  else if(!strcmp(argv[0],"new") || !strcmp(argv[0],"2"))
    test_new();
  else if(!strcmp(argv[0],"old") || !strcmp(argv[0],"1"))
    test_old();
  else if(!strcmp(argv[0],"motd"))
    test_motd();
  else if(!strcmp(argv[0],"query") || !strcmp(argv[0],"qy"))
    test_query(argc, argv);
  else if(!strcmp(argv[0],"auth") || !strcmp(argv[0],"a"))
    test_auth(argc, argv);
  else if(!strcmp(argv[0],"access"))
    test_access(argc, argv);
  else if(!strcmp(argv[0],"dcm"))
    test_dcm();
  else if(!strcmp(argv[0],"script") || !strcmp(argv[0],"s"))
    test_script(argc, argv);
  else if(!strcmp(argv[0],"list_requests") ||
	  !strcmp(argv[0],"lr") || !strcmp(argv[0],"?"))
    test_list_requests();
  else if(!strcmp(argv[0],"quit") || !strcmp(argv[0],"Q"))
    quit=1;
  else fprintf(stderr, "moira: Unknown request \"%s\"."
	       "Type \"?\" for a request list.\n", argv[0]);
}

int
parse(buf, argv)
     char *buf, *argv[MAXARGS];
{
  char *p;
  int argc, num;
	
  for(p=buf, argc=0, argv[0]=buf; *p && *p!='\n'; p++) {
    if(*p=='"') {
      char *d=p++;
      /* skip to close-quote, copying back over open-quote */
      while(*p!='"') {
	if(!*p || *p=='\n') {
	  fprintf(stderr, "moira: Unbalanced quotes in command line\n");
	  return 0;
	}
	if(*p=='\\') {
	  if(*++p!='"' && (*p<'0' || *p>'9')) {
	    fprintf(stderr, "moira: Bad use of \\\n");
	    return 0;
	  } else if (*p!='"') {
	    num=(*p-'0')*64 + (*++p-'0')*8 + (*++p-'0');
	    *p=num;
	  }
	}
	*d++=*p++;
      }
      if(p==d+1) {*d='\0'; p++;}
      else while(p>=d) *p--=' ';
    }
    if(*p==' ' || *p=='\t') {
      /* skip whitespace */
      for(*p++='\0'; *p==' ' || *p=='\t'; p++);
      if(*p && *p!='\n') argv[++argc]=p--;
    }
  }
  if(*p=='\n') *p='\0';
  return argc+1;
}

test_noop()
{
	int status = mr_noop();
	if (status) com_err("moira (noop)", status, "");
}

test_new()
{
	sending_version_no = MR_VERSION_2;
}

test_old()
{
	sending_version_no = MR_VERSION_1;
}

test_connect(argc, argv)
int argc;
char *argv[];
{
    	char *server = "";
	int status;

	if (argc > 1) {
	    server = argv[1];
	}
	status = mr_connect(server);
	if (status) com_err("moira (connect)", status, "");
}

test_disconnect()
{
	int status = mr_disconnect();
	if (status) com_err("moira (disconnect)", status, "");
}

test_host()
{
        char host[BUFSIZ];
        int status;

        memset(host, 0, sizeof(host));

	if (status = mr_host(host, sizeof(host) - 1))
	    com_err("moira (host)", status, "");
	else
	    printf("You are connected to host %s\n", host);
}

test_auth(argc, argv)
int argc;
char *argv[];
{
	int status;

	status = mr_auth("mrtest");
	if (status) com_err("moira (auth)", status, "");
}

test_script(argc, argv)
int argc;
char *argv[];
{
    FILE *inp;
    char input[BUFSIZ], *cp;
    int status, oldstdout, oldstderr;

    if (recursion > 8) {
	com_err("moira (script)", 0, "too many levels deep in script files\n");
	return;
    }

    if (argc < 2) {
	com_err("moira (script)", 0, "Usage: script input_file [ output_file ]");
	return;
    }

    inp = fopen(argv[1], "r");
    if (inp == NULL) {
	sprintf(input, "Cannot open input file %s", argv[1]);
	com_err("moira (script)", 0, input);
	return;
    }

    if (argc == 3) {
	printf("Redirecting output to %s\n", argv[2]);
	fflush(stdout);
	oldstdout = dup(1);
	close(1);
	status = open(argv[2], O_CREAT|O_WRONLY|O_APPEND, 0664);
	if (status != 1) {
	    close(status);
	    dup2(oldstdout, 1);
	    argc = 2;
	    sprintf(input, "Unable to redirect output to %s\n", argv[2]);
	    com_err("moira (script)", errno, input);
	} else {
	    fflush(stderr);
	    oldstderr = dup(2);
	    close(2);
	    dup2(1, 2);
	}
    }

    recursion++;

    for(;;) {
	if (fgets(input, BUFSIZ, inp) == NULL)
	  break;
	if ((cp = strchr(input, '\n')) != (char *)NULL)
	  *cp = 0;
	if (input[0] == 0) {
	    printf("\n");
	    continue;
	}
	if (input[0] == '%') {
	    for (cp = &input[1]; *cp && isspace(*cp); cp++);
	    printf("Comment: %s\n", cp);
	    continue;
	}
	printf("Executing: %s\n", input);
	execute_line(input);
    }

    recursion--;

    fclose(inp);
    if (argc == 3) {
	fflush(stdout);
	close(1);
	dup2(oldstdout, 1);
	close(oldstdout);
	fflush(stderr);
	close(2);
	dup2(oldstderr, 2);
	close(oldstderr);
    }
}

print_reply(argc, argv)
	int argc;
	char **argv;
{
	int i;
	for (i = 0; i < argc; i++) {
		if (i != 0) printf(", ");
		printf("%s", argv[i]);
	}
	printf("\n");
	count++;
	return(MR_CONT);
}

test_query(argc, argv)
	int argc;
	char **argv;
{
	int status;
	if (argc < 2) {
		com_err("moira (query)", 0, "Usage: query handle [ args ... ]");
		return;
	}

	count = 0;
	status = mr_query(argv[1], argc-2, argv+2, print_reply, (char *)NULL);
	printf("%d tuple%s\n", count, ((count == 1) ? "" : "s"));
	if (status) com_err("moira (query)", status, "");
}

test_access(argc, argv)
	int argc;
	char **argv;
{
	int status;
	if (argc < 2) {
		com_err("moira (access)", 0, "Usage: access handle [ args ... ]");
		return;
	}
	status = mr_access(argv[1], argc-2, argv+2);
	if (status) com_err("moira (access)", status, "");
}


test_dcm(argc, argv)
	int argc;
	char **argv;
{
	int status;

	if (status = mr_do_update())
	  com_err("moira (dcm)", status, " while triggering dcm");
}


test_motd(argc, argv)
	int argc;
	char **argv;
{
	int status;
	char *motd;

	if (status = mr_motd(&motd))
	  com_err("moira (motd)", status, " while getting motd");
	if (motd)
	  printf("%s\n", motd);
	else
	  printf("No message of the day.\n");
}

test_list_requests()
{
	printf("Available moira requests:\n");
	printf("\n");
	printf("noop\t\t\tAsk Moira to do nothing\n");
	printf("connect, c\t\tConnect to Moira server\n");
	printf("disconnect, d\t\tDisconnect from server\n");
	printf("host\t\t\tIdentify the server host\n");
	printf("new, 2\t\t\tUse new protocol\n");
	printf("old, 1\t\t\tUse old protocol\n");
	printf("motd, m\t\t\tGet the Message of the Day\n");
	printf("query, qy\t\tMake a query.\n");
	printf("auth, a\t\t\tAuthenticate to Moira.\n");
	printf("access\t\t\tCheck access to a Moira query.\n");
	printf("dcm\t\t\tTrigger the DCM\n");
	printf("script, s\t\tRead commands from a script.\n");
	printf("list_requests, lr, ?\tList available commands.\n");
	printf("quit, Q\t\t\tLeave the subsystem.\n");
}
