@part[addendum, root="sms.mss"]
@section(ADDENDUM)

The following information is information which supplements the 
formal text.  In general the addendum will provide critical
information regarding programming application and use.

@subsection(List of pre-defined queries)

The following list of queries are a pre-defind list.  This list provides
the mechanism for reading, writing, and updating information
to the database.  


@subssection(/**** USERS ****/)
@begin(verbatim)

get_all_logins
  Args: none
  Returns: {login, shell, last, first, middle, status, expdate, modtime}
  Acl: tbinfo.acl where tbinfo.table = "users", READ

get_user_by_login
  Args: (login(*))
  Returns: {login, shell, last, first, middle, status, mit_id, expdate,
	    modtime}
  Acl: tbinfo.acl where tbinfo.table = "users", READ
  Errors: SMS_NO_MATCH

get_user_by_firstname
  Args: (firstname(*))
  Returns: {login, shell, last, first, middle, status, mit_id, expdate,
	    modtime}
  Acl: tbinfo.acl where tbinfo.table = "users", READ
  Errors: SMS_NO_MATCH

get_user_by_lastname
  Args: (lastname(*))
  Returns: {login, shell, last, first, middle, status, mit_id, expdate,
	    modtime}
  Acl: tbinfo.acl where tbinfo.table = "users", READ
  Errors: SMS_NO_MATCH

get_user_by_first_and_last
  Args: (firstname(*), lastname(*))
  Returns: {login, shell, last, first, middle, status, mit_id, expdate,
	    modtime}
  Acl: tbinfo.acl where tbinfo.table = "users", READ
  Errors: SMS_NO_MATCH

update_user_shell
  Args: (login, shell)
  Returns: none
  Acl: users.acl where users.login = $1, WRITE
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE
  Integrity: application should check for valid shell program

update_user_status
  Args: (login, status)
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "users", WRITE (assume only a few
	administrative types will have WRITE permission for this table)
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE
  Integrity: application should check for valid status value

add_user
  Args: (login, shell, last, first, middle, status, expdate, *modtime)
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "users", APPEND (see note above)
  Errors: SMS_EXISTS, SMS_DATE
  Integrity: application should check for valid shell and status values;
	     modtime set by server.
  Side Effects: blank finger entry created

delete_user
  Args: (login)
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "users", DELETE (see note above)
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE
  Side Effects: finger entry deleted

Question: Is it correct for add_user and delete_user to add/delete finger
entries?  Or should there be add_finger and delete_finger?  Problem: want to
require that there be a finger entry if there is a user entry; can
application program be trusted to do this?  

Question: Should there be a separate year field in the users table that is
not modifiable by the user?  If so, should the mit_year field in the finger
program be renamed to mit_affiliation?

@end(verbatim)
@subsection(/**** FINGER ****/)
@begin(verbatim)

get_finger_by_login
  Args: (login(*))
  Returns: {login, fullname, nickname, home_addr, home_phone, office_addr,
	    office_phone, mit_dept, mit_year, modtime}
  Acl: tbinfo.acl where tbinfo.table = "finger", READ
  Errors: SMS_NO_MATCH

update_finger_by_login
  Args: (login, fullname, nickname, home_addr, home_phone, office_addr, 
	 office_phone, mit_dept, mit_year, *modtime)
  Returns: none
  Acl: users.acl where users.login = $1, WRITE				/****/
  Errors: SMS_NO_MATCH
  Integrity: modtime set by server, all other fields validated by 
	     application. (Perhaps there will be a table specifying valid
	     values for the mit_year field.)

?get_finger_by_{first(*),last(*),first+last}
@end(verbatim)
@subsection(/**** MACHINE ****/)
@begin(verbatim)

get_machine_by_name
  Args: (name(*))
  Returns: {name, type, model, status, serial, ethernet, sys_type}
  Acl: tbinfo.acl where tbinfo.table = "machine", READ
  Errors: SMS_NO_MATCH

add_machine
  Args: (name, type, model, status, serial, ethernet, sys_type)
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "machine", APPEND
  Errors: SMS_EXISTS
  Integrity: application must check for valid type, model, and status
	     fields.  (Perhaps there will be a database table specifying
	     valid values.)

update_machine
  Args: (name, type, model, status, serial, ethernet, sys_type)
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "machine", WRITE
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE
  Integrity: application must check for valid type, model, and status
	     fields. 

delete_machine
  Args: (name)
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "machine", DELETE
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE
@end(verbatim)

@subsection(/**** CLUSTER (cluster, alias) ****/)
@begin(verbatim)

get_cluster_info
  Args: (name(*))
  Returns: {name, desc, location}
  Acl: tbinfo.acl where tbinfo.table = "cluster", READ
  Errors: SMS_NO_MATCH

add_cluster
  Args: (name, desc, location)
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "cluster", APPEND
  Errors: SMS_EXISTS

