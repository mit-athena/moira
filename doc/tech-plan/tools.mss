@part[tools, root "sms.mss"]
@Define(TItemize=Itemize, LeftMargin +2, Indent -2, RightMargin 0, Spread 0)
@Define(TVerbatim=Verbatim, LeftMargin -5, RightMargin 0, Group, Size -1)
@NewPage()
@MajorSection(Specialized Management Tools - User Interface)
@label(Tools)

SMS will include a set of specialized management tools to enable system
administrators to control system resources.  As the system evolves, more
management tools will become a part of the SMS's application program
library.  These tools provide the fundamental administrative use of SMS.
@i[For each system service there is an administrative interface.] In this 
section, each interface discussed will provide information on the services
it effects.  The following diagram:

@blankspace(3 inches)

The user interface is indicated by the left hand side of this diagram, the 
component surrounded by the dotted line.

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

The following list of programs will be found on subsequent pages:
@begin(itemize, spread 0)
ATTACH_MAINT - Associate information to filesystems

CHFN - change finger information

CHPOBOX - change forwarding post office

CHSH - change default shell

CLUSTER_MAINT - machine and cluster management

DB_MAINT - Database integrity check.

DCM_MAINT - Update DCM table entries, including service / machine mapping.

LIST_MAINT - List administration (madm & gadm)

PRINTER_MAINT - MDQS printer maintenance

REG_TAPE - Registrar's tape entry program

RVD_MAINT - Create/update RVD server

SERVICE_MAINT - Services management

USER_MAINT - User information including NFS and PO information

USERREG - New user registration.
@end(itemize)

@i[For clarity, each new program begins on a new page.]

@NewPage()
@Begin(Description, LeftMargin +18, Indent -18)
PROGRAM NAME:@\ATTACH_MAINT - Associate information to filesystems. 

DESCRIPTION:@\@Multiple[This program will allow the administrator to
associate a user, a project, or a course to a filesystem, whether it is an
RVD pack, or an NFS-exported filesystem.  Right now, each workstation has
the file /etc/rvdtab which is manually updated by the operations staff.  By
associating a course to a filesystem in the SMS database, Hesiod, the Athena
name server, will be able to find arbitrary filesystem information, and the
system will no longer require /etc/rvdtab.

This program will maintain the database tables ufs (fields userid,
filesys), and filesys (label, type, machine_id, name, mount, access)
]

PRE-DEFINED QUERIES USED:@\
@Begin(TItemize)

update_user_home - for user to filesys mapping

add_alias - user, project, or course

add_filesys

update_filesys

delete_filesys
@End(TItemize)

Manipulates the following fields:@\
@Begin(TItemize)
(home) - USERS relation

(name, type, translation) - ALIAS relation.

(label, type, machine_id, name, mount, access) - FILESYS relation
@End(TItemize)

SUPPORTED SERVICE(S):@\
@Begin(TItemize)
Hesiod - filesys.db
@End(TItemize)

END USERS:
Administrators.

A SESSION USING ATTACH MAINT:
@Begin(TVerbatim)
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
Command: 1 
@Hinge()

Filesystem Maintenance

 1. (get         ) Get Filesystem Name.
 2. (add         ) Add Filesystem.
 3. (change      ) Update Filesystem.
 4. (delete      ) Delete Filesystem.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command: 
@Hinge()

Command: 2
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
@End(TVerbatim)

@NewPage()

PROGRAM NAME:@\CHFN - Finger Information.

DESCRIPTION:@\@Multiple[This program allows users to change their 
finger information.

The functionality of the old finger should not be changed.  A new
program (athenafinger) should be provided that will ask Hesiod for a
user's information, including the location of their home directory.  If
the .plan and .project files appear there, and are world-accessible,
they will be printed out.  So far, there are no changes in functionality
-- but the difference is that no machine need be specified.  The user no
longer has to know (or guess at) which of the many possible machines his
target might be logged into.
]

PRE-DEFINED QUERIES USED:@\
@Begin(TItemize)

get_finger_by_login

get_finger_by_first_last

update_finger_by_login
@End(TItemize)

Manipulates the following fields:@\
@Begin(TItemize)
(fullname, nickname, home_address, home_phone, office_phone, department,
year) - FINGER relation
@End(TItemize)

SUPPORTED SERVICE(S):@\
@Begin(TItemize)
User Community - finger
@End(TItemize)

END USER:@\All.

