/*
 * Update mechanism description structure.
 */
struct update_desc {
     int last_time;		/* when did we try last? */
     int success;		/* did it succeed? */
     int interval;		/* interval in minutes between updates */
     char *service_name;
     char *host_name;
     char *target_path;		/* where to put the file */
     int override;		/* override interval */
     int enable;		/* can we update at all? */
     char *instructions;	/* script pathname */
};

char *whoami;
extern char sms_data_dir[];

#define log_DEBUG 0
#define log_INFO  1
#define log_WARNING 2
#define log_ERROR 3
int log_priority;
