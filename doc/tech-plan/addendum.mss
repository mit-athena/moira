@part[addendum, root="sms.mss"]
@NewPage()
@MajorSection[Addendum]

The following information supplements the formal text.  In general the
addendum will provide critical information regarding programming application
and use.

@Section(Predefined Queries - List of Database Interfaces)
@Label(Queries)

The following list of queries are a predefined list.  This list provides
the mechanism for reading, writing, updating, and deleting information
in the database.

In each query description below there are descriptions of the required
arguments, the return values, integrity constraints, possible error codes,
and side effects, if any.  In addition to the error codes specifically
listed for each query, the following two error codes may be returned by any
query: SMS_SUCCESS for successful completion of the query, and SMS_PERM
indicating that permission was denied for the query.

@Begin(Verbatim)

@SubSection(Users)
@label(Users)

@B(get_all_logins)
  Args: none
  Returns: {login, shell, home, last, first, middle, status, expdate, modtime}

@B(get_user_by_login)
  Args: (login(*))
  Returns: {login, shell, home, last, first, middle, status, mit_id, expdate,
	    modtime}
  Errors: SMS_NO_MATCH

@B(get_user_by_firstname)
  Args: (firstname(*))
  Returns: {login, shell, home, last, first, middle, status, mit_id, expdate,
	    modtime}
  Errors: SMS_NO_MATCH

@B(get_user_by_lastname)
  Args: (lastname(*))
  Returns: {login, shell, home, last, first, middle, status, mit_id, expdate,
	    modtime}
  Errors: SMS_NO_MATCH

@B(get_user_by_first_and_last)
  Args: (firstname(*), lastname(*))
  Returns: {login, shell, home, last, first, middle, status, mit_id, expdate,
	    modtime}
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
  Args: (login, shell, home, last, first, middle, status, mit_id, expdate)
  Returns: none
  Integrity: application must check for valid shell and status values; home
	     must be a valid filesys entry; expdate must be reasonable;
	     modtime is set by the server.
  Errors: SMS_EXISTS, SMS_FILESYS, SMS_DATE
  Side Effects: blank finger entry created

@B(update_user)
  Args: (login, shell, home, last, first, middle, status, mit_id, expdate)
  Returns: none
  Integrity: application must check for valid shell and status values; home
	     must be a valid filesys entry; expdate must be reasonable;
	     modtime is set by the server.
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
  Returns: {login, fullname, nickname, home_addr, home_phone, office_addr,
	    office_phone, mit_dept, mit_year, modtime}
  Errors: SMS_NO_MATCH

@B(get_finger_by_first_last)
  Args: (first(*), last(*))
  Returns: {login, fullname, nickname, home_addr, home_phone, office_addr,
	    office_phone, mit_dept, mit_year, modtime}
  Errors: SMS_NO_MATCH

@B(update_finger_by_login)
  Args: (login, fullname, nickname, home_addr, home_phone, office_addr, 
	 office_phone, mit_dept, mit_year, *modtime)
  Returns: none
  Integrity: modtime set by server, all other fields validated by 
	     application. (Perhaps there will be a table specifying valid
	     values for the mit_year field.)
  Errors: SMS_NO_MATCH

@SubSection(Machine)
@label(Machine)

@B(get_machine_by_name)
  Args: (name(*))
  Returns: {name, type, model, status, serial, ethernet, sys_type}
  Errors: SMS_NO_MATCH

@B(add_machine)
  Args: (name, type, model, status, serial, ethernet, sys_type)
  Returns: none
  Integrity: application must check for valid type, model, and status
	     fields.  (Perhaps there will be a database table specifying
	     valid values.)
  Errors: SMS_EXISTS

@B(update_machine)
  Args: (name, type, model, status, serial, ethernet, sys_type)
  Returns: none
  Integrity: application must check for valid type, model, and status
	     fields. 
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(delete_machine)
  Args: (name)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

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
  Args: (name, desc, location)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(delete_cluster)
  Args: (name)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(get_machine_to_cluster_map)
  Args: none
  Returns: {machine, cluster}

@B(add_machine_to_cluster)
  Args: (machine, cluster)
  Returns: none
  Integrity: machine and cluster must exist is machine and cluster tables.
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
  Integrity: cluster must exist in cluster table; cluster/service-label must
	     be unique.
  Errors: SMS_CLUSTER, SMS_EXISTS

@B(update_service_cluster)
  Args: (cluster, service-label, service-cluster)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(delete_service_cluster)
  Args: (cluster, service-label)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@SubSection(Servers)
