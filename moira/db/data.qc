/*  (c) Copyright 1988 by the Massachusetts Institute of Technology. */
/*  For copying and distribution information, please see the file */
/*  <mit-copyright.h>. */

#include <mit-copyright.h>
#include <stdio.h>
##define USERSMS 17069

FILE *open_file();
char *strsave(), *strtrim();

do_users()
##{
    register FILE *u;
##  int flag;
##	char 	t_login[9];
##	int	t_users_id;
##	int	t_uid;
##	char 	t_shell[33];
##	char 	t_home[33];
##	char 	t_last[17];
##	char 	t_first[17];
##	char 	t_middle[17];
##	int	t_status;
##	char	t_mit_id[14];
##	char 	t_mit_year[9];
##	char	t_expdate[26];
##	char	t_modtime[26];
##	int	t_acl_id;
##	int	err;

    u = open_file("users");
    start_counter(10);

##  range of u is users
##  delete u where u.users_id > 0

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

	inc_count();
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
    (void) fclose(u);
    end_counter();
##  modify users to cbtree unique on users_id
##  inquire_equel(err = "errorno");
    if (err != 0)
      punt("modify on users failed");
##  index on users is i_usr_login (login)
##  inquire_equel(err = "errorno");
    if (err != 0)
      punt("index on users failed");
##  modify i_usr_login to hash
##  inquire_equel(err = "errorno");
    if (err != 0)
      punt("modify on users index failed");
##}


do_finger()
##{
    register FILE *f;
##	int	t_fusers_id;
##	char 	t_fullname[33];
##	char 	t_nickname[17];
##	char 	t_home_addr[81];
##	char 	t_home_phone[17];
##	char 	t_office_addr[17];
##	char 	t_office_phone[13];
##	char 	t_mit_dept[13];
##	char 	t_mit_affil[5];
##	char	t_modtime[26];
##	int rowcount;

    f = open_file("finger");
##  range of u is users
    start_counter(10);

    while (!feof(f)) {
	t_fusers_id = parse_int(f);
	if (feof(f))
	  break;
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
	parse_str(f, t_modtime, 26);
	parse_nl(f);

	inc_count();
##	repeat replace users (
##		fullname = @t_fullname,
##		nickname = @t_nickname,
##		home_addr = @t_home_addr,
##		home_phone = @t_home_phone,
##		office_addr = @t_office_addr,
##		office_phone = @t_office_phone,
##		mit_dept = @t_mit_dept,
##		mit_affil = @t_mit_affil,
##		fmodtime = @t_modtime,
##		fmodwith = "convert",
##		fmodby = USERSMS
##	) where users.users_id = @t_fusers_id
##	inquire_equel(rowcount = "rowcount")
	if (rowcount == 0)
	  printf("\rOrphaned finger entry %d for \"%s\"\n",
		 t_fusers_id, t_fullname);
    }
    fclose(f);
    end_counter();
##}


char **machs;
##int maxmach;

do_mach()
##{
    register FILE *f;
##	char 	t_name[33];
##	int	t_mach_id;
##	char 	t_type[13];
    char buf[13];

    f = open_file("machine");
##  range of m is machine
    start_counter(25);

    while(!feof(f)) {
	parse_str(f, t_name, 33);
	if (feof(f)) break;
	parse_sep(f);
	t_mach_id = parse_int(f);
	parse_sep(f);
	parse_str(f, t_type, 13);
	parse_sep(f);
	parse_str(f, buf, 13);
	parse_sep(f);
	parse_str(f, buf, 13);
	parse_sep(f);
	parse_str(f, buf, 13);
	parse_sep(f);
	parse_str(f, buf, 13);
	parse_nl(f);

	inc_count();
##	append machine (name = t_name, mach_id = t_mach_id,
##			type = t_type, modtime = "now",
##			modby = USERSMS, modwith = "convert")
    }
    fclose(f);
    end_counter();
##}


do_clu()
##{
    register FILE *f;
##	char 	t_name[33];
##	int	t_clu_id;
##	char 	t_desc[33];
##	char 	t_location[65];
##	int	t_acl_id;
##  int rowcount;

    f = open_file("cluster");
