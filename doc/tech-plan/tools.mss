@part[tools, root "sms.mss"]
@MajorSection(Specialized Management Tools - User Interface)

SMS will include a set of specialized management tools to enable system
adminstrators to control system resources.  As the system evolves, more
management tools will become a part of the SMS's application program
library.  These tools provide the fundamental administrative use of SMS.

In response to complaints about the user interface of current database
maintenance tools such as madm, gadm, and (to a lesser extent) register,
the SMS tools will use a slightly different strategy.  To accomodate
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

PROGRAM NAME: rvd_maint - Create/update an RVD server.  
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

DATABASE QUERIES USED:


get_rvd_physical();
add_rvd_physical();
delete_rvd_physical();
manipulates the following fields:
(machine id, device, size create-time, modify-time)

get_rvd_virtual();
add_rvd_virtual();
delete_rvd_physical();
manipulates the following fields:
(machine id, physical device, name, pack id, owner, rocap,
excap, shcap, modes, offset, blocks, ownhost, create-time, modify-time)

END USERS:
Administrators.

@End(Multiple)

@begin(multiple)
attach_maint - This program will allow the administrator to associate a
user, a project, or a course to an filesystem, whether it is an RVD
pack, or an NFS-exported filesystem.  Right now, each workstation has
the file /etc/rvdtab which is manually updated by the operations staff.
By associating a course to a filesystem in the SMS database, Hesiod, the
Athena name server, will be able to find arbitrary filesystem
information, and the system will no longer require /etc/rvdtab.

This program will maintain the database tables ufs (fields userid,
filesys), and filesys (label, type, machine_id, name, mount, access,
acl_id).
@end(multiple)

@Begin(Multiple)

user_maint - Adding/changing user information.  Presently, there are two
programs which the system administrator uses to register a new user:
register and admin.  These programs register the user and enter the
private key information to Kerberos, respectively.  The new application
will provide these, and offer a third feature which allows the
administrator to check the fields of the SMS database and verify that
all of the database fields are correct.  (Currently this is done by
exiting the program and using INGRES query commands to verify data.)
The user registration fields which this program will affect are:

login, mit_id, first, last, mid_init, exp_date, shell, status, users_id,
modtime, quota

This program will also allocate and change home directory storage space.
It will allow the adminstrator to check storage allocation on a server
and allocate or change a storage space for a user.  The information will
be held in the SMS database and will be passed to the name service.  The
allocation of a user's quota can be done automatically at register
time, using the amount of quota already allocated on each server, and
querying each server as to the amount of disk space actually available.
There are other circumstances, such as the user's living group, that
should be taken into consideration when assigning a server.  (Trying to
assign people to servers reasonably "near" them is an attempt to
decrease the load on the network.)

@end(multiple)

@begin(multiple)
chfn - This will allow users to change their finger information.  Fields
affected will be: fullname, nickname, home_address, home_phone,
office_address, office_phone, department, year, modtime.

The functionality of the old finger should not be changed.  A new
program (athenafinger) should be provided that will ask Hesiod for a
user's information, including the location of their home directory.  If
the .plan and .project files appear there, and are world-accessible,
they will be printed out.  So far, there are no changes in functionality
-- but the difference is that no machine need be specified.  The user no
longer has to know (or guess at) which of the many possible machines his
target might be logged into.
@end(multiple)

po_maint - Allocate/change home mail host.  The name service and a mail
forwarding service need to know where a user's post office is.  When a
new user registers this program checks the number of users already
assigned to various post offices and allocates space to a user in a post
office with available space.

reg_tape - Add or remove students from the system using Registrar's
tape.  Each term, when the Registrar releases a tape of current
students, the system administrator must load the names of new users and
delete all old users.  This program will automatically use the
Registrar's tape as a means of keeping current the SMS database.

server_maint - Add/update system servers.  This program not only interacts
with the database and updates the server tuple set, but also allows the
administrator to update the SDF, server description files, and the
frequency of updates.  This gives the DCM the knowledge necessary to
update the system.

mail_maint - This program handles mailing lists, in all of their
infinite variety.  It is similar in function to the current madm.

cluster_maint - This program manages machines and clusters.  It handles
the following tables and fields: machine (name, machine_id, type, model,
satus, serial, ethernet, sys_type); cluster (name, description,
location, cluster_id, acl_id); svc (cluster_id, environment variable,
service cluster).

service_maint - This program manages what today is /etc/services:  it
informs Hesiod of the association between services and reserved ports.

printer_maint - This program will manage printers and queues for the
Multiple Device Queueing System (MDQS).  It handles associations between
printers and machines, printers and abilities (such as windowdumps,
postscript, etc.), machines, printers, and queues, and printers and
printcaps.

sms_maint - This is the master program.  It can do anything that any of
the above-described programs can do, but attendant with that ability is
an increase in complexity unsuited for random users or faculty
administrators.  Given this program (and a listing in the appropriate
database acls), there is nothing in the SMS database that you cannot
view and update.  It is intended for the one or two people whose main
responsibility is the care and feeding of SMS.

@End(Itemize)