@label(Servers)

@B(get_server_info)
  Args: (service(*))
  Returns: {service, update_int, target_dir, script, dfgen, value}
  Errors: SMS_NO_MATCH

@B(add_server_info)
  Args: (service, update_int, target_dir, script, dfgen, value)
  Returns: none
  Integrity: application must verify that target_dir and script exist;
	     dfgen must be a valid date; value is server/application
	     specific data.
  Errors: SMS_EXISTS, SMS_DATE

@B(update_server_info)
  Args: (service, update_int, target_dir, script, dfgen, value)
  Returns: none
  Integrity: application must verify that target_dir and script exist;
	     dfgen must be a valid date; value is server/application
	     specific data.
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE, SMS_DATE

@B(delete_server_info)
  Args: (service)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(get_all_server_hosts)
  Args: none
  Returns: {service, machine, enable, overide, last}

@B(get_server_hosts)
  Args: (service(*))
  Returns: {service, machine, enable, overide, last}
  Errors: SMS_NO_MATCH

@B(add_server_host)
  Args: (service, machine, enable, overide, last)
  Returns: none
  Integrity: machine must exist; last must be a valid date or null.
  Errors: SMS_EXISTS, SMS_MACHINE, SMS_DATE

@B(update_server_host)
  Args: (service, machine, enable, overide, last)
  Returns: none
  Integrity: last must be a valid date or null.
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE, SMS_DATE

@B(delete_server_host)
  Args: (service, machine)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

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
  Errors: SMS_NO_MATCH

@B(add_filesys)
  Args: (label, type, machine, name, mount, access)
  Returns: none
  Integrity: type must be a known type; machine must exist in machine table
  Errors: SMS_EXISTS, SMS_TYPE, SMS_MACHINE

@B(update_filesys)
  Args: (label, type, machine, name, mount, access)
  Returns: none
  Integrity: type must be a known type; machine must exist in machine table
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE, SMS_TYPE, SMS_MACHINE

@B(delete_filesys)
  Args: (label)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@SubSection(RVD)
@label(Rvdsrv)
@label(Rvdphys)
@label(Rvdvirt)

@B(get_rvd_physical)
  Args: (machine)
  Returns: {device, size, created, modified}
  Errors: SMS_NO_MATCH  

@B(add_rvd_physical)
  Args: (machine, device, size, created, modified)
  Returns: none
  Integrity: machine must exist in machine table
  Errors: SMS_EXISTS, SMS_MACHINE

@B(delete_rvd_physical)
  Args: (machine, device)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(get_rvd_virtual)
  Args: (machine)
  Returns: {device, name, packid, owner, rocap, excap, shcap, modes, offset,
	    size, created, modified, ownhost}
  Errors: SMS_NO_MATCH  

@B(add_rvd_virtual)
  Args: (machine, device, name, packid, owner, rocap, excap, shcap, modes,
	 offset, size, created, modified, ownhost)
  Returns: none
  Integrity: machine, ownhost must exist in machine table; machine/device
	     must exist in rvdphys table.
  Errors: SMS_EXISTS, SMS_MACHINE, SMS_DEVICE

@B(update_rvd_virtual)
  Args: (machine, device, name, packid, owner, rocap, excap, shcap, modes,
	 offset, size, created, modified, ownhost)
  Returns: none
  Integrity: ownhost must exist in machine table.
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE, SMS_MACHINE

@B(delete_rvd_virtual)
  Args: (machine, device, name)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@SubSection(NFS)
@label(Nfsphys)
@label(Nfsquota)

@B(get_nfs_physical)
  Args: (machine)
  Returns: {device}
  Errors: SMS_NO_MATCH

@B(add_nfs_physical)
  Args: (machine, device)
  Returns: none
  Integrity: machine must exist
  Errors: SMS_EXISTS, SMS_MACHINE

@B(delete_nfs_physical)
  Args: (machine, device)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(get_nfs_quota)
  Args: (machine, login)
  Returns: {machine, login, quota}
  Errors: SMS_NO_MATCH, SMS_MACHINE, SMS_USER

@B(add_nfs_quota)
  Args: (machine, login, quota)
  Returns: none
  Integrity: machine must exist, user must exist
  Errors: SMS_EXISTS, SMS_MACHINE, SMS_USER

@B(update_nfs_quota)
  Args: (machine, login, quota)
  Returns: none
  Integrity: machine must exist, user must exist
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE, SMS_MACHINE, SMS_USER