##  range of c is cluster
    start_counter(10);

    while(!feof(f)) {
	parse_str(f, t_name, 33);
	if (feof(f)) break;
	parse_sep(f);
	t_clu_id = parse_int(f);
	parse_sep(f);
	parse_str(f, t_desc, 33);
	parse_sep(f);
	parse_str(f, t_location, 65);
	parse_sep(f);
	t_acl_id = parse_int(f);
	parse_nl(f);

	inc_count();
##	append cluster (
##		name = t_name,
##		clu_id = t_clu_id,
##		desc = t_desc,
##		location = t_location,
##		modtime = "now",
##		modby = USERSMS,
##		modwith = "convert")
    }
    fclose(f);
    end_counter();
##}


do_mcm()
##{
    register FILE *f;
##	int	t_clu_id;
##	int	t_mach_id;

    f = open_file("machclumap");
    start_counter(25);

    while(!feof(f)) {
	t_clu_id = parse_int(f);
	if (feof(f)) break;
	parse_sep(f);
	t_mach_id = parse_int(f);
	parse_nl(f);

	inc_count();
##	append mcmap (
##		clu_id = t_clu_id,
##		mach_id = t_mach_id)
    }
    (void) fclose(f);
    end_counter();
##}


do_cld()
##{
    register FILE *f;
##	int	t_clu_id;
##	char 	t_serv_label[17];
##	char 	t_serv_cluster[33];

    f = open_file("svc");
    start_counter(10);

    while(!feof(f)) {
	t_clu_id = parse_int(f);
	if (feof(f)) break;
	parse_sep(f);
	parse_str(f, t_serv_label, 17);
	parse_sep(f);
	parse_str(f, t_serv_cluster, 33);
	parse_nl(f);

	inc_count();
##	append svc (
##		clu_id = t_clu_id,
##		serv_label = t_serv_label,
##		serv_cluster = t_serv_cluster)
    }
    (void) fclose(f);
    end_counter();
##}


do_servers()
##{
    register FILE *f;
##	char 	t_service[17];
##	int	t_update_int;
##	char 	t_target_file[65];
##	char 	t_script[129];
##	char	t_dfgen[26];

    f = open_file("servers");
    start_counter(1);

    while(!feof(f)) {
	parse_str(f, t_service, 17);
	if (feof(f)) break;
	parse_sep(f);
	t_update_int = parse_int(f);
	parse_sep(f);
	parse_str(f, t_target_file, 65);
	parse_sep(f);
	parse_str(f, t_script, 129);
	parse_sep(f);
	parse_str(f, t_dfgen, 26);
	parse_nl(f);

	inc_count();
##	append servers (
##			name = uppercase(t_service),
##			update_int = t_update_int,
##			target_file = t_target_file,
##			script = t_script,
##			dfgen = 0,
##			type = "UNIQUE",
##			enable = 0,
##			inprogress = 0,
##			harderror = 0,
##			errmsg = "",
##			acl_type = "NONE",
##			acl_id = 0,
##			modtime = "now",
##			modby = USERSMS,
##			modwith = "convert"
##			)
    }
    (void) fclose(f);
    end_counter();
##}


do_serverhosts()
##{
    register FILE *f;
##	char 	t_service[17];
##	int	t_mach_id;
##	char 	t_enable[33];
##	int	t_overide;
##	char	t_ltt[26];
##	char	t_success[2];
##	int	t_value1;
##	int	t_value2;
##	int	err;

    f = open_file("serverhosts");
    start_counter(5);

    while(!feof(f)) {
	parse_str(f, t_service, 17);
	if (feof(f)) break;
	parse_sep(f);
	t_mach_id = parse_int(f);
	parse_sep(f);
	parse_str(f, t_enable, 33);
	parse_sep(f);
	t_overide = parse_int(f);
	parse_sep(f);
	parse_str(f, t_ltt, 26);
	parse_sep(f);
	parse_str(f, t_success, 2);
	parse_sep(f);
	t_value1 = parse_int(f);
	parse_sep(f);
	t_value2 = parse_int(f);
	parse_nl(f);

	inc_count();
	if (t_mach_id == 0)
	  continue;
##	append serverhosts (
##			service = uppercase(t_service),
##			mach_id = t_mach_id,
##			enable = 0,
##			override = t_overide,
##			ltt = 0,
##			lts = 0,
##			success = int1(t_success),
##			inprogress = 0,
##			hosterror = 0,
##			hosterrmsg = "",
##			value1 = t_value1,
##			value2 = t_value2,
##			modtime = "now",
##			modby = USERSMS,
##			modwith = "convert")
    }
    (void) fclose(f);
    end_counter();
