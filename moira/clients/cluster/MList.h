/*
 * $XConsortium: MList.h,v 1.17 90/05/08 15:18:37 converse Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*  This is the MList widget, it is useful to display a list, without the
 *  overhead of having a widget for each item in the list.  It allows 
 *  the user to select an item in a list and notifies the application through
 *  a callback function.
 *
 *	Created: 	8/13/88
 *	By:		Chris D. Peterson
 *                      MIT X Consortium
 */

#ifndef _XawMList_h
#define _XawMList_h

/***********************************************************************
 *
 * MList Widget
 *
 ***********************************************************************/

#include <X11/Xaw/Simple.h>

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		Pixel		XtDefaultBackground
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 callback            Callback           XtCallbackMList  NULL       **6
 columnSpacing       Spacing            Dimension       6
 cursor		     Cursor		Cursor		left_ptr
 defaultColumns      Columns            int             2          **5
 destroyCallback     Callback		Pointer		NULL 
 font		     Font		XFontStruct*	XtDefaultFont
 forceColumns        Columns            Boolean         False      **5
 foreground	     Foreground		Pixel		XtDefaultForeground
 height		     Height		Dimension	0          **1
 insensitiveBorder   Insensitive	Pixmap		Gray
 internalHeight	     Height		Dimension	2
 internalWidth	     Width		Dimension	4
 list                MList               String *        NULL       **2
 longest             Longest            int             0          **3  **4
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 numberStrings       NumberStrings      int             0          **4
 pasteBuffer         Boolean            Boolean         False
 rowSpacing          Spacing            Dimension       4
 sensitive	     Sensitive		Boolean		True
 verticalMList        Boolean            Boolean         False
 width		     Width		Dimension	0          **1
 x		     Position		Position	0
 y		     Position		Position	0

 **1 - If the Width or Height of the list widget is zero (0) then the value
       is set to the minimum size necessay to fit the entire list.

       If both Width and Height are zero then they are adjusted to fit the
       entire list that is created width the number of default columns 
       specified in the defaultColumns resource.

 **2 - This is an array of strings the specify elements of the list.
       This resource must be specified. 
       (What good is a list widget without a list??  :-)

 **3 - Longest is the length of the widest string in pixels.

 **4 - If either of these values are zero (0) then the list widget calculates
       the correct value. 

       (This allows you to make startup faster if you already have 
        this information calculated)

       NOTE: If the numberStrings value is zero the list must 
             be NULL terminated.

 **5 - By setting the MList.Columns resource you can force the application to
       have a given number of columns.	     
        
 **6 - This returns the name and index of the item selected in an 
       XawMListReturnStruct that is pointed to by the client_data
       in the CallbackProc.

*/


/*
 * Value returned when there are no highlighted objects. 
 */

#define XAW_LIST_NONE -1	

#define XtCMList "MList"
#define XtCSpacing "Spacing"
#define XtCColumns "Columns"
#define XtCLongest "Longest"
#define XtCNumberStrings "NumberStrings"

#define XtNcursor "cursor"
#define XtNcolumnSpacing "columnSpacing"
#define XtNdefaultColumns "defaultColumns"
#define XtNforceColumns "forceColumns"
#define XtNlist "list"
#define XtNlongest "longest"
#define XtNnumberStrings "numberStrings"
#define XtNpasteBuffer "pasteBuffer"
#define XtNrowSpacing "rowSpacing"
#define XtNverticalList "verticalList"
 
/* Class record constants */

extern WidgetClass mlistWidgetClass;

typedef struct _MListClassRec *MListWidgetClass;
typedef struct _MListRec      *MListWidget;

/* The list return structure. */

typedef struct _XawMListReturnStruct {
  String string;
  int list_index;
} XawMListReturnStruct;
#ifdef XAW_BC
/*************************************************************
 * For Compatibility only.                                   */

#define XtMListReturnStruct   XawMListReturnStruct

#define XT_LIST_NONE         XAW_LIST_NONE 

#define XtMListChange         XawMListChange
#define XtMListUnhighlight    XawMListUnhighlight
#define XtMListHighlight      XawMListHighlight
#define XtMListShowCurrent    XawMListShowCurrent

/*************************************************************/
#endif /* XAW_BC */

/******************************************************************
 *
 * Exported Functions
 *
 *****************************************************************/

#ifdef __cplusplus
extern "C" {				/* for C++ V2.0 */
#endif

/*	Function Name: XawMListChange.
 *	Description: Changes the list being used and shown.
 *	Arguments: w - the list widget.
 *                 list - the new list.
 *                 nitems - the number of items in the list.
 *                 longest - the length (in Pixels) of the longest element
 *                           in the list.
 *                 resize - if TRUE the the list widget will
 *                          try to resize itself.
 *	Returns: none.
 *      NOTE:      If nitems of longest are <= 0 then they will be caluculated.
 *                 If nitems is <= 0 then the list needs to be NULL terminated.
 */

extern void XawMListChange(
#if NeedFunctionPrototypes
    Widget		/* w */,
    String*		/* list */,
    int			/* nitems */,
    int			/* longest */,
#if NeedWidePrototypes
    /* Boolean */ int	/* resize */
#else
    Boolean		/* resize */
#endif
#endif
);

/*	Function Name: XawMListUnhighlight
 *	Description: unlights the current highlighted element.
 *	Arguments: w - the widget.
 *	Returns: none.
 */

extern void XawMListUnhighlight(
#if NeedFunctionPrototypes
    Widget,		/* w */
    int			/* item */
#endif
);

/*	Function Name: XawMListHighlight
 *	Description: Highlights the given item.
 *	Arguments: w - the list widget.
 *                 item - the item to highlight.
 *	Returns: none.
 */

extern void XawMListHighlight(
#if NeedFunctionPrototypes
    Widget		/* w */,
    int			/* item */
#endif
);


/*	Function Name: XawMListShowCurrent
 *	Description: returns the currently highlighted object.
 *	Arguments: w - the list widget.
 *		numselected - for return value
 *	Returns: the info about the currently highlighted objects.
 *		numselected - the number of objects returned.
 */

extern XawMListReturnStruct * XawMListShowCurrent(
#if NeedFunctionPrototypes
    Widget		/* w */
    int *		/* numselected */
#endif
);

#ifdef __cplusplus
}						/* for C++ V2.0 */
#endif

#endif /* _XawMList_h */
/* DON'T ADD STUFF AFTER THIS #endif */