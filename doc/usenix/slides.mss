@Comment[

	Standard slide format for the Athena presentations at the
	1988 Winter Usenix.

]
@Device(PostScript)
@Make(Report)
@Style(
	Font AvantGarde,
	TopMargin 2.5 inches, BottomMargin 1.75 inches,
	LeftMargin 1 inch, RightMargin 1 inch
)
@Define(LogoText,
	FlushRight, Spaces Kept, Break,
	Font BodyFont, Size 17, FaceCode B,
	Above 0, Below 0,
	Fixed 1.5 inches,
	Indent 0
)
@Define(SlideText,
	Font BodyFont, Size 19, FaceCode B,
	Justification off,
	Indent 0, 
	LineWidth 6.5 inches
)
@Define(SubNote, Size -4, FaceCode I)
@Modify(CopyRightNotice, Centered, Fixed -0.5 inch)
@Modify(HDG, Fixed 1.0 inch)
@Modify(FTG, Fixed -1.0 inch)
@Modify(MajorHeading, Above 1.75 inches, Below 0.75 inches,
	RightMargin -1 inch, LeftMargin -1 inch)
@Modify(Itemize, 
	Above 1 line, Below 1 line, 
	Justification off,
	Spread 1.0 lines
)
@Modify(Description, 
	Above 1 line, Below 1 line, 
	Justification off,
	Spread 1.0 lines
)

@Comment[ ======================== Author Variables ======================== ]

@String(
	ConferenceTitle="MIT Project Athena, Winter Usenix 1988",
        LectureTitle="The Athena Service Management System",
	LectureDate="12 February 1988",
        Lecturer="by Mark Rosenstein",
	Organization="MIT Project Athena",
	Address="Massachusetts Institute of Technology @* Cambridge, MA 02139",
	NetAddress="mar@@athena.mit.edu",
        CopyrightDate="1988"
)

@Comment[ ==================== Title Slide ==================== ]

@PageHeading(
	Left=<@Graphic(Height = 1.05 inches, PostScript = owl.PS)>,
	Right=<@LogoText(@Value(ConferenceTitle))>,
	Immediate
)
@PageFooting(
	Left=<@Value(LectureTitle), Slide @Value(Page)>,
        Right=<@Value(LectureDate)>,
	Immediate
)

@LogoText(@Value(ConferenceTitle))
@Begin(Transparent, Fixed 1.0 inch, Above 0, Below 0)
@Graphic(Height = 1.05 inches, PostScript = owl.PS)
@End(Transparent)
@BlankSpace(1 line)
@Begin(SlideText, Centered)
@MajorHeading(@Value(LectureTitle))
@Value(Lecturer)
@BlankSpace(1 line)
@Value(Organization)
@Value(Address)
@BlankSpace(1 line)
@Value(NetAddress)
@End(SlideText)
@CopyRightNotice{by the Massachusetts Institute of Technology}

@Comment[ ==================== Slide One ==================== ]

@NewPage()
@Begin(SlideText)
@MajorHeading{The Problem}

How to manage:
@Begin(Itemize)
14,000 User accounts (5000 active)

650 Workstations

66 Servers
@Itemize{
32 NFS Servers

17 RVD Servers

9 Printer servers

3 @p(Hesiod) servers

3 Post office servers

2 @p(Kerberos) servers
}
@End(Itemize)

@End(SlideText)
@CopyRightNotice{by the Massachusetts Institute of Technology}
@NewPage()

@Comment[ ==================== Slide Two ==================== ]

@Begin(SlideText)
@MajorHeading{Goals}
@BlankSpace(0.5 inches)

@itemize{
Manage Servers

Coordinate Servers for Consistency

Centralize Authority
}
@End(SlideText)
@CopyRightNotice{by the Massachusetts Institute of Technology}
@NewPage()

@Comment[ ==================== Slide Three ==================== ]

@Begin(SlideText)
@MajorHeading{Design Criteria}
@BlankSpace(0.5 inches)
In order of importance:
@Itemize{
Reliability

Consistency

Flexibility

Time Efficiency

Space Efficiency
}

@End(SlideText)
@CopyRightNotice{by the Massachusetts Institute of Technology}
@NewPage()

@Comment[ ==================== Slide Four ==================== ]

@Begin(SlideText)
@MajorHeading{SMS Organization}
@BlankSpace(1 inches)

@graphic(Height = 3 inch, PostScript = organization.ps)

@End(SlideText)
@CopyRightNotice{by the Massachusetts Institute of Technology}
@NewPage()

@Comment[ ==================== Slide Five ==================== ]

@Begin(SlideText)
@MajorHeading{SMS Protocol}
@BlankSpace(0.5 inches)
@Itemize{
Built on a remote procedure call library

Requires Authentication

Has Four Operations
@Itemize{
Authenticate

Query

Access

No-op
}
}

