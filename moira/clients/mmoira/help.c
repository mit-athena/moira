/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/help.c,v 1.4 1992-10-28 16:06:43 mar Exp $
 *
 *  	Copyright 1991 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include	<mit-copyright.h>
#include	<stdio.h>
#include	<Xm/Xm.h>
#include	<moira.h>
#include	"mmoira.h"


help(node)
char *node;
{
    FILE *helpfile = NULL;
    char buf[1024], key[32], *msg, helpbuf[10240], *filename;
    char *realloc(), *getenv();

    sprintf(key, "*%s\n", node);
    filename = getenv("MOIRAHELPFILE");
    if (filename == NULL)
      filename = HELPFILE;
    helpfile = fopen(filename, "r");
    if (helpfile == NULL) {
	display_error("Sorry, help is currently unavailable.\n");
	return;
    }
    while (fgets(buf, sizeof(buf), helpfile))
      if (!strcmp(buf, key))
	break;
    if (strcmp(buf, key)) {
	sprintf(buf, "Sorry, unable to find help on topic \"%s\".\n", node);
	display_error(buf);
	fclose(helpfile);
	return;
    }
    msg = NULL;
    while (fgets(buf, sizeof(buf), helpfile))
      if (buf[0] == '*')
	break;
      else {
	  if (msg) {
	      if (!strcmp(buf, "\n"))
		strcpy(buf, " \n");
	      msg = realloc(msg, strlen(msg) + strlen(buf));
	      strcat(msg, buf);
	  } else
	    msg = strsave(buf);
      }
    fclose(helpfile);
    if (msg) {
	PopupHelpWindow(msg);
	free(msg);
    }
    return;
}

help_form_callback(dummy, form)
int dummy;
EntryForm *form;
{
    UserPrompt **p;
    int count;

    /* undocumented Motif internal routine to advance in tab group.
     * In this case we're going backwards because for some reason
     * the form advances whenever this button is pressed.
     * However, it doesn't seem to go backwards even though source 
     * implies that it should.  So we go forward until we wrap.
     */
    count = 0;
    for (p = form->inputlines; *p; p++)
      if (!((*p)->insensitive))
	count++;
    while (count-- > 1)
      _XmMgrTraversal(form->formpointer, XmTRAVERSE_PREV_TAB_GROUP);
    help(form->formname);
}

