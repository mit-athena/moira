@Device(Postscript)
@Make(Plan)
@LibraryFile(Stable)
@Modify(Enumerate, Numbered <@A. @,@1. @,@a. @,@i. >)
@Modify(Title, Below .2inch)
@Modify(Author, Below 0, Above 0)

@PlanHead(
	VersionDate "Draft@Y<M>May 13, 1987",
	PlanSection "Section E.1",
	Title "Service Management System",
	CopyrightDate "1987",
	Author "Peter Levine"
)
@Author(Michael R. Gretzinger)
@BlankSpace(.5inch)

@MajorSection(Abstract)

The problem of maintaining, managing, and supporting an increasing number of
distributed network services with multiple server instances requires
development and integration of centralized data management and automated
information distribution. This paper presents the Athena Service Management
System, focusing on the system components and interface design.  The system
provides centralized data administration, a protocol interface to the
database, tools for accessing and modifying the database and an automated
mechanism of data distribution.

@MajorSection(Purpose)

The primary purpose of SMS is to provide a single point of contact for
making administrative changes that may affect more than one Athena service
or server.  The secondary purpose is to provide a single point of contact
from which one can obtain authoritative information about Athena
administration.  Inherently, the implementation of the purpose creates
significant economic value in regard to the support and maintainence of
system services.

@include[intro.mss]
@include[structure.mss]
@include[protocol.mss]
@include[tools.mss]
@include[deploy.mss]
