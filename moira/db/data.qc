
#include <stdio.h>
##define USERSMS 17069

FILE *open_file();

do_users()
##{
    FILE *u, *f, *p;
##  int flag;
/* 	login		= text(8), */
##	char 	t_login[9];
/* 	users_id	= i4, */
##	int	t_users_id;
/* 	uid		= i2, */
##	int	t_uid;
/* 	shell		= text(32), */
##	char 	t_shell[33];
/* 	home		= text(32), */
##	char 	t_home[33];
/* 	last		= text(16), */
##	char 	t_last[17];
/* 	first		= text(16), */
##	char 	t_first[17];
/* 	middle		= text(16), */
##	char 	t_middle[17];
/* 	status		= i2, */
##	int	t_status;
/* 	mit_id		= c13, */
##	char	t_mit_id[14];
/* 	mit_year	= text(8), */
##	char 	t_mit_year[9];
/* 	expdate		= date, */
##	char	t_expdate[26];
/* 	modtime		= date, */
##	char	t_modtime[26];
/* 	acl_id		= i4 */
##	int	t_acl_id;
/* 	users_id	= i4, */
##	int	t_fusers_id;
/* 	fullname	= text(32), */
##	char 	t_fullname[33];
/* 	nickname	= text(16), */
##	char 	t_nickname[17];
/* 	home_addr	= text(80), */
##	char 	t_home_addr[81];
/* 	home_phone	= text(16), */
##	char 	t_home_phone[17];
/* 	office_addr	= text(16), */
##	char 	t_office_addr[17];
/* 	office_phone	= text(12), */
##	char 	t_office_phone[13];
/* 	mit_dept	= text(12), */
##	char 	t_mit_dept[13];
/* 	mit_affil	= text(4), */
##	char 	t_mit_affil[5];
/* 	modtime		= date */
##	char	t_fmodtime[26];
/* 	users_id	= i4, */
##	int	t_pusers_id;
/* 	type		= text(8), */
##	char 	t_type[9];
/* 	mach_id		= i4, */
##	int	t_mach_id;
/* 	box		= text(32), */
##	char 	t_box[33];
/* 	created		= date */
##	char	t_pcreated[26];

    u = open_file("users");
    f = open_file("finger");
    p = open_file("pobox");

