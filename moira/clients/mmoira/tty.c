/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/tty.c,v 1.2 1992-12-10 11:00:53 mar Exp $
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
#include	<moira.h>
#include	"mmoira.h"
#include	"parser.h"

static char rcsid[] = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/tty.c,v 1.2 1992-12-10 11:00:53 mar Exp $";


struct parse_node *TtyCommands = NULL, *TtyRoot = NULL;

extern int NumMenus;
extern MenuItem MenuRoot;
char prompt[] = "moira> ";

static struct sgttyb otty, ntty;


TtyMainLoop()
{
    char buf[1024], cbuf[2], c, *p;
    int arg, i, done;

    if (TtyCommands == NULL)
      parse_menus();

#ifdef DEBUG
    print_parse_tree(TtyRoot, 0);
    fflush(stdout);
    sleep(10);
#endif /* DEBUG */

    ioctl(0, TIOCFLUSH, &arg);
    ioctl(0, TIOCGETP, &otty);
    ntty = otty;
    ntty.sg_flags |= RAW;
    ntty.sg_flags &= ~ECHO;
    raw_mode();

    while (1) {
	parser(prompt, TtyRoot);
    }
}

cooked_mode()
{
    ioctl(0, TIOCSETP, &otty);
}

raw_mode()
{
    ioctl(0, TIOCSETP, &ntty);
}

static NumWords(s)
char *s;
{
    int ret;

    for (ret = 1; *s; s++)
      if (*s == ' ') ret++;
    return(ret);
}


parse_menus()
{
    int arg;
    struct parse_node *p;

    TtyCommands = (struct parse_node *)malloc(sizeof(struct parse_node) *
					      NumMenus * 3);
    bzero(TtyCommands, sizeof(struct parse_node) * NumMenus * 3);
    arg = 0;
    parse_menu_recursive(&MenuRoot, "", &arg);
    TtyCommands[arg].p_word = "help";
    TtyCommands[arg].p_next = TtyRoot;
    for (p = TtyRoot; p; p = p->p_peer)
      if (strcmp(p->p_peer->p_word, "help") > 0)
	break;
    TtyCommands[arg].p_peer = p->p_peer->p_peer;
    p->p_peer = &TtyCommands[arg];
    arg++;
}


