@part[queries, root="sms.mss"]
@NewPage()

@MajorSection(Predefined Queries - List of Database Interfaces)
@Label(Queries)

All access to the database is provided through the application
library/database server interface.  This interface provides a limited set of
predefined, named queries, which allows for tightly controlled access to
database information.  Queries fall into four classes: retrieve, update,
delete, and append.  An attempt has been made to define a set of queries
that provide sufficient flexibility to meet all of the needs of the Data
Control Manager and each of the indivual application programs.  However,
since the database can be modified and extended in the future, the server
and application library have been designed to allow for the easy addtion of
queries.  

Providing a generallized layer of functions affords SMS the capability
of being database independent.  Today, we are using INGRES; however,
in the future, if a different database is required, the application
interface will not change.  The only change needed at that point will
be a new SMS server, linking the pre-defined queries to a new
set of data manipulation procedures.  

The following list of queries are a predefined list.  This list provides
the mechanism for reading, writing, updating, and deleting information
in the database.

In each query description below there are descriptions of the required
arguments, the return values, integrity constraints, possible error codes,
and side effects, if any.  In addition to the error codes specifically
listed for each query, the following two error codes may be returned by any
query: SMS_SUCCESS for successful completion of the query, and SMS_PERM
indicating that permission was denied for the query.

@Begin(Verbatim, Size -1)

@SubSection(Users)
@label(Users)

@B(get_all_logins)
  Args: none
  Returns: {login, uid, shell, home, last, first, middle}

@B(get_all_active_logins)
  Args: none
  Returns: {login, uid, shell, home, last, first, middle}

@B(get_all_active_users)
  Args: none
  Returns: {login, uid}

@B(get_user_by_login)
  Args: (login(*))
  Returns: {login, uid, shell, home, last, first, middle, status, 
	    mit_id, mit_year, expdate, modtime}
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(get_user_by_firstname)
  Args: (firstname(*))
  Returns: {login, uid, shell, home, last, first, middle, status, 
	    mit_id, mit_year, expdate, modtime}
  Errors: SMS_NO_MATCH

@B(get_user_by_lastname)
  Args: (lastname(*))
  Returns: {login, uid, shell, home, last, first, middle, status, 
	    mit_id, mit_year, expdate, modtime}
  Errors: SMS_NO_MATCH

@B(get_user_by_first_and_last)
  Args: (firstname(*), lastname(*))
  Returns: {login, uid, shell, home, last, first, middle, status, 
	    mit_id, mit_year, expdate, modtime}
  Errors: SMS_NO_MATCH

@B(get_user_by_mitid)
  Args: (mit_id)
  Returns: {login, uid, shell, home, last, first, middle, status,
	    mit_id, mit_year, expdate, modtime}

@B(get_user_by_year)
  Args: (year)
  Returns: {login, uid, shell, home, last, first, middle, status, 
	    mit_id, mit_year, expdate, modtime}
  Errors: SMS_NO_MATCH

@B(update_user_shell)
  Args: (login, shell)
  Returns: none
  Integrity: application should check for valid shell program
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(update_user_status)
  Args: (login, status)
  Returns: none
  Integrity: application should check for valid status value
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(update_user_home)
  Args: (login, home)
  Returns: none
  Integrity: home must be a known filesys entry
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE, SMS_FILESYS

@B(add_user)
  Args: (login, uid, shell, home, last, first, middle, status, 
	 mit_id, mit_year, expdate)
  Returns: none
  Integrity: application must check for valid shell and status
	     values; home must be a valid filesys entry; expdate must
	     be reasonable; modtime is set by the server.
  Errors: SMS_EXISTS, SMS_FILESYS, SMS_DATE
  Side Effects: blank finger entry created

@B(update_user)
  Args: (login, newlogin, uid, shell, home, last, first, middle, 
	 status, mit_id, mit_year, expdate)
  Returns: none
  Integrity: application must check for valid shell and status
	     values; home must be a valid filesys entry; expdate must
	     be reasonable; modtime is set by the server.
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE, SMS_FILESYS, SMS_DATE

@B(delete_user)
  Args: (login)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE
  Side Effects: finger entry deleted