update_cluster
  Args: (name, desc, location)
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "cluster", WRITE
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

delete_cluster
  Args: (name)
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "cluster", DELETE
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

get_machine_to_cluster_map
  Args: none
  Returns: {machine, cluster}
  Acl: tbinfo.acl where tbinfo.table = "alias-mach-clu-map", READ

add_machine_to_cluster
  Args: (machine, cluster)
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "alias-mach-clu-map", APPEND
  Errors: SMS_MACHINE, SMS_CLUSTER, SMS_EXISTS
  Integrity: machine and cluster must exist is machine and cluster tables.

delete_machine_from_cluster
  Args: (machine, cluster)
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "alia-mach-clu-map", DELETE
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@end(verbatim)
@subsection(/**** SVC (svc, alias:prcluster) ****/)
@begin(verbatim)

get_all_service_clusters
  Args: none
  Returns: {cluster, service-label, service-cluster}
  Acl: tbinfo.acl where tbinfo.table = "svc", READ

add_service_cluster
  Args: (cluster, service-label, service-cluster)
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "svc", APPEND
  Errors: SMS_CLUSTER, SMS_EXISTS
  Integrity: cluster must exist in cluster table; cluster/service-label must
	     be unique.

delete_service_cluster
  Args: (cluster, service-label)
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "svc", DELETE
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE
@end(verbatim)

@subsection(/**** SERVERS ****/)
@begin(verbatim)

get_all_sloc
  Args: none
  Returns: {service, machine}
  Acl: tbinfo.acl where tbinfo.table = "servers", READ

get_server_info
  Args: (service(*))
  Returns: {service, machine, update_interval, target_dir}
  Acl: tbinfo.acl where tbinfo.table = "servers", READ
  Errors: SMS_NO_MATCH

get_sloc_by_service
  Args: (service(*))
  Returns: {machine}
  Acl: tbinfo.acl where tbinfo.table = "servers", READ
  Errors: SMS_NO_MATCH

add_server
  Args: (service, machine, update_interval, target_dir)
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "servers", READ
  Errors: SMS_EXISTS, SMS_MACHINE
  Integrity: machine must exist in machine table; application should verify
	     that target_dir is a valid directory.

delete_server
  Args: (service, machine)
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "servers", READ
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

update_server
  Args: (service, machine, update_interval, target_dir)
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "servers", READ
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE
  Integrity: application should verify that target_dir is a valid directory.
@end(verbatim)

@subsection(/**** SERVICES (services, alias) ****/)
@begin(verbatim)

get_all_services
  Args: none
  Returns: {service, protocol, port, description}
  Acl: tbinfo.acl where tbinfo.table = "services", READ

add_service
  Args: (service, protocol, port, description)
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "service", APPEND
  Errors: SMS_EXISTS
  Integrity: application should validate all fields

delete_service
  Args: (service, protocol(*))
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "service", DELETE
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

update_service
  Args: (service, protocol, port, description)
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "service", WRITE
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

get_all_service_aliases
  Args: none
  Returns: {service-alias, service}
  Acl: tbinfo.acl where tbinfo.table = "alias-service", READ

add_service_alias
  Args: (service-alias, service)
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "alias-service", APPEND
  Errors: SMS_SERVICE, SMS_EXISTS
  Integrity: service must exist in service table

delete_service_alias
  Args: (service-alias, service)
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "alias-service", DELETE
  Errors: SMS_NO_MATCH
@end(verbatim)

@subsection(/**** FILESYS (filesys, alias) ****/)
@begin(verbatim)

get_all_filesys
  Args: none
  Returns: {label, type, machine, name, mount, access}
  Acl: tbinfo.acl where tbinfo.table = "filesys", READ

get_filesys_by_label
  Args: (label(*))
  Returns: {label, type, machine, name, mount, access}
  Acl: tbinfo.acl where tbinfo.table = "filesys", READ
  Errors: SMS_NO_MATCH

get_filesys_by_machine
  Args: (machine(*))
  Returns: {label, type, machine, name, mount, access}
  Acl: tbinfo.acl where tbinfo.table = "filesys", READ
  Errors: SMS_NO_MATCH

add_filesys

update_filesys

delete_filesys
@end(verbatim)

@subsection(/**** UFS ****/)
@begin(verbatim)

get_ufs

add_ufs

delete_ufs
@end(verbatim)

@subsection(/**** RVD (rvdsrv, rvdphys, rvdvirt) ****/)
@begin(verbatim)

get_rvd_physical
  Args: (machine)
  Returns: {device, size, created, modified}
  Acl: rvdsrv.acl where rvdsrv.machine = $1, READ 			/****/
  Errors: SMS_NO_MATCH  

get_rvd_virtual
  Args: (machine)
  Returns: {device, name, packid, owner, rocap, excap, shcap, modes, offset,
	    size, created, modified, ownhost}
  Acl: rvdsrv.acl where rvdsrv.machine = $1, READ			/****/
  Errors: SMS_NO_MATCH  

