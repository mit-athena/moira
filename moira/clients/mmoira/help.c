/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/help.c,v 1.2 1991-06-05 12:15:57 mar Exp $
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
    help(form->formname);
}

