@part[db, root="sms.mss"]
@section(Server Arrangement)

Currently, sms acts to update a variety of servers.  Although the data 
control manager performs this update task, each server requires a 
different set of update parameters.  To date, the DCM uses c programs,
not SDFs, to implement the construction of the server specific files. 
Each c program is checked in via the dcm_maint program.   The DCM then
calls the appropriate module when the update interval is reached.

For each server file propagated, ther is at least one application 
interface which provides the capability to manipulate the sms database.
Since the sms database acts as a single point of contact, the
changes made to the database are reflected in the contents of 
recipient servers. 

The following diagram:

@blankspace(3 inches)

This section focusses on the above component contained within
the dotted-lined box.  This section of the system is the server side,
receiving propagated data.  Reference, however, is made to 
each application interface which ultimately effects the server contents.

The services which sms now supports are:
@begin(itemize, spread 1)
Hesiod - The athena nameserver.

RVD - Remote virtual disk.

NFS - Network file system.

/usr/lib/aliases - Mail Service.

MDQS - Multiple device queueing system. (currently not avaliable)

Zephyr - The athena notification service. (currently not available)
@end(itemize)

@subsection(Server Assumptions)

The requirements of each server suggests a level of detail describing
the following:
@begin(itemize, spread 1)
Service name.

Service description.

Propagation interval.

Data format.

Target location.

Generated files.

File description.

Queries used to generate the file (including fields queried).

How the file is modified (application interface).

Example of file contents.
@end(itemize)
@begin(itemize, spread 1)
Service: Hesiod

Description:  The hesiod server is a primary source of contact for many athena
operations.  It is reponsible for providing information reliably and quickly.
SMS's responsibility to hesiod is to provide authoritative data.  Hesiod uses
a BIND data format in all of it's data files.  SMS will provide BIND format to
hesiod.  There are several files which hesiod uses.  To date, they are known
to include the following:
@begin(itemize, spread 0)
cluster.db

service.db 

printers.db

lpr.db

pobox.db

sloc.db

rvdtab.db

passwd.db

printcap.db
@end(itemize)

Each of these files are described in detail below.  The hesiod server uses
these files from virtual memory on the target machine.  The server
automatically loads the files from disk into memory when it is started.
SMS will propagate hesiod files to the target disk and the run a shell script 
which will kill the running server and then restart it, causing the newly
updated files to be read into memory. 

With hesiod, all target machines receive identical files.  Practically, 
therefore, the DCM will prepare only one set of files and then will
propagate to several target hosts.

For additional technical information on @i[hesiod], please refer to the
Hesiod technical plan.

Propagation interval : 6 Hours, 0:00, 6:00, 12:00, 18:00

Data format : BIND

Target locations : 
JASON.MIT.EDU: /etc/athena/nameserver
ZEUS.MIT.EDU: /etc/athena/nameserver
MENELAUS.MIT.EDU: /etc/athena/nameserver

@begin(display)
   HESIOD.DB - Hesiod data 
        Description:

        Queries used:
             NOT CREATED FROM SMS QUERY
        How modified:
             EDITOR by system administrator.
        Example contents:


   CLUSTER.DB - Cluster data
        Description:

        Queries used:
             get_all_service_clusers() -> 
                 (cluster, service_label, service_cluster)
             get_machine_to_cluster_map(*,*) -> (machine, cluster)
        How modified:

        Example contents:


   SERVICE.DB - services
        Description:

        Queries used:
             get_all_services -> (service, protocol, port, description)
             get_alias(*, service) -> (name, type, translation)
        How modified:

        Example contents:


   PASSWD.DB - password
        Description:

        Queries used:

        How modified:

        Example contents:

   PRINTERS.DB - MDQS printer info
        Description:

        Queries used:
             get_all_printer_clusters() -> (cluster)
             get_printers_of_cluster(cluster) -> 
                 (pname, qname, serverhost, ability, hwtype)

        How modified:

        Example contents:

   LPR.DB - lpr printer info
        Description:

        Queries used:

        How modified:

        Example contents:

   PINTCAP.DB - line printer information
        Description:

        Queries used:

        How modified:

        Example contents:

   POBOX.DB - post office info
        Description:

        Queries used:
             get_poboxes_pop(*)
             get_poboxes_local(*)
             get_poboxes_foreign(*) -> (login, type, machine, box)

        How modified:

        Example contents:

   SLOC.DB - service location
        Description:

        Queries used:
             get_server_location(*) -> (server, location)

        How modified:

        Example contents:

   RVDTAB.DB - RVD info
        Description:

        Queries used:
             get_all_filesys() -> (label, type, machine, name, mount, access)
             get_alias(*, FILESYS) -> (name, type, trans)
       
        How modified:

        Example contents:

@end(display)
Update mechanism:
Updating hesiod is a relatively simple process.  Every six
hours the DCM will initiate a build on each of the above
files(assuming the informtion has changed).  Once a file
is constructed, the update mechanism will transport the file
to each of the above machine.   
@end(itemize)

Service : RVD

