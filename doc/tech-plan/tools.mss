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

A SESSION USING ATTACH MAINT:
@begin(display)
%attachmaint

                         Attach/Filesystem Maintenance
 1. (filesystem  ) Filesystem Work.
 2. (update      ) Update User's Home.
 3. (+           ) Associate with a Filesystem.
 4. (-           ) Disassociate from a Filesystem.
 5. (check       ) Check An Association.
 6. (toggle      ) Toggle Verbosity of Delete.
 7. (help        ) Help ...
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command:1 

                             Filesystem Maintenance
 1. (get         ) Get Filesystem Name.
 2. (add         ) Add Filesystem.
 3. (change      ) Update Filesystem.
 4. (delete      ) Delete Filesystem.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command: 

Command:2
Update User's Home
Login name: 
Home Filesys: 

Command: 3
Associate with a Filesystem
name (user/course/project): 
Filesystem Name:

Command: 4 
Disassociate from a Filesystem
name (user/course/project):
Filesystem Name:

Command: 5 
Check An Association
name (user/group/course): 

Command: 6 
Toggle Verbosity of Delete
Delete functions will first confirm

Command:q
%

@end(display)
@end(multiple)

@Begin(Multiple)

PROGRAM NAME: USER_MAINT - Adding/changing user information, including NFS 
and post office information.  

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

A SESSION USING USERMAINT:
@begin(display)

                             SMS User Maintenance
 1. (add         ) Add new user.
 2. (modify      ) Modify user.
 3. (chsh        ) Change a user's login shell.
 4. (chdir       ) Change a user's home directory.
 5. (chstat      ) Change a user's status.
 6. (chpw        ) Change a user's password.
 7. (show_login  ) Show user entry by login name.
 8. (show_last   ) Show user entry by last name.
 9. (show_first  ) Show user entry by first name.
10. (show_full   ) Show user by first and last names.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command:

@end(display)
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
(fullname, nickname, home_address, home_phone, office_phone, department,
year)

END USER: All.

A SESSION USING CHFN:
@begin(display)
% chfn

Changing finger information for pjlevine.
Default values are printed inside of of '[]'.
To accept the default, type <return>.
To have a blank entry, type the word 'none'.

Full name [peter levine]: 

