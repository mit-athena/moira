@Comment[
	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/protocol.mss,v $
	$Author: wesommer $
	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/protocol.mss,v 1.2 1987-05-20 14:39:59 wesommer Exp $

	$Log: not supported by cvs2svn $

]

@part[protocol, root "sms.mss"]
@Section(The SMS Protocol)

The SMS protocol allows all clients of SMS to speak a common,
network-transparent language.  This protocol will be the fundamental
interface to SMS for client applications.  This network interface provides
support for remote application programs and the mechanism for communication
between client and server over the network.  A typical scenario might be for
a mail list administrator to make a change in his mailing list from a public
workststion.  This change would be registered in the SMS database.  Once
changed, this information would be available to the Data Control Manager for
use in updating system resources, such as the post office server.

