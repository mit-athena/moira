/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/startmoira.c,v $
 *	$Author: mar $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/startmoira.c,v 1.7 1990-03-19 20:52:07 mar Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 * 	This program starts the moira server in a "clean" environment.
 *	and then waits for it to exit.
 * 
 */

#ifndef lint
static char *rcsid_mr_starter_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/server/startmoira.c,v 1.7 1990-03-19 20:52:07 mar Exp $";
#endif lint

#include <mit-copyright.h>
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <moira_site.h>

#define PROG	"moirad"

int rdpipe[2];
char *sigdescr[] = {
	0,
	"hangup",
	"interrupt",	
	"quit",
	"illegal instruction",
	"trace/BPT trap",
	"IOT trap",
	"EMT trap",
	"floating exception",
	"kill",
	"bus error",
	"segmentation violation",
	"bad system call",
	"broken pipe",
	"alarm clock",
	"termination",
	"urgent I/O condition",
	"stopped",
	"stopped",
	"continued",
	"child exited",
	"stopped (tty input)",
	"stopped (tty output)",
	"I/O possible",
	"cputime limit exceeded",
	"filesize limit exceeded",
	"virtual timer expired",
	"profiling timer expired",
	"window size changed",
	"signal 29",
	"user defined signal 1",
	"user defined signal 2",
	"signal 32"
};

cleanup()
{
	union wait stat;
	char buf[BUFSIZ];
	extern int errno;
	int serrno = errno;

	buf[0]='\0';
	
	while (wait3(&stat, WNOHANG, 0) > 0) {
		if (WIFEXITED(stat)) {
			if (stat.w_retcode)
				sprintf(buf,
					"exited with code %d\n",
					stat.w_retcode);
		}
		if (WIFSIGNALED(stat)) {
			sprintf(buf, "exited on %s signal%s\n",
				sigdescr[stat.w_termsig],
				(stat.w_coredump?"; Core dumped":0));
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
	
	extern int errno;
	extern char *sys_errlist[];
	
	int nfds = getdtablesize();
	
	setreuid(0);
	signal(SIGCHLD, cleanup);
	
	sprintf(buf, "%s/moira.log", SMS_DIR);
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
		exit();
	}
	chdir("/");	
	close(0);
	close(1);
	close(2);
	dup2(inf, 0);
	dup2(inf, 1);
	dup2(inf, 2);
	
	tty = open("/dev/tty");
	ioctl(tty, TIOCNOTTY, 0);
	close(tty);
	sprintf(buf, "%s/%s", BIN_DIR, PROG);
	
	if ((pid = fork()) == 0) {
		
		dup2(inf, 0);
		dup2(rdpipe[1], 1);
		dup2(1,2);
		for (i = 3; i <nfds; i++) close(i);
		execl(buf, PROG, 0);
		perror("cannot run moirad");
		exit(1);
	}
	if (pid<0) {
		perror("moira_starter");
		exit(1);
	}

	log = fdopen(logf, "w");
	prog = fdopen(rdpipe[0], "r");
	
	
	do {
		char *time_s;
		extern char *ctime();
		long foo;
		
		done = 0;
		errno = 0;
		if (fgets(buf, BUFSIZ, prog) == NULL) {
			if (errno) {
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



	  
