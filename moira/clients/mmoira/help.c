/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/help.c,v 1.1 1991-05-31 16:46:37 mar Exp $
 *
 *  	Copyright 1991 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include	<mit-copyright.h>
#include	<stdio.h>
#include	<Xm/Xm.h>
#include	"mmoira.h"


help(node)
char *node;
{
    FILE *helpfile = NULL;
    char buf[1024], key[32];

    sprintf(key, "*%s\n", node);
    helpfile = fopen(HELPFILE, "r");
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
    while (fgets(buf, sizeof(buf), helpfile))
      if (buf[0] == '*')
	break;
      else
	fprintf(stderr, buf);
    fclose(helpfile);
    return;
}

help_form_callback(dummy, form)
int dummy;
EntryForm *form;
{
    help(form->formname);
}