@End(SlideText)
@CopyRightNotice{by the Massachusetts Institute of Technology}
@NewPage()

@Comment[ ==================== Slide Six ==================== ]

@Begin(SlideText)
@MajorHeading{SMS Queries}
@BlankSpace(0.5 inches)
@Itemize{
Over 100 queries

Grouped into 4 types:
@Itemize{
Retrieve

Update

Append

Delete
}

Table Driven

Access Control Lists
}
@End(SlideText)
@CopyRightNotice{by the Massachusetts Institute of Technology}
@NewPage()

@Comment[ ==================== Slide Seven ==================== ]

@Begin(SlideText)
@MajorHeading{SMS Protocol}
@Heading(Example: @p(chsh))
@Begin(enumerate, rightmargin -1 inch)
@p(Establish connection)

authenticate(A@-(mar))@*
@ @ => success()

query(get_user_by_login, mar)@*
@ @ => success(@^mar, 17822, /bin/csh, ... )@*

access(update_user_shell, mar, /bin/csh)@*
@ @ => success()

@p(Prompt user for new information)

query(update_user_shell, mar, /bin/tcsh)@*
@ @ => success()

@p(Close SMS connection)
@end(enumerate)
@End(SlideText)
@CopyRightNotice{by the Massachusetts Institute of Technology}
@NewPage()

@Comment[ ==================== Slide Eight ==================== ]

@Begin(SlideText)
@Heading{Database}
The database includes:
@Begin(Itemize, Spread 0.8 lines)
Users

Machines

Clusters

General services

File systems

NFS & RVD physical information

Printers

Post Office assignments

Lists: groups, mailing lists, access control lists

Aliases

Internal control information
@End(Itemize)
@End(SlideText)
@CopyRightNotice{by the Massachusetts Institute of Technology}
@NewPage()

@Comment[ ==================== Slide Nine ==================== ]

@Begin(SlideText)
@MajorHeading{Update Protocol}
@BlankSpace(0.5 inches)

There are 4 operations:

@Itemize{
Authenticate

Transfer

Instructions

Execute
}

@End(SlideText)
@CopyRightNotice{by the Massachusetts Institute of Technology}
@NewPage()

@Comment[ ==================== Slide Ten ==================== ]

@Begin(SlideText)
@MajorHeading{SMS Components}
@BlankSpace(1 inches)

@graphic(Height = 3 inch, PostScript = components.ps)

@End(SlideText)
@CopyRightNotice{by the Massachusetts Institute of Technology}
@NewPage()

@Comment[ ==================== Slide Eleven ==================== 

@Begin(SlideText)
@MajorHeading{SMS Client Programs}
@BlankSpace(0.25 inches)
Standard Unix programs:
@begin(itemize, spread 0)
@p(chfn)

@p(chsh)
@end(itemize)
New programs for regular users:
@begin(itemize, spread 0)
@p(register)

@p(mailmaint)
@end(itemize)
For system administrators:
@begin(itemize, spread 0)
@p(attachmaint)

@p(chpobox)

@p(clustermaint)

@p(dcmmaint)

@p(listmaint)

@p(nfsmaint)

@p(portmaint)

@p(regtape)

@p(rvdmaint)

@p(usermaint)
@end(itemize)
For debugging SMS:
@itemize{
@p(smstest)
}

@End(SlideText)
@CopyRightNotice{by the Massachusetts Institute of Technology}
@NewPage()
]
@Comment[ ==================== Slide Twelve ==================== ]

@Begin(SlideText)
@MajorHeading{SMS Server}
@BlankSpace(1 inches)

@Itemize{
Multi-threaded

Authorization checks

Checks legal values

Sole access to database
}

@End(SlideText)
@CopyRightNotice{by the Massachusetts Institute of Technology}
@NewPage()

@Comment[ ==================== Slide Thirteen ==================== ]

@Begin(SlideText)
@MajorHeading{The Data Control Manager}
@BlankSpace(1 inches)

@Itemize{
@p(cron) driven

Configured by data in database

Status per host/service pair

Locking
}

@End(SlideText)
@CopyRightNotice{by the Massachusetts Institute of Technology}
@NewPage()

@Comment[ ==================== Final Slide ==================== ]

@PageHeading(
	Left=<@Graphic(Height = 1.05 inch, PostScript = owl.PS)>,
	Right=<@LogoText(@Value(ConferenceTitle))>,
	Immediate
)
@PageFooting(
	Left=<@Value(LectureTitle), Slide @Value(Page)>,
        Right=<@Value(LectureDate)>,
	Immediate
)
@BlankSpace(1 line)
@Begin(SlideText, Centered, Spacing 1.0)
@MajorHeading(@Value(LectureTitle))
@Value(Lecturer)
@BlankSpace(1 line)

This package is not yet available.
Contact us to be put on an interest list.

@End(SlideText)
@CopyRightNotice{by the Massachusetts Institute of Technology}
