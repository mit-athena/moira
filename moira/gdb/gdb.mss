@device(imprint10)
@make(Report, Form 1)
@style(hyphenation on)
@style(BindingMargin 0)
@style(justification off)
@style(FontFamily ComputerModernRoman10)
@style(footnotes "")
@begin(center)
@majorheading(GDB)
@heading(Global Databases for Project Athena)
@subheading(Noah Mendelsohn)
@subheading(April 17, 1986)
@end(center)
@blankspace(1 cm)
This note is intended as a brief introduction to the Global Database
(GDB)  system being
developed at MIT Project Athena.
GDB is an ongoing effort, still in its early stages, to provide the
services of a high performance shared relational database to the
heterogeneous systems comprising the Athena network.  Specifications
have been developed for a set of library routines to be used by
@i[clients] to access the database.  
Current plans are to use the Ingres relational database product from
RTI as a local data manager, but to support access via the client
library from any Berkeley Unix@+[TM]@foot(@+[TM]Unix is a trademark of
AT&T Bell Laboratories) system in the internet.  Though early
versions will manage only a single copy of any given relation,
replication may be added at some point in the future.  
In the meantime the client library provides a uniform framework for
writing database applications at Athena.

While designing the client library it became apparent that many of its
underlying services for structured data storage and transmission would
be of value for a variety of applications.  Most of these interfaces
have been exposed, and the GDB project has undertaken as a secondary
goal the development of these simple services for structured data
maintenance and transmission.

@section(Raison d'etre@+[1]@foot[@+[1]with apologies for lack of accents in the
font!])

The GDB project was motivated by the observation that Athena
applications tend to exploit the computational and display services of
the system much more effectively than they use the network.
Furthermore,
those applications which do use the network tend to have strong
machine type affinities, running comfortably on either a Vax or an
RT/PC,
but rarely both.  Indeed, the @i[strategic] Athena database system is
currently unavailable on the RT/PC's.

Of the many unexplored uses of the network, globally accessible
databases seem to have great value in a variety of disciplines, and
they are also badly needed for certain aspects of Athena
administration.  By providing well architected services for global
data sharing, we hope to achieve at least two goals: (1) set the
precedent that user written applications and Athena supplied services,
like @b[madm], @b[chhome], and @b[passwd], run compatibly 
from any machine in the network, and (2) encourage
the development of new database applications by eliminating the need
for individual projects and departments to develop their own
transmission and encapsulation protocols.

@section(Implementation Goals)

The following goals have been established for the architecture and
implementation of GDB:

@begin(itemize)
Access to databases stored on incompatible machines (e.g. RT/PC to
Vax) should be supported transparently.

Multiple databases, possibly at several sites,  should be accessible
simultaneously.  The ability to do concurrent activity on the several
databases is desirable.

Appropriate facilities for managing structured data returned from the
database should be provided for programmers (e.g. access fields by
name.)

Asynchronous operation should be supported, for several reasons:
@begin(itemize)
Required for control of simultaneous access to multiple databases.

Needed for graceful interruption of long-running or erroneous
requests.

Facilitates pipelining of requests, thereby maximizing overlap of
server and client processing.
@end(itemize)

When the internal interfaces used for session control and data
transmission can be generalized without adding unnecessarily to their
complexity, then those interfaces should be documented and exposed.
@end(itemize)

@section(Implementation Strategy)

Several approaches to achieving these goals were considered, and an
implementation strategy has been chosen.  

One approach to achieving the required function would be to rely on
the appearance of RTI products containing the necessary facilities.
At the very least, we would need a full function Ingres port to the
4.2 system on the RT/PC.  RTI would further have to extend Ingres for
access to databases through the internet, and they would have to
support such access across multiple machine types.  These extensions
would give us a core of function suitable for limited application,
though we would have to see whether flexibility and performance were
truly appropriate for our needs.  If RTI should come forward with a
commitment to produce these products within the next few months, then
need for the libraries described herein might not be so great.
Lacking such products from RTI, it seems essential that we carry
forward with a strategy for database access from @i[all] of the
workstations in the Athena network.

Having decided to do at least some of the necessary work ourselves,
several implementation strategies are possible.  One, which is
currently being pursued by Roman Budzianowski, is to interpose the
appropriate transmission services between the RTI Ingres front end and
back end.  This technique has a number of interesting advantages, and
some disadvantages.  The primary advantage is the ability to run
existing Ingres applications, including some of the forms and query
facilities, through the network.  Also, Roman reports that he has
succeeded in running an interesting subset of applications without too
much effort.  The disadvantages of Roman's approach are the lack of 
a strategy for supporting non-Vax machines until RTI comes out with
the appropriate base products, and the dependence on undocumented
interfaces.   
In some cases, the front-end and the back-end are sharing files, while
in others signals seem to be sent.  It remains to be seen how successfully
we can divine these undocumented interfaces, how stable they remain
over time, and whether--given an RTI Ingres on the RT/PC--we can
figure out how to do the right byte swapping on the binary data sent
through Ingres' pipes and files.  Our conclusion is that Roman's
effort should continue, because it can
achieve valuable results without excessive effort.
Nevertheless, this scheme falls short of our requirement for balanced
support of all the machines in the Athena network, so we recommend an
alternate implementation as the primary base for Athena application
development. 

A machine independent access method for relational databases could be
constructed in many different ways.  One technique we have considered
and rejected is to base an implementation on the RPC prototype
developed by Steve Miller and Bob Souza.  While RPC is
convenient, and the prototype appears to be of very high quality, it
fails to meet our needs in several crucial areas.  We have concluded
that asynchronous interaction between clients and servers is
essential for performance, for parallel execution of multiple queries,
and for interruptibility of ongoing operations.  Synchronous RPC seems
ill-suited to these requirements.  A secondary concern is the lack of
support for procedure calls across heterogeneous architectures in the
current version of the RPC prototype.  The right hooks are supposedly
there, but the necessary alignment and type conversion routines have
not been built.  Indeed, the prototype has yet to be ported to the
RT/PC.

As a result of this analysis, we have designed a system which uses RTI
Ingres for the things it does (or purports to do) well, and we have
added a flexible, asynchronous transport mechanism for transmission of
structured data between heterogeneous processors.  The specification,
outlined in a separate document, includes libraries for creation and
management of tuples and relations in virtual memory, along with a
simple mechanism for typing the fields comprising a tuple.  Layered
upon these are services for transmitting fields, tuples and relations
through the internet, doing the necessary conversions and
re-alignments when moving between incompatible machines.  These
services, in turn, are used by a library which provides almost
all of the services of Ingres EQUEL to clients throughout the network.

@section(Project Status)

The specification for the interfaces to the client library routines is
available in draft form and is now being refined.  In parallel, design
is proceeding on the protocol to be used between the sites, and on the
related software structures used to encapsulate and parse the
transmitted data.  The design does include a simple but flexible
proposal for managing asynchronous activities.  Coding will start soon
on those pieces of the library which seem to be stable;  refinement of
the other parts will take a few more weeks.  While our rate of
progress will depend greatly on the number of people doing the work
and on their other responsibilities (neither of which are clear at
this time), I'm optimistic that a basic implementation will start
showing signs of life within a couple of months, with polishing taking
a bit longer.