@SubSection(Finger)
@label(Finger)

@B(get_finger_by_login)
  Args: (login(*))
  Returns: {login, fullname, nickname, home_addr, home_phone,
	    office_addr, office_phone, mit_dept, mit_year, modtime}
  Errors: SMS_NO_MATCH, SMS_USER

@B(get_finger_by_first_last)
  Args: (first(*), last(*))
  Returns: {login, fullname, nickname, home_addr, home_phone, 
	    office_addr, office_phone, mit_dept, mit_year, modtime}
  Errors: SMS_NO_MATCH

@B(update_finger_by_login)
  Args: (login, fullname, nickname, home_addr, home_phone, 
	 office_addr, office_phone, mit_dept, mit_year, *modtime)
  Returns: none
  Integrity: modtime set by server, all other fields validated by
	     application. (Perhaps there will be a table specifying
	     valid values for the mit_year field.)
  Errors: SMS_NO_MATCH, SMS_USER

@SubSection(Machine)
@label(Machine)

@B(get_machine_by_name)
  Args: (name(*))
  Returns: {name, type, model, status, serial, sys_type}
  Errors: SMS_NO_MATCH

@B(add_machine)
  Args: (name, type, model, status, serial, sys_type)
  Returns: none
  Integrity: type, model, and sys_type are checked against valid
	     values in the database
  Errors: SMS_EXISTS, SMS_TYPE

@B(update_machine)
  Args: (name, newname, type, model, status, serial, sys_type)
  Returns: none
  Integrity: type, model, and sys_type are checked against valid
	     values in the database
  Errors: SMS_MACHINE, SMS_TYPE

@B(delete_machine)
  Args: (name)
  Returns: none
  Errors: SMS_MACHINE

@SubSection(Cluster)
@label(Cluster)

@B(get_cluster_info)
  Args: (name(*))
  Returns: {name, desc, location}
  Errors: SMS_NO_MATCH

@B(add_cluster)
  Args: (name, desc, location)
  Returns: none
  Errors: SMS_EXISTS

@B(update_cluster)
  Args: (name, newname, desc, location)
  Returns: none
  Errors: SMS_CLUSTER

@B(delete_cluster)
  Args: (name)
  Returns: none
  Errors: SMS_CLUSTER

@B(get_machine_to_cluster_map)
  Args: none
  Returns: {machine, cluster}

@B(add_machine_to_cluster)
  Args: (machine, cluster)
  Returns: none
  Integrity: machine and cluster must exist is machine and cluster
	     tables.
  Errors: SMS_MACHINE, SMS_CLUSTER, SMS_EXISTS

@B(delete_machine_from_cluster)
  Args: (machine, cluster)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@SubSection(Service Clusters)
@label(Svc)

@B(get_all_service_clusters)
  Args: none
  Returns: {cluster, service-label, service-cluster}

@B(add_service_cluster)
  Args: (cluster, service-label, service-cluster)
  Returns: none
  Integrity: cluster must exist in cluster table;
	     cluster/service-label must be unique.
  Errors: SMS_CLUSTER, SMS_EXISTS

@B(update_service_cluster)
  Args: (cluster, service-label, service-cluster)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(delete_service_cluster)
  Args: (cluster, service-label)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@SubSection(Printer Clusters)
@Label(Prclusters)

@B(get_all_printer_clusters)
  Args: none
  Returns: {prcluster}

@B(get_printers_of_cluster)
  Args: (prcluster)
  Returns: {printer, queue, machine, printer-type, abilities, 
	    default}
  Errors: SMS_CLUSTER

@SubSection(Servers)
@label(Servers)

@B(get_server_info)
  Args: (service(*))
  Returns: {service, update_int, target_file, script, dfgen}
  Errors: SMS_NO_MATCH

@B(add_server_info)
  Args: (service, update_int, target_file, script, dfgen)
  Returns: none
  Integrity: application must verify that target_dir and script
	     exist; dfgen must be a valid date
  Errors: SMS_EXISTS, SMS_DATE

@B(update_server_info)
  Args: (service, update_int, target_file, script, dfgen)
  Returns: none
  Integrity: application must verify that target_dir and script
	     exist; dfgen must be a valid date
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE, SMS_DATE

