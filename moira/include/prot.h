/* 
 * 	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/include/prot.h,v $
 * 	$Author: danw $
 * 	$Locker:  $
 *
 * Copyright 1985,1986 by the Massachusetts Institute of Technology
 *
 */


#define		KRB_PORT		750 /* PC's don't have /etc/services */
#define		KRB_PROT_VERSION 	4
#define 	MAX_PKT_LEN		1000
#define		MAX_TXT_LEN		1000
#define		TICKET_GRANTING_TICKET	"krbtgt"

/* Macro's to obtain various fields from a packet */

#define pkt_version(packet)   (unsigned int) *(packet->dat)
#define pkt_msg_type(packet)  (unsigned int) *(packet->dat+1)
#define pkt_a_name(packet)    (packet->dat+2)
#define pkt_a_inst(packet)    (packet->dat+3+strlen(pkt_a_name(packet)))
#define pkt_a_realm(packet)   (pkt_a_inst(packet)+1+strlen(pkt_a_inst(packet)))


/* Macro to obtain realm from application request */
#define apreq_realm(auth)     (auth->dat + 3)


/* If there are long alignment problems, these */
/* will have to be rewritten			 */

#define pkt_time_ws(packet) (unsigned long *) \
        (packet->dat+5+strlen(pkt_a_name(packet))+strlen(pkt_a_inst(packet)) \
		+strlen(pkt_a_realm(packet)))
#define pkt_no_req(packet) (unsigned short) \
        *(packet->dat+9+strlen(pkt_a_name(packet))+ \
	strlen(pkt_a_inst(packet))+strlen(pkt_a_realm(packet)))

#define pkt_x_date(packet) (unsigned long *) \
        (packet->dat+10+strlen(pkt_a_name(packet))+strlen(pkt_a_inst(packet)) \
		+strlen(pkt_a_realm(packet)))

#define pkt_err_code(packet) ( (unsigned long *) \
        (packet->dat+9+strlen(pkt_a_name(packet))+strlen(pkt_a_inst(packet)) \
		+strlen(pkt_a_realm(packet))))
#define pkt_err_text(packet) \
        (packet->dat+13+strlen(pkt_a_name(packet))+ \
		strlen(pkt_a_inst(packet))+strlen(pkt_a_realm(packet)))

/* Routines to create and read packets may be found in prot.c */

struct ktext	*create_packet_to_kdc();
struct ktext	*create_auth_reply();
struct ktext	*create_death_packet();
struct ktext	*pkt_cipher();

/* Message types , always leave lsb for byte order */

#define		AUTH_MSG_KDC_REQUEST		1<<1
#define 	AUTH_MSG_KDC_REPLY			2<<1
#define		AUTH_MSG_APPL_REQUEST		3<<1
#define		AUTH_MSG_APPL_REQUEST_MUTUAL 4<<1
#define		AUTH_MSG_ERR_REPLY			5<<1
#define		AUTH_MSG_PRIVATE			6<<1
#define		AUTH_MSG_SAFE				7<<1
#define		AUTH_MSG_APPL_ERR			8<<1
#define 	AUTH_MSG_DIE				63<<1

/* values for kerb error codes 		*/
#define			KERB_ERR_OK			0
#define			KERB_ERR_NAME_EXP		1
#define			KERB_ERR_SERVICE_EXP		2
#define			KERB_ERR_AUTH_EXP		3
#define			KERB_ERR_PKT_VER		4
#define			KERB_ERR_NAME_MAST_KEY_VER	5
#define			KERB_ERR_SERV_MAST_KEY_VER	6
#define			KERB_ERR_BYTE_ORDER		7
#define			KERB_ERR_PRINCIPAL_UNKNOWN	8
#define			KERB_ERR_PRINCIPAL_NOT_UNIQUE   9
#define			KERB_ERR_NULL_KEY	       10

/* Byte ordering														   */
static int	ONE = 1;
#define		HOST_BYTE_ORDER	(* (char *) &ONE)
#define		MSB_FIRST		0	/*  68000								   */
#define		LSB_FIRST		1	/*  Vax, PC8086							   */
