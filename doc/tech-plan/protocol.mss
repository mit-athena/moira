@Comment[
	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/protocol.mss,v $
	$Author: wesommer $
	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/protocol.mss,v 1.3 1987-05-20 15:18:55 wesommer Exp $

	$Log: not supported by cvs2svn $
Revision 1.2  87/05/20  14:39:59  wesommer
Split non-protocol sections into separate parts.


]

@part[protocol, root "sms.mss"]
@Section(The SMS Protocol)

@Comment[ Original version follows ]
@begin(comment)
The SMS protocol allows all clients of SMS to speak a common,
network-transparent language.  This protocol will be the fundamental
interface to SMS for client applications.  This network interface provides
support for remote application programs and the mechanism for communication
between client and server over the network.  A typical scenario might be for
a mail list administrator to make a change in his mailing list from a public
workststion.  This change would be registered in the SMS database.  Once
changed, this information would be available to the Data Control Manager for
use in updating system resources, such as the post office server.
@end(comment)

All communication with the SMS database is done through the SMS
server, using a relatively simple remote procedure call protocol built
on top of GDB@cite(GDB).  SUN RPC was considered but rejected, as it
does not handle easy and asynchronous movement of large quantities of
data such as might be returned by a complicated query.

The SMS server runs as one or more UNIX processes on the SMS database
machine.  It listens for TCP/IP connections on the SMS service port
(T.B.D.), and processes remote procedure call requests on each
connection it accepts.

One of the major concerns for efficiency in SMS is the time it takes
to start up an application's connection to the server. The limiting
factor for Athenareg, SMS's predecessor, is the time it takes to start
up two processes: the user information daemon, and the Ingres back end
subprocess which it uses to access the database.  As starting up a
backend process is a rather heavyweight operation, the SMS server does
this only once, at the start up time of the daemon.

GDB allows the programmer to set up a single process server which
handles multiple simultaneous TCP connections, through the use of
non-blocking I/O.  The SMS server will be able to make progress
reading RPC requests and sending replies simultaneously, which is
important if a reasonably large amount of data (larger than the TCP
window size) is to be sent back.


