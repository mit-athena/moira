@Comment[
	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/server.mss,v $
	$Author: danw $
	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/server.mss,v 1.4 2000-01-21 17:20:56 danw Exp $
]
@part(server, root="moira.mss")
@Section(The Moira Server)

All remote @footnote(The DCM and the Moira backup programs, which run on
the host where the Moira database is located, do go through the server,
both for performance reasons and to avoid clogging the server)
communication with the Moira database is done through the Moira server,
using a remote procedure call protocol built on top of GDB@cite(GDB).
The Moira server runs as a single UNIX process on the Moira database
machine.  It listens for TCP/IP connections on a well known service
port (T.B.D.), and processes remote procedure call requests on each
connection it accepts.

One of the major concerns for efficiency in Moira is the time it takes
to start up an application's connection to the server. One of the
limiting factors for Athenareg, Moira's predecessor, is the time it
takes to start up the Ingres back end subprocess which it uses to
access the database.  This was done for every client connection to the
database.  As starting up a backend process is a rather heavyweight
operation, the Moira server will do this only once, at the start up time
of the daemon.

GDB, through the use of BSD UNIX non-blocking I/O, allows the
programmer to set up a single process server which handles multiple
simultaneous TCP connections.  The Moira server will be able to make
progress reading new RPC requests and sending old replies
simultaneously, which is important if a reasonably large amount of
data is to be sent back.

SUN RPC was also considered for use in the RPC layer, but was rejected
because it cannot handle large return values, such as might be
returned by a complex query.

@Section(Access control)
The server performs access control on all queries which might
side-effect the database.  As most information in the database will be
loaded into the nameserver and/or other configuration files, placing
access control on read-only queries is unnecessary.

Because one of the requests that the server supports is a request to
check access to a particular query, it is expected that many access
checks will have to be performed twice: once to allow the client to
find out that it should prompt the user for information, and again
when the query is actually executed.  It is expected that some form of
access caching will eventually be worked into the server for
performance reasons.




