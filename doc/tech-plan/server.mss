@Comment[
	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/server.mss,v $
	$Author: wesommer $
	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/server.mss,v 1.1 1987-05-20 14:41:57 wesommer Exp $

	$Log: not supported by cvs2svn $

]
@part(server, root="sms.mss")
@Section(The SMS Server)

The SMS server is the primary interface to the database and for client
applications.  All interaction with SMS is done through this server.  The
server interprets the SMS protocol and invokes INGRES database commands.
Clients will communicate with the SMS server using a well-defined
information protocol.

