/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/sms_untar.c,v 1.1 1988-11-22 16:09:17 mar Exp $
 *
 * Reads a tar file from standard input, and extracts the contents as
 * the user specified by uid as an argument.
 *
 *  (c) Copyright 1988 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */

#include <mit-copyright.h>

main(argc,argv)
int argc;
char **argv; 
{
    if (getuid() != 0) {
	write(2, "You must be root to run sms_untar\n", 34);
	exit(1);
    }

    if (argc != 2) {
	write(2, "Usage: sms_untar uid\n", 21);
	write(2, "sms_untar takes a tar file as standard input.\n", 46);
	exit(2);
    }

    setuid(atoi(argv[1]));
    execl("/bin/tar", "tar", "xfp", "-", 0);
    write(2, "sms_untar: unable to exec /bin/tar\n", 35);
    exit(3);
}
