/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dcm/startdcm.c,v $
 *	$Author: danw $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dcm/startdcm.c,v 1.8 1997-01-29 23:13:54 danw Exp $
 *
 *	Copyright (C) 1987, 1988 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 * 	This program starts the DCM in a "clean" environment.
 *	and then waits for it to exit.
 */

#ifndef lint
static char *rcsid_mr_starter_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/dcm/startdcm.c,v 1.8 1997-01-29 23:13:54 danw Exp $";
#endif lint

#include <mit-copyright.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <moira_site.h>
#include <time.h>

#define PROG 	"dcm"

int rdpipe[2];
extern int errno;

void cleanup()
{
	int stat, serrno = errno;
	char buf[BUFSIZ];

	buf[0]='\0';
	
	while (waitpid(-1, &stat, WNOHANG) > 0) {
		if (WIFEXITED(stat)) {
			if (WEXITSTATUS(stat))
				sprintf(buf,
					"exited with code %d\n",
					WEXITSTATUS(stat));
		}
		if (WIFSIGNALED(stat)) {
			sprintf(buf, "exited on signal %d%s\n",
				WTERMSIG(stat),
				(WCOREDUMP(stat)?"; Core dumped":0));
		}
		write(rdpipe[1], buf, strlen(buf));
		close(rdpipe[1]);
	}
	errno = serrno;
}

main(argc, argv)
{
	char buf[BUFSIZ];
	FILE *log, *prog;
	int logf, inf, i, done, pid, tty;
	struct rlimit rl;
	
	extern int errno;
	extern char *sys_errlist[];
	
	struct sigaction action;
	int nfds;
	
	getrlimit(RLIMIT_NOFILE, &rl);
	nfds = rl.rlim_cur;

	action.sa_handler = cleanup;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaction(SIGCHLD, &action, NULL);
	
	sprintf(buf, "%s/%s.log", SMS_DIR, PROG);
	logf = open(buf, O_CREAT|O_WRONLY|O_APPEND, 0640);
	if (logf<0) {
		perror(buf);
		exit(1);
	}
	inf = open("/dev/null", O_RDONLY , 0);
	if (inf < 0) {
		perror("/dev/null");
		exit(1);
	}
	pipe(rdpipe);
	if (fork()) {
		exit(0);
	}
	chdir("/");	
	close(0);
	close(1);
	close(2);
	dup2(inf, 0);
	dup2(inf, 1);
	dup2(inf, 2);
	
	setpgrp();
	sprintf(buf, "%s/%s", BIN_DIR, PROG);
	
	if ((pid = fork()) == 0) {
		
		dup2(inf, 0);
		dup2(rdpipe[1], 1);
		dup2(1,2);
		for (i = 3; i <nfds; i++) close(i);
		execl(buf, PROG, 0);
		perror("cannot run dcm");
		exit(1);
	}
	if (pid<0) {
		perror("startdcm");
		exit(1);
	}

	log = fdopen(logf, "w");
	prog = fdopen(rdpipe[0], "r");
	
	
	do {
		char *time_s;
		long foo;
		
		done = 0;
		errno = 0;
		if (fgets(buf, BUFSIZ, prog) == NULL) {
			if (errno && errno!=EINTR) {
				strcpy(buf, "Unable to read from program: ");
				strcat(buf, sys_errlist[errno]);
				strcat(buf, "\n");
			} else break;
		}
		time(&foo);
		time_s = ctime(&foo)+4;
		time_s[strlen(time_s)-6]='\0';
		fprintf(log, "%s <%d> %s", time_s, pid, buf);
		fflush(log);
	} while (!done);
	exit(0);
}
