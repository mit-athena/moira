/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gen/mitdir.qc,v 1.1 1989-12-04 14:58:41 mar Exp $
 *
 * This generates the input file for the mkdirdb program for the MIT.EDU
 * mailhub.
 *
 *  (c) Copyright 1989 by the Massachusetts Institute of Technology.
 *  For copying and distribution information, please see the file
 *  <mit-copyright.h>.
 */


#include <mit-copyright.h>
#include <stdio.h>
#include <string.h>
#include <sms.h>
#include <sms_app.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>


extern int errno;
char *whoami = "finger.gen";
char *ingres_date_and_time();


main(argc, argv)
int argc;
char **argv;
{
    long tm = time(NULL);
    FILE *out= stdout;
    char filename[64], *targetfile;
    struct stat sb;
##  int flag1;
##  char *filetime;
    int ingerr();

    IIseterr(ingerr);
    initialize_sms_error_table();
##  ingres sms
##  set lockmode session where level = table

    if (argc == 2) {
	if (stat(argv[1], &sb) == 0) {
	    filetime = ingres_date_and_time(sb.st_mtime);
##	    retrieve (flag1 = int4(interval("min",tblstats.modtime - filetime)))
##		where tblstats.table = "users"
	    if (flag1 < 0) {
		fprintf(stderr, "File %s does not need to be rebuilt.\n",
			argv[1]);
		exit(SMS_NO_CHANGE);
	    }
	}
	targetfile = argv[1];
	sprintf(filename, "%s~", targetfile);
	if ((out = fopen(filename, "w")) == NULL) {
	    fprintf(stderr, "unable to open %s for output\n", filename);
	    exit(SMS_OCONFIG);
	}
    } else if (argc != 1) {
	fprintf(stderr, "usage: %s [outfile]\n", argv[0]);
	exit(SMS_ARGS);
    }

##  begin transaction
    get_info(out);
##  end transaction
##  exit

    if (fclose(out)) {
	perror("close failed");
	exit(SMS_CCONFIG);
    }

    if (argc == 2)
      fix_file(targetfile);
    exit(SMS_SUCCESS);
}


/*
 * ingerr: (supposedly) called when Ingres indicates an error.
 * I have not yet been able to get this to work to intercept a
 * database open error.
 */
#define INGRES_DEADLOCK 4700

static int ingerr(num)
    int *num;
{
    char buf[256];
    int ingres_errno;

    switch (*num) {
    case INGRES_DEADLOCK:
	ingres_errno = SMS_DEADLOCK;
	break;
    default:
	ingres_errno = SMS_INGRES_ERR;
    }
    com_err(whoami, SMS_INGRES_ERR, " code %d\n", *num);
    critical_alert("DCM", "Alias build encountered INGRES ERROR %d", *num);
    exit(ingres_errno);
}


get_info(out)
FILE *out;
##{
##  int id, pid, bid, mid;
##  char name[129], type[9], buf[257], fname[17], mname[17], lname[17];
##  char year[9], dept[13], oaddr[17], ophone[13], haddr[81], hphone[17];
    char stuemp, *mhost, *uname, *pob;
    struct hash *strings;

    /* get locks */
##  retrieve (buf = users.modtime) where users.users_id = 0

    strings = create_hash(2000);
##  retrieve (id = strings.string_id, name = strings.string) {
	hash_store(strings, id, strsave(strtrim(name)));
##  }


##  range of u is users
##  retrieve (id = u.users_id, name = u.login,
##	      fname = u.first, mname = u.middle, lname = u.last,
##	      year = u.mit_year, dept = u.mit_dept,
##	      oaddr = u.office_addr, ophone = u.office_phone,
##	      haddr = u.home_addr, hphone = u.home_phone,
##	      type = u.potype, bid = u.box_id) 
##    	where u.status != 3 {

    	if (id == 0)
	  continue;

	strtrim(year);
	if (atoi(year) || !strcmp(year, "G"))
	  stuemp = 'S';
	else if (!strcmp(year, "FACULTY") || !strcmp(year, "MITS") ||
		 !strcmp(year, "STAFF"))
	  stuemp = 'E';
	else if (!strcmp(year, "PROJECT") || !strcmp(year, "SYSTEM"))
	  continue;
	else
	  stuemp = '?';

	if (type[0] == 'S' & (pob = hash_lookup(strings, bid)) != NULL) {
	    uname = pob;
	    mhost = index(pob, '@');
	    if (mhost)
	      *mhost++ = 0;
	    else
	      mhost = &uname[strlen(uname) + 1];
	} else if (type[0] == 'N') {
	    mhost = "";
	    uname = "";
	} else {
	    mhost = "ATHENA.MIT.EDU";
	    uname = strtrim(name);
	}

	fprintf(out, "%c:%s:%s:%s:%s:",
		stuemp, "", strtrim(lname), strtrim(fname), strtrim(mname));
	fprintf(out, "%s:%s:%s:%s:%s:%s:%s:%s:",
		strtrim(oaddr), "", strtrim(ophone), "",
		"", "", uname, mhost);
	fprintf(out, "%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%d\n",
		strtrim(haddr), "", "", "", strtrim(hphone),
		"", "", "", "", year, "", id);
##  }
##}
