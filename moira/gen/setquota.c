/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/setquota.c,v $
 *	$Author: wesommer $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/setquota.c,v 1.1 1987-09-04 21:32:47 wesommer Exp $
 *
 *	Copyright (C) 1987 by the Massachusetts Institute of Technology
 *
 * 	Set quota on specified device for specified user to specified value.
 *
 *	Uses the NFS style quota system/quotactl rather than the Melbourne
 * quota system.
 * 	
 *	$Log: not supported by cvs2svn $
 */

#ifndef lint
static char *rcsid_setquota_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/setquota.c,v 1.1 1987-09-04 21:32:47 wesommer Exp $";
#endif lint

#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/time.h>
#include <ufs/quota.h>

#define kb(n)	(howmany(dbtob(n), 1024))

main(argc, argv)
	int argc;
	char **argv;
{
	char *device;
	int uid;
	struct dqblk db;
	
	if (argc != 4) {
	usage:
		fprintf(stderr, "usage: setquota special uid quota\n\
special is a mounted filesystem special device\n\
quota is in 1KB units\n");
		exit(1);
	}

	if (!isdigit(*argv[2]) || !isdigit(*argv[3])) {
		fprintf(stderr, "setquota: uid and quota must be numeric\n");
		goto usage;
	}
	
	device = argv[1];
	uid = atoi(argv[2]);
	
	db.dqb_bsoftlimit = atoi(argv[3]);
	db.dqb_bhardlimit = db.dqb_bsoftlimit * 1.2;
	db.dqb_fsoftlimit = atoi(argv[3]) * .5;
	db.dqb_fhardlimit = db.dqb_fsoftlimit * 1.2;
	db.dqb_btimelimit = DQ_FTIMELIMIT;
	db.dqb_btimelimit = DQ_BTIMELIMIT;

	db.dqb_bsoftlimit *= btodb(1024);
	db.dqb_bhardlimit *= btodb(1024);

	if (quotactl(Q_SETQLIM, device, uid, &db) < 0) {
		fprintf (stderr, "quotactl: %d on ", uid);
		perror (device);
		exit (1);
	}
	if (quotactl(Q_SYNC, device, 0, 0) < 0) {
		perror ("can't sync disk quota");
		exit (1);
	}
	exit (0);
}
	
