/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/regtape/resign.qc,v 1.1 1993-01-04 16:40:35 mar Exp $
 *
 * This program will bulk resign user records in the database.
 */

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <sys/time.h>
#include <moira.h>
#include <moira_site.h>
#include <des.h>
#include <krb.h>
#include <krb_err.h>
#include <gdss.h>


char *program;

main(argc, argv)
int argc;
char **argv;
##{
     char buf[BUFSIZ], *data, *p;
     struct save_queue *sq;
     SigInfo si;
     int found, status, i, wait;
##   char login[10], mid[32], rawsig[256];
##   int id, timestamp, sms, moira;

     program = "resign";
     init_krb_err_tbl();
     init_sms_err_tbl();

     /* Set the name of our kerberos ticket file */
     krb_set_tkt_string("/tmp/tkt_sign");
     status = 1;
     while (status) {
	 printf("Authenticating as moira.extra:\n");
	 status = krb_get_pw_in_tkt("moira", "extra", "ATHENA.MIT.EDU",
				    "krbtgt", "ATHENA.MIT.EDU",
				    DEFAULT_TKT_LIFE, 0);
	 if (status != 0)
	   com_err(program, status + krb_err_base, " in krb_get_pw_in_tkt");
     }

     for (i = 1; i < argc; i++) {
	if (!strcmp(argv[i], "-w"))
	  wait++;
	else if (!strcmp(argv[i], "-D"))
	  setenv("ING_SET", "set printqry");
	else
	  fprintf(stderr, "Usage: %s [-w] [-D]\n", argv[0]);
    }


##   ingres #sms
##   range of u is users
##   range of s is strings

     sms = 0;
##   retrieve (sms = s.string_id) where s.string="sms@ATHENA.MIT.EDU"
     if (sms == 0) {
	 com_err(program, 0, " failed to find string sms@ATHENA.MIT.EDU in database");
	 dest_tkt();
	 exit(1);
     }
     moira = 0;
##   retrieve (moira = s.string_id) where s.string="moira.extra@ATHENA.MIT.EDU"
     if (moira == 0) {
	 com_err(program, 0, " failed to find string moira.extra@ATHENA.MIT.EDU in database");
	 dest_tkt();
	 exit(1);
     }

     found = 0;
     sq = sq_create();

##   retrieve (id = u.users_id, login = u.#login, mid = u.mit_id)
##	where u.sigwho = sms {
	sprintf(buf, "%d:%s:%s", id, strtrim(login), strtrim(mid));
	sq_save_data(sq, strsave(buf));
	found++;
##   }

     printf("Found %d users to resign.\n", found);

     si.rawsig = (unsigned char *) &rawsig[0];

     while (sq_get_data(sq, &data)) {
	 p = index(data, ':');
	 if (!p) {
	     com_err(program, 0, " malformatted data");
	     continue;
	 }
	 *p++ = 0;
	 id = atoi(data);
	 data = p;
	 status = GDSS_Sign(data, strlen(data), buf);
	 if (status) {
	     com_err(program, gdss2et(status), "resigning data");
	     continue;
	 }
	 status = GDSS_Verify(data, strlen(data), buf, &si);
	 if (status) {
	     com_err(program, gdss2et(status), "verifying data");
	     continue;
	 }

	 timestamp = si.timestamp;
##	 replace u (signature = rawsig, sigwho = moira, sigdate = timestamp)
##	   where u.users_id = id
	 if (wait) {
	     printf("Next");
	     fflush(stdout);
	     gets(buf);
	 }
     }
##   exit
     dest_tkt();
     exit(0);
##}