A SESSION USING CHFN:
@Begin(TVerbatim)
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
@End(TVerbatim)

@NewPage()

PROGRAM NAME:@\CHPOBOX - Add / change home mail host.  (This
program is the new chhome.)

DESCRIPTION:@\@Multiple[The name service and a mail
forwarding service need to know where a user's post office is.  
This program allows the user the capability to forward 
his mail to a different machine.  This program is a command line
interface.  Basically there are two options:

@begin(Verbatim, Above 1, LeftMargin +4)
Usage: chpobox [-d|a address] [-u user]
@End(Verbatim)
@Begin(Format, LeftMargin +4, Above 1)
wher@^e:
@\-d deletes a currently used mail address
@\-a adds a mail address
@TabClear()
@End(Format)

Chpobox without any option will return the current state of the
user's mail addresses (see below).
]

PRE-DEFINED QUERIES USED:@\
@Begin(TItemize)

get_pobox

add_pobox

delete_pobox

@End(TItemize)

Manipulates the following fields:@\
@Begin(TItemize)
(login, type, machine, box) - POBOX relation
@End(TItemize)

SUPPORTED SERVICE(S):@\
@Begin(TItemize)
User Community - forward mail
@End(TItemize)

END USERS:@\All.

A SESSION USING CHPOBOX:
@Begin(TVerbatim)

% chpobox
Current mail address(es) for pjlevine is/are:
type: LOCAL
address: pjlevine@@menelaus.mit.edu
%
%chpobox -a pjlevine@@menelaus.mit.edu  ;adds a new mail address
%chpobox -d pjlevine@@menelaus.mit.edu  ;deletes a mail address
%
@end(TVerbatim)

@NewPage()

PROGRAM NAME:@\CHSH - Default shell.

DESCRIPTION:@\This program allows users to change their default shell.

PRE-DEFINED QUERIES USED:@\
@Begin(TItemize)
update_user_shell
@End(TItemize)

SUPPORTED SERVICE(S):@\
@Begin(TItemize)
Hesiod - passwd.db
@End(TItemize)

END USERS:@\All

A SESSION USING CHSH:
@Begin(TVerbatim)
% chsh

Changing login shell for pjlevine.
Current shell for pjlevine is /bin/csh
New shell: /bin/csh
Changing shell to /bin/csh
%
@End(TVerbatim)

@NewPage()

PROGRAM NAME:@\CLUSTER_MAINT - This program manages machines and clusters.  

DESCRIPTION:@\Handles the relationships of various machines and clusters.

PRE-DEFINED QUERIES USED:@\
@Begin(TItemize)

get_machine_by_name

add_machine

update_machine

delete_machine

get_cluster_info

add_cluster

update_cluster

delete_cluster

get_machine_to_cluster_map

add_machine_to_cluster

delete_machine_from_cluster

get_all_service_clusters

add_service_clusters

delete_service_clusters
@End(TItemize)

Manipulates the following fields:@\
@Begin(TItemize)

(name, machine_id, type, model, status, serial, ethernet, sys_type) -
MACHINE relation

(name, description, location, cluster_id) - CLUSTER relation

(cluster_id, serv_label, service_cluster) - SVC relation
@End(TItemize)

SUPPORTED SERVICE(S):@\
@Begin(TItemize)
Hesiod - cluster.db
@End(TItemize)

END USERS:@\Administrator. Staff.

A SESSION WITH CLUSTER_MAINT:@\[See next page]
@Begin(TVerbatim)
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
Command: 1

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
Command: 2

Cluster Information
 1. (get         ) Get Cluster Information.
 2. (delete      ) Delete Cluster.
 3. (add         ) Add a Cluster.
 4. (update      ) Update Cluster Info.
 5. (map         ) Machine to Cluster Mapping.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command: 
Command: 3

Service Cluster Maintenence
 1. (get         ) Get All Service Clusters.
 2. (add         ) Add a Service Cluster.
 3. (delete      ) Delete a Service from a Cluster.
 4. (update      ) Update Service Cluster.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command: 
Command: 4
Machine to Cluster Mapping
Machine Name: 
Cluster Name:

Command: 5
Toggle Delete Confirmation
Delete functions will first confirm

Command: q
%
@End(TVerbatim)

@newpage()

PROGRAM NAME:@\DB_MAINT - Data base integrity checker/intersective
constrainer.

