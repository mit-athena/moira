/* $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/regtape/stuconv.qc,v 1.1 1993-01-04 15:57:42 mar Exp $
 */

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <sys/time.h>
#include <moira.h>
#include <moira_site.h>


##define WHO 11859		/* root */
##define PROG "stu-tape"

#define MAX_ID_VALUE	32766
#define MIN_ID_VALUE	101

/* File format is:

0-29	name
30-38	id number
50-54	school code
55-79   year
80-109  address
110-124 room
125-144 city
145-158 state
159-168 dorm phone
169-212 home address
213-232 home city
243-251 mit phone (?)
*/

#define LOC_NAME 0
#define LOC_ID 30
#define LOC_COURSE 50
#define LOC_YEAR 55
#define LOC_ADDRESS 80
#define LOC_DORM_ROOM 110
#define LOC_CITY 125
#define LOC_STATE 145
#define LOC_DPHONE 155
#define LOC_MPHONE 243

#define LEN_NAME 30
#define LEN_ID 9
#define LEN_COURSE 5
#define LEN_YEAR 25
#define LEN_ADDRESS 30
#define LEN_DORM_ROOM 15
#define LEN_CITY 20
#define LEN_STATE 10
#define LEN_DPHONE 12
#define LEN_MPHONE 10

struct entry {
    char *name;
    char *last;
    char *first;
    char *middle;
    char *title;
    char *id;
    char *eid;
    char *course;
    char *year;
    char *address;
    char *dorm;
    char *city;
    char *state;
    char *dphone;
    char *mphone;
    char *class;
};


char *whoami;
int newfinger = 0;
int addxuser = 0;


main(argc, argv)
int argc;
char **argv;
##{
    FILE *in;
    struct entry *e, *get_next_entry();
    int i, wait = 0;
    char buf[BUFSIZ], *file = NULL;

    whoami = rindex(argv[0], '/');
    if (whoami)
      whoami++;
    else
      whoami = argv[0];

    for (i = 1; i < argc; i++) {
	if (!strcmp(argv[i], "-w"))
	  wait++;
	else if (!strcmp(argv[i], "-D"))
	  setenv("ING_SET", "set printqry");
	else if (!strcmp(argv[i], "-n"))
	  newfinger++;
	else if (!strcmp(argv[i], "-u"))
	  addxuser++;
	else if (file != NULL)
	  fprintf(stderr, "Usage: %s [-w] [-D] [-n] [-u] inputfile\n", whoami);
	else
	  file = argv[i];
    }

    in = fopen(file, "r");
    if (in == NULL) {
	fprintf(stderr, "Unable to open %s for input\n", file);
	exit(1);
    }

##  ingres sms
##  range of u is users

    while (e = get_next_entry(in)) {
	process_entry(e);
	if (wait) {
	    printf("Next");
	    fflush(stdout);
	    gets(buf);
	}
    }

##  exit
    exit(0);
##}


