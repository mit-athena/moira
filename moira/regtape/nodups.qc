/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/regtape/nodups.qc,v 1.1 1993-03-10 13:49:54 mar Exp $ */

#include <stdio.h>
#include <ctype.h>
#include <moira.h>

struct user {
    int	users_id;
    char first[33];
    char last[33];
    int status;
};

extern int errno;
int debug;
char *program;


main(argc, argv)
int argc;
char  **argv;
{
    char buf[1024], first[33], last[33];
    int i;
    char *inputfile;
    FILE *in;
    struct user *u;
    struct user users[32];
    struct user *get_user();

    program = argv[0];

    for (i = 1; i < argc; i++) {
	if (!strcmp(argv[i], "-d"))
	  debug++;
	else if (!strcmp(argv[i], "-D"))
	  setenv("ING_SET", "set printqry");
	else if (argv[i][0] == '-')
	  usage(argv);
	else if (inputfile != NULL)
	  usage(argv);
	else inputfile = argv[i];
    }

    if (inputfile) {
	in = fopen(inputfile, "r");
	if (in == NULL) {
	    com_err(program, errno, "opening file \"%s\" for input", inputfile);
	    exit(1);
	}
    } else
      in = stdin;

##  ingres sms

    i = 0;
    while (!feof(in)) {
	fgets(buf, sizeof(buf), in);
	u = get_user(buf);
	if (!u) continue;
	if (strcmp(u->last, users[0].last) ||
	    strcmp(u->first, users[0].first)) {
	    /* We've got a matching group! */
	    process_users(users, i);
	    bcopy(u, &(users[0]), sizeof(*u));
	    i = 1;
	} else {
	    bcopy(u, &(users[i]), sizeof(*u));
	    i++;
	}
    }
}

usage(argv)
char **argv;
{
    fprintf(stderr, "Usage: %s [-w] [-D]\n", argv[0]);
    exit(1);
}

struct user *get_user(buf)
char *buf;
{
    static struct user u;

    if (sscanf(buf, "| %d|%s |%s | %d|", &u.users_id, u.first,
	       u.last, &u.status) != 4) {
	com_err(program, 0, " parsing input line %s", buf);
	return(NULL);
    }
    return(&u);
}


process_users(users, count)
struct user users[];
int count;
##{
    char buf[256];
    int i;
##  char login[9], id[17], class[9], modtime[26], modwith[9], xmodtime[26];
##  char login2[9], id2[17], class2[9], modtime2[26],modwith2[9],xmodtime2[26];
##  char login3[9], id3[17], class3[9], modtime3[26],modwith3[9],xmodtime3[26];
##  int users_id, users_id2, users_id3, status, status2, status3, rowcount;

    printf("\nGot %d users named %s %s\n", count, users[0].first, users[0].last);
    if (count < 2) return;

    users_id = users[0].users_id;
##  range of u is users
##  retrieve (login = u.#login, id = u.mit_id, class = u.mit_year,
##	      modtime = u.#modtime, modwith = u.#modwith,
##	      xmodtime = u.#xmodtime, status = u.#status)
##	where u.#users_id = users_id

    users_id2 = users[1].users_id;
