@part[tools, root "sms.mss"]
@MajorSection(Specialized Management Tools - User Interface)
@label(Tools)

SMS will include a set of specialized management tools to enable system
administrators to control system resources.  As the system evolves, more
management tools will become a part of the SMS's application program
library.  These tools provide the fundamental administrative use of SMS.

In response to complaints about the user interface of current database
maintenance tools such as madm, gadm, and (to a lesser extent) register,
the SMS tools will use a slightly different strategy.  To accommodate
novice and occasional users, a menu interface similar to the interface
in register will be the default.  For regular users, a command-line
switch (such as @t(-nomenu)) will be provided that will use a
line-oriented interface such as those in discuss and kermit.  This
should provide speed and directness for users familiar with the system,
while being reasonably helpful to novices and occasional users.  
A specialized menu building tool has been developed in order that 
new application programs can be developed quickly.
An X
interface is being planned, but is of secondary importance to the
functioning of the base system.

Fields in the database will have associated with them lists of legal
values.  A null list will indicate that any value is possible.  This is
useful for fields such as user_name, address, and so forth.  The
application programs will, before attempting to modify anything in the
database, will request this information, and compare it with the
proposed new value.  If an invalid value is discovered, it will be
reported to the user, who will be given the opportunity to change the
value, or "insist" that it is a new, legal value.  (The ability to
update data in the database will not necessarily indicate the ability to
add new legal values to the database.)

Applications should be aware of the ramifications of their actions, and
notify the user if appropriate.  For example, an administrator deleting
a user should be informed of storage space that is being reclaimed,
mailing lists that are being modified.  Objects that need to be modified
at once (such as the ownership of a mailing list) should present
themselves to be dealt with.

@Begin(Itemize, Spread 1)
@Begin(Multiple)

PROGRAM NAME: RVD_MAINT - Create/update an RVD server.  

DESCRIPTION: This administrative program
will allow for the master copy of rvddb's to be updated and created.
The DCM will distribute the RVD information automatically to the servers
requiring RVD data.  Presently, the system administrator keeps an
up-to-date file of RVD data and then copies the data to the RVD server.

This program will handle three "tables" in the SMS database: rvdsrv
(machine id, operations pwd, admin pwd, shutdown pwd)@foot[See the
section on database structure for explanations of these fields.],
rvdphys (machine id, device, size create-time, modify-time) , and
rvdvirt (machine id, physical device, name, pack id, owner, rocap,
excap, shcap, modes, offset, blocks, ownhost, create-time, modify-time)

It will become the responsibility of SMS to maintain the present file
/site/rvd/rvddb.  The DCM will automatically load the RVD server with
information.

PRE_DEFINED QUERIES USED:
@begin(itemize, spread 0)

get_rvd_server

add_rvd_server

delete_rvd_server
@end(itemize)

manipulates the following fields:
(machine id, operations pwd, admin pwd, shutdown pwd)
RVDSRV relation
@begin(itemize, spread 0)

get_rvd_physical

add_rvd_physical

delete_rvd_physical
@end(itemize)

manipulates the following fields:
(machine id, device, size create-time, modify-time)
RVDPHYS relation

@begin(itemize, spread 0)

get_rvd_virtual

add_rvd_virtual

delete_rvd_virtual

update_rvd_virtual
@end(itemize)

manipulates the following fields:
(machine id, physical device, name, pack id, owner, rocap,
excap, shcap, modes, offset, blocks, ownhost, create-time, modify-time)
RVDVIRT relation

END USERS:
Administrators.

@End(Multiple)

@begin(multiple)
PROGRAM NAME: ATTACH_MAINT - Associate information to filesystems. 

DESCRIPTION: This program will allow the 
administrator to associate a
user, a project, or a course to a filesystem, whether it is an RVD
pack, or an NFS-exported filesystem.  Right now, each workstation has
the file /etc/rvdtab which is manually updated by the operations staff.
By associating a course to a filesystem in the SMS database, Hesiod, the
Athena name server, will be able to find arbitrary filesystem
information, and the system will no longer require /etc/rvdtab.

This program will maintain the database tables ufs (fields userid,
filesys), and filesys (label, type, machine_id, name, mount, access)

PRE-DEFINED QUERIES USED:
@begin(itemize, spread 0)

update_user_home - for user to filesys mapping

add_alias - user, project, or course
@end(itemize)

manipulates the following fields:
(home) - USERS relation.
(name, type, translation) ALIAS relation.

@begin(itemize, spread 0)

add_filesys

update_filesys

delete_filesys
@end(itemize)

manipulates the following fields:
(label, type, machine_id, name, mount, access) FILESYS relation

END USERS:
Administrators.

@end(multiple)

@Begin(Multiple)

