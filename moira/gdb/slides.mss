@device(PostScript)
@make(slides)
@style(justification off)
@style(FontFamily "TimesRoman")
@modify(text, spread 1cm)
@modify(heading, below 1.25in)
@blankspace(2 in)
@begin(center)
@begin(majorheading)
GDB: 

Global Databases on High Speed Networks
@end(majorheading)

@heading(Noah Mendelsohn)

@subheading(IBM T.J. Watson Research Center@*and@*MIT Project Athena)

@blankspace(1.8 in)
@value(Date)
@end(center)
@newpage
@Heading(Problems)
@begin(itemize)
There are few network applications at Athena

Network applications are difficult to implement

Heterogeneous hardware complicates the problem

Synchronization and performance issues are frequently misunderstood
@end(itemize)
@newpage
@Heading(GDB Approach@*A C Library to Support:)
@begin(Itemize)
Shared, global relational databases

Simplified implementation of servers and clients

Explicit support for single process Unix servers
@end(itemize)
@newpage
@heading(Potential Appplications of GDB)
@begin(itemize)
Subject specific databases

Calendars

Cooperative development environments

Access to network resources (e.g. videodisc)

Interactive game-like environments

Discussion systems (conferences)
@end(itemize)
@newpage
@Heading(GDB Database Services)
@begin(itemize)
Full services of RTI Ingres relational databases

Available from any GDB client on internet 

Machine dependencies hidden

Synchronous and asynchronous flavors of most services

Based on GDB data structuring facilities 
@end(itemize)
@newpage
@Heading(GDB Data Structuring Services)
@begin(itemize)
Intended to support GDB's specific needs,  @i[not] a generalized
extension to C language typing

Explicit support for `database types' (e.g. relation, tuple)

Representation may vary according to local hardware type

Automatic conversion when transmitting between different architectures
(e.g. VAX to RT/PC)

Limited polymorphism

Limited services for user defined types
@end(itemize)
@newpage
@Heading(GDB Communication Services)
@begin(itemize)
CONNECTIONS:  underlying implementation is TCP byte streams, but much
easier to use

Services provided for sending and receiving GDB typed data

Queuing for asynchronous operations

Synchronization services

All services are available to GDB applications

Very low latencies can be achieved on suitable networks

Asynchronous services can hide network delays from the application

GDB @i[database] services are built on top of GDB communication services
@end(itemize)
@newpage
@Heading(GDB Servers and Clients)
@begin(itemize)
@i[Forking] servers are particularly easy to implement

@i[Non-forking] servers provide high performance data sharing

Limited support for @i[peer-to-peer] communication is also provided
@end(itemize)
@newpage
@Heading(GDB Project Status)
@begin(itemize)
Project started: late winter of 1986

A basic, unoptimized, but 95% complete implementation of the specification
is now available for Berkeley Unix systems

Runs on both RT/PC and Vax machines, should port to others easily

Preliminary versions of a User's Guide and a Library Reference Manual
are available

A few applications have been built, and several others are under development

Several groups and individuals at MIT have expressed interest in the system
@end(itemize)
@newpage
@Heading(Plans for the Future)
@begin(itemize)
Develop applications

Find and support new `customers'

Evaluate design, and possibly re-implement some portions based on
experiences of users

Enhancements? (e.g. automatic encapsulation of C structures as GDB types)

Continue comparisons with other approaches to similar function
(integrated distributed databases, RPC, etc.)
@end(itemize)
