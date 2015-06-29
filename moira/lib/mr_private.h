/* $Id$
 *
 * Private declarations of the Moira library.
 *
 * Copyright (C) 1987-1998 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>
#include <moira.h>

#include <sys/types.h>

#ifndef _WIN32
#include <netinet/in.h>
#include <arpa/inet.h>
#endif /*_WIN32*/

extern int _mr_conn, mr_inited;

typedef struct mr_params {
  union {
    u_long mr_procno;     /* for call */
    u_long mr_status;     /* for reply */
  } u;
  int mr_argc;
  char **mr_argv;
  int *mr_argl;
  unsigned char *mr_flattened;
  u_long mr_filled;
} mr_params;

#define CHECK_CONNECTED if (!_mr_conn) return MR_NOT_CONNECTED

#define getlong(cp, l) do { l = ((((unsigned char *)cp)[0] * 256 + ((unsigned char *)cp)[1]) * 256 + ((unsigned char *)cp)[2]) * 256 + ((unsigned char *)cp)[3]; } while(0)
#define putlong(cp, l) do { ((unsigned char *)cp)[0] = (unsigned char)(l >> 24); ((unsigned char *)cp)[1] = (unsigned char)(l >> 16); ((unsigned char *)cp)[2] = (unsigned char)(l >> 8); ((unsigned char *)cp)[3] = (unsigned char)(l); } while(0)

/* prototypes from mr_call.h */
int mr_do_call(struct mr_params *params, struct mr_params *reply);
int mr_send(int fd, struct mr_params *params);
int mr_receive(int fd, struct mr_params *params);
int mr_cont_receive(int fd, struct mr_params *params);
void mr_destroy_reply(mr_params reply);

/* prototypes from mr_connect.h */
int mr_accept(int s, struct sockaddr_in *sin);
int mr_cont_accept(int conn, char **buf, int *nread);
int mr_connect_internal(char *server, char *port);
int mr_listen(char *port);

/* prototypes from mr_init.c */
void mr_init(void);
