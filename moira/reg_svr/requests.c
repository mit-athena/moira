/*
 *      $Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/requests.c,v $
 *      $Author: danw $
 *      $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/requests.c,v 1.9 1997-09-05 19:15:00 danw Exp $
 *
 *      Copyright (C) 1987, 1988 by the Massachusetts Institute of Technology
 *	For copying and distribution information, please see the file
 *	<mit-copyright.h>.
 *
 *      Server for user registration with Moira and Kerberos.
 *
 *      This file handles the processing of requests for the register
 *      server.
 */

#ifndef lint
static char *rcsid_requests_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/reg_svr/requests.c,v 1.9 1997-09-05 19:15:00 danw Exp $";
#endif lint

/*
 * Before you add anything to the list of things that are #included and
 * #defined, make sure that it is not already done in reg_svr.h 
 */

#include <mit-copyright.h>
#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/file.h>
#include <krb.h>
#include <des.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "moira.h"
#include "moira_site.h"
#include "reg_svr.h"

#define NUM_REQUESTS_SAVED 100	/* Number of transactions to save */
#define CUR_REQ (requests[cur_request_index]) /* The current request */
#define NEXT_INDEX(x)  (x == NUM_REQUESTS_SAVED - 1) ? 0 : (x + 1)
#define PREV_INDEX(x)  (x == 0) ? (NUM_REQUESTS_SAVED - 1) : (x - 1)
    
static struct servent *sp;	/* Service info from /etc/services */
static int s;			/* Socket descriptor */
static struct sockaddr_in sin;	/* Internet style socket address */
static int addrlen;		/* Size of socket address (sin) */
  
/* In order to elegantly handle multiple retransmissions, an instance 
   of this structure will be retained for the last NUM_REQUESTS_SAVED
   transactions with the client. */
struct request_save {
    char out_pkt[BUFSIZ];	/* Buffer for outgoing packet */
    int out_pktlen;		/* Length of outgoing packet */
    U_32BIT seqno;		/* Sequence number for packet transmission */
    u_long ip_address;		/* Internet address of client host */
    u_short cl_port;		/* Port number client used */
};

static struct request_save requests[NUM_REQUESTS_SAVED]; /* Saved packets */
static int cur_request_index = 0;	/* Index to the current request */

void clear_req(req)
  struct request_save *req;
{
    req->seqno = 0;
    req->ip_address = 0;
    req->cl_port = 0;
}

void req_initialize()
{
    register int i;

    /* Get service information from /etc/services */
    if ((sp = getservbyname("sms_ureg", "udp")) == NULL) 
    {
	com_err(whoami, errno, " unknown service sms_ureg/udp");
	exit(1);
    }
    
    /* Get an internet style datagram socket */
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0) 
    {
	com_err(whoami,errno," socket");
	exit(1);
    }
    memset(&sin, 0, sizeof(sin));
    
    sin.sin_family = AF_INET;
    sin.sin_port = sp->s_port;
    sin.sin_addr.s_addr = INADDR_ANY;
    
    /* Bind a name to the socket */
    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) 
    {
	com_err(whoami,errno," bind");
	exit(1);
    }

    for (i = 0; i < NUM_REQUESTS_SAVED; i++)
	clear_req(&(requests[i]));
}    

int handle_retransmitted()
{
    register int i;		/* A counter */
    int status = FALSE;		/* Return status */
    
    for (i = PREV_INDEX(cur_request_index); i != cur_request_index;
	 i = PREV_INDEX(i))
    {
	if ((requests[i].seqno == CUR_REQ.seqno) &&
	    (requests[i].ip_address == sin.sin_addr.s_addr) &&
	    (requests[i].cl_port == sin.sin_port))
	    /* This is a retransmitted packet */
	{
#ifdef DEBUG
	    com_err(whoami, 0, "Retransmitted packet detected.");
#endif
	    status = TRUE;
	    (void) sendto(s, requests[i].out_pkt, requests[i].out_pktlen, 
			  0, (struct sockaddr *)&sin, addrlen);
	    break;
	}
    }

    return status;
}

