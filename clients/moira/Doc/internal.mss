@Comment($Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/moira/Doc/internal.mss,v 1.1 1996-07-02 02:01:45 danw Exp $)
@Make(Manual)
@Device(PostScript)
@PageFooting(center "Draft of @value(date)")
@style(spacing = 1.5, indent 5, FontFamily = TimesRoman, size = 12)
@style(LeftMargin 1 inch, RightMargin 1 inch)

@Begin(TitlePage)
@Begin(TitleBox)
@b[The Athena Service Management System]
@blankspace ( 1 line )
@b[SMS Client Internals Document]
@blankspace ( 1 line )
@i[Chris D. Peterson]
@End(TitleBox)
@CopyrightNotice(Massachusetts Institute of Technology)
@End(TitlePage)

@Chapter(Introduction)

The @i(Sms Client) program was designed to allow ordinary users a
simple method of performing all database manipulations common here at
@i(Project Athena).  The user interacts with the program through a
terminal based menu.  This allows use of sms on a wide variety of
platforms.  Unlike previous packages where many different clients were
necessary to perform all these tasks, the @i(Sms Client) program
allows all operations to be performed from one application.  This
saves the user time and effort by having all common operations in one
convenient place.  I have spent a good deal of time cleaning up the
code to make it uniform throughout the entire client and am writing
this document to explain some of the decisions. My hope is that this
will allow you to add features with a minimum of amount effort, as
well as easing the transition between your code and mine.

@Chapter(Overview)

This section presents general concepts that hold throughout the code.
Some of the items may seem trivial, but together they keep the code
consistent and understandable.

@Section(File Organization)

Most files correspond to one type of item in the @i(SMS) database,
such as lists, machines, clusters, etc.  These files are self
contained so that you can pick and choose which of these you would
like to use in any new client you decide to build. There are a few
special files that do not follow this convention.
@begin(description)

@i[util.c]@\Utility functions used by many of the other files.

@i[menu.c]@\Menu definitions for the entire application.

@i[main.c]@\The main driver that handles application start up.

@i[delete.c]@\Functions that handle deletion of lists and users.

@i[globals.c]@\All global variable definitions.
@end(description)

@Section(Command Organization)

Each menu item has one and only one function associated with it,
although the same function may be used by two identical menu items.
The arguments passed to these functions from the menu package are an
array of character strings and the number of strings in the array.
The first argument in the array (argv[0]) is the name of the menu item
that has been selected.  Thus the actual user specified arguments are
passed starting at the second argument (argv[1]).

@Section(Naming and Comment Conventions)

@Index(Variables)
@Index(Functions)

I use all capital letters for constants, while variables are all small
letters, and functions are represented by mixing capital and small
letters, capitalizing each word in the function name.  I use long
function names as functions are usually not called often, and long
names are much more descriptive.  There are a few functions that do
not follow the mixed caps and small convention, these are generally
those I called out of @i(SMSLib), @i[CLib], or the menu package.

@Index(Comments)

Each file has a header containing the Revision Control System (RCS)
modification time and some history information, as well as some
information about the file.  The template for this is a file named
'header'.  Each function has a comment preceding it describing the
function, all variables used, and the value that this function
returns.  Notes of special concern for this function are also listed
here.  The function template is in a file called, surprisingly enough,
'function'.  Other comments are placed where appropriate.

@Section(Retrieving data from the server)
@Label(queue-def)

Since one of the limitations of @i[SMS] is that no query may be
executed recursively, all information returned by a query must be
received before another query can be executed.  To facilitate
programming simplicity I store the information returned from each
query into a queue as soon as it is received.  This allows a simple,
although rather memory intensive, method of getting around this
problem.  Another advantage of this method is that since no operations
are taking place on the data as it comes back from the server the time
that the server spends sending the data to the clients is kept to a
minimum, a necessity in large-scale environments.

@Chapter[Application Wide Definitions]

This section will cover all items that cross function boundaries,
including global variables, structure definitions, and constant
definitions.

@Section[Global Variables]

@Label(Globals)The @i[SMS Client] has a few global variables.  Since
you will run into them throughout the application they warrant mention
here.
@Index(program_name)
@Index(user)
@Index(verbose)
@Index(found_some)
@begin(description)
@i[program_name] @\This is the name of the program currently being
executed, and is retrieved from argv[0] in @i[main.c].

@i[user] @\This is the name of the user currently executing the program,
and is also set in @i[main.c].

@i[verbose] @\This is a boolean value that, when FALSE turns off the
delete confirmation.  For more information see section @ref(confirm).

