@part[intro, root "moira.mss"]
@MajorSection(Introduction)

Currently, many update tasks and routine service issues are managed
manually.  As the number of users and machines grows, managing the Athena
system becomes significantly more difficult and more economically
unfeasible.  The Athena Service Management System is being developed in
direct response to the problem of supporting the management of an
increasing number of independent workstations.  A network based,
centralized data administrator, Moira provides update and maintenance of
system servers.

The development of Moira addresses centralized administration, distributed
data services, and routine system updates:

@Begin(Itemize, Spread 1)

Conceptually, Moira provides mechanisms for managing Athena servers and
services.  This aspect comprises the fundamental design of Moira.

Economically, Moira provides a replacement for the labor-intensive,
hand-management now associated with maintaining services.

Technically, Moira consists of a database, an Moira server and protocol
interface, a data distribution manager, and tools for accessing and
changing Moira data.

@End(Itemize)

Moira provides a single coherent point of contact for the access and
update of data.  The access of data is performed by means of a standard
application interface.  Programs designed to update network servers,
edit mailing lists, and manage group members all talk to the application
interface.  The programs which update servers are commonly driven by
crontab and act as a server stuffing mechanism.  Applications which are
used as administrative tools are invoked by users.

Two examples of Moira use:

@Begin(Itemize, Spread 1)

One example is for the user accounts administrator to run an
application on her workstation which will change the disk quota
assigned to a user.  She doesn't need to log in to any other machine
to do this, and the change will automatically take place on the proper
server a short time later.

Another example is for a user to run an application to add themselves
to a public mailing list.  Again, the user can run this application on
any workstation.  Sometime later, the mailing lists file on the
central mail hub will be updated to show this change.

@End(Itemize)

This technical plan discusses Moira from a functional standpoint. Its
intention is to establish a relationship between the design of Moira and the
clients which use Moira.

Note that the system has changed names since work began.  Originally
it was simply called SMS, the initials of Service Management System.
The new name is a slightly anglicized spelling of the Greek term for
the fates.  According to the mythology, there are three aspects of
fate: Clotho, who spins the thread of life; Lachesis, who assigned to
each man his destiny in the great tapestry; and Atropos, who cut the
thread at death.  Similarly, the Athena Service Management System
controls the creation of user accounts, the assignment of resources,
and the termination of computer access.  Since the name change has
occured after much code development, the string "sms" still crops up
in some of the code.

@MajorSection(Requirements)

The design criteria and requirements are influenced by the following:

@Begin(Itemize, Spread 1)

Simplicity and clarity of the design are more important than complexity
or speed.  A clear, simple design will guarantee that Moira will be a well
structured product capable of being integrated with other system
resources.  Other systems, such as Hesiod, will provide a speedy
interface to the data kept by Moira; the purpose of Moira is to be the
authoritative keeper of the database, updating slave systems such as
Hesiod as needed.

A simple interface based on existing, tested products.  Wherever
possible, Moira uses existing products.

Moira must be independent of individual services.  Each server receiving
information from Moira requires information with particular data format
and structure. However, the Moira database stores data in one coherent
format.  Through its own knowledge of each server's needs, a data
control manager will access Moira data and convert it to
server-appropriate structure.

Clients must not touch the database directly; that is, they should not
see the database system actually used by Moira.  An application must talk
to an application library.  This library is a collection of functions
allowing access to the database.  The application library communicates
with the Moira server via a network protocol.

Maximize local processing in applications.  Moira is a centralized
information manager. It is not a computing service for local processors.
For efficiency, the Moira protocol supports simple methods of requesting
information; it is not responsible for processing complex requests.
Applications can select pieces of the supplied information, or produce
simple requests for change.
 
Ability for expansion and routine upgrades.  Moira is explicitly
responsible for supporting new information requirements; as new services
are added, the mechanism which supports those services must be easily
added.

The system must provide no direct services, i.e. none at user level, so
that an environment can exist with or without Moira.  (Without it,
however, the economic consequence of managing system services by hand
must be recognized.)  Moira should be reasonably easy to install at other
sites.

Moira must be tamper-proof.  It should be safe from denial-of-service
attacks and malicious network attacks (such as replay of transactions,
or arbitrary "deathgrams").

Moira must be secure.  Authentication will be done using Athena's
Kerberos@cite(KRB) private-key authentication system.  Once the identity
of the user is verified, their right to view or modify data is
determined according to the contents of access control lists (acl's)
which reside with the data.

Fail gracefully.

Moira does not have to be 100% available.  Moira provides timely information 
to other services which are 100% available (Hesiod, Zephyr, NFS).  Once again,
the purpose is to provide a centralized source of authoritative information.

@End(Itemize)