add_rvd_physical

delete_rvd_physical

add_rvd_virtual

update_rvd_virtual

delete_rvd_virtual
@end(verbatim)

subsection(/**** NFS ****/)
@begin(verbatim)

add_nfs_physical

delete_nfs_physical

@end(verbatim)

@subsection(/**** PRINTERS (printer, prability, pqm, queue, qdev,	****/)
/****           qdm, printcap, alias)			****/
@begin(verbatim)

get_all_printers

get_printer_info

add_printer

update_printer

delete_printer

add_printer_ability

delete_printer_ability

get_all_queues

get_queue_info

add_queue

update_queue

delete_queue

add_printer_to_queue

get_qdev

add_qdev

update_qdev

delete_qdev

add_queue_device_map

update_queue_device_map

delete_queue_device_map

get_all_printcap

get_printcap

add_printcap

update_printcap

delete_printcap

@end(verbatim)
@subsection(/**** POBOX ****/)
@begin(verbatim)

get_pobox

add_pobox

delete_pobox

@end(verbatim)
@subsection(/**** LISTS (list, members, strings) ****/)
@begin(verbatim)

get_list_info
  Args: (list_type(*), list_name(*))
  Returns: {list_name, list_type, description, expdate, modtime}
  Acl: tbinfo.acl where tbinfo.table = "list-all", READ
  Errors: SMS_NO_MATCH  

add_list
  Args: (list_type, list_name, description, expdate, *modtime)
  Returns: none
  Acl: tbinfo.acl where tbinfo.table = "list-$1", APPEND		/****/
  Errors: SMS_LIST_TYPE, SMS_DATE
  Integrity: list_type must be a known list type; expdate must be
	     reasonable; modtime set by server.

update_list

delete_list

add_member_to_list
  Args: (list_type, list_name, member_type, member_name, status)
  Returns: none
  Acl: [get_member_status(requestor): member.status & IS_ADMINISTRATOR] /****/
  Errors: SMS_LIST_TYPE, SMS_MEM_TYPE, SMS_NO_MATCH, SMS_NOT_UNIQUE
  Integrity: list_type and member_type must be known; list_name and
	     member_name must be unique; if member_type is "user" or "list",
	     then corresponding user or list must exist.
  Side Effects: if member_type = "string", entry added to strings table; 
		list modtime updated.

delete_member_from_list
  Args: (list_type, list_name, member_type, member_name)
  Returns: none
  Acl: [get_member_status(requestor): member.status & IS_ADMINISTRATOR] /****/
  Errors: SMS_LIST_TYPE, SMS_MEM_TYPE, SMS_NO_MATCH, SMS_NOT_UNIQUE
  Integrity: list_type and member_type must be known; list_name and
	     member_name must be unique.
  Side Effects: if member_type is "string", then corresponding string entry
		is deleted; list modtime updated.

update_member_status
  Args: (list_type, list_name, member_type, member_name, status)
  Returns: none
  Acl: [get_member_status(requestor): member.status & IS_ADMINISTRATOR] /****/
  Errors: SMS_LIST_TYPE, SMS_MEM_TYPE, SMS_NO_MATCH, SMS_NOT_UNIQUE
  Integrity: list_type and member_type must be known; list_name and
	     member_name must be unique.
  Side Effects: list modtime updated.

NOTE: Administrator level determines maximum status value that can be set.
      Specifically, NORMAL_ADMIN can set regular protection bits (R,W,D,A);
		    MASTER_ADMIN can also set NORMAL_ADMIN bit;
		    SUPER_ADMIN can also set MASTER_ADMIN

get_members_of_list
  Args: (list_type, list_name)
  Returns: {member_type, member_name, status}
  Acl: tbinfo.acl where tbinfo.table = list-$1, READ AND		/****/
       NOT (list.flags & L_HIDDEN)
  Errors: SMS_LIST_TYPE, SMS_NO_MATCH, SMS_NOT_UNIQUE

@end(verbatim)
@subsection(/**** TBINFO ****/)
@begin(verbatim)

get_all_table_info
  Args: none
  Returns: {table, subtable, value, acl}
  Acl: tbinfo.acl where tbinfo.table = "tbinfo", READ

get_table_info
  Args: (table(*))
  Returns: {table, subtable, value, acl}
  Acl: tbinfo.acl where tbinfo.table = "tbinfo", READ
  Errors: SMS_NO_MATCH

add_table_info

update_table_info

delete_table_info

@end(verbatim)
@subsection(/**** END ****/)
@begin(verbatim)

General errors:

SMS_NO_PERM - permission denied

Query specific errors:

SMS_NO_MATCH - no records match query arguments
SMS_NOT_UNIQUE - argument not unique as required
SMS_LIST_TYPE - unknown list type
SMS_MEM_TYPE - unknown member type
SMS_DATE - invalid date

@end(verbatim)