##  modify serverhosts to btree on service
##  inquire_equel(err = "errorno")
    if (err != 0)
      punt("unable to modify serverhosts");
##  index on serverhosts is i_sho_mach (mach_id)
##  inquire_equel(err = "errorno")
    if (err != 0)
      punt("unable to index serverhosts");
##  modify i_sho_mach to cbtree
##  inquire_equel(err = "errorno")
    if (err != 0)
      punt("unable to modify serverhosts index");
##}


do_list()
##{
    register FILE *f;
##	char 	t_name[33];
##	int	t_list_id;
##	int	t_flags;
##	char 	t_desc[257];
##	int	t_created;
##	int	t_acl_id;
##	char	t_expdate[26];
##	char	t_modtime[26];
##	int	t_active, t_public, t_hidden, err;

/* delete default & dbadmin */
##  delete list where list.list_id = 1 or list.list_id = 2

    f = open_file("list");
    start_counter(5);

    while(!feof(f)) {
	parse_str(f, t_name, 33);
	if (feof(f)) break;
	parse_sep(f);
	t_list_id = parse_int(f);
	parse_sep(f);
	t_flags = parse_int(f);
	parse_sep(f);
	parse_str(f, t_desc, 257);
	parse_sep(f);
	t_acl_id = parse_int(f);
	parse_sep(f);
	parse_str(f, t_expdate, 26);
	parse_sep(f);
	parse_str(f, t_modtime, 26);
	parse_nl(f);

	t_active = t_flags & 1;
	t_public = (t_flags>>1) & 1;
	t_hidden = (t_flags>>2) & 1;
	inc_count();
##	append list (
##		name = t_name,
##		list_id = t_list_id,
##		active = t_active, public = t_public, hidden = t_hidden,
##		desc = t_desc,
##		acl_type = "LIST",
##		acl_id = t_acl_id,
##		modtime = t_modtime,
##		modby = USERSMS,
##		modwith = "convert")
    }
    (void) fclose(f);
    end_counter();

##  modify list to cbtree unique on list_id
##  inquire_equel(err = "errorno")
    if (err != 0)
      punt("unable to modify list");
##  index on list is i_lis_name (name)
##  inquire_equel(err = "errorno")
    if (err != 0)
      punt("unable to index lists by name");
##  modify i_lis_name to hash
##  inquire_equel(err = "errorno")
    if (err != 0)
      punt("unable to modify list name index");
##}


do_maillists()
##{
    register FILE *f;
##	int	t_list_id;
##	int	t_ltid;
##	int	err;

    f = open_file("maillists");
##  range of l is list
    start_counter(5);

    while(!feof(f)) {
	t_list_id = parse_int(f);
	if (feof(f)) break;
	parse_sep(f);
	t_ltid = parse_int(f);
	parse_nl(f);

	inc_count();
##	repeat replace l (maillist = 1) where l.list_id = @t_list_id
    }
    end_counter();
    (void) fclose(f);
##  index on list is i_lis_mail (maillist)
##  inquire_equel(err = "errorno")
    if (err != 0)
      punt("unable to index lists by maillist");
##}


do_groups()
##{
    register FILE *f;
##	int	t_list_id;
##	int	t_ltid;
##	int	t_gid;
##	int	err;

    f = open_file("groups");
##  range of l is list
    start_counter(5);

    while(!feof(f)) {
	t_list_id = parse_int(f);
	if (feof(f)) break;
	parse_sep(f);
	t_ltid = parse_int(f);
	parse_sep(f);
	t_gid = parse_int(f);
	parse_nl(f);

	inc_count();
##	repeat replace l (group = 1, gid = @t_gid) where l.list_id = @t_list_id
    }
    end_counter();
    (void) fclose(f);
