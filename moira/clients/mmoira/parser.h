/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/parser.h,v 1.1 1992-12-10 10:58:56 mar Exp $
 * Constants and structures for TOPS-20 style parser
 *
 */

/* type codes for nodes & return values in parse tree */
#define NONE	0
#define KEYWORD 1
#define INTEGER 2
#define STRING  3
#define ADDRESS 4
#define PROC	5
#define ERROR	6
#define EXIT	7
#define ABORT	8


/* return codes from single_parse */
#define OK	0
#define MATCH	1	/* exact match found */
#define	INCOMP	2	/* incomplete field */
#define	AMBIG	3	/* ambiguous response */
#define NOMATCH	4	/* no match */


/* maximum line length for parser */
#define BUFLEN 128

/* the nodes which make up the parse tree */

struct parse_node {
	char	*p_word;	/* keyword, data description */
	MenuItem *p_menu;	/* MMoira menu entry for this command */
	struct parse_node *p_peer;	/* next node this level */
	struct parse_node *p_next;	/* next level nodes */
	struct parse_node *p_link;	/* for temporary chains */
};


#define NULNODE ((struct parse_node *) 0)


/* this structure is used for collecting data to pass to parse functions */

#define pval	struct pvalst

pval {
    char *p_dat;	/* data value or pointer */
    int p_free;		/* size of data to free */
};

/* how many return values are allowed in a single parse */
#define PMAXDAT	8
