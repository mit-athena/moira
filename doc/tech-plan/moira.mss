@Comment($Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/doc/tech-plan/moira.mss,v 1.15 2000-01-21 17:20:56 danw Exp $)
@Device(Postscript)
@Make(Plan)
@LibraryFile(Stable)
@Modify(Enumerate, Numbered <@A. @,@1. @,@a. @,@i. >)
@Modify(Title, Below .2inch)
@Modify(Author, Below 0, Above 0)
@Modify(hd3, below 1)

@PlanHead(
	VersionDate "Draft of @value<date>",
	PlanSection "Section E.1",
	Title "Moira, the Athena Service Management System",
	FootingTitle "Moira",
	CopyrightDate "1987, 1989",
	Author "Peter Levine"
)
@Author(Michael R. Gretzinger)
@Author(Mark A. Rosenstein)
@Author(Jean Marie Diaz)
@Author(Bill Sommerfeld)
@Author(Ken Raeburn)
@BlankSpace(.5inch)

@MajorSection(Abstract)

The problem of maintaining, managing, and supporting an increasing
number of distributed network services with multiple server instances
requires development and integration of centralized data management
and automated information distribution. This paper presents @i(Moira),
the Athena Service Management System, focusing on the system
components and interface design.  The system provides centralized data
administration, a protocol for interface to the database, tools for
accessing and modifying the database and an automated mechanism of
data distribution.

@MajorSection(Purpose)

The primary purpose of Moira is to provide a single point of contact for
administrative changes that affect more than one Athena service.  The
secondary purpose is to provide a single point of contact for
authoritative information about Athena administration.

@include[intro.mss]
@include[sys_model.mss]
@include[db_analysis.mss]
@include[protocol.mss]
@include[server.mss]
@include[clientlib.mss]
@include[dcm.mss]
@include[server_arrang.mss]
@include[update.mss]
@include[user_reg.mss]
@include[structure.mss]
@include[queries.mss]
@include[tools.mss]
@unnumbered[References]
@bibliography
