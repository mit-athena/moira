#include <moira_schema.h>

/* for MAXPATHLEN */
#include <sys/param.h>

#include <stdio.h>

void fix_file(char *targetfile);
char *dequote(char *s);
void db_error(int code);

int ModDiff(int *flag, char *tbl, time_t ModTime);
time_t unixtime(char *timestring);
#define UNIXTIME_FMT "J HH24 MI SS"

struct tarheader {
  char name[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char chksum[8];
  char typeflag[1];
  char linkname[100];
  char magic[6];
  char version[2];
  char uname[32];
  char gname[32];
  char devmajor[8];
  char devminor[8];
  char prefix[155];
  char pad[12];
};

typedef struct _tarfile {
  FILE *fp;
  struct tarheader th;
  long offset;
} TARFILE;

TARFILE *tarfile_open(char *file);
void tarfile_close(TARFILE *tf);
FILE *tarfile_start(TARFILE *tf, char *name, mode_t mode, uid_t uid, gid_t gid,
		    char *user, char *group, time_t mtime);
void tarfile_end(TARFILE *tf);
void tarfile_mkdir(TARFILE *tf, char *name, mode_t mode, uid_t uid, gid_t gid,
		   char *user, char *group, time_t mtime);

struct imember {
  char type;
  char *name, *tag;
};
void freeimember(struct imember *m);

void init_acls(void);
void canon_krb(struct imember *m, int vers, char *buf, int len);
struct save_queue *get_acl(char *type, int id,
			   char *(merge_func)(char *, char *));
void dump_krb_acl(FILE *out, char *type, int id, int vers);
void dump_user_list(FILE *out, char *type, int id);
char *user_lookup(int users_id);
char *string_lookup(int string_id);