PROGRAM NAME: USER_MAINT - Adding/changing user information, including NFS 
quota information.  

DESCRIPTION: Presently, there are two
programs which the system administrator uses to register a new user:
register and admin.  These programs register the user and enter the
private key information to Kerberos, respectively.  The new application
will provide these, and offer a third feature which allows the
administrator to check the fields of the SMS database and verify that
all of the database fields are correct.  (Currently this is done by
exiting the program and using INGRES query commands to verify data.)

PRE-DEFINED QUERIES USED

@begin(itemize, spread 0)
get_user_by_login

get_user_by_firstname

get_user_by_lastname

get_user_by_first_and_last

add_user

update_user_shell

update_user_status

update_user_home

update_user
@end(itemize)

manipulates the following fields:
(login, mit_id, first, last, mid_init, exp_date, shell, status, users_id,
modtime, home) USER relation

This program will also allocate and change home directory storage space.
It will allow the administrator to check storage allocation on a server
and allocate or change a storage space for a user.  The information will
be held in the SMS database and will be passed to the name service.  The
allocation of a user's quota can be done automatically at register
time, using the amount of quota already allocated on each server, and
querying each server as to the amount of disk space actually available.
There are other circumstances, such as the user's living group, that
should be taken into consideration when assigning a server.  (Trying to
assign people to servers reasonably "near" them is an attempt to
decrease the load on the network.)

For home directory allocation and change, the following predefined
queries are used:

@begin(itemize, spread 0)

get_nfs_quota

add_nfs_quota

update_nfs_quota

@end(itemize)

manipulates the following fields:
(machine, login, quota) NFSQUOTA relation


@begin(itemize, spread 0)

get_server_info

add_server

update_server

@end(itemize)

manipulates the following fields:
(value) SERVERS relation

@begin(itemize, spread 0)

get_value - for default PO allocation

@end(itemize)

manipulates the following field:
(value) VALUE relation.

In the SERVERS relation, the @i[value] field represents the total
currently allocated space (but not necessarily used).  In the VALUE
relation, the @i[value] field represents the default nfs quota for a
user (used in new user home allocation).  For example, if 20 users have
been allocated to a machine and each has a filesys quota of 2 Meg, then
the value field (SERVER relation) will be 40 Meg.  If the server reports
back that its free space is 80 Meg, then another 20 users can be given
allocated space on this disk.  As long as the free space minus the
allocated space is greater or equal to the quota of the current
allocation, the disk is OK to use.  This mechanism will prevent over
allocation of home directory storage space.

END USER: Administrator.

@end(multiple)

@begin(multiple)
PROGRAM NAME: CHFN - Finger Information.

DESCRIPTION: This program allows users to change their 
finger information.

The functionality of the old finger should not be changed.  A new
program (athenafinger) should be provided that will ask Hesiod for a
user's information, including the location of their home directory.  If
the .plan and .project files appear there, and are world-accessible,
they will be printed out.  So far, there are no changes in functionality
-- but the difference is that no machine need be specified.  The user no
longer has to know (or guess at) which of the many possible machines his
target might be logged into.

PRE-DEFINED QUERIES USED:
@begin(itemize, spread 0)

get_finger_by_login

get_finger_by_first_last

update_finger_by_login
@end(itemize)

manipulates the following fields:
(fullname, nickname, home_address, home_phone,
office_address, office_phone, department, year, modtime) FINGER relation

END USERS:
Administrator.  Staff.  Students.

@end(multiple)

@begin(multiple)
PROGRAM NAME: PO_MAINT - Allocate and change home mail host.  

DESCRIPTION: The name service and a mail
forwarding service need to know where a user's post office is.  When a
new user registers this program checks the number of users already
assigned to various post offices and allocates space to a user in a post
office with available space.

PRE-DEFINED QUERIES USED:
@begin(itemize, spread 0)

get_po_box

add_po_box

delete_po_box

@end(itemize)

manipulates the following fields:
(login, type, machine, box) POBOX relation

@begin(itemize, spread 0)

get_server_info

add_server

update_server

@end(itemize)

manipulates the following fields:
(value) SERVERS relation

@begin(itemize, spread 0)

get_value - for default PO allocation

@end(itemize)

manipulates the following field:
(value) VALUE relation.

In this program, the @i[value] field represents the number of allocated
post office boxes on a machine.  A default value of post office box allocations
will be compared to the actual number of allocated boxes.

END USERS:  Administrator. Staff. Students.
@end(multiple)

@begin(multiple)
PROGRAM NAME: REG_TAPE - Add or remove students from 
the system using Registrar's
tape.  

DESCRIPTION: Each term, when the Registrar releases a tape of current
students, the system administrator must load the names of new users and
delete all old users.  This program will automatically use the
Registrar's tape as a means of keeping current the SMS database.

