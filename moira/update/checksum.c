/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/checksum.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/checksum.c,v 1.2 1988-08-04 14:20:23 mar Exp $
 */

#ifndef lint
static char *rcsid_checksum_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/update/checksum.c,v 1.2 1988-08-04 14:20:23 mar Exp $";
#endif	lint

#include <stdio.h>
#include <sys/file.h>

/*
 * checksum_fd(fd)
 * returns 24-bit checksum of bytes in file
 */

int
checksum_file(path)
char *path;
{
    register int sum;
    register int ch;
    register FILE *f;

    sum = 0;
    f = fopen(path, "r");
    while ((ch = getc(f)) != EOF) {
	sum = (sum + ch) & ((1<<24)-1);
    }
    fclose(f);
    return(sum);
}
