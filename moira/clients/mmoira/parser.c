/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/parser.c,v 1.3 1993-01-04 10:21:57 mar Exp $
 *
 * TOPS-20 style command parser
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/signal.h>
#include <X11/Intrinsic.h>
#include "mmoira.h"
#include "parser.h"


/* trivial absolute-value macro */
#define abs(i)	((i) < 0 ? -(i) : (i))

/* message used when end of parse tree is reached */
char	eolmsg[] = "return to execute command";


/* main entry point.  Takes the prompt and top node of the parse tree, and
 * will collect user's input, providing help and completion.  When entry
 * is complete (<return> is pressed), the actions called for by each node
 * in the parse tree which is traversed will be executed.
 */

int	parser(prompt, nd)
char	*prompt;
struct parse_node	*nd;
{
    char line[BUFLEN];
    char *p, c;
    int	val;

    p = &line[0];
    *p = 0;
    write(1, prompt, strlen(prompt));
    for (c = (getchar() & 0x7F); 1; c = (getchar() & 0x7F)) {
	if (c == 0)
	  continue;
	switch (c) {
	case 127:
	case '\b':
	    if (p == &line[0]) {
		putchar(7);
		break;
	    }
	    if (*(--p) == 'I' - '@') {
		*p = 0;
		printf("\r%s%s", prompt, line);
		fflush(stdout);
		break;
	    }
	    *p = 0;
	    write(1, "\b \b", 3);
	    break;
	case 'C' - '@':
	case 'G' - '@':
	    write(1, " \007ABORT\r\n", 9);
	    return(ABORT);
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
	case '?':
	    write(1, "?  ", 3);
	    *p = 0;
	    do_help(&line[0], nd, prompt);
	    p = &line[strlen(line)];
	    printf("\n\r%s%s", prompt, line);
	    fflush(stdout);
	    break;
	case '[' - '@':
	case '\t':
	    *p = 0;
	    do_complete(&line[0], nd, prompt);
	    p = &line[strlen(line)];
	    break;
	case 'Z' - '@':
	    printf("\r\n");
	    cooked_mode();
	    kill(getpid(), SIGSTOP);
	    raw_mode();
	    /* when continued, fall through to */
	case 'R' - '@':
	    *p = 0;
	    printf("\r\n%s%s", prompt, line);
	    fflush(stdout);
	    break;
	case 'U' - '@':
	    while (p-- > &line[0])
	      write(1, "\b \b", 3);
	    *(++p) = 0;
	    printf("\r%s", prompt);
	    fflush(stdout);
	    break;
	case '\n':
	case '\r':
	    if ((val = do_parse(line, nd, prompt)) != ERROR) {
		write(1, "\r\n", 2);
		return(val);
	    }
	    p = &line[strlen(line)];
	    *p = 0;
	    printf("\r\n%s%s", prompt, line);
	    fflush(stdout);
	    break;
	default:
	    putchar(c);
	    *p++ = c;
	    *p = 0;
	}
    }
}


/* called when a ? is typed.  This parses the line as far as possible, then
 * displays possible completions and help strings.
 */