Description:  The nature of RVD servers recognizes a very different approach
from that of the hesiod discussion.  The RVD mechanism is updated through two
different means.  The first method is for RVD_MAINT (an application interface)
to talk to the RVD server directly.  This program is described in detail
in the section Specialized Management Interfaces.  The important note here
is that the RVD is updated by feeding the server directly with specific
information, not complete files.  The current program vdbdb performs
the updating process to each RVD server.  RVD_MAINT will use the same
protocol.  This process affords instantaneous changes to RVD's.

Secondly, when invoked, RVD_MAINT will also communicate with the sms database.
This communication path will allow the updating of all the fields necessary
to create rvddb, a RVD server specific file.  The generation of this file is
inherent to the DCM.  Every 15 minutes the DCM will check the last change
made to any RVD information.  If information has changed (via RVD_MAINT),
the dcm will invoke a module which creates an rvddb file.  This file
is then propagated to the relevant RVD server.  This file resides on the
target disk and is used in the event of server failure.

During a session with RVD_MAINT an administrator may make several changes
to the RVD server.  These changes will go into effect immediately.   However,
because the DCM is invoked every 15 minutes, should the RVD server crash
during a given 15 minute interval, the consequence will be that the server
will use an older version of rvddb.  Because the rvddb file on the server
is used exclusively for backup, the loss of changed data over a 15 minute
tiem interval is barely significant.

RVD support can best be illustrated by the following diagram:
@blankspace(3 inches)

The discussion which follows describes the generation and contents of the
rvddb file.
@begin(itemize, spread 1)

Propagation interval: 15 minutes, hour aligned

Data Format : RVD specific,  ASCII

Target Machines:

Target Path:

FILE : rvddb
Queries used:
get_rvd_servers(machine) -> (oper, admin, shutdown)
get_rvd_physical(machine) -> (device, size, created, modified)
get_all_rvd)_virtual(machine) -> (name, device, packid, ownert, rocap
(excap, shcap, modes, offset, size, created, modified, ownhost)
get_members_of_list(list) -> (member_type, member_name)
@end(itemize)

Service: NFS

Description:  Sms supports two files which are necessary components of 
NFS operation.   These files are:
@begin(itemize, spread 1)
/site/nfsid

/mit/quota
@end(itemze)

These files reside on the NFS target machine and are used to allocate NFS 
directories on a per user basis.  The mechanism employed is for all
programs to communicate to the sms database, and then for the dcm to handle
the propagation and creating of NFS lockers.  The best illustration of this 
process is indicated by the following example:

During new user registration, a person will sit down to a workstation and
type 'userreg' for his login name.  When validated the user will 
type a 'real' login name and a password.  In addition, the userreg
program will allocate, automatically, for the user a post office and
an NFS directory.  However, the user will not benefit from this
allocation for a maximum of six hours.  This lag time is due to the
operation of sms and its creation of NFS lockers.  During registration,
the userreg program communicates exclusively with
the sms database for NFS allocation.  Since the NFS file generation
is started by the DCM every 6 hours, the real change is not noticed
for a period of time.  When the 6 hour time is reached the DCM will
create the above two files and send them to the appropriate
target servers.  Once on the target machine, the dcm will invoke a shell 
script which reads the /mit/quota file and then creates the NFS directory.
The basic operation of the script is:
@begin(itemize, spread 0)

mkdir <username> - using /mit/quota file

chown  <UID> - using /site/nfsid

setquota <quota> - using /mit/quota
@end(itemize)

@begin(itemize, spread 1)
Propagation interval : 6 hours, 0:00, 6:00, 12:00, 18:00

Data Format : ASCII

Files updated:
/site/nfsid - this file contains the following format:
<username> <UID> <GID1, GID2,...GID16>
where: username is the user's login name (Ex: pjlevine)
@\UID is the users id number (Ex: 123456)
@\GIDn is the groups which the user is a member (max 16)
This file is distributed to every NFS server and contains all users.

QUERY USED:
get_all_logins() -> (login, uid, shell, home, last, first, middle)

/mit/quota - file containing username to quota mapping.
Its format:
<username> <quota>
where: username is the user's login name (Ex. pjlevine)
quota is the per user allocation (in Mbytes)

QUEIES USED:
get_all_nfsphys() -> (machine, dir, status, allocated, size)
get_nfs_quotas(machine, device) -> (login, quota)
@end(itemize)

@begin(itemize, spread 1)

Service: Mail (/usr/lib/aliases)

Description:  The generation of /usr/lib/aliases is a process which makes
use of a currently existing program aliasbld.c.  This program 
is called by the dcm every 24 hours.  The hooks into the sms database
are the significant changes made to aliasbld.c.  The /usr/lib/aliases
file is created and propagated to athena.mit.edu.  Only one file and
one propagation is required.  The use of /usr/lib/alaises file
is done manually by executing a shell script extract_aliases.  The use, 
however, is not an sms-related function or responsibility.

Data Type: ASCII

Propagation interval: 24 hours, 3:00

Target: ATHENA.MIT.EDU:/usr/lib/aliases

Queries used:
(to be furnished)

@end(itemize)