DESCRIPTION:@\This program systematically checks the database 
for user date expirations, INACTIVE status fields, and does a complete
integrity check of the lists when a user or list is deleted.
The program will also alert the system administrator if a user's
expiration time has been reached and if there is an inconsistency
with the database.  This is the program which provides interactive
constraint capability.  The program is invoked automatically every 24
hours.

PRE-DEFINED QUERIES USED:@\To be determined.

SUPPORTED SERVICE(S):@\
@Begin(TItemize)
SMS
@End(TItemize)

END USER:@\Administrator. Cron (automatically invoked).

@newpage()

PROGRAM NAME:@\DCM_MAINT - Add/update DCM table entries.

DESCRIPTION:@\This program allows the administrator to "check in" server
description files, and associated information.  The dcm reads the server
table and uses the information to update the system.  It is through this
program, therefore, that update interval, target path, and files used, to
name a few, are entered.  This program is a menu-driven program and is
invoked with the command dcm_maint.

PRE-DEFINED QUERIES USED:@\
@Begin(TItemize)

get_server_info

add_server

update_server

@End(TItemize)

Manipulates the following fields:@\
@Begin(TItemize)
(update_interval, target_dir) - SERVERS relation
@End(TItemize)

SUPPORTED SERVICE(S):@\
@Begin(TItemize)
SMS

Hesiod - sloc.db
@End(TItemize)

END USERS:@\Administrator.

A SESSION WITH DCM_MAINT:@\
@Begin(TVerbatim)
Data Control Manager Maintenance

 1. (change_host_info) Modify host-specific info for a server.
 2. (add_host_info) Create new entry for the table.
 3. (delete_host_info) Remove an entry from the table.
 4. (list_host_info) List entries by host or service.
 5. (change_service) Modify info for an existing service.
 6. (add_service ) Create new service.
 7. (delete_service) Remove an entry from the table.
 8. (list_service) List services.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command: 1
@Hinge()

Change table entry [host: ATHENA-PO-1.MIT.EDU, service: ]

 1. (show        ) Show values of entry.
 2. (last_time   ) Change the last_time field.
 3. (success     ) Change the success field.
 4. (override    ) Change the override field.
 5. (enable      ) Change the enable field.
 6. (value1      ) Change the value1 field.
 7. (value2      ) Change the value2 field.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command: 
@Hinge()

Command: 2
Create new entry for the table
Which host[]:
Which service[]:

Command: 3
Remove an entry from the table
Which host[]:
Which service[]:
@Hinge()

Command: 5
Modify info for an existing service
Which service[]:

       Modify existing service
 1. (show        ) Show fields of service.
 2. (interval    ) Change the interval field.
 3. (target_path ) Change the target_path field.
 4. (instructions) Change the instructions field.
 5. (dfgen       ) Change the dfgen field.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command: 

Command: 6
Create new service
Which service[]: 
service_name   interval   target_path   instructions   dfgen

Add service to database? [y]:
Service created

Command: 7
Remove an entry from the table
Which service[]:
No entry found for service .  

Command: 8
List services
Which service[]:
service_name   interval   target_path   instructions   dfgen
hesiod                    /dev/null     /dev/null
pop                       /dev/null     /dev/null

@End(TVerbatim)

@newpage()

PROGRAM NAME:@\LIST_MAINT - List Administration

DESCRIPTION:@\@Multiple[This program handles mailing lists, and group
lists.  The general approach to this program is to identify the list type
which will need updating.  The program is, among other things, a combination
of the current madm and gadm programs.

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
]

PRE-DEFINED QUERIES USED:@\
@Begin(TItemize)

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
@End(TItemize)

Manipulates the following fields:@\
@Begin(TItemize)
(name, type, list_id, flags, description, expdate, modtime) - LIST relation

(list_id, member_type, member_id) MEMBERS relation

(member_id, string) STRINGS relation

(capability, list_id) CAPACLS relation

(list_id) GROUPS relation
@End(TItemize)

END USERS:@\All.

SUPPORTED SERVICE(S):@\
@Begin(TItemize)
/usr/lib/aliases
@End(TItemize)

A SESSION WITH LISTMAINT:
@Begin(TVerbatim, LeftMargin -7)
%listmaint
List Maintenance Functions
 1. (list_menu   ) Manage List Parameters.
 2. (member_menu ) Manage Membership of Lists.
 3. (group_menu  ) Manage Groups.
 4. (mail_menu   ) Manage Mailing Lists.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command: 1 

