@Comment[
	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/protocol.mss,v $
	$Author: wesommer $
	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/protocol.mss,v 1.4 1987-06-23 15:53:50 wesommer Exp $

	$Log: not supported by cvs2svn $
Revision 1.3  87/05/20  15:18:55  wesommer
Added some stuff about GDB and why we're using it.

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
workstation.  This change would be registered in the SMS database.  Once
changed, this information would be available to the Data Control Manager for
use in updating system resources, such as the post office server.
@end(comment)

The SMS protocol is a remote procedure call protocol layered on top of
TCP/IP.  Clients of SMS make a connection to a well known port
(T.B.S.), send requests over that stream, and received replies.  Note:
the precise byte-level encoding of the protocol is not yet specified
here.

Each request consists of a major request number, and several counted
strings of bytes.  Each reply consists of a single number (an error
code) followed by zero or more "tuples" (the result of a query)
each of which consists of several counted strings of bytes.
Requests and replies also contain a version number, to allow
clean handling of version skew.

The following major requests are defined for SMS.  It should be noted
that each "handle" (named database action) defines its own signature
of arguments and results.  Also, the server may refuse to perform any
of these actions based on the authenticated identity of the user
making the request.

@begin(description)
Noop@\Do nothing.  This is useful for testing and profiling of the RPC
layer and the server in general.

Authenticate@\There is one argument, a Kerberos@Cite(KRB) authenticator.
All requests received after this request should be performed on behalf
of the principal identified by the authenticator.

Query@\There are a variable number of arguments.  The first is the
name of a pre-defined query (a "query handle"), and the rest are
arguments to that query.  If the query is allowed, any retrieved data
are passed back as several return values, each with an error code of
SMS_MORE_DATA indicating that there are more tuples coming.

Access@\There are a variable number of arguments.  The first is the
name of a pre-defined query useable for the "query" request, and the
rest are query arguments.  The server returns a reply with a zero
error code if the query would have been allowed, or a reply with a
non-zero error code explaining the reason why the query was rejected.

Shut down server@\Requests that the server cleanly shut down.  This
gets one argument, a string, which is entered into the server log
before the server shuts down as an explanation for the shutdown.

@end(description)