@B(delete_nfs_quota)
  Args: (machine, login)
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
  Returns: {printer, type, desc, machine}

@B(get_printer_info)
  Args: (printer)
  Returns: {printer, type, desc, machine}
  Errors: SMS_NO_MATCH

@B(add_printer)
  Args: (printer, type, desc, machine)
  Returns: none
  Integrity: machine must exist, type is validated by appliation
  Errors: SMS_EXISTS, SMS_MACHINE

@B(update_printer)
  Args: (printer, type, desc, machine)
  Returns: none
  Integrity: machine must exist, type is validated by appliation
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE, SMS_MACHINE

@B(delete_printer)
  Args: (printer)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(get_printer_ability)
  Args: (printer)
  Returns: {printer, ability}
  Errors: SMS_NO_MATCH

@B(add_printer_ability)
  Args: (printer, ability)
  Returns: none
  Integrity: printer must exist
  Errors: SMS_EXISTS, SMS_PRINTER

@B(delete_printer_ability)
  Args: (printer, ability)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(get_all_queues)
  Args: none
  Returns: {queue, machine, ability, status}

@B(get_queue_info)
  Args: (queue)
  Returns: {queue, machine, ability, status}
  Errors: SMS_NO_MATCH

@B(add_queue)
  Args: (queue, machine, ability, status)
  Returns: none
  Integrity: machine must exist; abilty  is validated by application
  Errors: SMS_EXISTS, SMS_MACHINE

@B(update_queue)
  Args: (queue, machine, ability, status)
  Returns: none
  Integrity: machine must exist; abilty  is validated by application
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE, SMS_MACHINE

@B(delete_queue)
  Args: (queue)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(add_printer_to_queue)
  Args: (printer, queue)
  Returns: none
  Integrity: printer and queue must exist
  Errors: SMS_PRINTER, SMS_QUEUE

@B(get_qdev)
  Args: (machine)
  Returns: {device, physical, machine, status}
  Errors: SMS_MACHINE

@B(add_qdev)
  Args: (device, physical, machine, status)
  Returns: none
  Integrity: machine must exist; application must verify that physical 
	     device exists.
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
  Integrity: queue, device, machine must exist; application must verify that
	     server program exists.
  Errors: SMS_QUEUE, SMS_DEVICE, SMS_MACHINE

@B(update_queue_device_map)
  Args: (queue, device, machine, server)
  Returns: none
  Integrity: only server may be updated; application must verify existence
	     of server program.
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(delete_queue_device_map)
  Args: (queue, device, machine)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(get_all_printcap)
  Args: none
  Returns: {printer, printcap}

@B(get_printcap)
  Args: (printer)
  Returns: {printer, printcap}
  Errors: SMS_PRINTER, SMS_NO_MATCH

@B(add_printcap)
  Args: (printer, printcap)
  Returns: none
  Integrity: printer must exists in printer table; printcap string is
	     application dependent.
  Errors: SMS_EXISTS, SMS_PRINTER

@B(update_printcap)
  Args: (printer, printcap)
  Returns: none
  Integrity: printcap string is application dependent.
  Errors: SMS_PRINTER, SMS_NO_MATCH

@B(delete_printcap)
  Args: (printer)
  Returns: none
  Errors: SMS_PRINTER, SMS_NO_MATCH

@SubSection(Post Office Boxes)
@label(Pobox)

@B(get_pobox)
  Args: (login)
  Returns: {login, type, machine, box}
  Errors: SMS_USER, SMS_NO_MATCH

@B(add_pobox)
  Args: (login, type, machine, box)
  Returns: none
  Integrity: user, machine must exist; application must verify type and
	     boxname. 
  Errors: SMS_EXISTS, SMS_USER, SMS_MACHINE

@B(delete_pobox)
  Args: (login, type, machine, box)
  Returns: none
  Errors: SMS_USER, SMS_MACHINE, SMS_NO_MATCH

@SubSection(Lists)
@label(List)
@label(Members)
@label(Strings)

@B(get_list_info)
  Args: (list_name(*))
  Returns: {list_name, description, flags, admin_acl, expdate, modtime}
  Errors: SMS_NO_MATCH  

@B(add_list)
  Args: (list_name, description, flags, admin_acl, expdate)
  Returns: none
  Integrity: expdate must be reasonable; application is responsible for
	     flags; modtime set by server.
  Errors: SMS_EXISTS, SMS_DATE

