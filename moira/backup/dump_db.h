/* $Id$ */

#include <stdio.h>
#include <moira.h>

#define SEP_CHAR ('|')

void do_backups(char *prefix);
void do_restores(char *prefix);

void dump_int(FILE *f, int n);
void dump_str(FILE *f, char *str);
#define dump_nl(f) { if (putc('\n', f) < 0) wpunt(); }
#define dump_sep(f) { if (putc(SEP_CHAR, f) < 0) wpunt(); }

int parse_int(FILE *f);
void parse_str(FILE *f, char *buf, int maxlen);
void parse_sep(FILE *f);
void parse_nl(FILE *f);

void wpunt(void);
void punt(char *msg);
void dbmserr(void);
void safe_close(FILE *stream);
FILE *open_file(char *prefix, char *suffix);
char *endtrim(char *save);