@B(delete_server_info)
  Args: (service)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(get_server_host_info)
  Args: (service(*), machine(*))
  Returns: {service, machine, enable, overide, ltt, success, value1, 
	    value2}
  Errors: SMS_NO_MATCH

@B(add_server_host)
  Args: (service, machine, enable, overide, ltt, success, value1, 
	 value2)
  Returns: none
  Integrity: machine must exist; last must be a valid date or null.
  Errors: SMS_EXISTS, SMS_MACHINE, SMS_DATE

@B(update_server_host)
  Args: (service, machine, enable, overide, ltt, success, value1, 
	 value2)
  Returns: none
  Integrity: last must be a valid date or null.
  Errors: SMS_MACHINE, SMS_NO_MATCH, SMS_NOT_UNIQUE, SMS_DATE

@B(delete_server_host)
  Args: (service, machine)
  Returns: none
  Errors: SMS_MACHINE, SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(get_server_locations)
  Args: (service)
  Returns: {service, machine}
  Errors: SMS_SERVICE

@SubSection(Services)
@label(Services)

@B(get_all_services)
  Args: none
  Returns: {service, protocol, port, description}

@B(add_service)
  Args: (service, protocol, port, description)
  Returns: none
  Integrity: application should validate all fields
  Errors: SMS_EXISTS

@B(delete_service)
  Args: (service, protocol(*))
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(update_service)
  Args: (service, protocol, port, description)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@SubSection(File Systems)
@label(Filesys)

@B(get_all_filesys)
  Args: none
  Returns: {label, type, machine, name, mount, access}

@B(get_filesys_by_label)
  Args: (label(*))
  Returns: {label, type, machine, name, mount, access}
  Errors: SMS_NO_MATCH

@B(get_filesys_by_machine)
  Args: (machine(*))
  Returns: {label, type, machine, name, mount, access}
  Errors: SMS_MACHINE, SMS_NO_MATCH

@B(add_filesys)
  Args: (label, type, machine, name, mount, access)
  Returns: none
  Integrity: type must be a known type; machine must exist in machine
             table.
  Errors: SMS_EXISTS, SMS_TYPE, SMS_MACHINE

@B(update_filesys)
  Args: (label, type, machine, name, mount, access)
  Returns: none
  Integrity: type must be a known type; machine must exist in machine
             table.
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE, SMS_TYPE, SMS_MACHINE

@B(delete_filesys)
  Args: (label)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@SubSection(RVD)
@label(Rvdsrv)
@label(Rvdphys)
@label(Rvdvirt)

@B(get_rvd_server)
  Args: (machine)
  Returns: {machine, oper_acl, admin_acl, shutdown_acl}
  Errors: SMS_MACHINE, SMS_NO_MATCH

@B(add_rvd_server)
  Args: (machine, oper_acl, admin_acl, shutdown_acl)
  Returns: none
  Integrity: oper, admin, and shutdown must be valid list names.
  Errors: SMS_EXISTS, SMS_MACHINE, SMS_LIST

@B(update_rvd_server)
  Args: (machine, oper_acl, admin_acl, shutdown_acl)
  Returns: none
  Integrity: oper, admin, and shutdown must be valid list names.
  Errors: SMS_LIST

@B(delete_rvd_server)
  Args: (machine)
  Returns: none
  Errors: SMS_MACHINE, SMS_NO_MATCH
  Side Effects: deletes all rvd_physical and rvd_virtual entries 
		associated with the server.

@B(get_all_rvd_physical)
  Args: (machine)
  Returns: {device, size, created, modified}
  Errors: SMS_MACHINE, SMS_NO_MATCH

@B(get_rvd_physical)
  Args: (machine, device)
  Returns: {size, created, modified}
  Errors: SMS_MACHINE, SMS_NO_MATCH  

@B(add_rvd_physical)
  Args: (machine, device, size, created, modified)
  Returns: none
  Integrity: machine must exist in machine table
  Errors: SMS_MACHINE, SMS_EXISTS

@B(delete_rvd_physical)
  Args: (machine, device)
  Returns: none
  Errors: SMS_MACHINE, SMS_NO_MATCH

