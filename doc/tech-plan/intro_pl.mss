@part[intro, root "sms.mss"]
@MajorSection(Introduction)

Currently, many update tasks and routine service issues are managed
manually.  As the number of users and machines grows, managing the Athena
system becomes significantly more difficult and more economically
unfeasible.  The Athena Service Management System is being developed in
direct response to the problem of supporting the management of an
increasing number of independent workstations.  A network based,
centralized data administrator, SMS provides update and maintenance of
system servers.

The development of SMS addresses centralized administration, distributed
data services, and routine system updates:

@Begin(Itemize, Spread 1)

Conceptually, SMS provides mechanisms for managing Athena servers and
services.  This aspect comprises the fundamental design of SMS.

Economically, SMS provides a replacement for the labor-intensive,
hand-management now associated with maintaining services.

Technically, SMS consists of a database, an SMS server and protocol
interface, a data distribution manager, and tools for accessing and
changing SMS data.

@End(Itemize)

SMS provides a single coherent point of contact for the access and
update of data.  The access of data is performed by means of a standard
application interface.  Programs designed to update network servers,
edit mailing lists, and manage group members all talk to the application
interface.  The programs which update servers are commonly driven by
crontab and act as a server stuffing mechanism.  Applications which are
used as administrative tools are invoked by users.

Two examples of SMS use:

@Begin(Itemize, Spread 1)

The simplest example is to run a database administration program on the
host running SMS.  The program will use the SMS application library as
its interface to the database.  At a later time, a different application
is executed to retrieve the SMS information and distribute it to the
specific servers.

Another example is to run an application (i.e., a mailing list
administration program) on a workstation.  In this case, the program
transmit requests to the SMS application library using the SMS network
protocol.  A server running on the SMS host interprets the application's
request.  The database is queried or updated (depending on the client's
request), and database information is sent back to the application.

@End(Itemize)

This technical plan discusses SMS from a functional standpoint. Its
intention is to establish a relationship between the design of SMS and the
clients which use SMS.

@MajorSection(Requirements)

The design criteria and requirements are influenced by the following:

@Begin(Itemize, Spread 1)

Simplicity and clarity of the design are more important than complexity
or speed.  A clear, simple design will guarantee that SMS will be a well
structured product capable of being integrated with other system
resources.  Other systems, such as Hesiod, will provide a speedy
interface to the data kept by SMS; the purpose of SMS is to be the
authoritative keeper of the database, updating slave systems such as
Hesiod as needed.

A simple interface based on existing, tested products.  Wherever
possible, SMS uses existing products.

SMS must be independent of individual services.  Each server receiving
information from SMS requires information with particular data format
and structure. However, the SMS database stores data in one coherent
format.  Through its own knowledge of each server's needs, a data
control manager will access SMS data and convert it to
server-appropriate structure.

Clients must not touch the database directly; that is, they should not
see the database system actually used by SMS.  An application must talk
to an application library.  This library is a collection of functions
allowing access to the database.  The application library communicates
with the SMS server via a network protocol.

Maximize local processing in applications.  SMS is a centralized
information manager. It is not a computing service for local processors.
For efficiency, the SMS protocol supports simple methods of requesting
information; it is not responsible for processing complex requests.
Applications can select pieces of the supplied information, or produce
simple requests for change.
 
Ability for expansion and routine upgrades.  SMS is explicitly
responsible for supporting new information requirements; as new services
are added, the mechanism which supports those services must be easily
added.

The system must provide no direct services, i.e. none at user level, so
that an environment can exist with or without SMS.  (Without it,
however, the economic consequence of managing system services by hand
must be recognized.)  SMS should be reasonably easy to install at other
sites.

SMS must be tamper-proof.  It should be safe from denial-of-service
attacks and malicious network attacks (such as replay of transactions,
or arbitrary "deathgrams").

SMS must be secure.  Authentication will be done using Athena's
Kerberos@cite(KRB) private-key authentication system.  Once the identity
of the user is verified, their right to view or modify data is
determined according to the contents of access control lists (acl's)
which reside with the data.

Fail gracefully.

Does not require 100% availability.  SMS is intended to work with
other services (i.e., Hesiod, Kerberos) which do provide 100%
availability to critical information.


@End(Itemize)

