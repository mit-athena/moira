@part[intro, root "sms.mss"]
@MajorSection(Introduction)

The difficulty of supporting the management of an increasing number of
independent workstations neccesitates the development of the Athena Service
Management System. A network based, centralized data administrator, SMS
provides systematic update and maintenence of system servers.  The attention
of SMS is centered on supporting services for the Athena plant.  Currently,
many update tasks and routine service issues are managed by manual, often
unreliable, methods.  As the physical number of users grow, therefore, the
issue of managing the system becomes significantly more difficult and more
economically unfeasable.

The development of SMS addresses the issue of centralized administration
while providing distributed data services and routine system updates:

@Begin(Itemize, Spread 1)

From a conceptual standpoint, SMS provides mechanisms for managing Athena
servers and services.  This aspect comprises the fundamental design of SMS.

From an economic standpoint, SMS provides a replacement for the
labor-intensive, hand-management now associated with maintaining services.

Technically, SMS consists of a database, an SMS server and protocol
interface, a data distribution manager, and tools for accessing and changing
SMS data.

@End(Itemize)

SMS is a system which provides a single point of contact for the access and
update of data.  The access of data is performed by means of a standard
application interface.  Whether a program is designed to either update
network servers or used as an administrative tool, both talk to the
application interface.  The application programs which update servers are
commonly driven by crontab and act a server stuffing mechanism.
Applications which are used as administration tools are invoked by users to
manipulate a varoety of information.

Two examples of SMS use:

@Begin(Itemize, Spread 1)

The simplest model is to log onto the machine running SMS and run a database
administration program.  The program will use the SMS application library as
its interface to the database.  At a later time, a different application is
executed to retreive the SMS information and distribute it to the specific
servers.

Another SMS model is to run an application (i.e., a mail list administration
program) on a workstation.  In this case, the program accesses the SMS
application library through the SMS network protocol to transmit requests. A
server running on the SMS machine interprets the application's request.  The
database is queried or updated (depending on the client's request), and
database information is sent back to the application.

@End(Itemize)

This technical plan discusses SMS from a functional standpoint. Its
intention is to establish a relationship between the design of SMS and the
clients which use SMS.

@MajorSection(Requirements)

The design criteria and requirements are influenced by the following:

@Begin(Itemize, Spread 1)

Simplicity and clarity of the design is more important than complexity and
speed. The overall design requirement of SMS is to provide the Athena plant
with a well structured product capable of being integrated with other system
resources.  The ability to integrate SMS into the current environment with
reasonable success is based on its focus of clear and simple design.

A simple interface based on existing, tested products.  Wherever possible,
SMS uses existing products as part of the entire development process.
INGRES for example is the database of choice because of its availability and
reliability.

Reliability and accuracy of information are more important than access speed
and response time.  The function of SMS is to maintain a database which
contains relevant, timely information.  However, the access speed can be
slower.  Where speed is required, a different interface to the data (e.g.,
Hesiod) should be used.

SMS must be independent of individual services.  Each server receiving
information from SMS may, in fact, require information with particular data
format and structure. However, the SMS database must store data in one
coherent format.  Through its own knowledge of each server's needs, the data
control manager will access SMS data and convert it to server dependent
structure.

Clients must be database independent; That is they should not depend on a
database system actually used by SMS.  An application must talk to an
application library.  This library is a collection of functions which allow
independent access to the database.  The application library communicates
with the SMS server via a network protocol.

Maximize local processing in applications.  SMS is a centralized information
manager. It is not intended to provide computing service for local
processors.  For efficiency, the SMS protocol must reflect a simple method
of requesting information and not be responsible for processing complex
requests.  Applications can select peices of the supplied information, or
produce simple requests for change.
 
Ability for expansion and routine upgrades.  For new technology and future
enhancements, SMS is implicitly responsible for maintaining new information
requirements.  As new services need support, the mechanism which supports
those services must be easily added.  A clear administrative interface will
allow for future expansion.

The system must be modular.  An environment can exist with or without SMS.
Without it, however, the economic consequence of managing system services by
hand must be recognized.  SMS should be capable of being installed at other
sites.

@End(Itemize)
