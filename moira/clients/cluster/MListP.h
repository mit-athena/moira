/*
 * $XConsortium: MListP.h,v 1.12 89/12/11 15:09:04 kit Exp $
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
 *
 * Author:  Chris D. Peterson, MIT X Consortium
 */


/* 
 * MListP.h - Private definitions for MList widget
 * 
 * This is the MList widget, it is useful to display a list, without the
 * overhead of having a widget for each item in the list.  It allows 
 * the user to select an item in a list and notifies the application through
 * a callback function.
 *
 *	Created: 	8/13/88
 *	By:		Chris D. Peterson
 *                      MIT - Project Athena
 */

#ifndef _XawMListP_h
#define _XawMListP_h

/***********************************************************************
 *
 * MList Widget Private Data
 *
 ***********************************************************************/

#include <X11/Xaw/SimpleP.h>
#include "MList.h"

#define NO_HIGHLIGHT            XAW_LIST_NONE
#define OUT_OF_RANGE            -1
#define OKAY                     0

/* New fields for the MList widget class record */

typedef struct {int foo;} MListClassPart;

/* Full class record declaration */
typedef struct _MListClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    MListClassPart	list_class;
} MListClassRec;

extern MListClassRec mlistClassRec;

/* New fields for the MList widget record */
typedef struct {
    /* resources */
    Pixel	foreground;
    Dimension	internal_width,
        	internal_height,
                column_space,
                row_space;
    int         default_cols;
    Boolean     force_cols,
                paste,
                vertical_cols;
    int         longest;
    int         nitems;		/* number of items in the list. */
    XFontStruct	*font;
    String *    list;
    XtCallbackList  callback;

    /* private state */

    int         *is_highlighted,/* set to the items currently highlighted. */
    		nhighlighted,	/* number of items hightlighted */
                highlight,	/*set to the item that should be highlighted.*/
                col_width,	/* width of each column. */
                row_height,	/* height of each row. */
                nrows,		/* number of rows in the list. */
                ncols;		/* number of columns in the list. */
    GC		normgc,		/* a couple o' GC's. */
                revgc,
                graygc;		/* used when inactive. */

} MListPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _MListRec {
    CorePart	core;
    SimplePart	simple;
    MListPart	list;
} MListRec;

#endif /* _XawMListP_h */