##  range of u is users
##  range of m is machine

    while(!feof(u)) {
	parse_str(u, t_login, 9);
	if (feof(u)) break;
	parse_sep(u);
	t_users_id = parse_int(u);
	parse_sep(u);
	t_uid = parse_int(u);
	parse_sep(u);
	parse_str(u, t_shell, 33);
	parse_sep(u);
	parse_str(u, t_home, 33);
	parse_sep(u);
	parse_str(u, t_last, 17);
	parse_sep(u);
	parse_str(u, t_first, 17);
	parse_sep(u);
	parse_str(u, t_middle, 17);
	parse_sep(u);
	t_status = parse_int(u);
	parse_sep(u);
	parse_str(u, t_mit_id, 14);
	parse_sep(u);
	parse_str(u, t_mit_year, 9);
	parse_sep(u);
	parse_str(u, t_expdate, 26);
	parse_sep(u);
	parse_str(u, t_modtime, 26);
	parse_sep(u);
	t_acl_id = parse_int(u);
	parse_nl(u);

	while (t_fusers_id < t_users_id) {
	    t_fusers_id = parse_int(f);
	    parse_sep(f);
	    parse_str(f, t_fullname, 33);
	    parse_sep(f);
	    parse_str(f, t_nickname, 17);
	    parse_sep(f);
	    parse_str(f, t_home_addr, 81);
	    parse_sep(f);
	    parse_str(f, t_home_phone, 17);
	    parse_sep(f);
	    parse_str(f, t_office_addr, 17);
	    parse_sep(f);
	    parse_str(f, t_office_phone, 13);
	    parse_sep(f);
	    parse_str(f, t_mit_dept, 13);
	    parse_sep(f);
	    parse_str(f, t_mit_affil, 5);
	    parse_sep(f);
	    parse_str(f, t_fmodtime, 26);
	    parse_nl(f);
	    if (t_fusers_id < t_users_id)
	      printf("orphaned finger info for user \"%s\" (%d)\n",
		     t_fullname, t_fusers_id);
	}

	while (t_pusers_id < t_users_id) {
	    t_pusers_id = parse_int(p);
	    parse_sep(p);
	    parse_str(p, t_type, 9);
	    parse_sep(p);
	    t_mach_id = parse_int(p);
	    parse_sep(p);
	    parse_str(p, t_box, 33);
	    parse_sep(p);
	    parse_str(p, t_pcreated, 26);
	    parse_nl(p);
	    if (t_pusers_id < t_users_id)
	      printf("orphaned pobox for user \"%s\" (%d)\n",
		     t_box, t_pusers_id);
	}
/*
 #	repeat retrieve (flag = any(u.login where u.login = @t_login
 #			or u.users_id = @t_users_id))
	if (flag) {
	    printf("Conflict!  User %s, users_id %d conflicts with another user in the database\n", t_login, t_users_id);
	    continue;
	}
*/
	printf("adding \"%s\"\n", t_login);
	if (t_users_id == t_fusers_id && t_users_id == t_pusers_id) {
##	append users (
##		login = t_login,
##		users_id = t_users_id,
##		uid = t_uid,
##		shell = t_shell,
##		last = t_last,
##		first = t_first,
##		middle = t_middle,
##		status = t_status,
##		mit_id = t_mit_id,
##		mit_year = t_mit_year,
##		modtime = t_modtime,
##		modwith = "convert",
##		modby = USERSMS,
##		fullname = t_fullname,
##		nickname = t_nickname,
##		home_addr = t_home_addr,
##		home_phone = t_home_phone,
##		office_addr = t_office_addr,
##		office_phone = t_office_phone,
##		mit_dept = t_mit_dept,
##		mit_affil = t_mit_affil,
##		fmodtime = t_modtime,
##		fmodwith = "convert",
##		fmodby = USERSMS,
##		potype = t_type,
##		pop_id = t_mach_id,
##		box_id = t_mach_id,
##		pmodtime = t_created,
##		pmodwith = "convert",
##		pmodby = USERSMS
##		)
	} else if (t_users_id == t_fusers_id) {
##	append users (
##		login = t_login,
##		users_id = t_users_id,
##		uid = t_uid,
##		shell = t_shell,
##		last = t_last,
##		first = t_first,
##		middle = t_middle,
##		status = t_status,
##		mit_id = t_mit_id,
##		mit_year = t_mit_year,
##		modtime = t_modtime,
##		modwith = "convert",
##		modby = USERSMS,
##		fullname = t_fullname,
##		nickname = t_nickname,
##		home_addr = t_home_addr,
##		home_phone = t_home_phone,
##		office_addr = t_office_addr,
##		office_phone = t_office_phone,
##		mit_dept = t_mit_dept,
##		mit_affil = t_mit_affil,
##		fmodtime = t_modtime,
##		fmodwith = "convert",
##		fmodby = USERSMS,
##		potype = "NONE",
##		pop_id = 0,
##		box_id = 0,
##		pmodtime = "now",
##		pmodwith = "convert",
##		pmodby = USERSMS
##		)
	} else {
##	append users (
##		login = t_login,
##		users_id = t_users_id,
##		uid = t_uid,
##		shell = t_shell,
##		last = t_last,
##		first = t_first,
##		middle = t_middle,
##		status = t_status,
##		mit_id = t_mit_id,
##		mit_year = t_mit_year,
##		modtime = t_modtime,
##		modwith = "convert",
##		modby = USERSMS,
##		mit_affil = t_mit_year,
##		fmodtime = "now",
##		fmodwith = "convert",
##		fmodby = USERSMS,
##		potype = "NONE",
##		pop_id = 0,
##		box_id = 0,
##		pmodtime = "now",
##		pmodwith = "convert",
##		pmodby = USERSMS
##		)
	}
    }
	(void) fclose(f);
    
##}

