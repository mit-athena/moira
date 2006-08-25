/* $Id: mrtest.c,v 1.51 2006-08-25 18:42:39 zacheiss Exp $
 *
 * Bare-bones Moira client
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>

#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define dup    _dup
#define dup2   _dup2
#define isatty _isatty
#define close  _close
#define open   _open
#define sigjmp_buf jmp_buf
#define siglongjmp longjmp
#define sigsetjmp(env, save) setjmp(env)
#endif /* _WIN32 */

#ifdef HAVE_READLINE
#include "readline/readline.h"
#include "readline/history.h"
#endif

RCSID("$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mrtest/mrtest.c,v 1.51 2006-08-25 18:42:39 zacheiss Exp $");

int recursion = 0, quote_output = 0, interactive;
int count, quit = 0, cancel = 0;
char *whoami;

sigjmp_buf jb;

#define MAXARGS 20

void discard_input(int sig);
char *mr_gets(char *prompt, char *buf, size_t len);
void execute_line(char *cmdbuf);
int parse(char *buf, char *argv[MAXARGS]);
int print_reply(int argc, char **argv, void *help);
void test_noop(void);
void test_connect(int argc, char **argv);
void test_disconnect(void);
void test_host(void);
void test_motd(void);
void test_query(int argc, char **argv);
void test_auth(void);
void test_proxy(int argc, char **argv);
void test_access(int argc, char **argv);
void test_dcm(void);
void test_script(int argc, char **argv);
void test_list_requests(void);
void test_version(int argc, char **argv);
void test_krb5_auth(void);
void set_signal_handler(int, void (*handler)(int));
void set_signal_blocking(int, int);

int main(int argc, char **argv)
{
  char cmdbuf[BUFSIZ];
  int c;

  whoami = argv[0];
  interactive = (isatty(0) && isatty(1));

  while ((c = getopt(argc, argv, "q")) != -1)
    {
      switch (c)
	{
	case 'q':
	  quote_output = 1;
	  break;

	default:
	  fprintf(stderr, "Usage: mrtest [-q]\n");
	  exit (1);
	}
    }

  initialize_sms_error_table();
  initialize_krb_error_table();

#ifdef HAVE_READLINE
  /* we don't want filename completion */
  rl_bind_key('\t', rl_insert);
#endif

  set_signal_handler(SIGINT, discard_input);
  sigsetjmp(jb, 1);

  while (!quit)
    {
      if (!mr_gets("moira:  ", cmdbuf, BUFSIZ))
	break;
      execute_line(cmdbuf);
    }
  mr_disconnect();
  exit(0);
}

void discard_input(int sig)
{
  putc('\n', stdout);

  /* if we're inside a script, we have to clean up file descriptors,
     so don't jump out yet */
  if (recursion)
    cancel = 1;
  else
    siglongjmp(jb, 1);
}

char *mr_gets(char *prompt, char *buf, size_t len)
{
  char *in;
#ifdef HAVE_READLINE
  if (interactive)
    {
      in = readline(prompt);

      if (!in)
	return NULL;
      if (*in)
	add_history(in);
      strncpy(buf, in, len - 1);
      buf[len] = 0;
      free(in);

      return buf;
    }
#endif
  printf("%s", prompt);
  fflush(stdout);
  in = fgets(buf, len, stdin);
  if (!in)
    return in;
  if (strchr(buf, '\n'))
    *(strchr(buf, '\n')) = '\0';
  return buf;
}

void execute_line(char *cmdbuf)
{
  int argc;
  char *argv[MAXARGS];

  argc = parse(cmdbuf, argv);
  if (argc == 0)
    return;
  if (!strcmp(argv[0], "noop"))
    test_noop();
  else if (!strcmp(argv[0], "connect") || !strcmp(argv[0], "c"))
    test_connect(argc, argv);
  else if (!strcmp(argv[0], "disconnect") || !strcmp(argv[0], "d"))
    test_disconnect();
  else if (!strcmp(argv[0], "host"))
    test_host();
  else if (!strcmp(argv[0], "motd") || !strcmp(argv[0], "m"))
    test_motd();
  else if (!strcmp(argv[0], "query") || !strcmp(argv[0], "qy"))
    test_query(argc, argv);
  else if (!strcmp(argv[0], "auth") || !strcmp(argv[0], "a"))
    test_krb5_auth();
  else if (!strcmp(argv[0], "proxy") || !strcmp(argv[0], "p"))
    test_proxy(argc, argv);
  else if (!strcmp(argv[0], "access"))
    test_access(argc, argv);
  else if (!strcmp(argv[0], "dcm"))
    test_dcm();
  else if (!strcmp(argv[0], "script") || !strcmp(argv[0], "s"))
    test_script(argc, argv);
  else if (!strcmp(argv[0], "list_requests") ||
	  !strcmp(argv[0], "lr") || !strcmp(argv[0], "?"))
    test_list_requests();
  else if (!strcmp(argv[0], "quit") || !strcmp(argv[0], "Q"))
    quit = 1;
  else if (!strcmp(argv[0], "version") || !strcmp(argv[0], "v"))
    test_version(argc, argv);
  else if (!strcmp(argv[0], "krb4_auth") || !strcmp(argv[0], "4"))
    test_auth();
  else
    {
      fprintf(stderr, "moira: Unknown request \"%s\".  "
	      "Type \"?\" for a request list.\n", argv[0]);
    }
}