PRE-DEFINED QUERIES USED:
@begin(itemize, spread 0)

update_user

update_user_status

@end(itemize)

manipulates the following fields:
(status, expdate) USERS relation.

END USERS: Administrator.

The problem of deleting users is a sensitive one.  The removal of a user
will reflect this sensitivity.  When deleting a user, the expiration
date field will be set to the current date, but the user will not be
removed.  The program db_maint will, among other things, check the
expiration stamp of the users.  If a stamp is within critical expiration
time, the program will notify the administrator that a time-to-live date
has been reached.  If correct, the administrator will set the user's
status field to INACTIVE and set the time to some date in the future.
When that date and INACTIVE status are reached, the user is flushed.  If
incorrect, the administrator will set the date to some time in the
future and leave the status field ACTIVE.
@end(multiple)

@begin(multiple)

PROGRAM NAME: SERVER_MAINT - Add/update system servers.  

DESCRIPTION: This program not only interacts
with the database and updates the server tuple set, but also allows the
administrator to update the SDF, server description files, and the
frequency of updates.  This gives the DCM the knowledge necessary to
update the system.

PRE-DEFINED QUERIES USED:
@begin(itemize, spread 0)

get_server_info

add_server

update_server

@end(itemize)

manipulates the following fields:
(update_interval, target_dir) SERVERS relation

END USERS: Administrator.  Staff.
@end(multiple)
@begin(multiple)

PROGRAM NAME: LIST_MAINT - List Administration

DESCRIPTION: 
This program handles mailing lists, group lists, and ACL administration.
The general approach to this program is to identify the list type 
which will need updating.  The program is, among other things, a 
combination of the current madm and gadm programs.

PRE-DEFINED QUERIES USED:
@begin(itemize, spread 0)

get_all_mail_lists

add_mail_list

delete_mail_list

get_list_info

add_list

update_list

delete_list

add_member_to_list

delete_member_from_list

get_members_of_list

get_acl_by_machine

add_acl_to_machine

get_all_groups

add_group

delete_group
@end(itemize)

manipulates the following fields:
(name, type, list_id, flags, description, expdate, modtime) LIST relation

(list_id, member_type, member_id) MEMBERS relation

(member_id, string) STRINGS relation

(list_id, machine_id) ACLS relation

(capability, list_id) CAPACLS relation

(list_id) GROUPS relation

END USERS: Administrator. Staff. Students.
@end(multiple)

@begin(multiple)
PROGRAM NAME: CLUSTER_MAINT - This program manages machines and clusters.  

DESCRIPTION: Handles the relationships of various machines and clusters.

PRE-DEFINED QUERIES USED:

MACHINE:
@begin(itemize, spread 0)

get_machine_by_name

add_machine

update_machine

delete_machine
@end(itemize)

manipulates the following fields:

(name, machine_id, type, model, status, serial, ethernet, 
sys_type) MACHINE relation

CLUSTERS:
@begin(itemize, spread 0)
get_cluster_info

add_cluster

update_cluster

delete_cluster

get_machine_to_cluster_map

add_machine_to_cluster

delete_machine_from_cluster
@end(itemize)

manipulates the following fields:

(name, description, location, cluster_id) CLUSTER relation

SERVICE CLUSTERS
@begin(itemize, spread 0)
get_all_service_clusters

add_service_clusters

delete_service_clusters
@end(itemize)

manipulates the following fields:

(cluster_id, serv_label, service_cluster) .

END USERS: Administrator. Staff.

@end(multiple)
@begin(multiple)

PROGRAM NAME: SERVICE_MAINT - Services management.

DESCRIPTION: 
This program manages what today is /etc/services:  it
informs Hesiod of the association between services and reserved ports.

PRE-DEFINED QUERIES USED:
@begin(itemize, spread 0)
get_all_services

add_service

delete_service

update_service

get_all_service_aliases

add_service_alias

delete_service_alias
@end(itemize)

manipulates the following fields:

(service, protocol, port) SERVICES relation

(name, type, trans) ALIAS relation

END USERS: Administrator. Staff
@end(multiple)

@begin(multiple)
PROGRAM NAME: PRINTER_MAINT - Printer maintenance.

DESCRIPTION: 
This program will manage printers and queues for the
Multiple Device Queueing System (MDQS).  It handles associations between
printers and machines, printers and abilities (such as windowdumps,
postscript, etc.), machines, printers, and queues, and printers and
printcaps.

PRE-DEFINED QUERIES USED:
@begin(itemize, spread 0)

get_all_printers

get_printer_info

add_printer

update_printer

delete_printer

get_printer_ability

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
@end(itemize)

updates the following fields:

(name, printer_id, type, machine_id) PRINTER relation

