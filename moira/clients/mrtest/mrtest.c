/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/mrtest.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/mrtest.c,v 1.1 1987-08-22 18:31:59 wesommer Exp $
 */

#ifndef lint
static char *rcsid_test_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/mrtest.c,v 1.1 1987-08-22 18:31:59 wesommer Exp $";
#endif	lint

/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/mrtest.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/mrtest.c,v 1.1 1987-08-22 18:31:59 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 */

#ifndef lint
static char *rcsid_test_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/mrtest.c,v 1.1 1987-08-22 18:31:59 wesommer Exp $";
#endif lint

#include <stdio.h>
#include <sms.h>
#include <ss.h>

int ss;
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

	ss = ss_create_invocation("sms", "0.1", (char *)NULL,
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
	printf("%d tuples\n", count);
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