void respond(status,text)
  int status;			/* Return status for the client */
  char *text;			/* Text for the client */
  /* This routine takes care of sending packets back to the client and 
     caching the necessary information for retransmission detection. 
     It is the only place in which cur_request_index should be
     changed. */
{
    CUR_REQ.out_pktlen = sizeof(CUR_REQ.out_pkt);

    if (format_pkt(CUR_REQ.out_pkt, &(CUR_REQ.out_pktlen), 
		   CUR_REQ.seqno, status, text))
	com_err(whoami,0,"Client error message was truncated.");
    (void) sendto(s, CUR_REQ.out_pkt, CUR_REQ.out_pktlen, 0,
		  (struct sockaddr *)&sin, addrlen);

    cur_request_index = NEXT_INDEX(cur_request_index);
}

void get_request(message)
  struct msg *message;		/* Will contain formatted packet information */
{
    static char packet[BUFSIZ];	/* Buffer for incoming packet */
    int pktlen;			/* Length of incoming packet */
    int status = FAILURE;	/* Error status */

    /* Sit around waiting for requests from the client. */
    for (;;)
    {
	com_err(whoami, 0, "*** Ready for next request ***");
	addrlen = sizeof(sin);
	/* Receive a packet */
	if ((pktlen = recvfrom(s,packet,sizeof(packet),0,
			       (struct sockaddr *)&sin,&addrlen)) < 0) 
	{
	    com_err(whoami, errno, " recvfrom");
	    /* Don't worry if error is interrupted system call. */
	    if (errno == EINTR) continue;
	    exit(1);
	}
	
	/* Store available information */
	CUR_REQ.seqno = 0;
	CUR_REQ.ip_address = sin.sin_addr.s_addr;
	CUR_REQ.cl_port = sin.sin_port;

	/* Parse a request packet and save sequence number */
	if ((status = parse_pkt(packet, pktlen, message)) 
	    != SUCCESS) 
	{
	    /* If error, format packet to send back to the client */
	    respond(status, (char *)NULL);
	}
	else
	{
	    /* Check for retransmitted packet.  handle_retransmitted() 
	       returns true if it handled a retransmitted packet. */
	    if (!handle_retransmitted())
		break;
	}
    }
}

void report(status, message)
  int status;
  char * message;
{
    respond(status, message);
}

int format_pkt(packet, pktlenp, seqno, cl_status, message)
  char *packet;			/* Packet buffer */
  int *pktlenp;			/* Pointer to packet size */
  U_32BIT seqno;		/* Sequence number */
  int cl_status;		/* Error status to return to client */
  char *message;		/* Error message to return to client */
  /* This routine prepares a packet to send back to the client.  A 
     non-zero return status means that the client error message was 
     truncated. */
{
    int len;			/* Amount of message to send */
    int status = SUCCESS;	/* Return status */

    /* Convert byte order to network byte order */
    U_32BIT vers = htonl((U_32BIT)CUR_UREG_VERSION);
    cl_status = htonl((U_32BIT)cl_status);
    /* Put current user registration protocol version into the packet */
    memcpy(packet, &vers, sizeof(U_32BIT));
    /* Put sequence number into the packet */
    memcpy(packet+sizeof(U_32BIT), &seqno, sizeof(U_32BIT));
    /* Put error status into the packet */
    memcpy(packet+ 2*sizeof(U_32BIT), &cl_status, sizeof(U_32BIT));
    
    /* Find out how much of the message to copy; truncate if too short. */
    /* How much room is there left? */
    len = *pktlenp - sizeof(U_32BIT)*3;
    if (message == NULL) message = "";
    if (len < strlen(message) + 1) /* Room for null terminator */
    {
	status = FAILURE;	/* Message was truncated */
	/* Truncate the message */
	message[len-1] = '\0';
    }

    /* Copy the message into the packet */
    (void) strcpy(packet+3*sizeof(U_32BIT), message);
    *pktlenp = 3*sizeof(U_32BIT) + strlen(message);
    
    return status;
}

/* The ureg_validate_char variable and routine were taken verbatim 
   out of server/qsupport.qc where they are called
   validate_chars.  At some point, it may be desirable
   to put this functionality in one place. */

/* ureg_validate_char: verify that there are no illegal characters in
 * the string.  Legal characters are printing chars other than 
 * ", *, ?, \, [ and ].
 */