Nickname [pete]: 
Home address (Ex: Bemis 304) [24 kilsyth rd Brookline]: 
Home phone number (Ex: 4660000) [1234567]: 
Office address (Exs: 597 Tech Square or 10-256) [E40-342a]: 
Office phone (Ex: 3-1300) [0000]: 
MIT department (Exs: EECS, Biology, Information Services) []: 
MIT year (Exs: 1989, '91, Faculty, Grad) [staf]: 
% 
@end(display)
@end(multiple)

@begin(multiple)
PROGRAM NAME: CHPOBOX - Add / change home mail host.  (This
program is the new chhome.)

DESCRIPTION: The name service and a mail
forwarding service need to know where a user's post office is.  
This program allows the user the capability to forward 
his mail to a different machine.  This program is a command line
interface.  Basically there are two options:
@begin(display)

Usage: chpobox [-d|a address] [-u user]

where:
@\@\-d deletes a currently used mail address
@\@\-a adds a mail address

Chpobox without any option will return the current state of the
user's mail addresses (see below).
@end(display)

PRE-DEFINED QUERIES USED:
@begin(itemize, spread 0)

get_po_box

add_po_box

delete_po_box

@end(itemize)

manipulates the following fields:
(login, type, machine, box) POBOX relation

END USERS:  All.

A SESSION USING CHPOBOX.
@begin(display)

% chpobox
Current mail address(es) for pjlevine is/are:
type: LOCAL
address: pjlevine@menelaus.mit.edu
%
%chpobox -a pjlevine@menelaus.mit.edu  ;adds a new mail address
%chpobox -d pjlevine@menelaus.mit.edu  ;deletes a mail address
%
@end(display)
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

The problem of deleting users is a sensitive issue.  The removal of a user
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

PROGRAM NAME: DCM_MAINT - Add/update DCM table entries.

DESCRIPTION: This program allows the administrator to "check in"
server description files, and associated information.  The dcm
reads the server table and uses the information to update the system.
It is through this program, therefore, that update interval, target path,
and files used, to name a few, are entered.
This program is a menu-driven program and is
invoked with the command dcm_maint.

PRE-DEFINED QUERIES USED:
@begin(itemize, spread 0)

get_server_info

add_server

update_server

@end(itemize)

manipulates the following fields:
(update_interval, target_dir) SERVERS relation

END USERS: Administrator.

A SESSION WITH DCM_MAINT:
@begin(display)

@end(display)
@end(multiple)
@begin(multiple)

PROGRAM NAME: LIST_MAINT - List Administration

DESCRIPTION: 
This program handles mailing lists, and group lists.
The general approach to this program is to identify the list type 
which will need updating.  The program is, among other things, a 
combination of the current madm and gadm programs.  

The notion of a list in the SMS world is an entity which 
contains members.  These members are not limited to users, and, in fact,
can be machines, users, mail addresses, and even other lists.
Additionally, a list has an owning access control list, acl.
For a user who wishes to have himself own a list
the process is simple. Upon registering, a user will be placed
on a list, where the list's name is the user's name and the
list's contents are the user himself.  This will allow that
user to "own", or more precisely, associate an acl, to a new list.
In listmaint, where a user creates lists, the entry "administrator ACL"
must be another list.  Based on the above, however, this entry may be the
user's name.  

A list also may be associated with many functionalities.  For example, 
a list may be a mailing list and a group list.  This allows the same
list to be used differently.  Listmaint provides the mechanism to
associated a list with a given type of function.  Because of this, however,
the user must create a list first.  This seems obvious in discussion, 
although a deficiency the the program does not intuitively presume. 
The sample session below highlights this case.  The command 
"listmaint" invokes this program.

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

get_all_groups

add_group

delete_group
@end(itemize)

manipulates the following fields:
(name, type, list_id, flags, description, expdate, modtime) LIST relation

(list_id, member_type, member_id) MEMBERS relation

(member_id, string) STRINGS relation

(capability, list_id) CAPACLS relation

(list_id) GROUPS relation

END USERS: All.

A SESSION WITH LISTMAINT
@begin(display)

%listmaint
                           List Maintenance Functions
 1. (list_menu   ) Manage List Parameters.
 2. (member_menu ) Manage Membership of Lists.
 3. (group_menu  ) Manage Groups.
 4. (mail_menu   ) Manage Mailing Lists.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command:1 

                           List Creation and Deletion
 1. (get_list_info) Get information about a list.
 2. (add_list    ) Create a new list.
 3. (delete_list ) Delete a list.
 4. (update_list ) Update characteristics of a list.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command: 


Command:2
                               Change membership
 1. (list_members) List all members of a list.
 2. (list_lists  ) List all lists to which a given (user,list,string)belongs.
 3. (add_member  ) Add a new member to a list.
 4. (delete_member) Delete a member from a list.
 r. (return      ) Quit.n to previous menu.
Command: 

Command:3
                                     Groups
 1. (list_groups ) List all groups.
 2. (add_group   ) Add a group.
 3. (delete_group) Delete a group.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command: 

Command:4
                                 Mailing Lists
 1. (list_mail   ) List all visible mailing lists.
 2. (add_mail    ) Add a mailing list.
 3. (delete_mail ) Delete a mailing list.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command: 

Command:q
%
@end(display)
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

A SESSION WITH CLUSTER_MAINT
@begin(display)
                             Cluster Maintenence
 1. (machine     ) Work on Machines.
 2. (cluster     ) Work on Clusters.
 3. (service     ) Service Clusters.
 4. (map         ) Machine to Cluster Mapping.
 5. (toggle      ) Toggle Delete Confirmation.
 6. (list        ) List All Valid Queries.
 7. (help        ) Help.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command: 

Command:1
                             Machine Maintenence
 1. (get         ) Get Machine by Name.
 2. (add         ) Add Machine.
 3. (update      ) Update Machine.
 4. (delete      ) Delete Machine.
 5. (put         ) Add Machine to Cluster.
 6. (remove      ) Delete Machine from Cluster.
 7. (map         ) Machine to Cluster Mapping.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command:  

Command:2
                              Cluster Information
 1. (get         ) Get Cluster Information.
 2. (delete      ) Delete Cluster.
 3. (add         ) Add a Cluster.
 4. (update      ) Update Cluster Info.
 5. (map         ) Machine to Cluster Mapping.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command: 

Command:3
                         Service Cluster Maintenence
 1. (get         ) Get All Service Clusters.
 2. (add         ) Add a Service Cluster.
 3. (delete      ) Delete a Service from a Cluster.
 4. (update      ) Update Service Cluster.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command: 

Command:4
Machine to Cluster Mapping
Machine Name: 
Cluster Name:

Command:5
Toggle Delete Confirmation
Delete functions will first confirm

Command:q
%
@end(display)
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

A SESSION WITH SERVICE_MAINT
@begin(display)

%servermaint

                               Server Maintenence
 1. (get         ) Services Info.
 2. (add         ) Add Service.
 3. (update      ) Update Service.
 4. (delete      ) Delete Service.
 5. (toggle      ) Toggle Verbosity of Delete.
 6. (list        ) List All Valid Queries.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command: 

Command: 1
Services Info
Service Name: 

Command: 2
Add Service
Name: 
Protocol: 
Port: 

Command: 3
Update Service
Name: 
Protocol: 
Port: 

Command: 4
Delete Service
Service Name: 
Protocol:

Command: 5
Toggle Verbosity of Deletey handle 
Delete function will first confirm

Command:q
%
@end(display)

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