do_help(line, nod, prompt)
char *line;
struct parse_node *nod;
char *prompt;
{
    char *ln;
    struct parse_node *n, *nd, *last;
    int kw, state, nomatch;
    struct parse_node *best, *ambig;

    n = nod;
    state = MATCH;
    best = NULNODE;
    for (ln = line; n && state == MATCH; best && (n = best->p_next)) {
	last = best;
	state = single_parse(&ln, n, &best, &ambig, &nomatch);
    }
    if (*ln && ((best && best->p_menu) || n == NULNODE))
      state = NOMATCH;
    switch (state) {
    case NOMATCH:
	if (!*ln) {
	    if (last && last->p_menu  && *line)
	      printf("Carriage return, or ");
	    break;
	}
	ln += nomatch;
	*ln = 0;
	write(1, "\r\nNOT a valid command line", 26);
	return;
    case AMBIG:
	write(1, "one of the following:\r\n   ", 26);
	for (; ambig; ambig = ambig->p_link)
	  printf("%s ", ambig->p_word);
	fflush(stdout);
	return;
    case INCOMP:
	printf("one of the following:\r\n   %s", best->p_word);
	fflush(stdout);
	return;
    default:
	write(1, eolmsg, strlen(eolmsg));
	return;
    }
    kw = 0;
    for (nd = n; nd; nd = nd->p_peer) {
	if (!kw) {
	    write(1, "one of the following:\r\n   ", 26);
	    kw = 3;
	}
	if (kw + strlen(nd->p_word) > 72) {
	    printf("\r\n   %s ", nd->p_word);
	    fflush(stdout);
	    kw = 4 + strlen(nd->p_word);
	} else {
	    printf("%s ", nd->p_word);
	    fflush(stdout);
	    kw += 1 + strlen(nd->p_word);
	}
    }
}


/* Do escape processing.  If a unique completion exists, use it.  Otherwise,
 * do the same as ?.
 */

do_complete(line, nod, prompt)
char *line;
struct parse_node *nod;
char *prompt;
{
    struct parse_node *n, *nd;
    char *ln;
    char *tmp;
    int	state, nomatch;
    struct parse_node *best, *ambig;

    ln = line;
    state = MATCH;
    nd = nod;
    while (state == MATCH) {
	tmp = ln;
	state = single_parse(&ln, nd, &best, &ambig, &nomatch);
	if (state == MATCH)
	  nd = best->p_next;
    }
    switch (state) {
    case AMBIG:
/*	printf("ln %X, tmp %X, ln - tmp %d, nomatch %d\r\n", ln, tmp, ln-tmp, nomatch);
	if (ln - tmp < nomatch) {
	    printf("attempting partial complete\r\n");
	    fflush(stdout);
	    sleep(1);
	    while ((ln > line) && !isspace(*ln))
	      ln--;
	    tmp = ambig->p_word;
	    while (nomatch--)
	      *ln++ = *tmp++;
	    *ln = 0;
	    putchar(7);
	    return;
	} */
	/* fall through to: */
    case NOMATCH:
	if (!(nd) || (nd->p_peer)) {
	    write(1, "  ", 2);
	    do_help(line, nod, prompt);
	    printf("\r\n%s%s", prompt, line);
	    fflush(stdout);
	    return;
	}
	best = nd;
	*ln++ = 'x';
	/* fall through to incomplete case */
    case INCOMP:
	ln = tmp;
	do {
	    tmp = best->p_word;
	    while (*tmp)
	      *ln++ = *tmp++;
	    *ln++ = ' ';
	    *ln = 0;
	} while (best->p_next && !best->p_next->p_peer &&
		 !best->p_menu && (best = best->p_next));
	printf("\r%s%s", prompt, line);
	fflush(stdout);
	break;
    default:
	write(1, "We shouldn't get here (parser error)\r\n", 38);
    }
}


/* Single parse parses through a single level of the parse tree.
 * There are 4 possible outcomes:
 *	an exact match is made: the matching node is returned, ambig = 0
 *	an incomplete match: the matching node is returned, ambig = node
 *	ambiguous: nothing is returned, ambig = list of completions
 *	no matches: nothing is returned, ambig = 0
 */