(printer_id, ability) PRABILITY relation

(name, queue_id, machine_id, ability, status) QUEUE relation

(printer_id, queue_id) PQM relation

(machine_id, qdev_id, name, device, status) QDEV relation

(machine_id, queue_id, device_id, server) QDM relation

END USERS: Administrator.  Staff.
@end(multiple)

@begin(multiple)
PROGRAM NAME: DB_MAINT - Data base integrity checker/intersective
constrainer.

DESCRIPTION - This program systematically checks the database 
for user date expirations, INACTIVE status fields, and does a complete
integrity check of the lists when a user or list is deleted.
The program will also alert the system administrator if a user's
expiration time has been reached and if there is an inconsistency
with the database.  This is the program which provides interactive
constraint capability.  The program is invoked automatically every 24
hours.

PRE-DEFINED QUERIES USED:
@begin(itemize, spread 0)

To be determined

@end(itemize)

END USER: Administrator. Cron (automatically invoked).
@end(multiple)

@begin(multiple)
PROGRAM NAME: SMS_MAINT - Master SMS program.

DESCRIPTION: This program can do anything that any of
the above-described programs can do, but attendant with that ability is
an increase in complexity unsuited for random users or faculty
administrators.  Given this program (and a listing in the appropriate
database acls), there is nothing in the SMS database that you cannot
view and update.  It is intended for the one or two people whose main
responsibility is the care and feeding of SMS.

PRE-DEFINED QUERIES USED:

ALL

manipulates the following fields:

ALL

END USER: Administrator.  God.

@end(multiple)

@End(Itemize)

@section(New User Registration)

A new student must be able to get an athena account without any
intervention from Athena user accounts staff.  This is important,
because otherwise, the user accounts people would be faced with having
to give out ~1000 accounts or more at the beginning of each term.

With athenareg, a special program (userreg) was run on certain
terminals connected to timesharing systems in several of the terminal
rooms.  It prompted the user for his name and ID number, looked him up
in the athenareg database, and gave him an account if he did not have
one already.  Userreg has been rewritten to work with SMS; in
appearance, it is virtually identical to the athenareg version (except
in speed).

Athena obtains a copy of the Registrar's list of registered students
shortly before registration day each term.  Each student on the
registrar's tape who has not been registered for an Athena account is
added to the "users" relation of the database, and assigned a unique
userid; the student is not assigned a login name, and is not known to
kerberos.  An encrypted form of the student's ID number is stored
along with the name; the encryption algorhythm is the UNIX C library
crypt() function (also used for passwords in /etc/passwd); the last
seven characters of the ID number are encrypted using the first letter
of the first name and the first letter of the last name as the "salt".
No other database resources are allocated at that time.

The SMS database server machine runs a special "registration server"
process, which listens on a well known UDP port for user registration
requests.  There are currently three defined requests:

@begin(display)
verify_user, First Last, {IDnumber, hashIDnumber}@-{hashIDnumber}
grab_login, First Last, {IDnumber, hashIDnumber, login}@-{hashIDnumber}
set_password, First Last, {IDnumber, hashIDnumber, password}@-{hashIDnumber}

where
	First Last is the student's name,
	IDnumber is the student's id number (for example: 123456789)
	hashIDnumber is the encrypted ID number (for example: lfIenQqC/O/OE)
@end(display)

({ a, b }@-(K) means that 'a, b' is encrypted using the error
propagating cypher-block-chaining mode of DES, as described in the
Kerberos document).

The registration server communicates with the kerberos admin_server,
and sets up a secure communication channel using "srvtab-srvtab"
authentication.  In all cases, the server first retrieves each 

When the student decides to register with athena, he walks up to a
workstation and logs in using the username of "userreg" (no password
is necessary).  This pops up a forms-like interface which prompts him
for his first name, middle initial, last name, and student ID number.
It calculates the hashed id number using crypt(), and sends a
verify_user request to the registration server.  The server responds
with one of already_registered, not_found, or OK.

If the user has been validated, userreg then prompts him for his
choice in login names.  It then goes through a two-step process to
verify the login name: first, it tries to get initial tickets for the
user name from Kerberos; if this fails (indicating that the username
is free and may be registered), it then sends a grab_login request.
On receiving a grab_login request, the registration server then
proceeds to register the login name with kerberos; if the login name
is already in use, it returns a failure code to userreg.  Otherwise,
it allocates a home directory for the user on the least-loaded
fileserver, builds a post office entry for the user, sets an initial
quota for the user, and returns a success code to userreg.

Userreg then prompts the user for an initial password, and sends a
set_password request to the registration server, which decrypts it and
forwards it to Kerberos.  At this point, pending propagation of
information to hesiod, the mail hub, and the user's home fileserver,
the user has been established.



