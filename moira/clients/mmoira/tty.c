/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/tty.c,v 1.1 1992-11-24 16:57:30 mar Exp $
 *
 *  	Copyright 1992 by the Massachusetts Institute of Technology.
 *
 *	For further information on copyright and distribution 
 *	see the file mit-copyright.h
 */

#include	<mit-copyright.h>
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/signal.h>
#include	<sgtty.h>
#include	<sys/ioctl.h>
#include	<ctype.h>
#include	<strings.h>
#include	<X11/Intrinsic.h>
#include	"mmoira.h"

static char rcsid[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/tty.c,v 1.1 1992-11-24 16:57:30 mar Exp $";


struct TtyMenu {
    char line[48];
    MenuItem *menu;
} *TtyCommands = NULL;

extern int NumMenus;
extern MenuItem MenuRoot;
char prompt[] = "moira> ";

static menucmp(a, b)
struct TtyMenu *a, *b;
{
    return(strcmp(a->line, b->line));
}


TtyMainLoop()
{
    char buf[1024], cbuf[2], c, *p;
    int arg, i, done;
    struct sgttyb tty, ntty;

    if (TtyCommands == NULL) {
	parse_menus();
    }

    ioctl(0, TIOCFLUSH, &arg);
    ioctl(0, TIOCGETP, &tty);
    ntty = tty;
    ntty.sg_flags |= RAW;
    ntty.sg_flags &= ~ECHO;
    ioctl(0, TIOCSETP, &ntty);

    while (1) {
	parser(prompt, &TtyCommands[0]);
    }{
	write(1, prompt, strlen(prompt));
	p = &buf[0];
	buf[0] = '\0';
	for (done = 0; !done;) {
	    i = read(0, cbuf, 1);
	    c = cbuf[0];
	    cbuf[1] = '\0';
	    switch (c) {
		/* Backspace, Delete */
	    case 0177:
	    case '\b':
		if (p > &buf[0]) {
		    if (p[-1] == '\t') {
			*(--p) = '\0';
			write(1, "\r", 1);
			write(1, prompt, strlen(prompt));
			write(1, buf, strlen(buf));
		    } else {
			write(1, "\b \b", 3);
			*(--p) = '\0';
		    }
		}
		break;
		/* Erase Word */
	    case 'W' - '@':
		if (p > &buf[0])
		  p--;
		while (p > &buf[0] && isspace(*p)) {
		    write(1, "\b \b", 3);
		    p--;
		}
		while (p >= &buf[0] && !isspace(*p)) {
		    write(1, "\b \b", 3);
		    p--;
		}
		if (p > &buf[0])
		  p++;
		else
		  p = &buf[0];
		*p = 0;
		break;
		/* Suspend */
	    case 'Z' - '@':
		ioctl(0, TIOCSETP, &tty);
		kill(getpid(), SIGSTOP);
		ioctl(0, TIOCSETP, &ntty);
		write(1, prompt, strlen(prompt));
		write(1, buf, strlen(buf));
		break;
		/* Exit */
	    case 'C' - '@':
		write(1, "\r\n", 2);
		ioctl(0, TIOCSETP, &tty);
		exit(0);
		break;
		/* Clear Line */
	    case 'U' - '@':
		while (p > &buf[0]) {
		    write(1, "\b \b", 3);
		    *(--p) = '\0';
		}
		write(1, "\r", 1);
		write(1, prompt, strlen(prompt));
		break;
		/* Redraw Line */
	    case 'L' - '@':
	    case 'R' - '@':
		write(1, "XXX\r\n", 5);
		write(1, prompt, strlen(prompt));
		write(1, buf, p - &buf[0]);
		break;
		/* Complete field */
	    case '\t':
	    case '[' - '@':
		do_complete(buf);
		break;
		/* Help */
	    case '?':
		write(1, "?  ", 3);
		do_help(buf);
		write(1, prompt, strlen(prompt));
		write(1, buf, strlen(buf));
		break;
		/* Execute Command */
	    case '\n':
	    case '\r':
		done = 1;
		write(1, "\r\n", 2);
		execute_command(buf);
		break;
		/* Quote character */
	    case 'Q'  - '@':
	    case 'V' - '@':
		i = read(0, cbuf, 1);
		c = cbuf[0];
		/* Insert character */
	    default:
		write(1, cbuf, 1);
		*p++ = c;
		*p = '\0';
	    }
	}
    }
}


static char *NextWord(s)
char *s;
{
    while (*s && !isspace(*s)) s++;
    while (*s && isspace(*s)) s++;
    return(s);
}

static do_match(line, word)
char *line;
int word;
{
    int i, j, k, match, len;
    char *p, *wrd, *linep;

    linep = NthWord(line, word);
    if (linep == NULL) {
	return(NULL);
    }
    p = index(linep, ' ');
    if (p)
      len = p - linep;
    else
      len = strlen(linep);
    match = -1;

    for (i = 0; TtyCommands[i].menu; i++) {
	j = strncmp(NthWord(TtyCommands[i].line, word), linep, len);
	if (j == 0 && match == -1) match = i;
	if (j > 0) {
	    if (match == -1) {
		return(NULL);
	    }
	    if (match == i - 1) {
		return(i);
	    }
	    if (NumWords(line) > 1) {
		word = NextWord(line);
		p = index(word, ' ');
		if (p)
		  len = p - word;
		else
		  len = strlen(word);
		for (k = match; k < i; i++) {
		}
	    }
	    for (j = match; j < i; j++) {
		write(1, "\r\n  ", 4);
		write(1, TtyCommands[j].line, strlen(TtyCommands[j].line));
	    }
	    write(1, "\r\n", 2);
	    return;
	}
    }
    printf("How'd we get here? j=%d, i=%d\r\n", j, i); fflush(stdout);
}

do_help(line)
char *line;
{
    int i, j, k, match, len;
    char *p, *word;

    p = index(line, ' ');
    if (p)
      len = p - line;
    else
      len = strlen(line);
    match = -1;

    printf("Help(%s) len %d\r\n", line, len); fflush(stdout);

    for (i = 0; TtyCommands[i].menu; i++) {
	printf("strncmp(%s, %s, %d)\r\n", TtyCommands[i].line, line, len);
	fflush(stdout);
	j = strncmp(TtyCommands[i].line, line, len);
	if (j == 0 && match == -1) match = i;
	if (j > 0) {
	    if (match == -1) {
		write(1, "Nothing matches\r\n", 17);
		printf("Comparing %s\r\n", TtyCommands[i].line); fflush(stdout);
		return;
	    }
	    if (match == i - 1) {
		write(1, "Command, only one matches:\r\n  ", 30);
		write(1, TtyCommands[i-1].line, strlen(TtyCommands[i-1].line));
		write(1, "\r\n", 2);
		return;
	    }
	    if (NumWords(line) > 1) {
		word = NextWord(line);
		p = index(word, ' ');
		if (p)
		  len = p - word;
		else
		  len = strlen(word);
		for (k = match; k < i; i++) {
		}
	    }
	    write(1, "Command, one of:", 16);
	    for (j = match; j < i; j++) {
		write(1, "\r\n  ", 4);
		write(1, TtyCommands[j].line, strlen(TtyCommands[j].line));
	    }
	    write(1, "\r\n", 2);
	    return;
	}
    }
    printf("How'd we get here? j=%d, i=%d\r\n", j, i); fflush(stdout);
}

do_complete(line)
char *line;
{
}

execute_command(line)
char *line;
{
}


static NumWords(s)
char *s;
{
    int ret;

    for (ret = 1; *s; s++)
      if (*s == ' ') ret++;
    return(ret);
}


static parse_menus()
{
    int arg;

    TtyCommands = (struct TtyMenu *)malloc(sizeof(struct TtyMenu) *
					   NumMenus * 3);
    arg = 0;
    parse_menu_recursive(&MenuRoot, "", &arg);
}


static parse_menu_recursive(m, parent, i)
MenuItem *m;
char *parent;
int *i;
{
    char buf[64], cmd[64], *word;
    struct parse_node *p, **prev;
    int j;

    if (m->submenu == NULL) {
	if (!strcmp(parent, "file") ||
	    !strcmp(parent, "misc"))
	  parent = "";
	if (NumWords(m->label) > 2)
	  strcpy(cmd, m->label);
	else
	  sprintf(cmd, "%s %s", m->label, parent);
	/* insert command into parse tree */
	p = &TtyCommands[0];
	prev = (struct parse_node **) NULL;
	for (word = cmd; word != (char  *)1; word = index(word, ' ') + 1) {
	    s = index(word, ' ');
	    if (s)
	      len = s - word;
	    else
	      len = strlen(word);
	    for (; !p || p->p_peer; p = p->p_peer) {
		if (!p || (j = strncmp(p->data, word, len)) < 0) {
		    TtyCommands[*i].p_type = KEYWORD;
		    strcpy(buf, word);
		    buf[len] = 0;
		    TtyCommands[*i].p_word = strsave(buf);
		    TtyCommands[*i].p_noise = NULL;
		    TtyCommands[*i].p_param = m;
		    TtyCommands[*i].p_help = NULL;
		    TtyCommands[*i].p_ret = NONE;
		    TtyCommands[*i].p_peer = p;
		    if (prev)
		      prev->p_peer = &TtyCommands[*i];
		    TtyCommands[*i].p_next = (struct parse_node *) NULL;
		    TtyCommands[*i].p_link = (struct parse_node *) NULL;
		    i++;
		    break;
		} else if (j == 0) {
		    prev = &(p->p_next);
		    if (p->p_next)
		      p = p->next;
		    else
		      p = (struct parse_node *) NULL;
		    break;
		} else {
		    prev = &(p->p_peer);
		}
	    }
	}
	*i = *i + 1;
	return;
    }
    if (!strcmp(m->label, "?help")) return;
    for (j = 0; m->submenu[j]; j++) {
	if (m == &MenuRoot)
	  buf[0] = 0;
	else if (strlen(parent))
	  sprintf(buf, "%s %s", parent, m->label);
	else
	  strcpy(buf, m->label);
	parse_menu_recursive(m->submenu[j], buf, i);
    }
}


TtyForm(f)
EntryForm *f;
{
}