List Creation and Deletion
 1. (get_list_info) Get information about a list.
 2. (add_list    ) Create a new list.
 3. (delete_list ) Delete a list.
 4. (update_list ) Update characteristics of a list.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command: 

Command: 2

Change membership
 1. (list_members) List all members of a list.
 2. (list_lists  ) List all lists to which a given member belongs.
 3. (add_member  ) Add a new member to a list.
 4. (delete_member) Delete a member from a list.
 r. (return      ) Quit.n to previous menu.
Command: 

Command: 3

Groups
 1. (list_groups ) List all groups.
 2. (add_group   ) Add a group.
 3. (delete_group) Delete a group.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command: 

Command: 4

Mailing Lists
 1. (list_mail   ) List all visible mailing lists.
 2. (add_mail    ) Add a mailing list.
 3. (delete_mail ) Delete a mailing list.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command: 

Command:q
%
@End(TVerbatim)

@newpage()

PROGRAM NAME:@\PRINTER_MAINT - Printer maintenance.

DESCRIPTION:@\This program will manage printers and queues for the Multiple
Device Queueing System (MDQS).  It handles associations between printers and
machines, printers and abilities (such as windowdumps, postscript, etc.),
machines, printers, and queues, and printers and printcaps.

PRE-DEFINED QUERIES USED:@\
@Begin(TItemize)

get_all_printers

get_printer_info

add_printer

update_printer

delete_printer

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
@End(TItemize)

Manipulates the following fields:@\
@Begin(TItemize)
(name, printer_id, type, machine_id) - PRINTER relation

(name, queue_id, machine_id, ability, status) - QUEUE relation

(printer_id, queue_id) - PQM relation

(machine_id, qdev_id, name, device, status) - QDEV relation

(machine_id, queue_id, device_id, server) - QDM relation
@End(TItemize)

SUPPORTED SERVICE(S):@\
@Begin(TItemize)
Hesiod - printers.db
@End(TItemize)

END USERS:@\
@Begin(TItemize)
Administrator.

Staff.
@End(TItemize)

@newpage()

PROGRAM NAME:@\REG_TAPE - Add or remove students from the system using
Registrar's tape.

DESCRIPTION:@\@Multiple[Each term, when the Registrar releases a tape of
current students, the system administrator must load the names of new users
and delete all old users.  This program will automatically use the
Registrar's tape as a means of keeping current the SMS database.

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
]

PRE-DEFINED QUERIES USED:@\
@Begin(TItemize)

update_user

update_user_status

@End(TItemize)

Manipulates the following fields:@\
@Begin(TItemize)
(status, expdate) - USERS relation.
@End(TItemize)

SUPPORTED SERVICE(S):@\
@Begin(TItemize)
SMS
@End(TItemize)

END USERS:@\Administrator.

@newpage()

PROGRAM NAME:@\RVD_MAINT - Create/update an RVD server.  

DESCRIPTION:@\@Multiple[This administrative program will allow for the
master copy of rvddb's to be updated and created.  The DCM will distribute
the RVD information automatically to the servers requiring RVD data.
Presently, the system administrator keeps an up-to-date file of RVD data and
then copies the data to the RVD server.

This program will handle three "tables" in the SMS database: rvdsrv
(machine id, operations pwd, admin pwd, shutdown pwd)@foot[See the
section on database structure for explanations of these fields.],
rvdphys (machine id, device, size create-time, modify-time) , and
rvdvirt (machine id, physical device, name, pack id, owner, rocap,
excap, shcap, modes, offset, blocks, ownhost, create-time, modify-time)

It will become the responsibility of SMS to maintain the present file
/site/rvd/rvddb.  The DCM will automatically load the RVD server with
information.
]

PRE_DEFINED QUERIES USED:@\
@Begin(TItemize)

get_rvd_server

add_rvd_server

delete_rvd_server

get_rvd_physical

add_rvd_physical

delete_rvd_physical

get_rvd_virtual

add_rvd_virtual

delete_rvd_virtual

update_rvd_virtual
@End(TItemize)

Manipulates the following fields:@\
@Begin(TItemize)
(machine id, operations pwd, admin pwd, shutdown pwd) - RVDSRV relation

(machine id, device, size create-time, modify-time) - RVDPHYS relation

(machine id, physical device, name, pack id, owner, rocap, excap, shcap,
modes, offset, blocks, ownhost, create-time, modify-time) - RVDVIRT relation

