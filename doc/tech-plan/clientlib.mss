@Comment[
	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/clientlib.mss,v $
	$Author: wesommer $
	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/clientlib.mss,v 1.2 1987-06-18 15:53:41 wesommer Exp $

	$Log: not supported by cvs2svn $
Revision 1.1  87/05/20  14:40:45  wesommer
Initial revision

]
@part(clientlib, root="sms.mss")
@Section(The Application Library)

@begin[comment]
The SMS application library consists of functions which application programs
use to communicate with the SMS database.  The library contains routines
which allows the client easy access to SMS data.  The application library
uses the SMS protocol for modularity and consistency.
@end[comment]
The SMS application library provides access to SMS through a simple
set of procedure calls.  The library is layered on top of GDB, and
itself consists of several layers.

@subsection (Error Handling) 

Because of all the possible failure points in a networked application,
we decided to use Ken Raeburn's @t[libcom_err] library.  Com_err
allows several different sets of error codes to be used in a program
simultaneously - every error code is an integer, and each error table
reserves a subrange of the integers (based on a hash function of the
table name).  UNIX system call error codes are included in this
system.  By convention, zero indicates success, or no error.  The
following routines may be useful to applications programmers who wish
to display the reasons for failure of a routine.

@begin(programexample)
char *error_message(code)
@\int code;
@end(programexample)
Returns the error message string associated with @t[code].

@begin(programexample)
void com_err(whoami, code, message)
@\char *whoami;@\@\/* what routine encountered the error */
@\int code;/* An error code */
@\char *message; /* printed after the error message */
@\int code;
@end(programexample)
By default, prints a message of the form 
@begin(display)
@i<whoami>: @i<error_message(code)> @i<message> @i<newline>
@end(display)
If @t[code] is zero, nothing is printed for the error message.

@begin(programexample)
void set_com_err_hook(hook)
@\void (*hook)();@\@\/* Function to call instead of printing to stderr */
@end(programexample)

If this routine is called with a non-NULL argument, it will cause
future calls to @t[com_err] to be directed into the hook function
instead.  This can be used to, for example, route error messages to
@t[syslog] or to display them using a dialogue box in a window-system
environment.

@section(Low level calls)
The lowest level library contains the following routines:

@begin(programexample) 
int sms_connect(); 
@end(programexample)

Connects to the SMS server.  This returns an error code, or zero on
success.  This does not attempt to authenticate the user, since for
simple read-only queries which may not need authentication, the
overhead of authentication can be comparable to that of the query.
This can return a number of operational error conditions, such as
ECONNREFUSED (Connection refused), ETIMEDOUT (Connection timed out),
or SMS_ALREADY_CONNECTED if a connection already exists.

@begin(programexample)
int sms_auth();
@end(programexample)
Attempts to authenticate the user to the system.  It can return
Kerberos failures, either local or remote (for example, "can't find
ticket" or "ticket expired"), SMS_NOT_CONNECTED if @t[sms_connect] was
not called or was not successful, or SMS_ABORTED if the attempt to
send or recieve data failed (and the connection is now closed).

@begin(programexample)
int sms_disconnect();
@end(programexample)
This drops the connection to SMS.  The only error code it currently
can return is SMS_NOT_CONNECTED, if no connection was there in the
first place.

@begin(programexample)
int sms_noop();
@end(programexample) 
This attempts to do a handshake with SMS (for testing and performance
measurement).  It can return SMS_NOT_CONNECTED or SMS_ABORTED if not
successful.

@begin(programexample)
int sms_query(name, argc, argv, callproc, callarg)
@\char *name;@\@\/* Name of query */
@\int argc;@\@\/* Number of arguments provided */
@\char *argv[];@\@\/* Argument vector */
@\int (*callproc)();@\@\Routine to call on each reply */
@\caddr_t callarg;@\@\/* Additional argument to callback routine */
@end(programexample)
This runs an SMS query named @[name] with arguments 
@t{argv[0]}...@t{argv[argc-1]}.  For each returned tuple of data, 
@t[callproc] is called with three arguments: the number of elements in
the tuple, a pointer to an array of characters (the data), and
@t[callarg].

@Subsection(Higher level library) The upper layer consists of
query-handle specific routines, such as "retrieve finger information
by login name" or "get all servers".  The actual specifications for
these routines will be somewhat fluid depending on exactly what query
handles are available; it is intended that these routines should be
easy to write in terms of the previous library.