@B(get_all_rvd_virtual)
  Args: (machine)
  Returns: {name, device, packid, owner, rocap, excap, shcap, modes, 
	    offset, size, created, modified, ownhost}
  Errors: SMS_MACHINE, SMS_NO_MATCH

@B(get_rvd_virtual)
  Args: (machine, name)
  Returns: {device, packid, owner, rocap, excap, shcap, modes, 
	    offset, size, created, modified, ownhost}
  Errors: SMS_NO_MATCH  

@B(add_rvd_virtual)
  Args: (machine, name, device, packid, owner, rocap, excap, shcap, 
	 modes, offset, size, created, modified, ownhost)
  Returns: none
  Integrity: machine, ownhost must exist in machine table;
             machine/device must exist in rvdphys table.
  Errors: SMS_EXISTS, SMS_MACHINE, SMS_DEVICE

@B(update_rvd_virtual)
  Args: (machine, name, newname, device, packid, owner, rocap, excap, 
	 shcap, modes, offset, size, created, modified, ownhost)
  Returns: none
  Integrity: ownhost must exist in machine table.
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE, SMS_MACHINE

@B(delete_rvd_virtual)
  Args: (machine, device, name)
  Returns: none
  Errors: SMS_MACHINE, SMS_NO_MATCH, SMS_NOT_UNIQUE

@SubSection(NFS)
@label(Nfsphys)
@label(Nfsquota)

@B(get_all_nfsphys)
  Args: (machine)
  Returns: {device, dir, status, allocated, size}
  Errors: SMS_MACHINE, SMS_NO_MATCH

@B(get_nfsphys)
  Args: (machine, device)
  Returns: {dir, status, allocated, size}
  Errors: SMS_MACHINE, SMS_NO_MATCH

@B(add_nfsphys)
  Args: (machine, device, dir, status, allocated, size)
  Returns: none
  Errors: SMS_MACHINE, SMS_EXISTS

@B(delete_nfsphys)
  Args: (machine, device)
  Returns: none
  Errors: SMS_MACHINE, SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(get_nfs_quotas)
  Args: (machine, device)
  Returns: {login, quota}
  Errors: SMS_NO_MATCH, SMS_MACHINE

@B(get_nfs_quotas_by_user)
  Args: (login)
  Returns: {machine, device, quota}
  Errors: SMS_USER

@B(add_nfs_quota)
  Args: (machine, device, login, quota)
  Returns: none
  Integrity: machine must exist, user must exist
  Errors: SMS_EXISTS, SMS_MACHINE, SMS_USER

@B(update_nfs_quota)
  Args: (machine, device, login, quota)
  Returns: none
  Integrity: machine must exist, user must exist
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE, SMS_MACHINE, SMS_USER

@B(delete_nfs_quota)
  Args: (machine, device, login)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE, SMS_MACHINE, SMS_USER

@SubSection(Printers)
@label(Printer)
@label(Prability)
@label(Queue)
@label(Pqm)
@label(Qdev)
@label(Qdm)
@label(Printcap)

@B(get_all_printers)
  Args: none
  Returns: {printer, type, desc}

@B(get_printer_info)
  Args: (printer)
  Returns: {printer, type, desc}
  Errors: SMS_NO_MATCH

@B(add_printer)
  Args: (printer, type, desc)
  Returns: none
  Integrity: type must be known
  Errors: SMS_EXISTS

@B(update_printer)
  Args: (printer, type, desc)
  Returns: none
  Integrity: type must be known
  Errors: SMS_PRINTER, SMS_NO_MATCH

@B(delete_printer)
  Args: (printer)
  Returns: none
  Errors: SMS_PRINTER, SMS_NO_MATCH

@B(get_all_queues)
  Args: none
  Returns: {queue, machine, abilities, default, status}

@B(get_queue_info)
  Args: (queue)
  Returns: {queue, machine, abilities, default, status}
  Errors: SMS_QUEUE, SMS_NO_MATCH

@B(add_queue)
  Args: (queue, machine, abilities, default, status)
  Returns: none
  Integrity: machine must exist; abilty is validated by application
  Errors: SMS_EXISTS, SMS_MACHINE