@End(TItemize)

SUPPORTED SERVICE(S):@\
@Begin(TItemize)
RVD - rvddb.
@End(TItemize)

END USERS:@\Administrators.

A SESSION WITH RVD_MAINT:@\
@Begin(TVerbatim)
%rvd_maint

SMS RVD Maintenance

 1. (modvd       ) Modify a virtual disk.
 2. (addvd       ) Add a new virtual disk.
 3. (addpd       ) Add a new physical disk.
 4. (rmvd        ) Delete a virtual disk.
 5. (rmpd        ) Delete a virtual disk.
 6. (exchvd      ) Exchange virtual disk names.
 7. (lookvd      ) Look up virtual disk names.
 8. (lookpd      ) Look up physical disk names.
 9. (rvdhelp     ) Get help with RVD commands.
 r. (return      ) Return to previous menu.
 q. (quit        ) Quit.
Command:
@End(TVerbatim)

@newpage()

PROGRAM NAME:@\SERVICE_MAINT - Services management.

DESCRIPTION:@\ This program manages what today is /etc/services: it informs
Hesiod of the association between services and reserved ports.

PRE-DEFINED QUERIES USED:@\
@Begin(TItemize)
get_all_services

add_service

delete_service

update_service

get_all_service_aliases

add_service_alias

delete_service_alias
@End(TItemize)

Manipulates the following fields:@\
@Begin(TItemize)
(service, protocol, port) SERVICES relation

(name, type, trans) ALIAS relation
@End(TItemize)

SUPPORTED SERVICE(S):@\
@Begin(TItemize)
Hesiod - service.db
@End(TItemize)

END USERS:@\Administrator. Staff

A SESSION WITH SERVICE_MAINT:@\
@Begin(TVerbatim)
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
@End(TVerbatim)

@newpage()

PROGRAM NAME:@\USER_MAINT - Adding/changing user information, including NFS
and post office information.

DESCRIPTION:@\@Multiple[Presently, there are two programs which the system
administrator uses to register a new user: register and admin.  These
programs register the user and enter the private key information to
Kerberos, respectively.  The new application will provide these, and offer a
third feature which allows the administrator to check the fields of the SMS
database and verify that all of the database fields are correct.  (Currently
this is done by exiting the program and using INGRES query commands to
verify data.)

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

In the NFSPHYS relation, the @i[size] field represents the total
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
]

PRE-DEFINED QUERIES USED:@\
@Begin(TItemize)
get_user_by_login

get_user_by_firstname

get_user_by_lastname

get_user_by_first_and_last

get_user_by_mitid

add_user

update_user_shell

update_user_status

update_user_home

update_user

delete_user

get_filesys_by_label

add_filesys

update_filesys

delete_filesys

get_nfs_physical

update_nfs_physical

get_nfs_quota

add_nfs_quota

update_nfs_quota

get_server_host_info

add_server_host_info

update_server_host_info

get_value - for default PO allocation
@End(TItemize)

Manipulates the following fields:@\
@Begin(TItemize)

(login, mit_id, first, last, mid_init, exp_date, shell, status, users_id,
modtime, home) - USER relation

(label, type, machine, name, mount, access) - FILESYS relation

(machine, device, dir, size, allocated) - NFSPHYS relation

(machine, device, login, quota) - NFSQUOTA relation

(value1, value2) - SERVERHOSTS relation

(name, value) - VALUES relation
@End(TItemize)

END USER:@\Administrator.

SUPPORTED SERVICE(S):
@Begin(TItemize)
Hesiod - passwd.db

Kerberos
@End(TItemize)

A SESSION USING USERMAINT:
@Begin(TVerbatim)

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

@End(TVerbatim)

@newpage()

PROGRAM NAME:@\SMS_MAINT - Master SMS program.

DESCRIPTION:@\This program can do anything that any of
the above-described programs can do, but attendant with that ability is
an increase in complexity unsuited for random users or faculty
administrators.  Given this program (and a listing in the appropriate
database acls), there is nothing in the SMS database that you cannot
view and update.  It is intended for the one or two people whose main
responsibility is the care and feeding of SMS.

SUPPORTED SERVICES:@\
@begin(Titemize)
All.
@End(TItemize)

PRE-DEFINED QUERIES USED:@\ALL

Manipulates the following fields:@\ALL

END USER:@\
@Begin(TItemize)
Administrator.

God.
@End(TItemize)

@End(Description)