##  retrieve (login2 = u.#login, id2 = u.mit_id, class2 = u.mit_year,
##	      modtime2 = u.#modtime, modwith2 = u.#modwith,
##	      xmodtime2 = u.#xmodtime, status2 = u.#status)
##	where u.#users_id = users_id2

    modtime[20] = modtime2[20] = xmodtime[20] = xmodtime2[20] = 0;

    printf("A: %s %5d %d\t\t\tB: %s %5d %d\n", login, users_id, status,
	   login2, users_id2, status2);
    printf("   %s %s\t\t   %s %s\n", class, id, class2, id2);
    printf("   %s %s\t   %s %s\n", modtime, modwith, modtime2, modwith2);
    printf("   %s\t\t\t   %s\n", xmodtime, xmodtime2);

    if (count > 2) {
	for (i = 2; i < count; i++) {
	    users_id3 = users[i].users_id;
##	    retrieve (login3 = u.#login, id3 = u.mit_id, class3 = u.mit_year,
##		      modtime3 = u.#modtime, modwith3 = u.#modwith,
##		      xmodtime3 = u.#xmodtime, status3 = u.#status)
##		where u.#users_id = users_id3
	    modtime3[20] = xmodtime3[20] = 0;
	    printf("\n%c: %s %5d %d\n", 'A' + i,
		   login3, users_id3, status3);
	    printf("   %s %s\n", class3, id3);
	    printf("   %s %s\n", modtime3, modwith3);
	    printf("   %s\n", xmodtime3);
	}
    }

    if (count == 2) {
	if (status == 3 || status2 == 3) return;
	if ((status == 0 || status == 4) &&
	    !isdigit(id[0]) && !isdigit(xmodtime[0])) {
	    printf("DELETING %s\n", login);
##	    replace u (#status = 3, #modtime=date("now"), #modwith="nodups",
##		       modby=8873) where u.#users_id = users_id
##	    inquire_equel(rowcount = "rowcount")
	    if (rowcount != 1)
	      printf("Failed!\007\007\007\n");
	    else
	      printf("User %s marked for deletion.\n", login);
	    return;
	}
	if ((status2 == 0 || status2 == 4) &&
	    !isdigit(id2[0]) && !isdigit(xmodtime2[0])) {
	    printf("DELETING %s\n", login2);
##	    replace u (#status = 3, #modtime=date("now"), #modwith="nodups",
##		       modby=8873) where u.#users_id = users_id2
##	    inquire_equel(rowcount = "rowcount")
	    if (rowcount != 1)
	      printf("Failed!\007\007\007\n");
	    else
	      printf("User %s marked for deletion.\n", login2);
	    return;
	}
    }

    printf("\n0,sp: Do nothing\n1: Delete B\n2: Delete A\n");
    printf("3: A<-B's ID; Delete B\n4: B<-A's ID; Delete A\n");
    if (count > 2)
      printf("5: Delete C\n");
    if (count > 3)
      printf("6: Delete D\n");
    printf("?");fflush(stdout);
    gets(buf);
    switch (buf[0]) {
    case '1':
##	replace u (#status = 3, #modtime=date("now"), #modwith="nodups",
##		   modby=8873) where u.#users_id = users_id2
##	inquire_equel(rowcount = "rowcount")
	if (rowcount != 1)
	  printf("Failed!\007\007\007\n");
	else
	  printf("User %s marked for deletion.\n", login2);
	break;
    case '2':
##	replace u (#status = 3, #modtime=date("now"), #modwith="nodups",
##		   modby=8873) where u.#users_id = users_id
##	inquire_equel(rowcount = "rowcount")
	if (rowcount != 1)
	  printf("Failed!\007\007\007\n");
	else
	  printf("User %s marked for deletion.\n", login);
	break;
    case '3':
##	replace u (#mit_id = id2, #modtime=date("now"), #modwith="nodups",
##		   modby=8873) where u.#users_id = users_id
##	inquire_equel(rowcount = "rowcount")
	if (rowcount != 1)
	  printf("Failed!\007\007\007\n");
##	replace u (#status = 3, #modtime=date("now"), #modwith="nodups",
##		   modby=8873) where u.#users_id = users_id2
##	inquire_equel(rowcount = "rowcount")
	if (rowcount != 1)
	  printf("Failed!\007\007\007\n");
	else
	  printf("User %s updated, %s marked for deletion.\n", login, login2);
	break;
    case '4':
##	replace u (#mit_id = id, #modtime=date("now"), #modwith="nodups",
##		   modby=8873) where u.#users_id = users_id2
##	inquire_equel(rowcount = "rowcount")
	if (rowcount != 1)
	  printf("Failed!\007\007\007\n");
##	replace u (#status = 3, #modtime=date("now"), #modwith="nodups",
##		   modby=8873) where u.#users_id = users_id
##	inquire_equel(rowcount = "rowcount")
	if (rowcount != 1)
	  printf("Failed!\007\007\007\n");
	else
	  printf("User %s updated, %s marked for deletion.\n", login2, login);
	break;
    case '5':
	users_id = users[2].users_id;
##	replace u (#status = 3, #modtime=date("now"), #modwith="nodups",
##		   modby=8873) where u.#users_id = users_id
##	inquire_equel(rowcount = "rowcount")
	if (rowcount != 1)
	  printf("Failed!\007\007\007\n");
	else
	  printf("User %d marked for deletion.\n", users[2].users_id);
	break;
    case '6':
	users_id = users[3].users_id;
##	replace u (#status = 3, #modtime=date("now"), #modwith="nodups",
##		   modby=8873) where u.#users_id = users_id
##	inquire_equel(rowcount = "rowcount")
	if (rowcount != 1)
	  printf("Failed!\007\007\007\n");
	else
	  printf("User %d marked for deletion.\n", users[3].users_id);
	break;
    case 'q':
	exit(0);
    default:
	printf("Leaving unchanged\n");
    }
##}