int parse(char *buf, char *argv[MAXARGS])
{
  char *p;
  int argc, num;

  if (!*buf)
    return 0;

  for (p = buf, argc = 0, argv[0] = buf; *p && *p != '\n'; p++)
    {
      if (*p == '"')
	{
	  char *d = p++;
	  /* skip to close-quote, copying back over open-quote */
	  while (*p != '"')
	    {
	      if (!*p || *p == '\n')
		{
		  fprintf(stderr,
			  "moira: Unbalanced quotes in command line\n");
		  return 0;
		}
	      /* deal with \### or \\ */
	      if (*p == '\\')
		{
		  if (*++p != '"' && (*p < '0' || *p > '9') && (*p != '\\'))
		    {
		      fprintf(stderr, "moira: Bad use of \\\n");
		      return 0;
		    }
		  else if (*p >= '0' && *p <= '9')
		    {
		      num = (*p - '0') * 64 + (*++p - '0') * 8 + (*++p - '0');
		      *p = num;
		    }
		}
	      *d++ = *p++;
	    }
	  if (p == d + 1)
	    {
	      *d = '\0';
	      p++;
	    }
	  else
	    {
	      while (p >= d)
		*p-- = ' ';
	    }
	}
      if (*p == ' ' || *p == '\t')
	{
	  /* skip whitespace */
	  for (*p++ = '\0'; *p == ' ' || *p == '\t'; p++)
	    ;
	  if (*p && *p != '\n')
	    argv[++argc] = p--;
	}
    }
  if (*p == '\n')
    *p = '\0';
  return argc + 1;
}

void test_noop(void)
{
  int status = mr_noop();
  if (status)
    com_err("moira (noop)", status, "");
}

void test_connect(int argc, char *argv[])
{
  char *server = "";
  int status;

  if (argc > 1)
    server = argv[1];
  status = mr_connect(server);
  if (status)
    com_err("moira (connect)", status, "");
  mr_version(-1);
}

void test_disconnect(void)
{
  int status = mr_disconnect();
  if (status)
    com_err("moira (disconnect)", status, "");
}

void test_host(void)
{
  char host[BUFSIZ];
  int status;

  memset(host, 0, sizeof(host));

  if ((status = mr_host(host, sizeof(host) - 1)))
    com_err("moira (host)", status, "");
  else
    printf("You are connected to host %s\n", host);
}

void test_auth(void)
{
  int status;

  status = mr_auth("mrtest");
  if (status)
    com_err("moira (auth)", status, "");
}

void test_krb5_auth(void)
{
  int status;

  status = mr_krb5_auth("mrtest");
  if (status)
    com_err("moira (krb5_auth)", status, "");
}

void test_proxy(int argc, char *argv[])
{
  int status;

  if (argc != 3)
    {
      com_err("moira (proxy)", 0, "Usage: proxy principal authtype");
      return;
    }
  status = mr_proxy(argv[1], argv[2]);
  if (status)
    com_err("moira (proxy)", status, "");
}

void test_script(int argc, char *argv[])
{
  FILE *inp;
  char input[BUFSIZ], *cp;
  int status, oldstdout, oldstderr;

  if (recursion > 8)
    {
      com_err("moira (script)", 0, "too many levels deep in script files\n");
      return;
    }

  if (argc < 2)
    {
      com_err("moira (script)", 0, "Usage: script input_file [ output_file ]");
      return;
    }

  inp = fopen(argv[1], "r");
  if (!inp)
    {
      sprintf(input, "Cannot open input file %s", argv[1]);
      com_err("moira (script)", 0, input);
      return;
    }

  if (argc == 3)
    {
      printf("Redirecting output to %s\n", argv[2]);
      fflush(stdout);
      oldstdout = dup(1);
      close(1);
      status = open(argv[2], O_CREAT|O_WRONLY|O_APPEND, 0664);
      if (status != 1)
	{
	  close(status);
	  dup2(oldstdout, 1);
	  argc = 2;
	  sprintf(input, "Unable to redirect output to %s\n", argv[2]);
	  com_err("moira (script)", errno, input);
	}
      else
	{
	  fflush(stderr);
	  oldstderr = dup(2);
	  close(2);
	  dup2(1, 2);
	}
    }

  recursion++;

  while (!cancel)
    {
      if (!fgets(input, BUFSIZ, inp))
	break;
      if ((cp = strchr(input, '\n')))
	*cp = '\0';
      if (input[0] == 0)
	{
	  printf("\n");
	  continue;
	}
      if (input[0] == '%')
	{
	  for (cp = &input[1]; *cp && isspace(*cp); cp++)
	    ;
	  printf("Comment: %s\n", cp);
	  continue;
	}
      printf("Executing: %s\n", input);
      execute_line(input);
    }

  recursion--;
  if (!recursion)
    cancel = 0;

  fclose(inp);
  if (argc == 3)
    {
      fflush(stdout);
      close(1);
      dup2(oldstdout, 1);
      close(oldstdout);
      fflush(stderr);
      close(2);
      dup2(oldstderr, 2);
      close(oldstderr);
    }
}

