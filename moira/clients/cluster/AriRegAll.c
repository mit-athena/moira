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
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Clock.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Grip.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/List.h>
#include "MList.h"
#include <X11/Xaw/Logo.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/StripChart.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/Viewport.h>

#include <X11/Xaw/Cardinals.h>

void AriRegisterAthena ( app )
    XtAppContext app;
{

#define RCN( name, class ) WcRegisterClassName ( app, name, class );
#define RCP( name, class ) WcRegisterClassPtr  ( app, name, class );

    /* -- register all Athena widget classes */
    /* Simple Widgets (Chapt 3) */
    RCN("Command",			commandWidgetClass	);
    RCP("commandWidgetClass",		commandWidgetClass	);
    RCN("Grip",				gripWidgetClass		);
    RCP("gripWidgetClass",		gripWidgetClass		);
    RCN("Label",			labelWidgetClass	);
    RCP("labelWidgetClass",		labelWidgetClass	);
    RCN("List",				listWidgetClass		);
    RCP("listWidgetClass",		listWidgetClass		);
    RCN("MList",			mlistWidgetClass	);
    RCP("mlistWidgetClass",		mlistWidgetClass	);
    RCN("Scrollbar",			scrollbarWidgetClass	);
    RCP("scrollbarWidgetClass",		scrollbarWidgetClass	);
    RCN("Simple",			simpleWidgetClass	);
    RCP("simpleWidgetClass",		simpleWidgetClass	);
    RCN("StripChart",			stripChartWidgetClass	);
    RCP("stripChartWidgetClass",	stripChartWidgetClass	);
    RCN("Toggle",			toggleWidgetClass	);
    RCP("toggleWidgetClass",		toggleWidgetClass	);

    /* Menus (Chapt 4) */
    RCN("SimpleMenu",			simpleMenuWidgetClass	);
    RCP("simpleMenuWidgetClass",	simpleMenuWidgetClass	);
    RCN("SmeBSB",			smeBSBObjectClass	);
    RCP("smeBSBObjectClass",		smeBSBObjectClass	);
    RCN("SmeLine",			smeLineObjectClass	);
    RCP("smeLineObjectClass",		smeLineObjectClass	);
    RCN("Sme",				smeObjectClass		);
    RCP("smeObjectClass",		smeObjectClass		);
    RCN("MenuButton",			menuButtonWidgetClass	);
    RCP("menuButtonWidgetClass",	menuButtonWidgetClass	);

    /* Text Widgets (Chapt 5) */
    RCN("AsciiText",			asciiTextWidgetClass	); /* NB name */
    RCP("asciiTextWidgetClass",		asciiTextWidgetClass	);
    RCN("AsciiSrc",			asciiSrcObjectClass	);
    RCP("asciiSrcObjectClass",		asciiSrcObjectClass	);
    RCN("AsciiSink",			asciiSinkObjectClass	);
    RCP("asciiSinkObjectClass",		asciiSinkObjectClass	);
    RCN("Text",				textWidgetClass		);
    RCP("textWidgetClass",		textWidgetClass		);

    /* Composite and Constraint Widgets (Chapt 6) */
    RCN("Box",				boxWidgetClass		);
    RCP("boxWidgetClass",		boxWidgetClass		);
    RCN("Dialog",			dialogWidgetClass	);
    RCP("dialogWidgetClass",		dialogWidgetClass	);
    RCN("Form",				formWidgetClass		);
    RCP("formWidgetClass",		formWidgetClass		);
    RCN("Paned",			panedWidgetClass	);
    RCP("panedWidgetClass",		panedWidgetClass	);
    RCN("Viewport",			viewportWidgetClass	);
    RCP("viewportWidgetClass",		viewportWidgetClass	);

    /* Other Interestng Widgets (not in ref manual) */
    RCN("ClockWidget",			clockWidgetClass	);
    RCP("clockWidgetClass",		clockWidgetClass	);
    RCN("LogoWidget",			logoWidgetClass		);
    RCP("logoWidgetClass",		logoWidgetClass		);

#undef RCN
#undef RCP
}