##  index on list is i_lis_group (group)
##  inquire_equel(err = "errorno")
    if (err != 0)
      punt("unable to index lists by group");
##}


do_members()
##{
    register FILE *f;
##	int	t_list_id;
##	char 	t_member_type[9];
##	int	t_member_id;

    f = open_file("members");
    start_counter(25);

    while(!feof(f)) {
	t_list_id = parse_int(f);
	if (feof(f)) break;
	parse_sep(f);
	parse_str(f, t_member_type, 9);
	parse_sep(f);
	t_member_id = parse_int(f);
	parse_nl(f);

	inc_count();
##	append members (
##		list_id = t_list_id,
##		member_type = t_member_type,
##		member_id = t_member_id)
    }
    end_counter();
    (void) fclose(f);
##}


do_strings()
##{
    register FILE *f;
##	int	t_string_id;
##	char 	t_string[129];
##	int	t_refc;

    f = open_file("strings");
    start_counter(10);

    while(!feof(f)) {
	t_string_id = parse_int(f);
	if (feof(f)) break;
	parse_sep(f);
	parse_str(f, t_string, 129);
	parse_sep(f);
	t_refc = parse_int(f);
	parse_nl(f);

	inc_count();
##	append strings (
##		string_id = t_string_id,
##		string = t_string)
    }
    end_counter();
    (void) fclose(f);
##}


do_nfsphys()
##{
    register FILE *f;
##	int	t_mach_id;
##	char 	t_device[17];
##	char 	t_dir[17];
##	int	t_status;
##	int	t_allocated;
##	int	t_size;
##	int	t_nfsphys_id;
##	int	err;

    f = open_file("nfsphys");
    start_counter(5);
    t_nfsphys_id = 0;

    while(!feof(f)) {
	t_mach_id = parse_int(f);
	if (feof(f)) break;
	parse_sep(f);
	parse_str(f, t_device, 17);
	parse_sep(f);
	parse_str(f, t_dir, 17);
	parse_sep(f);
	t_status = parse_int(f);
	parse_sep(f);
	t_allocated = parse_int(f);
	parse_sep(f);
	t_size = parse_int(f);
	parse_nl(f);
	t_nfsphys_id++;

	inc_count();
##	append nfsphys (
##		nfsphys_id = t_nfsphys_id,
##		mach_id = t_mach_id,
##		device = t_device,
##		dir = t_dir,
##		status = t_status,
##		allocated = t_allocated,
##		size = t_size,
##		modtime = "now",
##		modby = USERSMS,
##		modwith = "convert")
    }
    end_counter();
    (void) fclose(f);

##  modify nfsphys to cbtree unique on nfsphys_id
##  inquire_equel(err = "errorno")
    if (err != 0)
      punt("cannot modify filesys");
##  index on nfsphys is i_nfs_mach (mach_id)
##  inquire_equel(err = "errorno")
    if (err != 0)
      punt("cannot index filesys");
##  modify i_nfs_mach to btree
##  inquire_equel(err = "errorno")
    if (err != 0)
      punt("cannot modify i_nfs_mach");
##}


do_filesys()
##{
    register FILE *f;
