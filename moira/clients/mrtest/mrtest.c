/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/mrtest.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/mrtest.c,v 1.5 1988-01-22 12:26:13 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 */

#ifndef lint
static char *rcsid_test_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/mrtest.c,v 1.5 1988-01-22 12:26:13 mar Exp $";
#endif lint

#include <stdio.h>
#include <sys/file.h>
#include <ctype.h>
#include <sms.h>
#include <ss.h>

int ss;
int recursion = 0;
extern ss_request_table sms_test;

#ifndef __SABER__
main(argc, argv)
	int argc;
	char **argv;
#else __SABER__
sms()
#endif __SABER__
{	
	int status;
	char *whoami;
	
#ifndef __SABER__
	whoami = argv[0];
#else
	whoami = "sms";
#endif __SABER__
	
	init_ss_err_tbl();
	init_sms_err_tbl();
	init_krb_err_tbl();

	ss = ss_create_invocation("sms", "1.0", (char *)NULL,
				  &sms_test, &status);
	if (status != 0) {
		com_err(whoami, status, "Unable to create invocation");
		exit(1);
	}
	ss_listen(ss, &status);
	if (status != 0) {
		com_err(whoami, status, 0);
		exit(1);
	}
}

test_noop()
{
	int status = sms_noop();
	if (status) ss_perror(ss, status, 0);
}

test_connect()
{
	int status = sms_connect();
	if (status) ss_perror(ss, status, 0);
}

test_disconnect()
{
	int status = sms_disconnect();
	if (status) ss_perror(ss, status, 0);
}

test_auth()
{
	int status = sms_auth();
	if (status) ss_perror(ss, status, 0);
}

test_script(argc, argv)
int argc;
char *argv[];
{
    FILE *inp;
    char input[BUFSIZ], *cp, *index();
    int status, oldstdout, oldstderr;

    if (recursion > 8) {
	ss_perror(ss, 0, "too many levels deep in script files\n");
	return;
    }

    if (argc < 2) {
	ss_perror(ss, 0, "Usage: script input_file [ output_file ]");
	return;
    }

    inp = fopen(argv[1], "r");
    if (inp == NULL) {
	ss_perror(ss, 0, "Cannot open input file %s", argv[1]);
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
	    ss_perror(ss, errno, "Unable to redirect output to %s\n", argv[2]);
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
	if ((cp = index(input, '\n')) != (char *)NULL)
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
	ss_execute_line(ss, input, &status);
	if (status == SS_ET_COMMAND_NOT_FOUND) {
	    printf("Bad command: %s\n", input);
	}
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

char *concat(str1, str2)
	char *str1, *str2;
{
	char *rtn;
	extern char *malloc();
	
	if (!str1) {
		int len = strlen(str2) + 1 ;
		rtn = malloc(len);
		bcopy(str2, rtn, len);
	} else {
		int len1 = strlen(str1);
		int len2 = strlen(str2) + 1;
		rtn = malloc(len1+len2);
		bcopy(str1, rtn, len1);
		bcopy(str2, rtn+len1, len2);
	}
	return rtn;
}

test_shutdown(argc, argv)
	int argc;
	char **argv;
{
	char *reason = NULL;
	int status, i;
	
	if (argc < 2) {
		ss_perror(ss, 0, "Usage: shutdown reason ...");
		return;
	}
	
	for (i = 1 ; i < argc; i++) {
		if (i != 1) reason = concat(reason, " ");
		reason = concat(reason, argv[i]);
	}
	status = sms_shutdown(reason);
	if (status) ss_perror(ss, status, 0);
}
static int count;


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
}

test_query(argc, argv)
	int argc;
	char **argv;
{
	int status;
	if (argc < 2) {
		ss_perror(ss, 0, "Usage: query handle [ args ... ]");
		return;
	}
	count = 0;
	status = sms_query(argv[1], argc-2, argv+2, print_reply, (char *)NULL);
	printf("%d tuple%s\n", count, ((count == 1) ? "" : "s"));
	if (status) ss_perror(ss, status, 0);
}

test_access(argc, argv)
	int argc;
	char **argv;
{
	int status;
	if (argc < 2) {
		ss_perror(ss, 0, "Usage: access handle [ args ... ]");
		return;
	}
	status = sms_access(argv[1], argc-2, argv+2);
	if (status) ss_perror(ss, status, 0);
}

