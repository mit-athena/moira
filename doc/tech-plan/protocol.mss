@Comment[
	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/protocol.mss,v $
	$Author: danw $
	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/protocol.mss,v 1.5 2000-01-21 17:20:56 danw Exp $
]

@part[protocol, root "moira.mss"]
@Section(The Moira Protocol)

@Comment[ Original version follows ]
@begin(comment)
The Moira protocol allows all clients of Moira to speak a common,
network-transparent language.  This protocol will be the fundamental
interface to Moira for client applications.  This network interface provides
support for remote application programs and the mechanism for communication
between client and server over the network.  A typical scenario might be for
a mail list administrator to make a change in his mailing list from a public
workstation.  This change would be registered in the Moira database.  Once
changed, this information would be available to the Data Control Manager for
use in updating system resources, such as the post office server.
@end(comment)

The Moira protocol is a remote procedure call protocol layered on top of
TCP/IP.  Clients of Moira make a connection to a well known port
(T.B.S.), send requests over that stream, and received replies.  Note:
the precise byte-level encoding of the protocol is not yet specified
here.

Each request consists of a major request number, and several counted
strings of bytes.  Each reply consists of a single number (an error
code) followed by zero or more "tuples" (the result of a query)
each of which consists of several counted strings of bytes.
Requests and replies also contain a version number, to allow
clean handling of version skew.

The following major requests are defined for Moira.  It should be noted
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
MR_MORE_DATA indicating that there are more tuples coming.

Access@\There are a variable number of arguments.  The first is the
name of a pre-defined query useable for the "query" request, and the
rest are query arguments.  The server returns a reply with a zero
error code if the query would have been allowed, or a reply with a
non-zero error code explaining the reason why the query was rejected.

Trigger_DCM@\This takes no arguments.  It will request the Moira server
to immediately spawn a DCM process.  Access checking is done by
checking permissions for the pseudo-query "trigger_dcm" (tdcm).

@end(description)