parse_menu_recursive(m, parent, i)
MenuItem *m;
char *parent;
int *i;
{
    char buf[64], cmd[64], *word, *s;
    struct parse_node *p, **prev;
    int j, len;

    if (m->submenu == NULL) {
	if (!strcmp(parent, "file") ||
	    !strcmp(parent, "misc"))
	  parent = "";
	if (NumWords(m->label) > 2 || !*parent)
	  strcpy(cmd, m->label);
	else
	  sprintf(cmd, "%s %s", m->label, parent);
	/* insert command into parse tree */
	p = TtyRoot;
	prev = (struct parse_node **) &TtyRoot;
	for (word = cmd; word; word = index(word, ' ')) {
	    if (*word == ' ')
	      word++;
	    s = index(word, ' ');
	    if (s)
	      len = s - word;
	    else
	      len = strlen(word);
	    for (;;) {
		if (!p || !p->p_word ||
		    (j = strncmp(p->p_word, word, len)) > 0) {
		    strcpy(buf, word);
		    buf[len] = 0;
#ifdef DEBUG
		    printf("word: %s\n", buf);
#endif
		    TtyCommands[*i].p_word = strsave(buf);
		    if (len == strlen(word))
		      TtyCommands[*i].p_menu = m;
		    TtyCommands[*i].p_peer = p;
		    if (prev)
		      *prev = &TtyCommands[*i];
		    TtyCommands[*i].p_next = (struct parse_node *) NULL;
		    TtyCommands[*i].p_link = (struct parse_node *) NULL;
		    prev = &(TtyCommands[*i].p_next);
		    p = (struct parse_node *) NULL;
		    *i = *i + 1;
		    break;
		} else if (j == 0) {
		    prev = &(p->p_next);
		    if (p->p_next)
		      p = p->p_next;
		    else
		      p = (struct parse_node *) NULL;
		    break;
		} else {
		    prev = &(p->p_peer);
		    p = p->p_peer;
		}
	    }
	}
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
    int i, j, done, best, choice;
    char buf[512], *k;
    UserPrompt *p;

    printf("%s\r\n", f->instructions);
    choice = 0;
    for (i = 0, p = f->inputlines[i]; p = f->inputlines[i]; i++)
      if (p->choice) {
	  choice = 1;
	  break;
      }
    if (choice) {
	j = 0;
	*buf = 0;
	for (i = 0, p = f->inputlines[i]; p = f->inputlines[i]; i++) {
	    if (p->choice) {
		printf("  %s\r\n", buf);
		sprintf(buf, "%d: %s", ++j, p->prompt);
	    } else if (*buf == 0) {
		sprintf(buf, "%d: %s", ++j, p->prompt);
	    } else {
		strcat(buf, "/");
		strcat(buf, p->prompt);
	    }
	    if (buf[strlen(buf)-1] == ' ') buf[strlen(buf)-1] = 0;
	}
	printf("  %s\r\nSelect by [1]: ", buf);
	if (mgets(buf, sizeof(buf)))
	  return;
	if (buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = 0;
	if (strlen(buf) == 0)
	  choice = 1;
	else
	  choice = atoi(buf);
    } else choice = -1;
    for (i = 0, p = f->inputlines[i]; p = f->inputlines[i]; i++) {
	if (choice > 0 && p->choice) choice--;
	if (choice != -1 && choice != 1) continue;
	if (p->insensitive == True) continue;
	switch (p->type) {
	case FT_BOOLEAN:
	    done = 0;
	    while (!done) {
		done = 1;
		printf("%s(T/F): ", p->prompt);
		fflush(stdout);
		if (mgets(buf, sizeof(buf)))
		  return;
		if (buf[0] == 'T' || buf[0] == 't' ||
		    buf[0] == 'Y' || buf[0] == 'y')
		  p->returnvalue.booleanvalue = True;
		else if (buf[0] == 'F' || buf[0] == 'f' ||
			 buf[0] == 'N' || buf[0] == 'n')
		  p->returnvalue.booleanvalue = False;
		else {
		    done = 0;
		    printf("Please answer True or False\r\n");
		}
	    }
	    break;
	case FT_STRING:
	    if (*StringValue(f, i))
	      printf("%s[%s]: ", p->prompt, StringValue(f, i));
	    else
	      printf("%s: ", p->prompt);
	    fflush(stdout);
	    if (mgets(buf, sizeof(buf)))
	      return;
	    if (buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = 0;
	    if (strlen(buf) != 0)
	      StoreField(f, i, buf);
	    if (!strcmp(buf, "\"\""))
	      StoreField(f, i, "");
	    break;
	case FT_KEYWORD:
	    k = index(p->prompt, '|');
	    if (k) *k = 0;
	    done = 0;
	    while (done != 1) {
		if (*StringValue(f, i))
		  printf("%s[%s]: ", p->prompt, StringValue(f, i));
		else
		  printf("%s: ", p->prompt);
		fflush(stdout);
		if (mgets(buf, sizeof(buf)))
		  return;
		if (buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = 0;
		if (strlen(buf) == 0)
		  strcpy(buf, StringValue(f, i));
		done = 0;
		for (j = 0; p->keywords[j]; j++)
		  if (!strncasecmp(p->keywords[j], buf, strlen(buf)))
		    done++, best = j;
		if (done != 1) {
		    printf("You must enter one of these keywords:\r\n");
		    for (j = 0; p->keywords[j]; j++) {
			printf("  %s\r\n", p->keywords[j]);
		    }
		} else
		  StoreField(f, i, p->keywords[best]);
	    }
	    break;
	}
    }
    process_form(f, TRUE);
}


int mgets(line, linelen)
char *line;
int linelen;
{
    char c, *p;

    raw_mode();
    p = &line[0];
    *p = 0;
    for (c = (getchar() & 0x7f); 1; c = (getchar() & 0x7f)) {
	if (c == 0) continue;
	switch (c) {
	case 127:
	case '\b':
	    if (p == &line[0]) {
		putchar(7);
		break;
	    }
	    *(--p) = 0;
	    write(1, "\b \b", 3);
	    break;
	case 'C' - '@':
	case 'G' - '@':
	    cooked_mode();
	    return(-1);
	case 'Q' - '@':
	case 'V' - '@':
	    putchar('\\');
	    c = getchar();
	    if (c < ' ')
	      printf("\b^%c", c + '@');
	    else
	      printf("\b%c", c);
	    fflush(stdout);
	    *p++ = c;
	    break;
	case 'W' - '@':
	    if (p > &line[0])
	      p--;
	    while ((p >= &line[0]) && isspace(*p)) {
		write(1, "\b \b", 3);
		p--;
	    }
	    while ((p >= &line[0]) && !isspace(*p)) {
		write(1, "\b \b", 3);
		p--;
	    }
	    if (p > &line[0]) {
		p++;
	    } else {
		p = &line[0];
	    }
	    *p = 0;
	    break;
	case 'Z' - '@':
	    printf("\r\n");
	    kill(getpid(), SIGSTOP);
	    /* when continued, fall through to */
	case 'R' - '@':
	    *p = 0;
	    printf("\r\n%s", line);
	    fflush(stdout);
	    break;
	case 'U' - '@':
	    while (p-- > &line[0])
	      write(1, "\b \b", 3);
	    *(++p) = 0;
	    fflush(stdout);
	    break;
	case '\n':
	case '\r':
	    write(1, "\r\n", 2);
	    cooked_mode();
	    return(0);
	    break;
	default:
	    putchar(c);
	    *p++ = c;
	    *p = 0;
	}
    }
}


print_parse_tree(n, l)
struct parse_node *n;
int l;
{
    int i;

    for (i = l; i > 0; i--) putchar(' ');
    printf("%s%c\n", n->p_word, n->p_menu ? '*' : ' ');
    if (n->p_next)
      print_parse_tree(n->p_next, l+1);
    if (n->p_peer)
      return(print_parse_tree(n->p_peer, l));
}