int print_reply(int argc, char **argv, void *help)
{
  int i;
  for (i = 0; i < argc; i++)
    {
      if (i != 0)
	printf(", ");
      if (quote_output && !*(int *)help)
	{
	  unsigned char *p;

	  for (p = (unsigned char *)argv[i]; *p; p++)
	    {
	      if (isprint(*p) && *p != '\\' && *p != ',')
		putc(*p, stdout);
	      else
		printf("\\%03o", *p);
	    }
	}
      else
	printf("%s", argv[i]);
    }
  printf("\n");
  count++;
  return MR_CONT;
}

void test_query(int argc, char **argv)
{
  int status, help;

  if (argc < 2)
    {
      com_err("moira (query)", 0, "Usage: query handle [ args ... ]");
      return;
    }
  help = !strcmp(argv[1], "_help");

  count = 0;
  /* Don't allow ^C during the query: it will confuse libmoira's
     internal state. (Yay static variables) */
  set_signal_blocking(SIGINT, 1);
  status = mr_query(argv[1], argc - 2, argv + 2, print_reply, &help);
  set_signal_blocking(SIGINT, 0);
  printf("%d tuple%s\n", count, ((count == 1) ? "" : "s"));
  if (status)
    com_err("moira (query)", status, "");
}

void test_access(int argc, char **argv)
{
  int status;
  if (argc < 2)
    {
      com_err("moira (access)", 0, "Usage: access handle [ args ... ]");
      return;
    }
  status = mr_access(argv[1], argc - 2, argv + 2);
  if (status)
    com_err("moira (access)", status, "");
}

void test_dcm()
{
  int status;

  if ((status = mr_do_update()))
    com_err("moira (dcm)", status, " while triggering dcm");
}

void test_motd()
{
  int status;
  char *motd;

  if ((status = mr_motd(&motd)))
    com_err("moira (motd)", status, " while getting motd");
  if (motd)
    printf("%s\n", motd);
  else
    printf("No message of the day.\n");
}

void test_list_requests(void)
{
  printf("Available moira requests:\n");
  printf("\n");
  printf("noop\t\t\tAsk Moira to do nothing\n");
  printf("connect, c\t\tConnect to Moira server\n");
  printf("disconnect, d\t\tDisconnect from server\n");
  printf("host\t\t\tIdentify the server host\n");
  printf("motd, m\t\t\tGet the Message of the Day\n");
  printf("query, qy\t\tMake a query.\n");
  printf("auth, a\t\t\tAuthenticate to Moira via krb5.\n");
  printf("krb4_auth, 4\t\tAuthenticate to Moira via krb4.\n");
  printf("proxy, p\t\tProxy authenticate to Moira.\n");
  printf("access\t\t\tCheck access to a Moira query.\n");
  printf("dcm\t\t\tTrigger the DCM\n");
  printf("script, s\t\tRead commands from a script.\n");
  printf("list_requests, lr, ?\tList available commands.\n");
  printf("quit, Q\t\t\tLeave the subsystem.\n");
}

void test_version(int argc, char **argv)
{
  int status;

  if (argc != 2)
    {
      com_err("moira (version)", 0, "Usage: version versionnumber");
      return;
    }
  status = mr_version(atoi(argv[1]));
  if (status)
    com_err("moira (version)", status, "");
}

#ifdef HAVE_POSIX_SIGNALS

void set_signal_handler(int sig, void (*handler)(int))
{
  struct sigaction action;

  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  action.sa_handler = handler;
  sigaction(sig, &action, NULL);
}

void set_signal_blocking(int sig, int block)
{
  sigset_t sigs;
  sigemptyset(&sigs);
  sigaddset(&sigs, sig);
  sigprocmask(block ? SIG_BLOCK : SIG_UNBLOCK, &sigs, NULL);
}

#else

void set_signal_handler(int sig, void (*handler)(int))
{
  signal(sig, handler);
}

#ifdef _WIN32
BOOL WINAPI blocking_handler(DWORD dwCtrlType)
{
  return(TRUE);
}

void set_signal_blocking(int sig, int block)
{
  SetConsoleCtrlHandler(blocking_handler, block ? TRUE : FALSE);
}
#endif /* _WIN32 */

#endif /* HAVE_POSIX_SIGNALS */
