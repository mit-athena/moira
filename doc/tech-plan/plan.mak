@Comment[ Scribe definition file for M.I.T. Project Athena Technical
          Plan Notebook.  Defined only for PostScript and File output.

Note: The forms "Athenahead" and "Planhead", used to set up the logo,
title area, and page headings and footings, look for a library file
named owl.PS in the current working directory.

                                       J. H. Saltzer, December, 1986 ]

@Marker(Make,Plan,ScaleableLaser)
@FontFamily(AllPostScriptFonts)

@Define(BodyStyle, Font BodyFont, Spacing 1, Spread 0.8, size 11)
@Define(NoteStyle, Font BodyFont, FaceCode R, Spacing 1, size 10)
@Style(DoubleSided, BindingMargin=0.3inch)

@Modify(Insert, LeftMargin +3ems, RightMargin +3ems)
@Modify(Description, Spacing 1, Spread 0.5, below 1)
@Modify(Itemize, LeftMargin +3ems, Indent -3ems, RightMargin 3 ems)
@Modify(MajorHeading, flushleft, size 12)
@Modify(Heading, flushleft, size 11)
@Modify(SubHeading, flushleft, size 10, above 2, FaceCode P)
@Define(ParagraphHeading, flushleft, size 9, Above 0.3inch, Break before)
@Modify(Enumerate,
        LeftMargin +3ems, Indent 0,
        Numbered   <@1.  @,@#@:.@1.  @,@#@:.@1.  >, 
        Referenced <@1@,@#@:.@1@,@#@:.@1>
       )
@Modify(Verbatim, Size 10, spacing 1)
@Modify(Fnenv, Size 10, Indent 1em, Font BodyFont, Facecode R)
@Define(Fnctr, use center, above 0, size -1)
@Define(Sanserif, Font Helvetica)

@Define(HDx, Hyphenation off, Flushleft, Break,
        LeftMargin 0,Indent 0, Above 1,
        Need 4,
        Font TitleFont, FaceCode B
       )
@Define(Logotype, Use HDx, Size 10, flushright, Font BodyFont, FaceCode B,
        spaces kept)
@Define(Title, Use HDx, Size 16, flushright)
@Define(Author, Use HDx, Size 12, flushright, FaceCode R, Above 0.2inch,
                 below 0.6 inch)
@Define(Hd1, Use HDx, Size 12, Above 2, below 1)
@Define(Hd2, Use HDx, Size 11, Above 2, below 1.3)
@Define(Hd3, Use HDx, Size 10, Above 2, below 0.5, Facecode P)
@Define(Hd4, Use HDx, Size  9, Above 0.3inch, Break before)

@Counter(MajorSection, TitleEnv HD1,
         Numbered [@1.], Referenced [@1],
         IncrementedBy Use, Announced
        )
@Counter(UnNumbered, TitleEnv HD1, Announced, Alias MajorSection)

@Equate(Appendix= MajorSection)