##	char 	t_label[33];
##	int	t_order;
##	char 	t_type[9];
##	char 	t_name[33];
##	int	t_mach_id;
##	char	t_access[2];
##	char 	t_mount[33];
##	int	t_acl_id;
##	int	t_filsys_id;
##	int	t_phys_id;
##	int	err;

    f = open_file("filesys");
    start_counter(5);
    t_filsys_id = 0;

    while(!feof(f)) {
	parse_str(f, t_label, 33);
	if (feof(f)) break;
	parse_sep(f);
	parse_str(f, t_type, 9);
	parse_sep(f);
	t_mach_id = parse_int(f);
	parse_sep(f);
	parse_str(f, t_name, 33);
	parse_sep(f);
	parse_str(f, t_mount, 33);
	parse_sep(f);
	parse_str(f, t_access, 2);
	parse_sep(f);
	t_acl_id = parse_int(f);
	parse_nl(f);
	t_filsys_id++;
	if (!strcmp(t_type, "NFS"))
	  t_phys_id = match_phys(t_mach_id, t_name);
	else
	  t_phys_id = 0;

	inc_count();
##	append filesys (
##		filsys_id = t_filsys_id,
##		phys_id = t_phys_id,
##		label = t_label,
##		type = t_type,
##		mach_id = t_mach_id,
##		name = t_name,
##		mount = t_mount,
##		access = t_access,
##		owner = USERSMS,
##		owners = t_acl_id,
##		createflg = 0,
##		lockertype = "UNKNOWN",
##		modtime = "now",
##		modby = USERSMS,
##		modwith = "convert")
    }
    end_counter();
    (void) fclose(f);
##  modify filesys to cbtree unique on filsys_id
##  inquire_equel(err = "errorno")
    if (err != 0)
      punt("cannot modify filesys");
##  index on filesys is i_fil_name (label)
##  inquire_equel(err = "errorno")
    if (err != 0)
      punt("cannot index filesys on label");
##  modify i_fil_name to hash
##  inquire_equel(err = "errorno")
    if (err != 0)
      punt("cannot modify i_fil_name");
##  index on filesys is i_fil_mach (mach_id)
##  inquire_equel(err = "errorno")
    if (err != 0)
      punt("cannot index filesys on mach_id");
##  modify i_fil_mach to btree
##  inquire_equel(err = "errorno")
    if (err != 0)
      punt("cannot modify i_fil_mach");
##  index on filesys is i_fil_phys (phys_id)
##  inquire_equel(err = "errorno")
    if (err != 0)
      punt("cannot index filesys on phys_id");
##  modify i_fil_phys to btree
##  inquire_equel(err = "errorno")
    if (err != 0)
      punt("cannot modify i_fil_phys");
##}


do_nfsquota()
##{
    register FILE *f;
##	int	t_users_id;
##	int	t_mach_id;
##	char 	t_device[17];
##	int	t_quota;
##	int	t_filsys_id;

    f = open_file("nfsquota");
    start_counter(1);
##  range of f is filesys
##  range of u is users
##  range of n is nfsphys

    while(!feof(f)) {
	t_users_id = parse_int(f);
	if (feof(f)) break;
	parse_sep(f);
	t_mach_id = parse_int(f);
	parse_sep(f);
	parse_str(f, t_device, 17);
	parse_sep(f);
	t_quota = parse_int(f);
	parse_nl(f);
	t_filsys_id = match_filsys(t_mach_id, t_device, t_users_id);

	inc_count();
##	append nfsquota (
##		users_id = t_users_id,
##		filsys_id = t_filsys_id,
##		quota = t_quota,
##		modtime = "now",
##		modby = USERSMS,
##		modwith = "convert")
    }
    end_counter();
    (void) fclose(f);
##}


do_pobox()
##{
    register FILE *f;
##	int	t_users_id;
##	char 	t_type[9];
##	int 	t_mach_id;
##	char 	t_box[33];
##	char	t_created[26];
##	char	t_string[129];
##	int	t_string_id;
##	char	t_machine[65];
##	int rowcount;

    f = open_file("pobox");
##  range of u is users
##  range of m is machine
    start_counter(5);