@B(update_list)
  Args: (list_name, description, flags, admin_acl, expdate)
  Returns: none
  Integrity: expdate must be reasonable; application is responsible for flags.
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE, SMS_DATE

@B(update_list_admin)
  Args: (list_name, admin_acl)
  Returns: none
  Integrity: admin_acl must be a known list.
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE, SMS_LIST

@B(delete_list)
  Args: (list_name)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE
  Side Effects: All members of list are deleted.

@B(add_member_to_list)
  Args: (list_name, member_type, member_name)
  Returns: none
  Integrity: member_type must be known; list_name and member_name
	     must be unique; if member_type is "user" or "list",
	     then corresponding user or list must exist.
  Side Effects: if member_type = "string", entry added to strings table; 
		list modtime updated.
  Errors: SMS_EXISTS, SMS_TYPE, SMS_LIST, SMS_USER, SMS_NO_MATCH, 
	  SMS_NOT_UNIQUE

@B(delete_member_from_list)
  Args: (list_name, member_type, member_name)
  Returns: none
  Integrity: member_type must be known; list_name and member_name must 
	     be unique.
  Side Effects: if member_type is "string", then corresponding string entry
		is deleted; list modtime updated.
  Errors: SMS_TYPE, SMS_NO_MATCH, SMS_NOT_UNIQUE

@B(get_members_of_list)
  Args: (list_name)
  Returns: {member_type, member_name}
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@Subheading(Mailing Lists)
@label(Maillists)

@B(get_all_mail_lists)
  Args: none
  Returns: {list}

@B(add_mail_list)
  Args: (list)
  Returns: none
  Errors: SMS_EXISTS, SMS_LIST

@B(delete_mail_list)
  Args: (list)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE, SMS_LIST

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

@B(delete_group)
  Args: (list)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE, SMS_LIST


@SubHeading(Access Control Lists)
@label(Acls)
@label(Capacls)

@B(get_acl_by_machine)
  Args: (machine)
  Returns: {machine, list}
  Errors: SMS_MACHINE

@B(add_acl_to_machine)
  Args: (machine, list)
  Returns: none
  Integrity: machine, list must exist
  Errors: SMS_EXISTS, SMS_MACHINE, SMS_LIST

@B(delete_acl_from_machine)
  Args: (machine, list)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE, SMS_MACHINE, SMS_LIST

@SubSection(Aliases)
@label(Alias)

@B(get_alias)
  Args: (name(*), type)
  Returns: {name, type, trans}
  Errors: SMS_TYPE, SMS_NO_MATCH

@B(add_alias)
  Args: (name, type, trans)
  Returns: none
  Integrity: type must exist as a translation of get_alias("alias", "type").
  Errors: SMS_EXISTS, SMS_TYPE

@B(delete_alias)
  Args: (name, type)
  Returns: none
  Errors: SMS_NO_MATCH, SMS_NOT_UNIQUE

@End(Verbatim)

@SubSection(Values)
@label(Values)

@Begin(Text, Below 1.5)
This section contains values that are needed by the server or application
programs for updating the database.  Some examples are:
@Begin(Itemize)

next users_id

default user disk quota

max users per nfs server

max users per post office

@End(Itemize)
@End(Text)

@Begin(Verbatim)
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

@End(Verbatim)

@Section(Errors)

@Begin(Format)

General errors (returned by all queries):

SMS_SUCCESS - query completed successfully
SMS_PERM - permission denied

Query specific errors:

SMS_NO_MATCH - no records match query arguments
SMS_NOT_UNIQUE - argument not unique as required
SMS_EXISTS - record already exists
SMS_DATE - invalid date
SMS_USER - no such user
SMS_CLUSTER - no such cluster
SMS_MACHINE - no such machine
SMS_SERVICE - no such service
SMS_FILESYS - no such file system
SMS_PRINTER - no such printer
SMS_QUEUE - no such queue
SMS_DEVICE - no such device
SMS_LIST - no such list
SMS_TYPE - unknown member type

@End(Format)

@Section(Application Programmers Library)
@label(Applib)

The following is an application library used by consumers of the
SMS database.  The library reflects a direct mapping between 
its functions and the above listed pre-defined query handles.

To be furnished.

@section(Catastrophic Failure Recovery Procedure)
@label(Catfail)

This section reviews, from an operational standpoint, the procedures
necessary to bring SMS up after a catastrophic failure.  Catastrophic
failure is defined as any system crash which cannot recover on its
own accord, or any automatic system recovery procedure which results
in database inconsistency or operational failure of the SMS system.

To be furnished.