@B(update_queue)
  Args: (queue, machine, abilities, default, status)
  Returns: none
  Integrity: machine must exist; abilty is validated by application
  Errors: SMS_QUEUE, SMS_NO_MATCH, SMS_MACHINE

@B(delete_queue)
  Args: (queue)
  Returns: none
  Errors: SMS_QUEUE, SMS_NO_MATCH

@B(add_printer_to_queue)
  Args: (printer, queue)
  Returns: none
  Integrity: printer and queue must exist
  Errors: SMS_PRINTER, SMS_QUEUE

@B(delete_printer_from_queue)
  Args: (printer, queue)
  Returns: none
  Errors: SMS_PRINTER, SMS_QUEUE

@B(get_qdev)
  Args: (machine)
  Returns: {device, physical, machine, status}
  Errors: SMS_MACHINE

@B(add_qdev)
  Args: (device, physical, machine, status)
  Returns: none
  Integrity: machine must exist; application must verify that
	     physical device exists.
  Errors: SMS_EXISTS, SMS_MACHINE

@B(update_qdev)
  Args: (device, physical, machine, status)
  Returns: none
  Integrity: only status may be updated
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(delete_qdev)
  Args: (device, machine)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(get_queue_device_map)
  Args: (machine)
  Returns: {queue, device, machine, server}
  Errors: SMS_MACHINE

@B(add_queue_device_map)
  Args: (queue, device, machine, server)
  Returns: none
  Integrity: queue, device, machine must exist; application must
             verify that server program exists.
  Errors: SMS_QUEUE, SMS_DEVICE, SMS_MACHINE

@B(update_queue_device_map)
  Args: (queue, device, machine, server)
  Returns: none
  Integrity: only server may be updated; application must verify
             existence of server program.
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(delete_queue_device_map)
  Args: (queue, device, machine)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@SubSection(Post Office Boxes)
@label(Pobox)

@B(get_pobox)
  Args: (login)
  Returns: {login, type, machine, box}
  Errors: SMS_USER, SMS_NO_MATCH

@B(add_pobox)
  Args: (login, type, machine, box)
  Returns: none
  Integrity: type, user, machine must exist
  Errors: SMS_EXISTS, SMS_TYPE, SMS_USER, SMS_MACHINE

@B(delete_pobox)
  Args: (login, type, machine, box)
  Returns: none
  Errors: SMS_USER, SMS_MACHINE, SMS_NO_MATCH

@SubSection(Lists)
@label(List)
@label(Members)
@label(Strings)

@B(get_list_info)
  Args: (list_name)
  Returns: {list_name, description, flags, admin_acl, expdate, 
	    modtime}
  Errors: SMS_LIST

@B(add_list)
  Args: (list_name, flags, admin_acl, description, expdate)
  Returns: none
  Integrity: expdate must be reasonable; application is responsible
             for
	     flags; modtime set by server.
  Errors: SMS_EXISTS, SMS_ACL, SMS_DATE

@B(update_list)
  Args: (list_name, flags, admin_acl, description, expdate)
  Returns: none
  Integrity: expdate must be reasonable; application is responsible
             for flags.
  Errors: SMS_LIST, SMS_ACL, SMS_DATE

@B(update_list_admin)
  Args: (list_name, admin_acl)
  Returns: none
  Integrity: admin_acl must be a known list.
  Errors: SMS_LIST, SMS_ACL

@B(delete_list)
  Args: (list_name)
  Returns: none
  Errors: SMS_LIST
  Side Effects: All members of list are deleted.

@B(add_member_to_list)
  Args: (list_name, member_type, member_name)
  Returns: none
  Integrity: member_type must be known; list_name and member_name
	     must be unique; if member_type is "user" or "list",
	     then corresponding user or list must exist.
  Side Effects: if member_type = "string", entry added to strings
		table; list modtime updated.
  Errors: SMS_EXISTS, SMS_TYPE, SMS_LIST, SMS_USER, SMS_NO_MATCH, 
	  SMS_NOT_UNIQUE