##  retrieve (t_string_id = max(strings.string_id))
    t_string_id++;

    while (!feof(f)) {
	t_users_id = parse_int(f);
	if (feof(f)) break;
	parse_sep(f);
	parse_str(f, t_type, 9);
	parse_sep(f);
	t_mach_id = parse_int(f);
	parse_sep(f);
	parse_str(f, t_box, 33);
	parse_sep(f);
	parse_str(f, t_created, 26);
	parse_nl(f);

	inc_count();
	if (!strcmp(t_type, "POP")) {
##	    repeat replace users (
##		potype = @t_type,
##		pop_id = @t_mach_id,
##		pmodtime = @t_created,
##		pmodwith = "convert",
##		pmodby = USERSMS
##	    ) where users.users_id = @t_users_id
	} else if (!strcmp(t_type, "LOCAL")) {
##	    repeat retrieve (t_machine = machine.name) 
##		where machine.mach_id = @t_mach_id
##	    repeat replace users (
##		potype = "SMTP",
##		pop_id = 0,
##		box_id = @t_string_id,
##		pmodtime = @t_created,
##		pmodwith = "convert",
##		pmodby = USERSMS
##	    ) where users.users_id = @t_users_id
##	    append strings (string_id = t_string_id,
##			    string = t_box + "@" + t_machine)
	    t_string_id++;
	} else if (!strcmp(t_type, "FOREIGN")) {
##	    repeat retrieve (t_string = strings.string) 
##		where strings.string_id = @t_mach_id
##	    repeat replace users (
##		potype = "SMTP",
##		pop_id = 0,
##		box_id = @t_string_id,
##		pmodtime = @t_created,
##		pmodwith = "convert",
##		pmodby = USERSMS
##	    ) where users.users_id = @t_users_id
##	    append strings (string_id = t_string_id,
##			    string = t_box + "@" + t_string)
	    t_string_id++;
	} else {
	    printf("Unknown pobox type: %s\n", t_type);
	}
    }
    end_counter();
    fclose(f);
##}


do_services()
##{
    register FILE *f;
##	char 	t_name[17];
##	char 	t_protocol[9];
##	int	t_port;
##	char 	t_desc[65];

    f = open_file("services");
    start_counter(5);

    while(!feof(f)) {
	parse_str(f, t_name, 17);
	if (feof(f)) break;
	parse_sep(f);
	parse_str(f, t_protocol, 9);
	parse_sep(f);
	t_port = parse_int(f);
	parse_sep(f);
	parse_str(f, t_desc, 65);
	parse_nl(f);

	inc_count();
##	append services (
##		name = t_name,
##		protocol = t_protocol,
##		port = t_port,
##		desc = t_desc,
##		modtime = "now",
##		modby = USERSMS,
##		modwith = "convert")
    }
    (void) fclose(f);
    end_counter();
##}


do_printcap()
##{
    register FILE *f;
##	char 	t_name[17];
##	char 	t_pcap[513];

    f = open_file("printcap");
    start_counter(1);

    while(!feof(f)) {
	parse_str(f, t_name, 17);
	if (feof(f)) break;
	parse_sep(f);
	parse_str(f, t_pcap, 513);
	parse_nl(f);

	inc_count();
##	append printcap (
##		name = t_name,
##		mach_id = 0,
##		dir = "/usr/spool/printer/" + t_name,
##		rp = t_name,
##		modtime = "now",
##		modby = USERSMS,
##		modwith = "convert")
    }
    end_counter();
    (void) fclose(f);
##}


do_values()
##{
    register FILE *f;
##	char 	t_name[33];
##	int	t_value;

    f = open_file("values");
    while(!feof(f)) {
	parse_str(f, t_name, 33);
	if (feof(f)) break;
	parse_sep(f);
	t_value = parse_int(f);
	parse_nl(f);

	if (strcmp(t_name, "def_quota") &&
	    strcmp(t_name, "gid") &&
	    strcmp(t_name, "uid"))
	  continue;

##	replace values (value = t_value) where values.name = t_name
    }
    (void) fclose(f);
##}


do_alias()
##{
    register FILE *f;
##	char 	t_name[33];
##	char 	t_type[17];
##	char 	t_trans[129];

    f = open_file("alias");
    start_counter(5);
    while(!feof(f)) {
	parse_str(f, t_name, 33);
	if (feof(f)) break;
	parse_sep(f);
	parse_str(f, t_type, 17);
	parse_sep(f);
	parse_str(f, t_trans, 129);
	parse_nl(f);
	inc_count();

	if (!strcmp(t_type, "TYPE") ||
	    !strcmp(t_type, "TYPEDATA"))
	  continue;

##	append alias (
##		name = t_name,
##		type = t_type,
##		trans = t_trans)
    }
    end_counter();
    (void) fclose(f);
##}

