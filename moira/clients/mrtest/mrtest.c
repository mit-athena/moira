/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/mrtest.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/mrtest.c,v 1.20 1990-04-09 15:02:25 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 */

#ifndef lint
static char *rcsid_test_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/mrtest.c,v 1.20 1990-04-09 15:02:25 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include <stdio.h>
#include <sys/file.h>
#include <ctype.h>
#include <moira.h>
#include <ss.h>

int ss;
int recursion = 0;
extern ss_request_table moira_test;
extern int sending_version_no;

#ifndef __SABER__
main(argc, argv)
	int argc;
	char **argv;
#else __SABER__
moira()
#endif __SABER__
{	
	int status;
	char *whoami;
	
#ifndef __SABER__
	whoami = argv[0];
#else
	whoami = "mrtest";
#endif __SABER__
	
	init_ss_err_tbl();

	ss = ss_create_invocation("moira", "2.0", (char *)NULL,
				  &moira_test, &status);
	if (status != 0) {
		com_err(whoami, status, "Unable to create invocation");
		exit(1);
	}
	if (argc > 1) {
	    argv++;
	    ss_execute_command(ss, argv);
	}
	ss_listen(ss, &status);
	if (status != 0) {
		com_err(whoami, status, 0);
		exit(1);
	}
	exit(0);
}

test_noop()
{
	int status = mr_noop();
	if (status) ss_perror(ss, status, "");
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
    	char *server = "", *index();
	int status;

	if (argc > 1) {
	    server = argv[1];
	}
	status = mr_connect(server);
	if (status) ss_perror(ss, status, "");
}

test_disconnect()
{
	int status = mr_disconnect();
	if (status) ss_perror(ss, status, "");
}

test_host()
{
        char host[BUFSIZ];
        int status;

        bzero(host, sizeof(host));

	if (status = mr_host(host, sizeof(host) - 1))
	    ss_perror(ss, status, "");
	else
	    printf("You are connected to host %s\n", host);
}

test_auth()
{
	int status;

	status = mr_auth("mrtest");
	if (status) ss_perror(ss, status, "");
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
	return(MR_CONT);
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
	status = mr_query(argv[1], argc-2, argv+2, print_reply, (char *)NULL);
	printf("%d tuple%s\n", count, ((count == 1) ? "" : "s"));
	if (status) ss_perror(ss, status, "");
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
	status = mr_access(argv[1], argc-2, argv+2);
	if (status) ss_perror(ss, status, "");
}


test_dcm(argc, argv)
	int argc;
	char **argv;
{
	int status;

	if (status = mr_do_update())
	  ss_perror(ss, status, " while triggering dcm");
}


test_motd(argc, argv)
	int argc;
	char **argv;
{
	int status;
	char *motd;

	if (status = mr_motd(&motd))
	  ss_perror(ss, status, " while getting motd");
	if (motd)
	  printf("%s\n", motd);
	else
	  printf("No message of the day.\n");
}
