/*
** Copyright (c) 1990 David E. Smyth
**
** Redistribution and use in source and binary forms are permitted
** provided that the above copyright notice and this paragraph are
** duplicated in all such forms and that any documentation, advertising
** materials, and other materials related to such distribution and use
** acknowledge that the software was developed by David E. Smyth.  The
** name of David E. Smyth may not be used to endorse or promote products
** derived from this software without specific prior written permission.
** THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
** WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
*/

/*
* SCCS_data: @(#)AriRegAll.c 1.0 ( 19 June 1990 )
*
* Subsystem_group:
*
*     Widget Creation Library - Athena Resource Interpreter
*
* Module_description:
*
*     This module contains registration routine for all Athena
*     widget constructors and classes.  
*
* Module_interface_summary: 
*
*     void AriRegisterAthena ( XtAppContext app )
*
* Module_history:
*                                                  
*   mm/dd/yy  initials  function  action
*   --------  --------  --------  ---------------------------------------------
*   06/19/90  D.Smyth   all	  create.
*
* Design_notes:
*
*******************************************************************************
*/
/*
*******************************************************************************
* Include_files.
*******************************************************************************
*/

#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Clock.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>

#ifdef Use_AtLabel		/* djf's formatted text stuff... */
#include "Label.h"
#endif


#include <X11/Xaw/Cardinals.h>

void AriRegisterAthena ( app )
    XtAppContext app;
{

#define RCN( name, class ) WcRegisterClassName ( app, name, class );

    /* -- register all Athena widget classes */
    /* Simple Widgets (Chapt 3) */
    RCN("Command",			commandWidgetClass	);
    RCN("Label",			labelWidgetClass	);

    /* Menus (Chapt 4) */
    RCN("SimpleMenu",			simpleMenuWidgetClass	);
    RCN("SmeBSB",			smeBSBObjectClass	);
    RCN("SmeLine",			smeLineObjectClass	);
    RCN("MenuButton",			menuButtonWidgetClass	);

    /* Text Widgets (Chapt 5) */
    RCN("AsciiText",			asciiTextWidgetClass	);
    RCN("Text",				textWidgetClass		);

    /* Composite and Constraint Widgets (Chapt 6) */
    RCN("Dialog",			dialogWidgetClass	);
    RCN("Form",				formWidgetClass		);

    /* Other Interestng Widgets (not in ref manual) */
    RCN("Clock",			clockWidgetClass	);

#ifdef Use_AtLabel		/* djf's formatted text stuff... */
    RCN("AtLabel",			atLabelWidgetClass	);
#endif

#undef RCN
}
