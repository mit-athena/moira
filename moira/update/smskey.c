/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/smskey.c,v 1.3 1998-01-05 14:59:45 danw Exp $ */

/*  (c) Copyright 1988 by the Massachusetts Institute of Technology. */
/*  For copying and distribution information, please see the file */
/*  <mit-copyright.h>. */


#include <mit-copyright.h>
#include <stdio.h>
#include <sys/file.h>
#include <fcntl.h>
#include <des.h>
int debug=0;			/* goddamn des library breakage */
char string[] = "sms\0\0Athena\0\1";
char srvtab[] = "/etc/srvtab";
char insecure[] =
    "\7\7\7This program MUST be run on the console, for security reasons.\n";

main(argc,argv)
    int argc;
    char **argv;
{
    int fd;
    C_Block key;
    char *tty, *ttyname();
    tty = ttyname(0);
    if (!tty || strcmp(tty, "/dev/console")) {
	fprintf(stderr, insecure);
	exit(1);
    }
    fd = open(srvtab, O_WRONLY|O_APPEND, 0);
    if (!fd) {
	perror(srvtab);
	exit(1);
    }
    if (sizeof(string)-1 != 13) {
	fprintf(stderr, "string size bad\n");
	exit(1);
    }
    des_read_password(key, "Enter SMS update password: ", 0);
    write(fd, string, sizeof(string)-1);
    write(fd, key, sizeof(C_Block));
    if (fsync(fd))
	perror(srvtab);
    close(fd);
    printf("\nKey written.\n");
}