struct entry *get_next_entry(in)
FILE *in;
{
    static struct entry e;
    static char buf[BUFSIZ], eid[16], classbuf[10], titlebuf[12];
    static char name[LEN_NAME+1], id[LEN_ID+1], course[LEN_COURSE+1];
    static char year[LEN_YEAR+1], address[LEN_ADDRESS+1];
    static char dorm_room[LEN_DORM_ROOM+1], city[LEN_CITY+1];
    static char state[LEN_STATE+1], dphone[LEN_DPHONE+1], mphone[LEN_MPHONE+1];
    static char sname[LEN_NAME+1], title[128];
    static int nyear = 0;
    int ends_jr, ends_iii, ends_iv, ends_sr;
    char *p;

    if (nyear == 0) {
	struct tm *tm;
	struct timeval tv;

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);
	nyear = tm->tm_year;
	if (tm->tm_mon > 5)
	  nyear++;
    }

    if (fgets(buf, sizeof(buf), in) == NULL)
      return((struct entry *)NULL);

    strncpy(name, &buf[LOC_NAME], LEN_NAME); name[LEN_NAME] = 0;
    strncpy(id, &buf[LOC_ID], LEN_ID); id[LEN_ID] = 0;
    strncpy(course, &buf[LOC_COURSE], LEN_COURSE); course[LEN_COURSE] = 0;
    strncpy(year, &buf[LOC_YEAR], LEN_YEAR); year[LEN_YEAR] = 0;
    strncpy(address, &buf[LOC_ADDRESS], LEN_ADDRESS); address[LEN_ADDRESS] = 0;
    strncpy(dorm_room, &buf[LOC_DORM_ROOM], LEN_DORM_ROOM); dorm_room[LEN_DORM_ROOM] = 0;
    strncpy(city, &buf[LOC_CITY], LEN_CITY); city[LEN_CITY] = 0;
    strncpy(state, &buf[LOC_STATE], LEN_STATE); state[LEN_STATE] = 0;
    strncpy(dphone, &buf[LOC_DPHONE], LEN_DPHONE); dphone[LEN_DPHONE] = 0;
    strncpy(mphone, &buf[LOC_MPHONE], LEN_MPHONE); mphone[LEN_MPHONE] = 0;

    strcpy(sname, name);
    e.name = strtrim(sname);
    p = index(name, ',');
    if (p)
      *p = 0;
    e.last = strtrim(name);
    if (p) {
	p++;
	while (isspace(*p))
	  p++;
	e.first = p;	
	if (p = index(e.first, ' ')) {
	    *p = 0;
	    e.first = strtrim(e.first);
	    e.middle = strtrim(p + 1);
	} else {
	    e.first = strtrim(e.first);
	    e.middle = "";
	}
    } else {
	e.first = "";
	e.middle = "";
    }
    ends_jr = ends_iii = ends_iv = ends_sr = 0;
    LookForSt(e.last);
    LookForO(e.last);
    LookForJrAndIII(e.last, &ends_sr, &ends_jr, &ends_iii, &ends_iv);
    LookForJrAndIII(e.first, &ends_sr, &ends_jr, &ends_iii, &ends_iv);
    FixCase(e.last);
    FixCase(e.first);
    FixCase(e.middle);

    e.id = id;
    e.id[LEN_ID] = 0;
    e.eid = eid;
    EncryptID(e.eid, e.id, e.first, e.last);

    e.year = strtrim(year);
    e.title = title;
    if (e.year[0] == 'G') {
	e.class = "G";
	sprintf(title, "Grad Student");
    } else {
	e.class = classbuf;
	sprintf(classbuf, "%d", nyear + 4 - atoi(e.year) + 1900);
	sprintf(title, "Undergrad (class of %s)", classbuf);
    }

    e.course = strtrim(course);
    e.address = strtrim(address);
    e.dorm = strtrim(dorm_room);
    e.city = strtrim(city);
    e.state = strtrim(state);
    e.dphone = strtrim(dphone);
    e.mphone = strtrim(mphone);
    return(&e);
}


process_entry(e)
struct entry *e;
##{
    int changed, nochange;
    char buf[BUFSIZ], *from, *to;
##  char *first, *last, *eid, *title, *sid, *name;
##  char class[9], haddr[128], hphone[17], ophone[13], dept[24];
##  int id, status;

    first = e->first;
    if (strlen(first) > 16)
      first[16] = 0;
    last = e->last;
    if (strlen(last) > 16)
      last[16] = 0;
    eid = e->eid;
    id = 0;
##  repeat retrieve (id = u.users_id)
##	where u.#last = @last and u.#first = @first and u.mit_id = @eid
    if (id == 0) {
	com_err(whoami, 0, "New user found: %s %s\n", first, last);
	return;
    }
    eid = e->id;
##  repeat replace u (mit_id=@eid) where u.users_id = @id

    sid = e->id;
    name = e->name;
    strcpy(dept, e->course);
    title = e->title;
    strcpy(haddr, e->address);
    if (*e->dorm) {
	strcat(haddr, " ");
	strcat(haddr, e->dorm);
    }
    if (*e->city) {
	strcat(haddr, " ");
	strcat(haddr, e->city);
    }
    FixCase(haddr);
    if (*e->state) {
	strcat(haddr, " ");
	strcat(haddr, e->state);
    }
    strcpy(hphone, e->dphone);
    strcpy(ophone, e->mphone);
##  repeat replace u (xname = @name, xdept = @dept, xtitle = @title,
##		      xaddress = @haddr, xphone1 = @hphone, xphone2 = @ophone,
##		      xmodtime = "now")
##	where u.users_id = @id
##}


set_next_object_id(object, limit)
    char *object;
    int limit;
##{
##  char *name;
##  int rowcount, exists, value;

    name = object;
##  begin transaction
##  repeat retrieve (value = values.#value) where values.#name = @name
##  inquire_equel(rowcount = "rowcount")
    if (rowcount != 1) {
##	abort
	return(0);
    }

##  retrieve (exists = any(users.name where users.name = value))
##  inquire_equel(rowcount = "rowcount")
    if (rowcount != 1) {
##	abort
	return(0);
    }
    while (exists) {
	value++;
	if (limit && value > MAX_ID_VALUE)
	    value = MIN_ID_VALUE;
##	retrieve (exists = any(users.name where users.name = value))
    }

##  repeat replace values (#value = @value) where values.#name = @name
##  end transaction
    return(value);
##}