@i[found_some] @\This is a boolean that allows the @b[PrintByType()]
(sect. @ref(pbytype) function to be able to communicate with the
function calling the query that invokes it, as well as among calls to
itself.
@end(description)

@Section[Structure Definitions]

@Index(qelem)

The only global structure definition is the one for the queue element.
What I will refer to as the queue is actually a doubly linked list
that all my queue manipulation functions (sect. @ref(queuecount))
treat as a true queue.

@begin(Format)
struct qelem {
  struct    qelem *q_forw;
  struct    qelem *q_back;
  char * q_data;
};
@end(Format)

The queue data need not be a string; @i[char *] is used as a generic
pointer here.

@Section[Definitions]

@Index(FVoid)
@Index(FInt)
@Index(FCharStar)

For readability there are a few important global definitions.
Function pointers in @i[C] tend to be tough to read so I have typedef'ed the
function pointers; FVoid, FInt, and FCharStar.  These refer to functions
that return nothing, integers, and strings, respectively.

@Index(MOD_FORMAT)

Another global definition, MOD_FORMAT, is used to format the
modification information of an item for printing.  It is important that
all modification information is presented to the user with this
format, to keep the @i[SMS Client] consistent.

@Index(Infodefs)
@Label(infodefs)

Information about an item in the database is returned by sms_query()
in an array of strings.  Remembering what @i[info][7] is can be
difficult and it is possible that in future updates of the @i[SMS
Server] @i[info][7] may contain completely different information.  To
solve both of these problems I have created a set of definitions for
each element of the argument list of each data type that the @i[SMS
Client] uses.  A list of these can be found in appendix
@ref(a_infodefs), as well as the file @i[infodefs.h].

@Chapter(Utility Functions)

I created several utility functions that handle common operations used
by many of the functions in the @i[SMS Client].  These functions
perform things like the storing of information in a queue as it comes
in from the @i(SMS Server), queue manipulation, querying the user, 
and printing information.

@Section(Storing and Freeing the Queue) 

As noted in section @ref(queue-def), all incoming information from the
@i(SMS Server) is stored in a queue as soon as it is received.  The
function that does this, @b[StoreInfo()], as well the one that frees the
queue when we are finished with it, @b[FreeQueue()], are described here.

@Index(StoreInfo) 

Use the function @b[StoreInfo()] to store all information received
from the @i[SMS Server] into a queue.  Space for this queue is
dynamically allocated using malloc().  The actual information is
stored in a NULL terminated array of strings.  The number of strings
in this array can be easily found with the function @b[CountArgs()]
(sect. @ref(countargs)).
@begin(format)

int
StoreInfo(@i[argc, argv, data])
int @i[argc];
char ** @i[argv];
char * @i[data];
@end(format)
@begin(Description)

@i[argc] @\The number of strings passed in argv.

@i[argv] @\An array of strings to be stored in the queue.

@i[data] @\A pointer to the previous element in this queue, or NULL if@\it
should create a new queue.  This is updated with each call to @b[StoreInfo()]
and the value associated with this pointer is the last element in the
queue.

@end(Description)
@blankspace(1 line)
Here is an example of an @i[SMS] query that would use this function to store
its arguments.

@Begin[Format]

struct qelem * elem = NULL;
status = sms_query("query handle", num_args, args, StoreInfo, &elem);

@End[Format]

The variable elem must initially be set to NULL, or @b[StoreInfo()]
will try to add the first element of this queue to a currently
existing queue that is really just the garbage left on the stack.  The
value of elem when this query returns is the last element in the
queue, and the function @b[QueueTop()] (section @ref(queuetop)) should
be used to get the top element of the queue, which is expected by most
other queue operations.

@Center(________)

@Index(FreeQueue)

To free the queue and any data associated with it, the function
@b[FreeQueue()] is used.  This function expects a queue similar to the
one created by the @b[StoreInfo()] function.  @b[FreeQueue()] frees
all memory associated with this particular queue, and will do bad
things if the information stored in the queue is not a NULL
terminated, allocated array of allocated strings.  All allocations
are assumed to have been done with the @i[C] malloc() function.

@begin(Format)

void
FreeQueue(@i[elem])
struct qelem * @i[elem];
@end(Format)
@begin(Description)

@i[elem] @\Any element in the queue.  The entire queue will be freed
regardless of the element passed.
@end(Description)

@Section[Queue Manipulation]

The following group of functions perform common tasks that involve
retrieving information from or manipulating a queue.  All these functions
will handle NULL passed to them in an appropriate manner, either returning
zero, or NULL.

@Label(queuetop)
@Index(QueueTop)

To find the top element of a queue use the function @b[QueueTop()].  

@begin(Format)

struct qelem * 
QueueTop(@i[elem])
struct qelem * @i[elem];
@end(Format)
@begin(Description)
@i[elem] @\Any element in the queue.
@end(Description)

@blankspace(1 line)

@b[QueueTop()] returns the top element of the queue defined by its
argument.

@Center(________)


@label(queuecount)
@Index(QueueCount)

The function @b[QueueCount()] counts the number of elements in a queue.

@begin(Format)
int
QueueCount(@i[elem])
struct qelem * @i[elem];
@end(Format)
@begin(Description)
@i[elem] @\Any element in the queue.
@end(Description)

@blankspace(1 line)

@b[QueueCount()] returns the number of elements in the queue defined
by it argument, it need not be the top element, any
element will do.

@Section[Array Manipulation]

The arrays of strings have some manipulation functions of their own.
All arrays used in the @i[SMS Client] are assumed to be NULL terminated. 
In general this is no problem as @b[StoreInfo()] creates NULL terminated
arrays.

@Label(countargs)
@Index(CountArgs)

@b[CountArgs()] counts the number of elements in a NULL
terminated array of strings.

@begin(Format)
int
CountArgs(@i[info])
char ** @i[info];
@end(Format)
@begin(description)
info @\A NULL terminated array of character strings.
@end(description)

@blankspace(1 line)

@b[CountArgs()] returns the number of strings in the NULL terminated array
of strings passed to it.  A common use of @b[CountArgs()] is to tell
sms_query() how many items are in the argument list passed to the
query, as shown below.
@begin(format)

status = sms_query("query", CountArgs(args), args, StoreInfo, &elem);
@end(format)

@Center(________)
@Index(FreeInfo)

To free all elements in a NULL terminated array of strings use the 
@b[FreeInfo()] function.

@begin(Format)
void
FreeInfo(@i[info])
char ** @i[info];
@end(Format)

@begin(Description) 

@i[info] @\A NULL terminated array of strings allocated with malloc(),
to be freed.
@end(Description)

@Center(________)


@Label(sinn)
@Index(SlipInNewName)

Most update queries use the same argument list that is returned by the
information query for the database item is question.  For instance,
update_list uses almost the same arguments that get_list_info returns.
The difference in these arguments lists is that update queries expect
the new name to be the second element in the list.  The
@b[SlipInNewName()] function adds the new name as the second argument
in the list passed to it, bumping all the other arguments down one
slot.

@begin(Format)
void
SlipInNewName(@i[info, name])
char ** @i[info];
char * @i[name];
@end(Format)

@begin(Description)
@i[info] @\A NULL terminated array of strings.

@i[name] @\The new name to be slipped into @i[info] at position @i[info][1].
@end(Description)

@Blankspace(1 line)

Once @b[SlipInNewName()] has been performed on a argument list do not
try to use the definitions for that type of element,  as they will be
wrong.  Don't use @i[info][L_ACE] to try to retrieve the ace of the
list, as this function has shifted all the items except the old name
down one element.

@Section[Special Queue Functions]

The following two functions @b[Loop()] and @b[QueryLoop()] are used
when an operation needs to be performed on every element of the queue.
The difference between these two functions is that @b[QueryLoop()]
presents a method of asking the user if the operation should be
performed, while @b[Loop()] just performs it on each element.

@Index(Loop)
@Index(LoopFunc)

To perform an operation on every element in the queue use the
@b[Loop()] function.

@begin(Format)
void
Loop(@i[top, LoopFunc])
struct qelem * @i[top];
FVoid @i[LoopFunc];
@end(Format)
@begin(Description)
@i[top] @\The top element of the queue.

@i[LoopFunc] @\The function to perform on each element of the queue.
@end(Description)

@blankspace(1 line)

The @i[LoopFunc] has the following format and is performed with the
data of each element in the queue.

@begin(Format)
void
LoopFunc(@i[info])
char ** @i[info];
@end(Format)
@begin(Description)
@i[info] @\The array of strings that is stored as the @i[q_data] of each element
of the queue.
@end(Description)

@blankspace(1 line)

No provision is made for the @i[LoopFunc] function to return a value.

@Center(________)


@Label(qloop)
@Index(QueryLoop)
@Index(PrintFunc)
@Index(QueryLoopFunc)

If you want to query the user before your function is called on each
element of the queue, use the @b[QueryLoop()] routine.

@Begin[Format]
void
QueryLoop(@i[top, PrintFunc, QueryLoopFunc, query_string])
struct qelem * @i[top];
FCharStar @i[PrintFunc];
FVoid @i[QueryLoopFunc];
char * @i[query_string];
@End[Format]
@begin(Description)
@i[top] @\The top element of the queue.

@i[PrintFunc] @\The function that will print information to the user
about each specific element of the queue, its return value is appended
to the query string, and asked as a @b[YesNoQuitQuestion()] (sect.
@ref[ynqquestion]) of the user.

@i[QueryLoopFunc] @\This function is called  if the user answers yes to the
question asked of him, or if there is only one element in the queue.

query_string @\Basic prompt for the @b[YesNoQuitQuestion()] asked of
the user, the string returned by the @i[PrintFunc] is appended to this
string to form the full prompt.
@end(Description)

@blankspace(1 line)

@Index(PrintFunc)

@i[PrintFunc] is called using the data stored in each element of the
queue, unless there is only one element in the queue, in which case
@i[QueryLoopFunc] is called without querying the user. @i[PrintFunc]
has the following format.

@begin(Format)
char *
PrintFunc(@i[info])
char ** @i[info];
@end(Format)
@begin(Description)

@i[info] @\The array of strings that is stored as the @i[q_data] of
each element in the queue.
@end(Description)

@blankspace(1 line)

The @i[PrintFunc] returns a string that is to be appended to the
query_string() of the @b[QueryLoop()] function.  This string is then
used to ask the user if the @i[QueryLoopFunc] should be called.  The
return string from the @i[PrintFunc] is usually just the name of the
item contained in the data field of each queue element.

@Index(QueryLoopFunc)

@i[QueryLoopFunc] is called each time the user answers 'yes' to the
question asked of him, with the current element's information.  The
only exception to this is if the queue has only one element, in which
case @i[QueryLoopFunc] is called immediately with @i[one_item] set to TRUE.
@i[QueryLoopFunc] has this format.

@begin(Format)
void
QueryLoopFunc(@i[info, one_item])
char ** @i[info];
Bool @i[one_item]
@end(Format)
@begin(Description) 

@i[info] @\The array of strings that is stored as the @i[q_data] of
each element in the queue.

@i[one_item] @\This is TRUE if this function is being called without asking
the user, because there is only one item in the queue.
@end(Description)

@blankspace(1 line)

No provision is made for the @b[QueryLoop()] function to return a value.

@b[QueryLoop()] automatically performs the @i[QueryLoopFunc] without
calling the @i[PrintFunc] if there is only one item in the queue, but
the @i[QueryLoopFunc] is alerted to this fact by the @i[one_item]
variable.  If there is more than one item on the queue the the
@i[PrintFunc] is executed, and the user is prompted, as in the
following example.

@begin(format)
@tabset(10)

@b[Source Code:]

QueryLoop(top_of_queue, PrintListInfo, UpdateList, "Would you like to update list");

@tabclear()
@tabset(50, 60)
@b[User Sees:]

List: foo  @\Members: @\35
This list is a group@\GID: @\9803

Would you like to update list foo (y/n/q) [n]: 
@end(format)

This question is asked of the user for every item in the queue, and if
the answer is yes then the @i[QueryLoopFunc] is called.  If the answer
is no then it continues onto the next element, quit drops out of the
loop entirely.

@Index(NullPrint)

The return value of the @i[PrintFunc] is the name of the list in the
above example (i.e. foo).  A simple print function, @b[Nullprint()],
is detailed in section @ref(nullprint). If @b[Nullprint()] were used
in the above example following would be the result:

@blankspace(1 line)

@begin(Format)
Would you like to update list foo (y/n/q) [n]: 
@end(Format)

@Section[Generic @i{SMS} query routines]

These two very common routines are used as sms_query() callbacks.
Using these common routines helps other programmers immediately
identify what type of tuples your query is expected to return.

@Index(Scream)

Use the @b[Scream()] function when you do not expect any tuples to be
returned by the @i[SMS Server], as in an update, add, or delete query.

@begin(Format)
int
Scream()
@end(Format)

A Sample use of this function.

@begin(Format)
status = sms_query("update_list", CountArgs(args), args, Scream, NULL);
@end(Format)

@Center(________)


@Index(NullFunc)

@b[NullFunc()] does nothing at all with the tuples, it just ignores them.

@begin(Format)
int
NullFunc()
@end(Format)

@b[NullFunc()] is useful when you are expecting tuples to be returned,
but are not interested in their contents, I used it when I was testing
for existence of a list by performing a count_members_of_list query
and then testing the return value, but was not interested in the
tuples. A sample use of this @b[NullFunc()].

@begin(Format)
status = sms_query("get_list_info", CountArgs(args), args, NullFunc, NULL);
@end(Format)

@Section[Retrieving User Input]

To keep a consistent user interface across the entire application, the
@i[SMS Client] uses the following functions to handle most user input.
The only other accepted method of retrieving user input is by making
a call to @b[Prompt_input()], which is in the menu library, and the
basis for all these functions.

@Index(PromptWithDefault)

Use @b[PromptWithDefault()] when you need to prompt the user for a value
and you want to provide him with a default value.

@begin(Format)
int
PromptWithDefault(@i[prompt, buf, buflen, def])
char * @i[prompt], * @i[buf];
int @i[buflen];
char * @i[def];
@end(Format)

@begin(Description)
prompt @\The string to prompt the user with.

buf @\The buffer to return the answer in.

buflen @\The length of the return buffer.

def @\The default return value (if NULL then "" is the default).

@End[Description]

@Blankspace( 1 line)

A typical call to this function is as follows:
@begin(format)

@b[Source Code:]

PromptWithDefault("Input a value for the string", buf, BUFSIZ, "foo");

@b[User Sees:]

Input a value for the string [foo]: 

@end(format)

@b[PromptWithDefault()] will return (-1) if the user breaks out of this
prompt with control-C or some other break character.

@Center(________)


@Label(ynquestion)
@Index(YesNoQuestion)

@b[YesNoQuestion()] is used to ask the user a question that is best
answered with a response of either yes or no.  It also gives the user
a default value in the same form as @b[PromptWithDefault()]. 

@begin(Format)
Bool
YesNoQuestion(@i[prompt, bool_def])
char @i[*prompt];
Bool @i[bool_def];
@end(Format)

@begin(Description)
@i[prompt] @\The string to prompt the user with.

@i[bool_def] @\The default value, either TRUE or FALSE.
@end(Description)

@blankspace(1 line)

@b[YesNoQuestion()] returns either TRUE or FALSE, or possibly (-1) on
a break by the user.  The only accepted values from the user are 'y',
'Y', 'n', and 'N'.  All other values prompt the user with "Please
answer 'y' or 'n'." and ask the question again.

@Center(________)


@Label(ynqquestion)
@Index(YesNoQuitQuestion)

Use @b[YesNoQuitQuestion()] when you want the user to have the option to
quit as well as answer yes or no.

@begin(Format)
Bool
YesNoQuitQuestion(@i[prompt, bool_def])
char @i[*prompt];
Bool @i[bool_def];
@end(Format)

@begin(Description)

@i[prompt] @\The string to prompt the user with.

@i[bool_def] @\The default value, either TRUE or FALSE.
@end(Description)

@blankspace(1 line)

@b[YesNoQuitQuestion()] is very similar to @b[YesNoQuestion()], in
fact the only difference is that this function interprets the keys 'q'
and 'Q' the same as hitting a break, causing
@b[YesNoQuitQuestion()] to return (-1).

@Center(________)


@Label(confirm)
@Index(Confirm)

When you want to ask the user for delete confirmation use the @b[Confirm()]
function.

@begin(Format)
Bool
Confirm(@i[prompt])
char * @i[prompt];
@end(Format)
@begin(Description)
@i[prompt] @\The string to prompt the user with.
@end(Description)

@blankspace(1 line)

@b[Confirm()] is a specialized user input function.  This function is
meant to be used only when making delete confirmation.  It will
immediately return TRUE if the verbose global variable (see section
@ref(globals)) is FALSE, implying that no confirmation is wanted.
Otherwise it asks the user a @b[YesNoQuestion()] using its argument as
the prompt, with a default value of FALSE.  If the user answers yes
then the function will return TRUE, otherwise FALSE is returned.

@Center(________)


@Label(gvfu)
@Index(GetValueFromUser)

The @b[GetValueFromUser()] routine is used to update a string that was
previously allocated with malloc().

@begin(format)
int
GetValueFromUser(@i[prompt, pointer])
char * @i[prompt], ** pointer;
@end(format)
@begin(Description)
@i[prompt] @\The string to prompt the user with.

@i[pointer] @\A pointer to the malloced item that is to be updated, or a
pointer to NULL.
@end(Description)

@Blankspace( 1 line )

@b[GetValueFromUser()] returns SUB_ERROR if the user broke out of the
prompt with control-C, else SUB_NORMAL.  The old value is freed,
unless it is NULL, and the new value is allocated and pointed to by
@i[pointer].

@Center(________)


@Index(GetYesNoValueFromUser)

If the value to be updated is an @i[SMS] style boolean ("0" or "1")
then use the the @b[GetYesNoValueFromUser()] function, which asks the
user a @b[YesNoQuestion()].

@begin(Format)
int
GetYesNoValueFromUser(@i[prompt, pointer])
char * @i[prompt], ** @i[pointer];
@end(Format)

@begin(Description)
@i[prompt] @\The string to prompt the user with.

@i[pointer] @\A pointer to the malloced item that is to be updated, or a
pointer to NULL.
@end(Description)

@Blankspace( 1 line)

@b[GetYesNoValueFromUser()] also returns SUB_ERROR if the user broke
out of the prompt with control-C, and SUB_NORMAL otherwise.
@i[pointer] is freed - Unless it is NULL - and reset to point to a
properly allocated value of "0" or "1" depending upon the user's answer
to the question.

@Center(________)


@Index(GetFSTypes)

The @i[SMS Server] expects file types to be passed as an integer bit
mask.  Use the @b[GetFSTypes()] function to query the user about the
new value of this field.

@begin(Format)
int
GetFSTypes(@i[pointer])
char **  @i[pointer];
@end(Format)

@begin(Description) 
@i[pointer] @\A pointer to the current allocated filesystem type that
is to be updated, or a pointer to NULL.
@end(Description)

@Blankspace( 1 line)

@b[GetFSTypes()] returns SUB_ERROR if the user broke out of the prompt
with control-C, and SUB_NORMAL otherwise.  It also constructs the
correct bit field by asking the user about each type, providing a
default value, of course.  The new filesystem type is passed back as
the new value pointed to by @i[pointer].  If non-NULL the old pointer
is freed.

@Section[Printing Routines]

Here are a few general printing routines.  Most printing is done with
routines that are specific to the type of database item we are dealing
with, but these few printing functions are general enough to describe
here.

@Label(print)
@Index(Print)

Use the @b[Print()] function to print out all elements in each tuple 
returned by a call to sms_query().

@begin(Format)
int
Print(@i[argc, argv, callback]);
int @i[argc];
char ** @i[argv], * @i[callback];
@end(Format)

@begin(Description)
@i[argc] @\The number if strings passed in argv.

@i[argv] @\An array of strings that is the data to be stored in the queue.

@i[callback] @\Not Used.
@end(Description)

@blankspace(1 line)

@b[Print()] returns SMS_CONT.  Every time this function is called it
sets the global variable @i[found_some] to TRUE.

@Center(________)

@Label(pbytype)
@Index(PrintByType)

If you want something a bit more specialized then try
@b[PrintByType()].  If a string compare of @i[argv][0] and
@i[callback] shows them to be equal then the tuple is printed.  If
@i[callback] is NULL then it acts exactly like @b[Print()].

@begin(Format)
int
PrintByType(@i[argc, argv, callback]);
int @i[argc];
char ** @i[argv], * @i[callback];
@end(Format)

@begin(Description)
@i[argc] @\The number if strings passed in argv.

@i[argv] @\An array of strings that is the data to be stored in the queue.

@i[callback] @\@i[Argv][0] is compared to this with strcmp() and if
they are equal then the tuple printed.  In the special case where
callback is NULL all tuples are printed.
@end(Description)

@blankspace(1 line)

@b[PrintByType()] returns SMS_CONT.  Every time this function is
called it sets the global variable found_some to TRUE.

@Center(________)


@Index(PrintHelp)

If the help information is stored in a NULL terminated array of strings
then use the @b[PrintHelp()] function to print the information for the user.

@begin(Format)
int
PrintHelp(@i[message])
char ** @i[message];
@end(Format)

@begin(Description)
@i[message] @\The help message stored in a NULL terminated array of strings.
@end(Description)

@blankspace(1 line)

@b[PrintHelp()] returns DM_NORMAL to allow help routines to remain as
simple as possible.

@Center(________)


@Index(NullPrint)
@Label(nullprint)

When using the @b[QueryLoop()] function (sect. @ref(qloop)) there are
times when all the @i[PrintFunc] needs to return is the name of the item, if
the item's name is the first element of the tuple then use the @b[NullPrint()]
function to retrieve this name.

@begin(Format)
char *
NullPrint(@i[info])
char ** @i[info];
@end(Format)
@begin(Description)
info@\ An array of strings.
@end(Description)

@blankspace(1 line)

@b[NullPrint()] returns the first element in the array passed to it,
@i[info][0].

@Section[Miscellaneous Utility Functions]

This section contains the rest of the utility functions, these don't
really fit into a category.

@Index(FreeAndClear)

Use @b[FreeAndClear()] to free a pointer and set it to NULL.

@Begin[Format]
void
FreeAndClear(@i[pointer, free_it])
char ** @i[pointer];
Bool @i[free_it];
@End[Format]

@begin(Description) 

@i[pointer] @\A pointer to the item to free or clear, if this item is
NULL then the function does not try to free it.

@i[free_it] @\This function always sets the item to NULL, but only if
this is TRUE is it freed.

@end(Description)

@Blankspace(1 line)
@Center(________)


@Index(ValidName)

@b[ValidName()] check to see if string is a valid name for a new
item in the database.

@begin(Format)
Bool
ValidName(@i[s])
char * @i[s];
@end(Format)

@begin(Description)
@i[s] @\The string to check.
@end(Description)

@Blankspace( 1 line )

If the name is not valid, @b[ValidName()] alerts the user to the
problems encountered and returns FALSE.  Checks are made to see if
this name contains a wildcard, contains spaces, or has no characters
at all.  This is by no means an exhaustive search, but does handle some
common things that confuse the database.

@Center(________)


@Index(ToggelVerboseMode)

The @b[ToggleVerboseMode()] function is associated with the menu items
that allows the user to toggle the verbose global variable (see sect.
@ref(globals)).

@Center(________)


@Index(Strsave)

Use the @b[Strsave()] function to allocate space for a new string.

@begin(Format)
char *
Strsave(@i[str])
char * @i[str];
@end(Format)

@begin(Description)
@i[str] @\The string that needs to have space allocated for it.
@end(Description)

@Blankspace( 1 line )

@b[Strsave()] returns an allocated copy of the string passed to it.

@Center(________)


@Index(EncryptMITID)
@Index(RemoveHyphens)

To save an MIT ID number two functions are needed, @b[EncryptMITID()] and
@b[RemoveHyphens()].

@begin(Format)
void
EncryptMITID(@i[sbuf], @i[idnumber], @i[first], @i[last])
char * @i[sbuf], * @i[idnumber], * @i[first], * @i[last];
@end(Format)

@begin(Description)
@i[spbuf] @\The buffer used to return the encrypted id number.

@i[idnumber] @\The unencrypted id number.

@i[first, last] @\The name of the user whose id number we are encrypting.

@end(Description)

@Blankspace(1 line)
@Center(________)


@begin(Format)
void
RemoveHyphens(@i[str)]
char * @i[str];
@end(Format)

@begin(Description) 

@i[str] @\This is the string with hyphens to remove, when the function
returns it will no longer have hyphens.
@end(Description)

@Chapter[Function Organization within Files]

The functions within each of the files tend to be quite similar.  I
have organized these so that at the top of the file are all the
general routines for the type of element currently being worked upon.
In @i[attach.c], for example, there is a @b[PrintFSInfo()] function, an
@b[AskFSInfo()] function, and a @b[GetFSInfo()] function.  Each of the
item types will have a file that contains very similar functions, and
all will be grouped near the top of the file.  As an example consider the
general functions in @i[attach.c].

@b[PrintFSInfo()] is used to print out all the information about a filesystem.

@begin(Format)
static char *
PrintFSInfo(@i[info])
char ** @i[info];
@end(Format)

@begin(Description)
@i[info] @\A NULL terminated array of strings that contains all information 
about a filesystem.
@end(Description)

@blankspace(1 line)

@b[PrintFSInfo()] has the same format as @i[PrintFunc] (sect.
@ref(qloop)) and can be used as the @i[PrintFunc] for @b[QueryLoop()]
calls in the functions that update and delete filesystems.

@Center(________)


@b[AskFSInfo()] is used to ask the user to update the current filesystem
information.

@begin(Format)
static char **
AskFSInfo(@i[info, name])
char ** @i[info];
Bool @i[name];
@end(Format)

@begin(Description) 

@i[info] @\A NULL terminated array of malloced strings that contains
all information about a filesystem.  When the function is called this
array contains the default values to present to the user, and when the
function is returned this array contains the argument list for the
update or add query.

@i[name] @\A boolean value that when TRUE tells this function to ask
the user for a new value for the name of this filesystem, and then
slip it into the @i[info] list, using @b[SlipInNewName()] (sect.
@ref(sinn))
@end(Description)

@Blankspace( 1 line )

The return value of @b[AskFSInfo()] is just the @i[info] array after
it has been updated with the new values.  This function is used to
perform most of the information gathering in the update and add
filesystem queries.  For an update query the information returned from
a @i[get_filesys_by_label] can be used for the @i[info] array, but for
an add filesystem, the program must come up with a reasonable set
default values for the @i[info] array.

If the @i[name] flag is set then the @b[SlipInNewName()] function is
called to add a new name to the @i[info] array, this causes the array
indices for most values to be changed and thus the predefined values
used to access the information in this array can no longer be used
(see sections @ref(infodefs) and @Ref(sinn)).

@Center(________)


The @b[GetFSInfo()] function is used to to perform common information
gathering @i[SMS] queries for a filesystem.

@begin(Format)
static struct qelem *
GetFSInfo(@i[type, name])
int @i[type];
char * @i[name];
@end(Format)

@begin(Description)
@i[type] @\The type of filesystem query to make, can be one of;
LABEL, MACHINE, GROUP or ALIAS.  The valid names for @i[type] are 
typedef'ed at the beginning of each file.

@i[name] @\The name to pass as the argument to the query.
@end(Description)

@Blankspace(1 line)

The @b[GetFSInfo()] function is a simple method of making the most
common information gathering @i[SMS] queries, it handles all return
values from the call to sms_query() and returns the top element of a
queue containing all information returned by the server.  If an error
is detected then NULL is returned and an error message is printed to
the user.

After this group of general functions come those that actually
correspond to items in the menu.  These commonly perform get, add,
update, and delete operations.  For more information on the functions
in each of the files see the appendix @ref(fqlist).

@Appendix(The @i[SMS] Query String Array Definitions)

@Label(a_infodefs)

@Begin[Format]

/*
 *     This file contains all definitions that allow easy access to 
 * elements returned by most of the @i[SMS] queries.
 *
 *     If the order of the arguments in the @i[SMS] queries change (again)
 *  then all that needs to be changed are the values of these items 
 * and all should be well, (hopefully :-).
 *
 *                                 Chris D. Peterson - kit@@athena
 *                                   6/8/88
 */

@Begin[Verbatim]

#ifndef _infodefs_
#define _infodefs_

#include "mit-copyright.h"

#define NAME 0

/* get_ace_use */

#define ACE_TYPE 0
#define ACE_NAME 1
#define ACE_END  2

/* alias queries. */

#define ALIAS_NAME  0
#define ALIAS_TYPE  1
#define ALIAS_TRANS 2
#define ALIAS_END   3

/* Cluster information queries */

#define C_NAME       0
#define C_DESCRIPT   1
#define C_LOCATION   2
#define C_MODTIME    3
#define C_MODBY      4
#define C_MODWITH    5
#define C_END        6

/* Cluster Data information queries */

#define CD_NAME      0
#define CD_LABEL     1
#define CD_DATA      2
#define CD_END       3

/* Delete Member from list queries. */

#define DM_LIST      0
#define DM_TYPE      1
#define DM_MEMBER    2
#define DM_END       3

/* Filesys queries */

#define FS_NAME         0
#define FS_TYPE         1
#define FS_MACHINE      2
#define FS_PACK         3
#define FS_M_POINT      4
#define FS_ACCESS       5
#define FS_COMMENTS     6
#define FS_OWNER        7
#define FS_OWNERS       8
#define FS_CREATE       9
#define FS_L_TYPE       10
#define FS_MODTIME      11
#define FS_MODBY        12
#define FS_MODWITH      13
#define FS_END          14

/* Get List Of Member queries. */

#define GLOM_NAME     0
#define GLOM_ACTIVE   1
#define GLOM_PUBLIC   2
#define GLOM_HIDDEN   3
#define GLOM_MAILLIST 4
#define GLOM_GROUP    5
#define GLOM_END      6

/* General List information queries. */

#define L_NAME     0
#define L_ACTIVE   1
#define L_PUBLIC   2
#define L_HIDDEN   3
#define L_MAILLIST 4
#define L_GROUP    5
#define L_GID      6
#define L_ACE_TYPE 7
#define L_ACE_NAME 8
#define L_DESC     9
#define L_MODTIME  10
#define L_MODBY    11
#define L_MODWITH  12
#define L_END      13

/* List Member information queries. */

#define LM_LIST   0
#define LM_TYPE   1
#define LM_MEMBER 2
#define LM_END    3

/* Machine information queries */

#define M_NAME       0
#define M_TYPE       1
#define M_MODTIME    2
#define M_MODBY      3
#define M_MODWITH    4
#define M_END        5

/*  Machine to cluster Mapping */

#define MAP_MACHINE  0
#define MAP_CLUSTER  1
#define MAP_END      2

/*  NFS phys. queries. */

#define NFS_NAME    0
#define NFS_DIR     1
#define NFS_DEVICE  2
#define NFS_STATUS  3
#define NFS_ALLOC   4
#define NFS_SIZE    5
#define NFS_MODTIME 6
#define NFS_MODBY   7
#define NFS_MODWITH 8
#define NFS_END     9

/* PO box information queries */

#define PO_NAME    0
#define PO_TYPE    1
#define PO_BOX     2
#define PO_END     3

/* Quota queries */

#define Q_FILESYS   0
#define Q_LOGIN     1
#define Q_QUOTA     2
#define Q_DIRECTORY 3  
#define Q_MACHINE   4
#define Q_MODTIME   5
#define Q_MODBY     6
#define Q_MODWITH   7
#define Q_END       8

/* User information queries */

#define U_NAME    0
#define U_UID     1
#define U_SHELL   2
#define U_LAST    3
#define U_FIRST   4
#define U_MIDDLE  5
#define U_STATE   6  
#define U_MITID   7
#define U_CLASS   8
#define U_MODTIME 9
#define U_MODBY   10
#define U_MODWITH 11
#define U_END     12

#endif _infodefs_      /* Do not add anything after this line. */

@End[Verbatim]

@End[Format]

@Appendix(List of Menus, Functions, and Query Names)

@Label(fqlist)

@blankspace(2 lines)

The menu tree for the @i[SMS Client] has three levels, the root, tier
1 and tier 2.  A diagram of the menu tree is show in appendix @ref(tree).
The root of the tree is composed of calls to other menus.

@blankspace(2 lines)

@begin(Group)
@Center(Sms Database Manipulation)
@begin(Verbatim)

 1. (cluster)      Cluster Menu.
 2. (filesys)      Filesystem Menu.
 3. (list)         Lists and Group Menu.
 4. (machine)      Machine Menu.
 5. (nfs)          Physical NFS Menu.
 6. (user)         User Menu. 
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.

@end(Verbatim)
@Hinge()
@Section[First Tier Menus]

@Center{ @b[Cluster Menu] }

@begin(Verbatim)

 1. (show)         Get cluster information.
 2. (add)          Add a new cluster.
 3. (update)       Update cluster information.
 4. (delete)       Delete this cluster.
 5. (mappings)     Machine To Cluster Mappings Menu.
 6. (c_data)       Cluster Data Menu.
 7. (verbose)      Toggle Verbosity of Delete.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.

@end(Verbatim)

@Begin(Format)
@tabset(20,40,60)
show@\get_cluster
add@\get_cluster@\add_cluster
update@\get_cluster@\update_cluster
delete@\get_cluster@\delete_cluster

@End[Format]
@Hinge()
@Center{ @b[Filesystem Menu] }

@begin(Verbatim)

 1. (get)          Get Filesystem Name Information.
 2. (add)          Add New Filesystem to Database.
 3. (change)       Update Filesystem Information.
 4. (delete)       Delete Filesystem.
 5. (check)        Check An Association.
 6. (alias)        Associate with a Filesystem.
 7. (unalias)      Dissociate from a Filesystem.
 8. (quotas)       Quota Menu.
 9. (verbose)      Toggle Verbosity of Delete.
10. (help)         Help ...
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.

@end(Verbatim)

@begin(Format)
@tabset(20,40,60)
get@\get_filesys_by_label
add@\get_filesys_by_label@\add_filesys
change@\get_filesys_by_label@\update_filesys
delete@\get_filesys_by_label@\delete_filesys
check@\get_alias
alias@\get_alias@\add_alias
unalias@\get_alias@\delete_alias

@end(Format)
@Hinge()
@Center{ @b[List Menu] }

@begin(Verbatim)

 1. (show)         Display information about a list.
 2. (add)          Create new List.
 3. (update)       Update characteristics of a list.
 4. (delete)       Delete a List.
 5. (query_remove) Interactively remove an item from all lists.
 6. (members)      Member Menu - Change/Show Members of a List.
 7. (list_info)    List Info Menu.
 8. (quotas)       Quota Menu.
 9. (verbose)      Toggle Verbosity of Delete.
10. (help)         Print Help.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.

@end(Verbatim)

@begin(Format)
@tabset(20,40,60)
show@\get_list_info
add@\get_list_info@\add_list
update@\get_list_info@\update_list
delete@\get_list_info@\get_ace_use
@\count_members_of_list@\get_lists_of_member
@\get_members_of_list@\delete_list
@\delete_member_from_list
query_remove@\get_lists_of_member@\delete_member_from_list

@end(Format)
@Hinge()
@Center{ @b[Machine Menu] }

@begin(Verbatim)

 1. (show)         Get machine information.
 2. (add)          Add a new machine.
 3. (update)       Update machine information.
 4. (delete)       Delete this machine.
 5. (mappings)     Machine To Cluster Mappings Menu.
 6. (verbose)      Toggle Verbosity of Delete.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.

@end(Verbatim)

@begin(Format)
@tabset(20,40, 60)
show@\get_machine
add@\get_machine@\add_machine
update@\get_machine@\update_machine
delete@\get_machine@\delete_machine

@end(Format)

Delete machine needs to make some other checks. For lack of time these
have not been implemented.
@begin(Itemize)
@tabset(45)
Is the machine a server@\get_server_host_info

Is the machine an NFS server@\get_nfsphys

Are there any ACE's that this machine references@\get_server_host_access
@end(Itemize)

@Hinge()

@Center{ @b[NFS Physical Menu] }

@begin(Verbatim)

 1. (show)         Show an NFS server.
 2. (add)          Add NFS server.
 3. (update)       Update NFS server.
 4. (delete)       Delete NFS server.
 5. (quotas)       Quota Menu.
 6. (verbose)      Toggle Verbosity of Delete.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.

@end(Verbatim)

@begin(Format)
@tabset(20,40, 60)
show@\get_nfsphys
add@\get_nfsphys@\add_nfsphys
update@\get_nfsphys@\update_nfsphys
delete@\get_nfsphys@\get_filesys_by_nfsphys
@\delete_nfsphys

@end(Format)
@Hinge()
@Center{ @b[User Menu] }

@begin(Verbatim)

 1. (login)        Show user information by login name.
 2. (name)         Show user information by name.
 3. (class)        Show names of users in a given class.
 4. (modify)       Change all user fields.
 5. (adduser)      Add a new user to the database.
 6. (register)     Register a user.
 7. (delete)       Delete user.
 8. (udelete)      Delete user by uid.
 9. (pobox)        Post Office Box Menu.
10. (quota)        Quota Menu.
11. (verbose)      Toggle Verbosity of Delete.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.

@end(Verbatim)

@begin(Format)
@tabset(20,40, 60)
login@\get_user_by_login
name@\get_user_by_name
class@\get_user_by_class
modify@\get_user_by_login@\update_user
adduser@\add_user
register@\get_user_by_name@\register_user
@\Kerberos server is queried also.
delete@\delete_user@\get_filesys_by_label
@\delete_filesys@\get_members_of_list
@\delete_list@\count_members_of_list
@\get_lists_of_member@\delete_member_from_list
@\get_list_info@\get_ace_use
udelete@\get_user_by_uid@\(plus all of the above).

@end(Format)
@Hinge()
@Section[Second Tier Menus]

@Center{ @b[Cluster Data Menu] }

@begin(Verbatim)

 1. (show)         Show Data on a given Cluster.
 2. (add)          Add Data to a given Cluster.
 3. (delete)       Remove Data to a given Cluster.
 4. (verbose)      Toggle Verbosity of Delete.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.

@end(Verbatim)

@begin(Format)
@tabset(20,40, 60)
show@\get_cluster_data
add@\add_cluster_data
delete@\get_cluster_data@\delete_cluster_data

@end(Format)
@Hinge()
@Center{ @b[List Information Menu] }

@begin(Verbatim)

 1. (member)       Show all lists to which a given member belongs.
 2. (admin)        Show all items which a given member can administer.
 3. (groups)       Show all lists which are groups.
 4. (public)       Show all public mailing lists.
 5. (maillists)    Show all mailing lists.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.

@end(Verbatim)

@begin(Format)
@tabset(20,40, 60)
member@\get_lists_of_member
admin@\get_ace_use
groups@\qualified_get_lists
public@\qualified_get_lists
maillists@\qualified_get_lists

@end(Format)
@Hinge()
@Center{ @b[Mappings Menu] }

@begin(Verbatim)

 1. (map)          Show Machine to cluster mapping.
 2. (addcluster)   Add machines to a clusters.
 3. (remcluster)   Remove machines from clusters.
 4. (verbose)      Toggle Verbosity of Delete.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.

@end(Verbatim)

@begin(Format)
@tabset(20,40, 60)
map@\get_machine_to_cluster_map
addcluster@\get_machine@\get_cluster
@\add_machine_to_cluster
remcluster@\get_machine_to_cluster_map
@\delete_machine_from_cluster

@end(Format)
@Hinge()
@Center{ @b[Membership Menu] }

@begin(Verbatim)

 1. (add)          Add a member to this list.
 2. (remove)       Remove a member from this list.
 3. (all)          Show the members of this list.
 4. (user)         Show the members of type USER.
 5. (list)         Show the members of type LIST.
 6. (string)       Show the members of type STRING.
 7. (verbose)      Toggle Verbosity of Delete.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.

@end(Verbatim)

@begin(Format)
@tabset(20,40, 60)
add@\add_member_to_list
remove@\delete_member_from_list
all@\get_members_of_list
user@\get_members_of_list
list@\get_members_of_list
string@\get_members_of_list

@end(Format)
@Hinge()
@Center{ @b[Post Office Box Menu] }

@begin(Verbatim)

 1. (show)         Show a user's post office box.
 2. (set)          Set (Add or Change) a user's post office box.
 3. (remove)       Remove a user's post office box.
 4. (verbose)      Toggle Verbosity of Delete.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.

@end(Verbatim)

@begin(Format)
@tabset(20,40, 60)
show@\get_pobox
set@\get_pobox@\get_server_locations
@\set_pobox_pop@\set_pobox
remove@\delete_pobox

@end(Format)
@Hinge()
@Center{ @b[Quota Menu] }

@begin(Verbatim)

 1. (shdef)        Show default user quota (in KB).
 2. (chdef)        Change default user quota.
 3. (shquota)      Show a user's disk quota on a filesystem.
 4. (addquota)     Add a new disk quota for user on a filesystem.
 5. (chquota)      Change a user's disk quota on a filesystem.
 6. (rmquota)      Remove a user's disk quota on a filesystem.
 7. (verbose)      Toggle Verbosity of Delete.
 r. (return)       Return to previous menu.
 t. (toggle)       Toggle logging on and off.
 q. (quit)         Quit.

@end(Verbatim)

@begin(Format)
@tabset(20,40, 60)
shdef@\get_value
chdef@\update_value@\get_value
shquota@\get_nfs_quota
addquota@\add_nfs_quota
chquota@\get_nfs_quota@\update_nfs_quota
rmquota@\get_nfs_quota@\delete_nfs_quota
@end(Format)
@end(Group)

@Appendix(Menu Tree Diagram)

@Label(tree)
