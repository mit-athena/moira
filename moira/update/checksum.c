/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/checksum.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/checksum.c,v 1.1 1987-08-22 17:53:28 wesommer Exp $
 */

#ifndef lint
static char *rcsid_checksum_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/checksum.c,v 1.1 1987-08-22 17:53:28 wesommer Exp $";
#endif	lint

#include <stdio.h>
#include <sys/file.h>

/*
 * checksum_fd(fd)
 * returns 24-bit checksum of bytes in file
 */

int
checksum_fd(fd1)
    int fd1;
{
    int fd;
    register int sum;
    register int ch;
    register FILE *f;

    fd = dup(fd1);
    sum = 0;
    (void) lseek(fd, 0, L_SET);
    f = fdopen(fd, "r");
    while ((ch = getc(f)) != EOF) {
	sum = (sum + ch) & ((1<<24)-1);
    }
    fclose(f);
    return(sum);
}
