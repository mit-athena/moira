typedef struct _client {
	OPERATION pending_op;
	CONNECTION con;
	int state;
	int action;
	sms_params args, reply;
} client;

/*
 * States
 */

#define CL_DEAD 0
#define CL_STARTING 1

/*
 * Actions.
 */

#define CL_ACCEPT 0
#define CL_RECEIVE 1
#define CL_SEND 2

