@part[userreg, root "moira.mss"]
@section(New User Registration)

A new student must be able to get an athena account without any
intervention from Athena user accounts staff.  This is important,
because otherwise, the user accounts people would be faced with having
to give out ~1000 accounts or more at the beginning of each term.

With athenareg, a special program (userreg) was run on certain
terminals connected to timesharing systems in several of the terminal
rooms.  It prompted the user for his name and ID number, looked him up
in the athenareg database, and gave him an account if he did not have
one already.  Userreg has been rewritten to work with Moira; in
appearance, it is virtually identical to the athenareg version (except
in speed).

Athena obtains a copy of the Registrar's list of registered students
shortly before registration day each term.  Each student on the
registrar's tape who has not been registered for an Athena account is
added to the "users" relation of the database, and assigned a unique
userid; the student is not assigned a login name, and is not known to
kerberos.  An encrypted form of the student's ID number is stored
along with the name; the encryption algorithm is the UNIX C library
crypt() function (also used for passwords in /etc/passwd); the last
seven characters of the ID number are encrypted using the first letter
of the first name and the first letter of the last name as the "salt".
No other database resources are allocated at that time.

The Moira database server machine runs a special "registration server"
process, which listens on a well known UDP port for user registration
requests.  There are currently three defined requests:

@Begin(Description)

Verify User@\This operation take the user's first name, last name,
and authenticator as arguments.  The return value will indicate if the
user is found in the database, and if so the user's current status.

Grab Login@\This operation takes the user's first name, last name, and
authenticator with desired login name as arguments.  It will attempt
to assign the user that login name, and reserve the name in the
kerberos database.  Expected results are success or login name already
taken.

Set Password@\This operation takes the user's first name, last name,
and authenticator with desired password as arguments.  It will attempt
to set the user's password in kerberos.

@End(Description)

The authenticator used in the protocol is an encrypted form of the ID
number and any additional arguments.  The simple authenticator is the
ID in plaintext with hyphens removed, with the encrypted ID number
appended to it, and this whole quantity DES encrypted using the
encrypted ID number as a key.  This DES encryption is the error
propagating cypher-block-chaining mode of DES, as described in the
Kerberos document.
@format[
{IDnumber, hashIDnumber}@-{hashIDnumber}
where
	IDnumber is the student's id number (for example: 123456789)
	hashIDnumber is the encrypted ID number (for example: lfIenQqC/O/OE)
]
For the second and third request types, the login or password is also
encrypted:
@format[
{IDnumber, hashIDnumber, login}@-{hashIDnumber}
{IDnumber, hashIDnumber, password}@-{hashIDnumber}
]

The registration server communicates with the kerberos admin_server,
and sets up a secure communication channel using "srvtab-srvtab"
authentication.  In all cases, the server first verifies the request
by decrypting the ID number.

When the student decides to register with athena, he walks up to a
workstation and logs in using the username of "register", password
"athena".  This pops up a forms-like interface which prompts him for
his first name, middle initial, last name, and student ID number.  It
calculates the hashed id number using crypt(), and sends a verify_user
request to the registration server.  The server responds with one of
already_registered, not_found, or OK.

If the user has been validated, userreg then prompts him for his
choice in login names.  It then goes through a two-step process to
verify the login name: first, it tries to get initial tickets for the
user name from Kerberos; if this fails (indicating that the username
is free and may be registered), it then sends a grab_login request.
On receiving a grab_login request, the registration server then
proceeds to register the login name in the Moira database.  If this
succeeds, it then reserves the name with kerberos as well.

Userreg then prompts the user for an initial password, and sends a
set_password request to the registration server, which decrypts it and
forwards it to Kerberos.  At this point, pending propagation of
information to hesiod, the mail hub, and the user's home fileserver,
the user has been established.