@B(delete_member_from_list)
  Args: (list_name, member_type, member_name)
  Returns: none
  Integrity: member_type must be known; list_name and member_name
	     must be unique.
  Side Effects: if member_type is "string", then corresponding string
		entry is deleted; list modtime updated.
  Errors: SMS_TYPE, SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(get_members_of_list)
  Args: (list_name)
  Returns: {member_type, member_name}
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@Subheading(Mailing Lists)
@label(Maillists)

@B(get_all_maillists)
  Args: none
  Returns: {list}

@B(get_all_visible_maillists)
  Args: none
  Returns: {list}

@B(add_maillist)
  Args: (list)
  Returns: none
  Errors: SMS_EXISTS, SMS_LIST

@B(delete_maillist)
  Args: (list)
  Returns: none
  Errors: SMS_LIST, SMS_NO_MATCH

@SubHeading(Groups)
@label(Groups)

@B(get_all_groups)
  Args: none
  Returns: {list}

@B(add_group)
  Args: (list)
  Returns: none
  Integrity: list must exist
  Errors: SMS_EXISTS, SMS_LIST

@B(add_user_group)
  Args: (login)
  Returns: none
  Errors: SMS_USER, SMS_LIST
  Description: Optimized query for creating a user group (list) and
               adding the user as a member.  Returns SMS_LIST if a
               list already exists with the user's name.

@B(delete_group)
  Args: (list)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_LIST

@SubHeading(Access Control Lists)
@label(Acls)
@label(Capacls)

@B(get_acls)
  Args: (service)
  Returns: {list}
  Errors: SMS_SERVICE

@B(add_acls)
  Args: (service, list)
  Returns: none
  Errors: SMS_EXISTS, SMS_SERVICE, SMS_LIST

@B(delete_acls)
  Args: (service, list)
  Returns: none
  Errors: SMS_SERVICE, SMS_LIST, SMS_NO_MATCH

@SubSection(Aliases)
@label(Alias)

@B(get_alias)
  Args: (name(*), type)
  Returns: {name, type, trans}
  Errors: SMS_TYPE, SMS_NO_MATCH

@B(add_alias)
  Args: (name, type, trans)
  Returns: none
  Integrity: type must exist as a translation of 
	     get_alias("alias", "TYPE").
  Errors: SMS_EXISTS, SMS_TYPE

@B(delete_alias)
  Args: (name, type)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@SubSection(Values)
@label(Values)
@Begin(Text, Below 1.5, Above 1)
This section contains values that are needed by the server or application
programs for updating the database.  Some examples are:
@Begin(Itemize)

next users_id

next list_id

default user disk quota

@End(Itemize)
@End(Text)

@B(get_value)
  Args: (name)
  Returns: {value}
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(add_value)
  Args: (name, value)
  Returns: none
  Errors: SMS_EXISTS

@B(update_value)
  Args: (name, value)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(delete_value)
  Args: (name)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@SubSection(Table Statistics)
@Label(Tblstats)

@B(get_all_table_stats)
  Args: none
  Returns: {table, retrieves, appends, updates, deletes, modtime}

@B(qet_query_need)
  Args: (query, last_get_time)
  Returns: {true | false}

@End(Verbatim)

@Section(Errors)

@Begin(Format)

General errors (may be returned by all queries):

SMS_SUCCESS - Query completed successfully
SMS_PERM - Insufficient permission to perform requested database access

Query specific errors:

SMS_ACL - No such access control list
SMS_ARGS - Insufficient number of arguments
SMS_CLUSTER - Unknown cluster
SMS_DATE - Invalid date
SMS_DEVICE - No such device
SMS_EXISTS - Record already exists
SMS_FILESYS - Named file system does not exist
SMS_FILESYS_ACCESS - invalid filesys access
SMS_FILESYS_EXISTS - Named file system already exists
SMS_LIST - No such list
SMS_MACHINE - Unknown machine
SMS_NFS - specified directory not exported
SMS_NFSPHYS - Machine/device pair not in nfsphys
SMS_NOT_UNIQUE - Arguments not unique
SMS_PRINTER - Unknown printer
SMS_QUEUE - Unknown queue
SMS_RVD - no such rvd
SMS_SERVICE - Unknown service
SMS_STRING - Unknown string
SMS_TYPE - Invalid type
SMS_USER - No such user

@End(Format)