int single_parse(line, nd, best, ambig, nomatch)
char **line;
struct parse_node *nd;
struct parse_node **best;
struct parse_node **ambig;
int *nomatch;
{
    char *p;
    char c;			/* char we're working on (from line) */
    struct parse_node *n;	/* node loop counter */
    struct parse_node *tail;	/* tmp used to build chains */
    int i;			/* loop counter */
    int	match;			/* how many chars have we matched? */
    int len;			/* length of this keyword */

#ifdef DEBUG
    printf("single_parse(\"%s\") -> ", *line);
#endif /* DEBUG */
    *ambig = tail = *best = NULNODE;
    match = *nomatch = 0;
    /* skip leading whitespace */
    while (isspace(**line))
      (*line)++;
    /* step through each node */
    for (n = nd; n; n = n->p_peer) {
	len = strlen(n->p_word);
	/* step through each character in line */
	for (i = 0; 1; i++) {
	    /* if at end of word on line */
	    if (isspace((*line)[i]) || (*line)[i] == 0) {
		/* another ambiguous match */
		if (i == match && i) {
		    tail->p_link = n;
		    tail = n;
		    n->p_link = NULNODE;
		}
		/* a better match */
		if (i > match) {
		    match = i;
		    *best = tail = *ambig = n;
		    n->p_link = NULNODE;
		}
		break;
	    }
	    if (isupper(c = (*line)[i]))
	      c = tolower(c);
	    if (c != n->p_word[i]) {
		if (i > *nomatch)
		  *nomatch = i;
		break;
	    }
	}
    }
    if (match > 0) {
	(*line) += match;
	if (tail != *ambig) {
	    *best = NULNODE;
	    *nomatch = match;
	    if (isspace(**line)) {
#ifdef DEBUG
		printf("NOMATCH\n");
#endif /* DEBUG */
		return(NOMATCH);
	    } else {
#ifdef DEBUG
		printf("AMBIG\n");
#endif /* DEBUG */
		return(AMBIG);
	    }
	}
	if (isspace(**line)) {
	    *ambig = NULNODE;
	    while (isspace(**line))
	      (*line)++;
#ifdef DEBUG
	    printf("MATCH\n");
#endif /* DEBUG */
	    return(MATCH);
	}
#ifdef DEBUG
	printf("INCOMP\n");
#endif /* DEBUG */
	return(INCOMP);
    }
    *ambig = tail = *best = NULNODE;
#ifdef DEBUG
    printf("NOMATCH\n");
#endif /* DEBUG */
    return(NOMATCH);
}


/* execute the line.  First check to see that the line is legal.  If not,
 * do_help the line & return ERROR.  If so, execute each node passed through,
 * and return OK (or EXIT if an EXIT node was encountered).
 */

int do_parse(line, nod, prompt)
char *line;
struct parse_node *nod;
char *prompt;
{
    struct parse_node *n, *nd, *last;
    char *ln, *tmp;
    int state, i;
    struct parse_node *best, *ambig;
    int	nomatch;
    EntryForm *f;

    ln = line;
    n = nod;
    state = MATCH;
    best = NULNODE;
    while (n && ((state == MATCH) || (state == INCOMP))) {
	last = best;
	state = single_parse(&ln, n, &best, &ambig, &nomatch);
#ifdef DEBUG
	printf("best = %s, best->next = 0x%x\r\n", best ? best->p_word : "",
	       best ? best->p_next : 0);
#endif /* DEBUG */
	if ((state == NOMATCH) && (!*ln) && (n == nod))
	  return(OK);
	if ((state == MATCH) || (state == INCOMP)) {
	    n = best->p_next;
	}
    }
    if (((state == AMBIG) || (state == NOMATCH)) && !*ln)
      for (; n; n = n->p_peer)
	if (last && last->p_menu) {
	    n = last;
	    state = MATCH;
	    break;
	}
    if (state == NOMATCH && !*ln) {
	while (last && last->p_next && !last->p_next->p_peer && !last->p_menu)
	  last = last->p_next;
	if (last && last->p_menu) {
	    state = MATCH;
	    best = last;
	}
    }

    if ((state == NOMATCH) || (state == AMBIG)) {
	write(1, "  BAD command,  ", 16);
	do_help(line, nod, prompt);
	return(ERROR);
    }
    if (!best)
      best = n;
    write(1, "\r\n", 2);
    cooked_mode();
    for (i = 0; line[i] && !isspace(line[i]); i++);
    if (!strncmp("help", line, i))
      help(best->p_menu->form);
    else
      MoiraMenuRequest(best->p_menu);
    raw_mode();
    return(OK);
}