static int illegalchars[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* ^@ - ^O */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* ^P - ^_ */
    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, /* SPACE - / */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, /* 0 - ? */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* @ - O */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, /* P - _ */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ` - o */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, /* p - ^? */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

ureg_validate_char(s)
register char *s;
{
    while (*s)
      if (illegalchars[*s++])
	return(FAILURE);
    return(SUCCESS);
}

parse_pkt(packet, pktlen, message)
  char *packet;
  int pktlen;
  struct msg *message;
  /* This routine checks a packet and puts the information in it in
     a structure if it is valid.  It also saves the sequence number
     in the list of saved requests. */
{
    int status = SUCCESS;	/* Error status */

    com_err(whoami,0,"Packet received");

    if (pktlen < sizeof(U_32BIT)) status = UREG_BROKEN_PACKET;
    if (status == SUCCESS)
    {
	/* Extract the user registration protocol version from the packet */
	memcpy(&message->version, packet, sizeof(long));
	/* Convert byte order from network to host */
	message->version = ntohl(message->version);
	/* Verify version */
	if (message->version != CUR_UREG_VERSION) 
	    status = UREG_WRONG_VERSION;
    }

    if (status == SUCCESS)
    {
	packet += sizeof(U_32BIT);
	pktlen -= sizeof(U_32BIT);
	
	if (pktlen < sizeof(U_32BIT))
	    status = UREG_BROKEN_PACKET;
    }

    if (status == SUCCESS)
    {
	/* Extract the sequence number from the packet */
	memcpy(&CUR_REQ.seqno, packet, sizeof(long));
	
	packet += sizeof(U_32BIT);
	pktlen -= sizeof(U_32BIT);
	
	if (pktlen < sizeof(U_32BIT))
	    status = UREG_BROKEN_PACKET;
    }

    if (status == SUCCESS)
    {
	/* Extract the request from the packet */
	memcpy(&message->request, packet, sizeof(U_32BIT));
	message->request = ntohl(message->request);
	packet += sizeof(U_32BIT);
	pktlen -= sizeof(U_32BIT);
	
	/* Make sure that the packet contains only valid characters up 
	   to the next null */
	if (ureg_validate_char(packet) != SUCCESS)
	{
	    com_err(whoami,0,"Packet contains invalid characters.");
	    status = UREG_USER_NOT_FOUND;
	}
	else
	{
	    /* Extract first name from the packet */
	    message->first = packet;
	    
	    /* Scan forward until null appears in the packet or there
	       is no more packet! */
	    for (; *packet && pktlen > 0; --pktlen, ++packet) continue;
	    if (pktlen <= 0) 
		status = UREG_BROKEN_PACKET;
	}
    }
    
    if (status == SUCCESS)
    {
	/* Skip over the null */
	packet++, pktlen--;
	
	if (ureg_validate_char(packet) != SUCCESS)
	{
	    com_err(whoami,0,"Packet contains invalid characters.");
	    status = UREG_USER_NOT_FOUND;
	}
	else
	{
	    /* Extract last name from the packet */
	    message->last = packet;
	    
	    for (; *packet && pktlen > 0; --pktlen, ++packet) continue;
	    if (pktlen <= 0)
		status = UREG_BROKEN_PACKET;
	}
    }

    if (status == SUCCESS)
    {
	packet++, pktlen--;
	
	if (pktlen <= 0)
	    status = UREG_BROKEN_PACKET;
    }

    /* Extract encrypted information from packet */
    message->encrypted = packet;
    message->encrypted_len = pktlen;
    
    if (status == SUCCESS)
    {
#ifdef DEBUG
	com_err(whoami,status,"%s\n%s%d\n%s%d\n%s%s\n%s%s",
		"Packet parsed successfully.  Packet contains:",
		"   Protocol version: ",message->version,
		"   Request: ",message->request,
		"   First name: ",message->first,
		"   Last name: ",message->last);
#else /* DEBUG */
	com_err(whoami,status,"Request %d for %s %s",message->request,
		message->first,message->last);
#endif DEBUG    
    }
    else
	com_err(whoami,status," - parse packet failed.");

    return status;
}


u_long cur_req_sender()
{
    return (CUR_REQ.ip_address);
}