@Counter(Section, Within MajorSection,
         TitleEnv HD2,
         Numbered [@#@:.@1.], Referenced [@#@:.@1],
         IncrementedBy Use
        )
@Counter(SubSection, Within Section,
	 TitleEnv HD3,
         Numbered [@#@:.@1.], Referenced [@#@:.@1],
	 IncrementedBy Use
        )
@Counter(Paragraph, Within SubSection,
         TitleEnv HD4,
         Numbered [@#@:.@1.],Referenced [@#@:.@1],
         TitleForm {@Begin@ParmQuote(TitleEnv)@parm(Numbered)  @~
                    @parm(Title).@ @ @'@End@ParmQuote(TitleEnv)},
	 IncrementedBy Use
        )

@Modify(Page, Numbered <@#@:-@1>, Referenced <@#@:-@1>)
@Modify(EquationCounter, Within MajorSection)
@Modify(TheoremCounter, Within MajorSection)

@Counter(Global)
@equate(chapter=Global)
@Equate(tc2=hd2) 
@LibraryFile(Figures)
@Disable(FigureContents)
@Disable(TableContents)

@Form{planhead={
@pagefooting(left=
@imbed(footingtitle,
      Def=<       "@B(@parm(footingtitle))",>,
      Undef=<     "@B(@parm(title))",>
      )
             right="@B<@parm(versiondate)>",
@imbed(copyrightdate,
      Def=<  line="@fnctr[Copyright @Y(C) @parm(copyrightdate) @~
                   by the Massachusetts Institute of Technology]",>
      )
	     immediate
            )
@pageheading(odd,
	     left="@B<Athena Technical Plan>",
@imbed(plansection,
      Def=<  right="@B[@parm(plansection), page @value(page)]">,
      Undef=<right="@B[Page @value(page)]">
      )
            )
@pageheading(even,
             left="@B<Page @value(page), @parm(plansection)>",
	     right="@B<Athena Technical Plan>"
            )
@pagefooting(left=
@imbed(footingtitle,
      Def=<       "@B(@parm(footingtitle))",>,
      Undef=<     "@B(@parm(title))",>
      )
             right="@B<@parm(versiondate)>"
            )
@Logotype[PROJECT ATHENA TECHNICAL PLAN]
@picture(height = .4 inches, ScaleableLaser = owl.PS)
@title[@parm(plansection)]
@imbed(title,
      Def=<@title[@parm(title)]>
      )
@imbed(author,
      Def=<@author[@parm(author)]>
      )
@comment[@blankspace(0.3inch)]
}
}
@Form{Athenahead={
@pageheading(
@imbed(title,
      Def=< left = "@B[@parm(title)]",>
      )  
            right = "@B[Page @Value(page)]"
            )
@Logotype[M. I. T.  PROJECT  ATHENA]
@picture(height = .4 inches, ScaleableLaser = owl.PS)
@imbed(title,
      Def=<@title[@parm(title)]>
      )
@imbed(author,
      Def=<@author[@parm(author)]>
      )
@imbed(date,
      Def=<@author[@parm(date)]>
      )
@comment[@blankspace(0.6inch)]
}
}
@Begin(Text, Indent 1em, Use BodyStyle, FaceCode R,
       LeftMargin   1inch,
       TopMargin    1inch,
       BottomMargin 1inch,
       RightMargin  1inch
      )


@Marker(Make, Plan, File)
@Define(BodyStyle, Spacing 1)
@Define(NoteStyle, Spacing 1)

@Modify(Insert, LeftMargin +3ems, RightMargin +3ems)
@Modify(Description, Spacing 1, Spread 1, below 1)
@Modify(Itemize, LeftMargin +3ems, Indent -3ems, RightMargin 3 ems)
@Modify(MajorHeading, flushleft)
@Modify(Heading, flushleft)
@Modify(SubHeading, flushleft, above 2)
@Define(ParagraphHeading, flushleft, above 1, break before)
@Modify(Enumerate,
        LeftMargin +3ems, Indent 0,
        Numbered   <@1.  @,@#@:.@1.  @,@#@:.@1.  >, 
        Referenced <@1@,@#@:.@1@,@#@:.@1>
       )
@Modify(Fnenv, Indent 1em)
@Define(Fnctr, use center, above 0)
@Define(SanSerif)

@Define(HDx, Hyphenation off, Flushleft, Break,
        LeftMargin 0,Indent 0, Above 1,
        Need 4
       )

@Define(Logotype, Use HDx, flushright, spaces kept)
@Define(Title, Use HDx, flushright)
@Define(Author, Use Hdx, flushright, Above 1, Below 1)
@Define(Hd1, Use HDx, Above 2, below 1)
@Define(Hd2, Use HDx, Above 2, below 1)
@Define(Hd3, Use HDx, Above 2, below 1)
@Define(Hd4, Use HDx, Above 1, Break before)

@Counter(MajorSection, TitleEnv HD1,
         Numbered [@1.], Referenced [@1],
         IncrementedBy Use, Announced
        )
@Counter(UnNumbered, TitleEnv HD1, Announced, Alias MajorSection)

@Equate(Appendix= MajorSection)

@Counter(Section, Within MajorSection,
         TitleEnv HD2,
         Numbered [@#@:.@1.], Referenced [@#@:.@1],
         IncrementedBy Use
        )
@Counter(SubSection, Within Section,
	 TitleEnv HD3,
         Numbered [@#@:.@1.], Referenced [@#@:.@1],
	 IncrementedBy Use
        )
@Counter(Paragraph, Within SubSection,
         TitleEnv HD4,
         Numbered [@#@:.@1.],Referenced [@#@:.@1],
         TitleForm {@Begin@ParmQuote(TitleEnv)@parm(Numbered)  @~
                    @parm(Title).@ @ @'@End@ParmQuote(TitleEnv)},
	 IncrementedBy Use
        )

@Modify(Page, Numbered <@#@:-@1>, Referenced <@#@:-@1>)
@Modify(EquationCounter, Within MajorSection)
@Modify(TheoremCounter, Within MajorSection)

@Counter(Global)
@equate(chapter=Global)
@Equate(tc2=hd2) 
@LibraryFile(Figures)
@Disable(FigureContents)
@Disable(TableContents)

@Form{planhead={
@pagefooting(left=
@imbed(footingtitle,
      Def=<       "@B(@parm(footingtitle))",>,
      Undef=<     "@B(@parm(title))",>
      )
             right="@B<@parm(versiondate)>",
@imbed(copyrightdate,
      Def=<  line="@fnctr[Copyright @Y(C) @parm(copyrightdate) @~
                   by the Massachusetts Institute of Technology]",>
      )
	     immediate
            )
@pageheading(odd,
	     left="@B<Athena Technical Plan>",
@imbed(plansection,
      Def=<  right="@B[@parm(plansection), page @value(page)]">,
      Undef=<right="@B[Page @value(page)]">
      )
            )
@pageheading(even,
             left="@B<Page @value(page), @parm(plansection)>",
	     right="@B<Athena Technical Plan>"
            )
@pagefooting(left=
@imbed(footingtitle,
      Def=<       "@B(@parm(footingtitle))",>,
      Undef=<     "@B(@parm(title))",>
      )
             right="@B<@parm(versiondate)>"
            )
@Logotype[PROJECT ATHENA TECHNICAL PLAN]
OWL
@blankspace(2 lines)
@title[@parm(plansection)]
@imbed(title,
      Def=<@title[@parm(title)]>
      )
@imbed(author,
      Def=<@author[@parm(author)]>
      )
@blankspace(2 lines)
}
}

@Form{Athenahead={

@Logotype[M. I. T.  PROJECT  ATHENA]
OWL
@Blankspace(2 lines)
@imbed(title,
      Def=<@title[@parm(title)]>
      )
@imbed(author,
      Def=<@author[@parm(author)]>
      )
@imbed(date,
      Def=<@author[@parm(date)]>
      )
@blankspace(2 lines)
}
}

@Begin(Text, Indent 1em, Use BodyStyle, FaceCode R,
       LeftMargin   0,
       TopMargin    1inch,
       BottomMargin 1inch,
       font CharDef,
       linewidth    7.8inches
      )

@Marker(Make,Changelog)
@Comment[

]


