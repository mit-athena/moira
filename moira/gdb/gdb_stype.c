/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_stype.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_stype.c,v 1.4 1993-10-22 14:36:54 mar Exp $
 */

#ifndef lint
static char *rcsid_gdb_stype_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_stype.c,v 1.4 1993-10-22 14:36:54 mar Exp $";
#endif	lint


/************************************************************************
 *	
 *			   gdb_stype.c
 *	
 *	      GDB - System Data Type Definitions
 *	
 *	Author: Noah Mendelsohn
 *	Copyright: 1986 MIT Project Athena 
 *		For copying and distribution information, please see
 *	  	the file <mit-copyright.h>.
 *	
 *	This file initializes the definitions for all system defined
 *	data types, and it includes the type specific semantic routines
 *	for each of the system defined types.
 *	
 *	The initialization routine which adds these type definitions 
 *	to the type definition table is at the end of this source file.
 *	
 ************************************************************************
 *	
 *	This file is organized into one section for each system
 *	defined type followed at the end by a final section which
 *	initializes the type tables.  Each of the type specific
 *	sections does #defines for each type specific parameter.  The
 *	gdb_i_stype initialization routine at the end of this source
 *	file uses these defines to initialize the appropriate entry in
 *	the type definition tables.
 *	
 *	NOTE: some of the type definitions in this file may be machine
 *	dependent.
 *	
 ************************************************************************/

#include <mit-copyright.h>
#include <stdio.h>
#include <string.h>
#include "gdb.h"
#ifdef vax			/* XXX */
	extern u_long ntohl(), htonl();
#endif vax
#include <netinet/in.h>				/* for htonl routine */


/************************************************************************
 *	
 *			     INTEGER_T
 *	
 ************************************************************************/

#define IN_LEN 		(sizeof(int))
#define IN_ALI 		IN_LEN
#define IN_NULL 	g_in_null
#define IN_CDLEN 	g_in_cdlen
#define IN_ENC 		g_in_enc
#define IN_DEC		g_in_dec
#define IN_FORM		g_in_form
#define IN_NAME		"INTEGER_T"

#define IN_EXTERNSIZE 4				/* length of an encoded */
						/* integer */
	/*----------------------------------------------------------*/
	/*	
	/*			g_in_null
	/*	
	/*	Fill in a null value for an integer.
	/*	
	/*----------------------------------------------------------*/

int
g_in_null(dp)
char *dp;					/* pointer to the data */
{
	*((int *)dp) = 0;			/* fill in a null value */
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_in_cdlen
	/*	
	/*	Return coded length for an integer.  We're currently
	/*	using the Berkeley 'htonl' routine which converts 
	/*	an integer (actually a long, ahem!) to a canonical
	/*	4 byte form.>
	/*	
	/*----------------------------------------------------------*/


/*ARGSUSED*/
int
g_in_cdlen(dp,hcon)
char *dp;					/* pointer to the data */
HALF_CONNECTION hcon;
{
	return IN_EXTERNSIZE;
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_in_enc
	/*	
	/*	Encode an integer for transmission
	/*	
	/*----------------------------------------------------------*/

/*ARGSUSED*/
int
g_in_enc(dp, hcon, outp)
char *dp;					/* pointer to data */
HALF_CONNECTION hcon;				/* connection descriptor */
char *outp;					/* place to put the output */
{
	register char *cp;			/* next char in output */
	register char *op = outp;
	register char *endp = outp+IN_EXTERNSIZE;

	unsigned long converted;		/* the integer goes here */
						/* in network byte order*/

       /*
        * Put it in network format, then copy one byte at a time to
        * account for the fact that the RT has trouble with unaligned longs
        */

	converted = htonl(*(u_long *)dp);

	cp = (char *)&converted;
	*op++ = *cp++;
	*op++ = *cp++;
	*op++ = *cp++;
	*op++ = *cp++;

	return (int)(endp);			/* return pointer to next */
						/* unused output byte*/
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_in_dec
	/*	
	/*	Decode an integer from external form to local
	/*	representation.
	/*	
	/*	
	/*----------------------------------------------------------*/

/*ARGSUSED*/
int
g_in_dec(outp, hcon, inp)
char *inp;					/* pointer to data */
HALF_CONNECTION hcon;				/* connection descriptor */
char *outp;					/* place to put the output */
{
	register char *ip = inp;		/* next byte of input */
	int buffer;
	register char *bp;			/* next byte in buffer */

       /*
        * Copy a byte at a time to buffer to account for RT difficulties
        * with unaligned ints.
        */
	bp = (char *)&buffer;
	*bp++ = *ip++;
	*bp++ = *ip++;
	*bp++ = *ip++;
	*bp++ = *ip++;

       /*
        * Convert it and return pointer to next byte of input.
        */

	*(int *)outp = ntohl((u_long)buffer);
	return (int)(ip);
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_in_form
	/*	
	/*	Format an integer on output logging file for
	/*	debugging.
	/*	
	/*----------------------------------------------------------*/

int
g_in_form(name, dp)
char *name;					/* string name of the field */
char *dp;					/* pointer to the data */
{
	fprintf(gdb_log, "INTEGER_T\t%s=%d\n",name,(*(int *)dp));
}


/************************************************************************/
/*	
/*			     STRING_T
/*	
/************************************************************************/

#define ST_LEN 		(sizeof(STRING))
#define ST_ALI 		(sizeof(int))
#define ST_NULL 	g_st_null
#define ST_CDLEN 	g_st_cdlen
#define ST_ENC 		g_st_enc
#define ST_DEC		g_st_dec
#define ST_FORM		g_st_form
#define ST_NAME		"STRING_T"

	/*----------------------------------------------------------*/
	/*	
	/*			g_st_null
	/*	
	/*	Fill in a null value for a string.
	/*	
	/*----------------------------------------------------------*/
int
g_st_null(dp)
char *dp;					/* pointer to the data */
{
	register STRING *stp = (STRING *)dp;	/* re-type as string */
	STRING_DATA(*stp) = NULL;		/* no data */
	MAX_STRING_SIZE(*stp) = 0;		/* for cleanliness */
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_st_cdlen
	/*	
	/*	Return coded length for a string.  We have to send the
	/*	actual length of the data along with the data itself.
	/*	For this reason, we leave space for a coded integer
	/*	in addition to the data bytes.  We actually call the
	/*	integer coding routines to code the length.
	/*	
	/*	Note that a separate type understanding null termination
	/*	might be an interesting optimization someday.
	/*	
	/*----------------------------------------------------------*/

int
g_st_cdlen(dp,hcon)
char *dp;					/* pointer to the data */
HALF_CONNECTION hcon;
{
	register STRING *stp = (STRING *)dp;	/* re-type as string */

	return (MAX_STRING_SIZE(*stp) + 
		g_in_cdlen((char *)&MAX_STRING_SIZE(*stp),hcon));
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_st_enc
	/*	
	/*	Encode a string for transmission
	/*	
	/*----------------------------------------------------------*/

int
g_st_enc(dp, hcon, outp)
char *dp;					/* pointer to data */
HALF_CONNECTION hcon;				/* connection descriptor */
char *outp;					/* place to put the output */
{
	register STRING *stp = (STRING *)dp;	/* re-type as string */
	int len;
	register char *nextp;			/* place to put next output */
						/* byte */
       /*
        * Use the integer coding routine to get the length encoded first
        */

	len = MAX_STRING_SIZE(*stp);		/* length of both source */
						/* and coded form*/
	nextp = (char *)g_in_enc((char *)&len, hcon, outp);
	
       /*
        * Now, copy the data itself after the encoded integer length
        */
	if (len > 0)
		memcpy(nextp, STRING_DATA(*stp), len);
						/* copy the data without */
						/* changing representation*/
	return (int)(nextp+len);
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_st_dec
	/*	
	/*	Decode a string from external form.  We always
	/*	allocate new space for the string, intentionally
	/*	ignoring any which may have been in use before.  If we
	/*	freed it, we would not be robust against calls on
	/*	uninitialized fields.  This may have nasty side
	/*	effects if the intention was to leave 'gas' at the end
	/*	of the string, but we want to accurately copy the
	/*	data.  Note that string_free is robust against null
	/*	pointers.
	/*	
	/*----------------------------------------------------------*/

int
g_st_dec(outp, hcon, inp)
char *inp;					/* pointer to input data */
HALF_CONNECTION hcon;				/* connection descriptor */
char *outp;					/* place to put the output */
{
	register STRING *stp = (STRING *)outp;	/* re-type as string */
	int len;
	register char *nextp;			/* next byte to scan */
       /*
        * Use the integer coding routine to get the length encoded first
        */

	nextp = (char *)g_in_dec((char *)&len, hcon, inp);


       /*
        * Allocate memory for the string.  If length is 0, then null it
        * out.  Note that we had considered freeing any existing strings
        * which might be there, but this turns out to cause lots of
        * trouble for the many callers who don't want to initialize before
        * a decode.
        */
	if (len == 0) {
		STRING_DATA(*stp) = NULL;
		MAX_STRING_SIZE(*stp) = 0;
		return (int)(nextp);
	}
	(void) string_alloc(stp, len);		/* this sets string length */
						/* in addition to doing the */
						/* allocation */
	
       /*
        * Now, copy the data itself 
        */
	memcpy(STRING_DATA(*stp), nextp, len);	/* copy the data without */
						/* changing representation*/
	return (int)(nextp+len);
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_st_form
	/*	
	/*	Format a string on output logging file for
	/*	debugging.
	/*	
	/*----------------------------------------------------------*/

int
g_st_form(name, dp)
char *name;					/* string name of the field */
char *dp;					/* pointer to the data */
{
	register STRING *stp = (STRING *)dp;	/* re-type as string */
	int len;
	register char *cp;			/* next char to print */
	register char *past_end;		/* 1st one not to print */

	len = MAX_STRING_SIZE(*stp);
	fprintf(gdb_log, "STRING_T\t%s[%d]=\"", name,len);
	
	if (len == 0 ) {
		fprintf(gdb_log, "\"\n");
		return;
	}
	   

	cp = STRING_DATA(*stp);
	past_end = cp + len;

	while (cp < past_end)
		(void) putc(*cp++, gdb_log);

	fprintf(gdb_log,"\"\n");
}


/************************************************************************/
/*	
/*			     REAL_T
/*	
/************************************************************************/

#define RL_LEN 		(sizeof(double))
#define RL_ALI 		RL_LEN
#define RL_NULL 	g_rl_null
#define RL_CDLEN 	g_rl_cdlen
#define RL_ENC 		g_rl_enc
#define RL_DEC		g_rl_dec
#define RL_FORM		g_rl_form
#define RL_NAME		"REAL_T"

#define RL_EXTERNSIZE 32			/* length of ascii coding */
						/* must change lengths in */
						/* encode and decode */
						/* routines to match*/
	/*----------------------------------------------------------*/
	/*	
	/*			g_rl_null
	/*	
	/*	Fill in a null value for an real.
	/*	
	/*----------------------------------------------------------*/
int
g_rl_null(dp)
char *dp;					/* pointer to the data */
{
	*((double *)dp) = 0.0;			/* fill in a null value */
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_rl_cdlen
	/*	
	/*	Return coded length for an real.  For now, we just
	/*	code as a 12 digit ASCII converted string.  Obviously,
	/*	we can do much better in the future.
	/*	
	/*----------------------------------------------------------*/


/*ARGSUSED*/
int
g_rl_cdlen(dp,hcon)
char *dp;					/* pointer to the data */
HALF_CONNECTION hcon;
{
	return RL_EXTERNSIZE;
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_rl_enc
	/*	
	/*	Encode an real for transmission
	/*	
	/*----------------------------------------------------------*/

/*ARGSUSED*/
int
g_rl_enc(dp, hcon, outp)
char *dp;					/* pointer to data */
HALF_CONNECTION hcon;				/* connection descriptor */
char *outp;					/* place to put the output */
{
	register char *cp;			/* next char in output */
	register char *endp = outp+RL_EXTERNSIZE;

       /*
        * Convert the data into printable ASCII in the output stream
        * Note that the width in the format below must be less than
        * RL_EXTERNSIZE, because sprintf needs space for its terminating
        * null.
        */

	(void) sprintf(outp,"%30le",*((double *)dp));

       /*
        * Sprintf produces output of unpredictable length, and with 
        * a null termination.  Pad it out to the desired length.
        */

	cp = outp + strlen(outp);		/* find out where convertd */
						/* string stops*/
	while (cp < endp)
		*cp++ = ' ';			/* pad to desired length */

	return (int)(outp+RL_EXTERNSIZE);	/* return pointer to next */
						/* unused output byte*/
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_rl_dec
	/*	
	/*	Decode an real from external form
	/*	
	/*----------------------------------------------------------*/

/*ARGSUSED*/
int
g_rl_dec(outp, hcon, inp)
char *inp;					/* pointer to data */
HALF_CONNECTION hcon;				/* connection descriptor */
char *outp;					/* place to put the output */
{
	(void) sscanf(inp,"%30le", (double *)outp);
	return (int)(inp+RL_EXTERNSIZE);
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_rl_form
	/*	
	/*	Format an real on output logging file for
	/*	debugging.
	/*	
	/*----------------------------------------------------------*/

int
g_rl_form(name, dp)
char *name;					/* string name of the field */
char *dp;					/* pointer to the data */
{
	fprintf(gdb_log, "REAL_T\t\t%s=%le\n",name,*((double *)dp) );
}


/************************************************************************/
/*	
/*			     DATE_T
/*	
/************************************************************************/

#define DT_LEN 		25			/* see ingres definition */
#define DT_ALI 		1			/* char data, need not align */
#define DT_NULL 	g_dt_null
#define DT_CDLEN 	g_dt_cdlen
#define DT_ENC 		g_dt_enc
#define DT_DEC		g_dt_dec
#define DT_FORM		g_dt_form
#define DT_NAME		"DATE_T"

#define DT_EXTERNSIZE   DT_LEN			/* length of ascii coding */
						/* must change lengths in */
						/* encode and decode */
						/* routines to match*/
	/*----------------------------------------------------------*/
	/*	
	/*			g_dt_null
	/*	
	/*	Fill in a null value for a date.
	/*	
	/*----------------------------------------------------------*/
int
g_dt_null(dp)
char *dp;					/* pointer to the data */
{
	register char *cp = dp;			/* next character to fill in */
	register char *endp = dp + DT_LEN;

       /*
        * Fill the field with character blanks
        */
	while (cp < endp)
		*cp++ = ' ';
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_dt_cdlen
	/*	
	/*	Return coded length for an date.  For now, we just
	/*	code as a 25 digit ASCII converted string.
	/*	
	/*----------------------------------------------------------*/


/*ARGSUSED*/
int
g_dt_cdlen(dp,hcon)
char *dp;					/* pointer to the data */
HALF_CONNECTION hcon;
{
	return DT_EXTERNSIZE;
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_dt_enc
	/*	
	/*	Encode a date for transmission
	/*	
	/*----------------------------------------------------------*/

/*ARGSUSED*/
int
g_dt_enc(dp, hcon, outp)
char *dp;					/* pointer to data */
HALF_CONNECTION hcon;				/* connection descriptor */
char *outp;					/* place to put the output */
{
	register char *ip = dp;			/* next char in input */
	register char *op = outp;		/* next char in output */
	register char *endp = op+DT_EXTERNSIZE;

       /*
        * Copy the input untransformed to the output 
        */

	while (op < endp)
		*op++ = *ip++;			/* pad to desired length */

	return (int)(endp);			/* return pointer to next */
						/* unused output byte*/
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_dt_dec
	/*	
	/*	Decode an date from external form
	/*	
	/*----------------------------------------------------------*/

/*ARGSUSED*/
int
g_dt_dec(outp, hcon, inp)
char *inp;					/* pointer to data */
HALF_CONNECTION hcon;				/* connection descriptor */
char *outp;					/* place to put the output */
{
	register char *ip = inp;		/* next char in input */
	register char *op = outp;		/* next char in output */
	register char *endp = op+DT_EXTERNSIZE;

       /*
        * Copy the input untransformed to the output 
        */

	while (op < endp)
		*op++ = *ip++;			/* pad to desired length */

	return (int)(endp);			/* return pointer to next */
						/* unused output byte*/
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_dt_form
	/*	
	/*	Format a date on output logging file for
	/*	debugging.
	/*	
	/*----------------------------------------------------------*/

int
g_dt_form(name, dp)
char *name;					/* string name of the field */
char *dp;					/* pointer to the data */
{
	char buf[DT_EXTERNSIZE+1];

	memcpy(buf, dp, DT_EXTERNSIZE);		/* copy date to buffer */
	buf[DT_EXTERNSIZE] = '\0';		/* null terminate it */
	fprintf(gdb_log, "DATE_T\t\t%s=%s\n",name,buf);
}


/************************************************************************/
/*	
/*			     TUPLE_DESCRIPTOR_T
/*	
/*	The external representation of a tuple descriptor will be to
/*	send the count of the number of fields, and then a one byte
/*	signed integer describing each type followed by all the
/*	corresponding null terminated strings.  The tuple descriptor
/*	will really get re-created wth proper offsets and lengths upon
/*	receipt by the create_tuple_descriptor operation.
/*	
/************************************************************************/

#define TPD_LEN 	(sizeof(TUPLE_DESCRIPTOR))
#define TPD_ALI 	(sizeof(TUPLE_DESCRIPTOR))
#define TPD_NULL 	g_tpd_null
#define TPD_CDLEN 	g_tpd_cdlen
#define TPD_ENC 	g_tpd_enc
#define TPD_DEC		g_tpd_dec
#define TPD_FORM	g_tpd_form
#define TPD_NAME	"TUPLE_DESCRIPTOR_T"

 	/*----------------------------------------------------------*/
	/*	
	/*			g_tpd_null
	/*	
	/*	Fill in a null value for a tuple_descriptor.
	/*	
	/*----------------------------------------------------------*/
int
g_tpd_null(dp)
char *dp;					/* pointer to the data */
{
	register TUPLE_DESCRIPTOR *tdp = (TUPLE_DESCRIPTOR *)dp; 
						/* re-type as */
						/* tuple_descriptor */
	(*tdp) = NULL;				/* no data */
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_tpd_cdlen
	/*	
	/*	Return coded length for a tuple_descriptor.  
	/*	
	/*----------------------------------------------------------*/

int
g_tpd_cdlen(dp,hcon)
char *dp;					/* pointer to the data */
HALF_CONNECTION hcon;
{
	register TUPLE_DESCRIPTOR tdp = *((TUPLE_DESCRIPTOR *)dp);	
						/* re-type as */
						/* tuple_descriptor */
	register int coded_len;			/* the value we're trying */
						/* to compute */

       /*
        * Validate the descriptor
        */
	if (tdp == NULL)
		GDB_GIVEUP("g_tpd_cdlen (coded length) was given a null tuple descriptor\nthis may be due to an attempt to transmit invalid data")
	GDB_CHECK_TPD(tdp,"g_tpd_cdlen: compute coded length of tuple descriptor")

	coded_len = g_in_cdlen((char *)&(tdp->field_count),hcon);
						/* we're going to send */
						/* the field count as a */
						/* true integer*/

	coded_len += tdp->str_len + tdp->field_count;
						/* space for all the */
						/* strings, with nulls, */
						/* and for the one byte */
						/* types*/

	return coded_len;
		
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_tpd_enc
	/*	
	/*	Encode a tuple_descriptor for transmission
	/*	
	/*----------------------------------------------------------*/

int
g_tpd_enc(dp, hcon, outp)
char *dp;					/* pointer to data */
HALF_CONNECTION hcon;				/* connection descriptor */
char *outp;					/* place to put the output */
{
	register TUPLE_DESCRIPTOR tdp = *((TUPLE_DESCRIPTOR *)dp);
						/* re-type as */
						/* tuple_descriptor */
	register char *nextp;			/* place to put next output */
						/* byte */
	register int i;				/* a loop counter  */

       /*
        * Validate the descriptor
        */
	if (tdp == NULL)
		GDB_GIVEUP("g_tpd_enc (encode) was given a null tuple descriptor\nthis may be due to an attempt to transmit invalid data")
	GDB_CHECK_TPD(tdp,"g_tpd_enc: encode tuple descriptor")

       /*
        * Use the integer coding routine to send the number of fields first
        */
						/* and coded form*/
	nextp = (char *)g_in_enc((char *)&(tdp->field_count), hcon, outp);

       /*
        * Next, put in the one byte codes for each of the field types
        */

	for (i=0; i<tdp->field_count; i++) {
		*nextp++ = tdp->var[i].type & 0xff; /* put out the one byte */
						   /* type codes */
	}

       /*
        * Finally, copy all the null terminated strings.
        */
	memcpy(nextp,((char *)(tdp))+gdb_descriptor_length(tdp->field_count), 
	       tdp->str_len);		/* copy the string data all */
						/* at once */
	return (int)(nextp+tdp->str_len);
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_tpd_dec
	/*	
	/*	Decode a tuple_descriptor from external form.  For
	/*	safety in memory management, we always re-allocate the
	/*	space for the tuple_descriptor. If the pointer passed
	/*	to us is not null, then we assume that it points to a
	/*	legal tuple descriptor, which we first free.  Because
	/*	data representation may change, we must re-do the
	/*	create-tuple-descriptor, so it can determine the local
	/*	machine dependent representation and alignment rules
	/*	for the data.
	/*	
	/*----------------------------------------------------------*/

#define GDB_MAX_DECODED_FIELDS 100

int
g_tpd_dec(outp, hcon, inp)
char *inp;					/* pointer to input data */
HALF_CONNECTION hcon;				/* connection descriptor */
char *outp;					/* place to put the output */
{
	register TUPLE_DESCRIPTOR *tdp = (TUPLE_DESCRIPTOR *)outp;	
						/* re-type as */
						/* tuple_descriptor */
	int field_count;			/* number of fields in the */
						/* newly received descriptor*/
	register int i;				/* a loop counter */

	register int tmp;			/* working variable to hold */
						/* type while they're being */
						/* sign extended */
	char *nextt;				/* next byte to scan for */
						/* a type code byte*/
	char *nextn;				/* next byte to scan for */
						/* a string name */
	char *field_names[GDB_MAX_DECODED_FIELDS];
						/* put pointers to the */
						/* field names here */
	FIELD_TYPE field_types[GDB_MAX_DECODED_FIELDS];
						/* put the field types in */
						/* the array here*/
       /*
        * Use the integer coding routine to get the number of fields
        */

	nextt = (char *)g_in_dec((char *)&field_count, hcon, inp);
	if (field_count > GDB_MAX_DECODED_FIELDS)
		GDB_GIVEUP("g_tpd_dec: Trying to decode tuple descriptor with too many fields.\n")


       /*
        * For each field, pick up its type code, being sure to sign extend,
        * and a pointer to its string name.
        */
	nextn = nextt + field_count;		/* there is one byte of */
						/* type info for each field, */
						/* after that comes the */
						/* first string.  nextn */
						/* now points to the first */
						/* string */
	for (i=0; i<field_count; i++) {
		tmp = *nextt++;			/* type code, may need */
						/* sign extension */
		if (tmp & 0x80)
			tmp |= ((~0) ^ 0xff);	/* sign extend if needed */
						/* this is the most machine */
						/* independent sign extension */
						/* I could come up with. */
						/* Presumes char is one byte, */
						/* but makes no assumption */
						/* about sizeof(int) */
		field_types[i] = tmp;
		field_names[i] = nextn;		/* pointer to name of the */
						/* field */
		nextn += strlen(nextn) +1;	/* set up for possible name */
						/* to follow */
	}

       /*
        * In case there was already a tuple descriptor here, free it.
        */

	delete_tuple_descriptor(*tdp);

       /*
        * Create a new descriptor based on the information we have received.
        */
	*tdp = create_tuple_descriptor(field_count, field_names, field_types);

	return (int)nextn;
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_tpd_form
	/*	
	/*	Format a tuple_descriptor on output logging file for
	/*	debugging.
	/*	
	/*----------------------------------------------------------*/

int
g_tpd_form(name, dp)
char *name;					/* tuple_descriptor name of the field */
char *dp;					/* pointer to the data */
{
	register TUPLE_DESCRIPTOR tdp = *((TUPLE_DESCRIPTOR *)dp);	
						/* re-type as */
						/* tuple_descriptor */
	register int i;				/* loop variable through */
						/* field definitions */


       /*
        *  Handle the special case where the descriptor is null
        */
	if (tdp == NULL) {
		fprintf(gdb_log, "TUPLE_DESCRIPTOR %s (loc=NULL)\n", name);
		return;
	}

       /*
        * Validate the descriptor
        */
	GDB_CHECK_TPD(tdp,"g_tpd_form: format tuple descriptor")

       /*
        * Descriptor is not null
        */
	fprintf(gdb_log, "TUPLE_DESCRIPTOR %s (loc=0x%x)\n", name, tdp);

	for (i=0; i<tdp->field_count; i++) {
		fprintf(gdb_log,"\tField Type Code = %3d %20s\tField Name=%s\n" ,
			tdp->var[i].type, 
			STR_PROPERTY(tdp->var[i].type,NAME_PROPERTY),
			tdp->var[i].name);
	}
	fprintf(gdb_log,"\n");
}


/************************************************************************/
/*	
/*			     TUPLE_T
/*	
/*	There is a distinction between the type tuple_t and the
/*	type tuple_data_t.  Tuple_t is a complete self-contained
/*	tuple, with its descriptor.  It actually refers to the
/*	tuple variable itself, which is a pointer.  Tuple_data 
/*	is only the data portion of the tuple, not the descriptor.
/*	It is used when the receiving tuple is already allocated,
/*	with a correct descriptor, for sending just the data.
/*	
/*	Note that some of the routines for tuple_t could have been
/*	implemented in terms of tuple_data_t routines.  For the
/*	moment, they have not been, but that may later be changed.
/*	Doesn't seem to make much difference as long as they are
/*	short and simple, and this way does save a bit of overhead.
/*	
/************************************************************************/

#define TP_LEN 		(sizeof(TUPLE))
#define TP_ALI 		TP_LEN
#define TP_NULL 	g_tp_null
#define TP_CDLEN 	g_tp_cdlen
#define TP_ENC 		g_tp_enc
#define TP_DEC		g_tp_dec
#define TP_FORM		g_tp_form
#define TP_NAME		"TUPLE_T"

	/*----------------------------------------------------------*/
	/*	
	/*			g_tp_null
	/*	
	/*	Fill in a null value for a tuple.
	/*	
	/*----------------------------------------------------------*/
int
g_tp_null(dp)
char *dp;					/* pointer to the data */
{
	*((TUPLE *)dp) = NULL;
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_tp_cdlen
	/*	
	/*	Return coded length for a tuple.  We have to send the
	/*	descriptor along with the data itself.  We do this
	/*	with calls to the appropriate encodeing routines.
	/*	
	/*----------------------------------------------------------*/

int
g_tp_cdlen(dp,hcon)
char *dp;					/* pointer to the data */
HALF_CONNECTION hcon;
{
	register TUPLE tup = *((TUPLE *)dp);	/* deref as tuple */
	register int len;			/* accumulated length */
	register int i;				/* index to fields */
	TUPLE_DESCRIPTOR tpd;			/* descriptor for this tuple */

       /*
        * Validate the tuple
        */
	if (tup == NULL)
		GDB_GIVEUP("g_tp_cdlen (coded length) was given a null tuple\nthis may be due to an attempt to transmit invalid data")
	GDB_CHECK_TUP(tup,"g_tp_cdlen: compute coded length of tuple")

       /*
        * First, get length of the descriptor when coded.
        */

	tpd = DESCRIPTOR_FROM_TUPLE(tup);
        len = g_tpd_cdlen((char *)&tpd,hcon);

       /*
        * Now, for each field, add in its coded length
        */
	
	for (i=0; i<tpd->field_count; i++) {
		len += (int)FCN_PROPERTY(FIELD_TYPE_IN_TUPLE(tpd,i),
				    CODED_LENGTH_PROPERTY)
		           (FIELD_FROM_TUPLE(tup, i),hcon);
	}
	
	return len;
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_tp_enc
	/*	
	/*	Encode a tuple for transmission
	/*	
	/*----------------------------------------------------------*/

int
g_tp_enc(dp, hcon, outp)
char *dp;					/* pointer to data */
HALF_CONNECTION hcon;				/* connection descriptor */
char *outp;					/* place to put the output */
{
	register TUPLE tup = *((TUPLE *)dp);	/* deref as tuple */
	register int i;				/* index to fields */
	TUPLE_DESCRIPTOR tpd;			/* descriptor for this tuple */
	char *op;				/* next byte of output */

       /*
        * Validate the tuple
        */
	if (tup == NULL)
		GDB_GIVEUP("g_tp_enc (encode) was given a null tuple\nthis may be due to an attempt to transmit invalid data")
	GDB_CHECK_TUP(tup,"g_tp_enc: encode tuple")

       /*
        * First, get the tuple descriptor and encode it
        */

	tpd = DESCRIPTOR_FROM_TUPLE(tup);
        op = (char *)g_tpd_enc((char *)&tpd, hcon, outp);

       /*
        * Now, for each field, code it
        */
	
	for (i=0; i<tpd->field_count; i++) {
		op = (char *)FCN_PROPERTY(FIELD_TYPE_IN_TUPLE(tpd,i),
				    ENCODE_PROPERTY)
		           (FIELD_FROM_TUPLE(tup, i),hcon, op);
	}
	
	return (int)op;
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_tp_dec
	/*	
	/*	Decode a tuple from external form.  For safety
	/*	in memory management, we always re-allocate the
	/*	space for the tuple, so the lengths come out right.
	/*	This may have nasty side effects if the intention
	/*	was to leave 'gas' at the end of the tuple, but
	/*	we want to accurately copy the data.  Note that
	/*	tuple_free is robust against null pointers.
	/*	
	/*----------------------------------------------------------*/

int
g_tp_dec(outp, hcon, inp)
char *inp;					/* pointer to input data */
HALF_CONNECTION hcon;				/* connection descriptor */
char *outp;					/* place to put the output */
{
	register TUPLE tup;			/* the new tuple */
	register int i;				/* index to fields */
	TUPLE_DESCRIPTOR tpd;			/* descriptor for this tuple */
	char *ip;				/* next byte of input */

       /*
        * First, get the tuple descriptor and decode it
        */

	tpd = NULL;				/* so decode will know */
						/* there's no existing one */
						/* to free */
        ip = (char *)g_tpd_dec((char *)&tpd, hcon, inp);

       /*
        * Now make an empty tuple based on the descriptor
        */

	tup = create_tuple(tpd);

       /*
        * The tuple descriptor has a reference count of 2 here, one 
        * from the tpd_dec routine, and one from the create_tuple.
        * Since we don't expect to explicitly undo the two separately,
        * we decrement the count here.
        */

	UNREFERENCE_TUPLE_DESCRIPTOR(tpd);	/* decr. the reference count */

       /*
        * Now, for each field, decode it.
        */
	
	for (i=0; i<tpd->field_count; i++) {
		ip = (char *)FCN_PROPERTY(FIELD_TYPE_IN_TUPLE(tpd,i),
				    DECODE_PROPERTY)
		           (FIELD_FROM_TUPLE(tup, i),hcon, ip);
	}
	
	*((TUPLE *)outp) = tup;			/* put the new tuple */
						/* pointer where the */
						/* caller wants it */
	return (int)ip;
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_tp_form
	/*	
	/*	Format a tuple on output logging file for
	/*	debugging.
	/*	
	/*----------------------------------------------------------*/

int
g_tp_form(name, dp)
char *name;					/* tuple name of the field */
char *dp;					/* pointer to the data */
{
	register TUPLE tup = *((TUPLE *)dp);	/* deref as tuple */
	register int i;				/* index to fields */
	TUPLE_DESCRIPTOR tpd;			/* descriptor for this tuple */


       /*
        * Handle special case where tuple is null
        */

	if (tup==NULL) {
		fprintf(gdb_log,"\nTUPLE Name=%s is NULL\n---------------------------\n",name);
		return;
	}

	GDB_CHECK_TUP(tup,"g_tp_form: format tuple")
       /*
        * Get the descriptor--for now, we won't print it 
        */
	tpd = DESCRIPTOR_FROM_TUPLE(tup);

       /*
        * Print a header
        */

	fprintf(gdb_log,"\nTUPLE at address: 0x%x Name=%s\n---------------------------\n",tup,name);

       /*
        * Now, for each field, print it
        */
	
	for (i=0; i<tpd->field_count; i++) {
		FCN_PROPERTY(FIELD_TYPE_IN_TUPLE(tpd,i),
				    FORMAT_PROPERTY)
		           (tpd->var[i].name,FIELD_FROM_TUPLE(tup, i));
	}
	
	fprintf(gdb_log,"END_OF_TUPLE\n");
}


/************************************************************************/
/*	
/*			     TUPLE_DATA_T
/*	
/*	The distinction between tuple_data_t and tuple_t is a
/*	subtle one.  Tuple_t is used when a single tuple is to
/*	be decoded, outside of any larger context.  It (re)allocates
/*	memory for both the tuple itself and its descriptor.
/*	
/*	Tuple_data is used in the case where the tuple and its
/*	descriptor are already allocated, but only the data is 
/*	to be received.  This is useful in cases like receiving an
/*	entire relation, in which the descriptor is common to 
/*	all the tuples, and should not be resent or reallocated
/*	with each one.  Receive relation can send the tuple descriptor
/*	once, then do a create_tuple followed by a decode tuple_data
/*	to receive the tuple field data into the existing tuple.
/*	
/*	Note that the definition of null is different in the two cases.
/*	The null value for a tuple is just a null pointer.  The null
/*	for tuple data is to null each of the fields in the tuple
/*	recursively.  The routines in this section may dereference
/*	null pointers if the tuples they are passed are null.  Note
/*	also that there is one less level of indirection in passing
/*	data to these routines than to those of tuple_t.  
/*	
/*	Note  also that the null and decode routines supplied here
/*	presume that any fields with dependent memory (e.g. string_t
/*	fields have already been cleaned up.)
/*	
/*	Note that this is not quite a kosher type, in the sense that
/*	it's length is not fixed.  The entry for length below 
/*	is meaningless, because the real length is computed from the
/*	desc.  Among other things, this means that TUPLEs cannot
/*	contain fields of this type.
/*	
/************************************************************************/

#define TDT_LEN 	(sizeof(TUPLE))
#define TDT_ALI 	TDT_LEN
#define TDT_NULL 	g_tdt_null
#define TDT_CDLEN 	g_tdt_cdlen
#define TDT_ENC 	g_tdt_enc
#define TDT_DEC		g_tdt_dec
#define TDT_FORM	g_tdt_form
#define TDT_NAME	"TUPLE_DATA_T"

	/*----------------------------------------------------------*/
	/*	
	/*			g_tdt_null
	/*	
	/*	Fill in a null value for a tuple.
	/*	
	/*----------------------------------------------------------*/
int
g_tdt_null(dp)
char *dp;					/* pointer to the data */
{
	TUPLE tup = (TUPLE)dp;			/* dp is of type TUPLE, */
						/* which is actually */
						/* a pointer to the */
						/* tuple data */
	TUPLE_DESCRIPTOR tpd;			/* the descriptor for this */
						/* tuple*/
	register int i;				/* a loop counter */

       /*
        * For each field in the tuple, call its null routine
        */
	tup->id = GDB_TUP_ID;

	tpd = DESCRIPTOR_FROM_TUPLE(tup);

	for (i=0; i<tpd->field_count; i++) {
		FCN_PROPERTY(FIELD_TYPE_IN_TUPLE(tpd,i),NULL_PROPERTY)
		    (FIELD_FROM_TUPLE(tup,i));
	}
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_tdt_cdlen
	/*	
	/*	Return coded length for tuple data.  Since the descriptor
	/*	for the tuple is known at both sides, we send only
	/*	the coded fields, not even the field counts.
	/*	
	/*----------------------------------------------------------*/

int
g_tdt_cdlen(dp,hcon)
char *dp;					/* pointer to the data */
HALF_CONNECTION hcon;
{
	register TUPLE tup = (TUPLE)dp;		/* arg typed as tuple */
	register int len;			/* accumulated length */
	register int i;				/* index to fields */
	TUPLE_DESCRIPTOR tpd;			/* descriptor for this tuple */

       /*
        * Validate the tuple data
        */
	if (tup == NULL)
		GDB_GIVEUP("g_tdt_cdlen (coded length) was given null tuple data\nthis may be due to an attempt to transmit invalid data")
	GDB_CHECK_TUP(tup,"g_tdt_cdlen: compute coded length of tuple data")
       /*
        * First, find the tuple descriptor and set initial coded len to 0
        */

	tpd = DESCRIPTOR_FROM_TUPLE(tup);
        len = 0;

       /*
        * Now, for each field, add in its coded length
        */
	
	for (i=0; i<tpd->field_count; i++) {
		len += (int)FCN_PROPERTY(FIELD_TYPE_IN_TUPLE(tpd,i),
				    CODED_LENGTH_PROPERTY)
		           (FIELD_FROM_TUPLE(tup, i),hcon);
	}
	
	return len;
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_tdt_enc
	/*	
	/*	Encode tuple data for transmission.
	/*	
	/*----------------------------------------------------------*/

int
g_tdt_enc(dp, hcon, outp)
char *dp;					/* pointer to data */
HALF_CONNECTION hcon;				/* connection descriptor */
char *outp;					/* place to put the output */
{
	register TUPLE tup = (TUPLE)dp;		/* type as tuple */
	register int i;				/* index to fields */
	TUPLE_DESCRIPTOR tpd;			/* descriptor for this tuple */
	char *op = outp;			/* next byte of output */

       /*
        * Validate the tuple data
        */
	if (tup == NULL)
		GDB_GIVEUP("g_tdt_enc (encode) was given null tuple data\nthis may be due to an attempt to transmit invalid data")
	GDB_CHECK_TUP(tup,"g_tdt_enc: encode of tuple data")
       /*
        * First, get the tuple descriptor 
        */

	tpd = DESCRIPTOR_FROM_TUPLE(tup);

       /*
        * Now, for each field, code it
        */
	
	for (i=0; i<tpd->field_count; i++) {
		op = (char *)FCN_PROPERTY(FIELD_TYPE_IN_TUPLE(tpd,i),
				    ENCODE_PROPERTY)
		           (FIELD_FROM_TUPLE(tup, i),hcon, op);
	}
	
	return (int)op;
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_tdt_dec
	/*	
	/*	Decode tuple data from external form.  We presume
	/*	that the tuple itself is allocated, and the descriptor
	/*	properly set up for the local machine representation.
	/*	Here we just decode the fields.
	/*	
	/*----------------------------------------------------------*/

int
g_tdt_dec(outp, hcon, inp)
char *inp;					/* pointer to input data */
HALF_CONNECTION hcon;				/* connection descriptor */
char *outp;					/* place to put the output */
{
	register TUPLE tup = (TUPLE)outp;	/* the filled in tuple */
	register int i;				/* index to fields */
	TUPLE_DESCRIPTOR tpd;			/* descriptor for this tuple */
	char *ip = inp;				/* next byte of input */

       /*
        * Validate the tuple data
        */
	if (tup == NULL)
		GDB_GIVEUP("g_tdt_dec (decode) was given null tuple data\nthis may be due to an attempt to transmit invalid data")
	GDB_CHECK_TUP(tup,"g_tdt_dec: decode of tuple data")
       /*
        * First, get the tuple descriptor 
        */

        tpd = DESCRIPTOR_FROM_TUPLE(tup);

       /*
        * Now, for each field, decode it.
        */
	
	for (i=0; i<tpd->field_count; i++) {
		ip = (char *)FCN_PROPERTY(FIELD_TYPE_IN_TUPLE(tpd,i),
				    DECODE_PROPERTY)
		           (FIELD_FROM_TUPLE(tup, i),hcon, ip);
	}
	
	return (int)ip;
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_tdt_form
	/*	
	/*	Format tuple data on output logging file for
	/*	debugging.
	/*	
	/*----------------------------------------------------------*/

int
g_tdt_form(name, dp)
char *name;					/* tuple name of the field */
char *dp;					/* pointer to the data */
{
	register TUPLE tup = (TUPLE)dp;		/* as tuple */
	register int i;				/* index to fields */
	TUPLE_DESCRIPTOR tpd;			/* descriptor for this tuple */


       /*
        * Handle special case where we're given a null address for the 
        * tuple
        */
	if (tup==NULL) {
		fprintf(gdb_log,"\nTUPLE Name=%s is NULL\n---------------------------\n",name);
		return;
	}


       /*
        * Validate the tuple data
        */
	GDB_CHECK_TUP(tup,"g_tdt_form: format tuple data")
       /*
        * Get the descriptor--for now, we won't print it 
        */
	tpd = DESCRIPTOR_FROM_TUPLE(tup);

       /*
        * Print a header
        */

	fprintf(gdb_log,"\nTUPLE at address: 0x%x  Name=%s\n---------------------------\n",tup,name);

       /*
        * Now, for each field, print it
        */
	
	for (i=0; i<tpd->field_count; i++) {
		FCN_PROPERTY(FIELD_TYPE_IN_TUPLE(tpd,i),
				    FORMAT_PROPERTY)
		           (tpd->var[i].name,FIELD_FROM_TUPLE(tup, i));
	}
	
	fprintf(gdb_log,"END_OF_TUPLE\n");
}


/************************************************************************/
/*	
/*			     RELATION_T
/*	
/*	Relations consist of link lists of tuples, all of which are
/*	presumed to share a tuple descriptor.  For transmission, 
/*	these are encoded as follows:
/*	
/*	1) A count of the number of tuples, sent as a properly coded
/*	   integer.
/*	
/*	2) The tuple descriptor itself, encoded by its encoding routine.
/*	
/*	3) For each tuple, its tuple data, encoded using the routines
/*	   of the tuple_data_t type.
/*	
/************************************************************************/

#define REL_LEN 	(sizeof(RELATION))
#define REL_ALI 	REL_LEN
#define REL_NULL 	g_rel_null
#define REL_CDLEN 	g_rel_cdlen
#define REL_ENC 	g_rel_enc
#define REL_DEC		g_rel_dec
#define REL_FORM	g_rel_form
#define REL_NAME	"RELATION_T"


	/*----------------------------------------------------------*/
	/*	
	/*			g_rel_null
	/*	
	/*	Fill in a null value for a relation.  Maybe we should
	/*	check for an existing relation and properly free it,
	/*	but for now, we don't.
	/*	
	/*----------------------------------------------------------*/
int
g_rel_null(dp)
char *dp;					/* pointer to the data */
{
	*((RELATION *)dp) = NULL;
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_rel_cdlen
	/*	
	/*	Return coded length for a relation.  
	/*	
	/*----------------------------------------------------------*/

int
g_rel_cdlen(dp,hcon)
char *dp;					/* pointer to the data */
HALF_CONNECTION hcon;
{
	register RELATION rel = *((RELATION *)dp); /* deref as relation */
	int  len;				/* accumulated length */
	register TUPLE t;			/* index to a tuple */
	int tuple_count = 0;			/* number of tuples in this */
						/* relation*/
	TUPLE_DESCRIPTOR tpd;			/* descriptor for this */
						/* relation */

       /*
        * Validate the relation
        */
	if (rel == NULL)
		GDB_GIVEUP("g_rel_cdlen (coded  length) was given null relation\nthis may be due to an attempt to transmit invalid data")
	GDB_CHECK_REL(rel,"g_rel_cdlen: compute coded length of relation")
       /*
        * First, get the tuple descriptor for this relation
        */

	tpd = DESCRIPTOR_FROM_RELATION(rel);

       /*
        * Count the number of tuples in the relation
        */
	for (t=FIRST_TUPLE_IN_RELATION(rel); t != NULL; 
	     t = NEXT_TUPLE_IN_RELATION(rel,t))
		tuple_count++;
       /*
        * Start with the coded length for the tuple count and the
        * descriptor, which are sent first.
        */

	len = g_in_cdlen((char *)&tuple_count, hcon);	/* length of tuple_count */
						/* in coded form */
        len += g_tpd_cdlen((char *)&tpd, hcon);

       /*
        * Now, for each tuple, add in its coded length
        */
	
	for (t=FIRST_TUPLE_IN_RELATION(rel); t != NULL; 
	     t = NEXT_TUPLE_IN_RELATION(rel,t))
		len += g_tdt_cdlen((char *)t, hcon);
	
	return len;
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_rel_enc
	/*	
	/*	Encode a relation for transmission
	/*	
	/*----------------------------------------------------------*/

int
g_rel_enc(dp, hcon, outp)
char *dp;					/* pointer to data */
HALF_CONNECTION hcon;				/* connection descriptor */
char *outp;					/* place to put the output */
{
	register RELATION rel = *((RELATION *)dp); /* deref as relation */
	char *op;				/* pointer to next unused */
						/* output byte*/
	register TUPLE t;			/* index to a tuple */
	int tuple_count = 0;			/* number of tuples in this */
						/* relation*/
	TUPLE_DESCRIPTOR tpd;			/* descriptor for this */
						/* relation */

       /*
        * Validate the relation
        */
	if (rel == NULL)
		GDB_GIVEUP("g_rel_enc (encode) was given null relation\nthis may be due to an attempt to transmit invalid data")
	GDB_CHECK_REL(rel,"g_rel_enc: encode relation")

       /*
        * First, get the tuple descriptor for this relation
        */

	tpd = DESCRIPTOR_FROM_RELATION(rel);

       /*
        * Count the number of tuples in the relation
        */
	for (t=FIRST_TUPLE_IN_RELATION(rel); t != NULL; 
	     t = NEXT_TUPLE_IN_RELATION(rel,t))
		tuple_count++;
       /*
        * Encode the count and the tuple descriptor for this relation
        */

	op = (char *)g_in_enc((char *)&tuple_count, hcon,outp); 
						/* length of tuple_count */
						/* in coded form */
        op = (char *)g_tpd_enc((char *)&tpd, hcon,op);

       /*
        * Now, encode each tuple
        */
	
	for (t=FIRST_TUPLE_IN_RELATION(rel); t != NULL; 
	     t = NEXT_TUPLE_IN_RELATION(rel,t))
		op = (char *)g_tdt_enc((char *)t, hcon, op);
	
	return (int)op;
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_rel_dec
	/*	
	/*	Decode a relation from external form.  We should 
	/*	really check to make sure the relation we are given
	/*	is null, and if not, call delete_relation on it 
	/*	first.  For the moment, we just presume it's null.
	/*	
	/*	We proceed by decoding the integer count and the
	/*	tuple descriptor, from which we create the null
	/*	relation.  We then loop for each tuple, doing a
	/*	create, a decode, and an add to relation.
	/*	
	/*----------------------------------------------------------*/

int
g_rel_dec(outp, hcon, inp)
char *inp;					/* pointer to input data */
HALF_CONNECTION hcon;				/* connection descriptor */
char *outp;					/* place to put the output */
{
	register RELATION rel;			/* build the relation here */
	char *ip;				/* pointer to next unused */
						/* input byte*/
	register TUPLE t;			/* index to a tuple */
	register int i;				/* loop counter on tuples */
	int tuple_count = 0;			/* number of tuples in this */
						/* relation*/
	TUPLE_DESCRIPTOR tpd;			/* descriptor for this */
						/* relation */

       /*
        * First, get the field count and tuple descriptor for this relation
        */

	ip = (char *)g_in_dec((char *)&tuple_count, hcon, inp);

	tpd = NULL;				/* so decode will know */
						/* there's no existing one */
						/* to free */
	ip = (char *)g_tpd_dec((char *)&tpd, hcon, ip);

       /*
        * Now, create a null relation using the descriptor
        */
	
	rel = create_relation(tpd);

       /*
        * The reference count for the tuple descriptor is currently 2, 
        * one from the tpd_dec and one from the create relation.  Since
        * these will not be undone separately, we decrement the reference
        * count to 1
        */

	UNREFERENCE_TUPLE_DESCRIPTOR(tpd);

       /*
        * For each tuple, create it, receive it, add it to the relation
        */
	
	for (i=0; i<tuple_count; i++) {
		t = create_tuple(tpd);
		ip = (char *)g_tdt_dec((char *)t, hcon, ip);
		ADD_TUPLE_TO_RELATION(rel, t);		
	}

       /*
        * Now store the address of the created relation where requested
        * and return pointer to next available input byte.
        */

	*((RELATION *)outp) = rel;

	return (int)ip;
}

	/*----------------------------------------------------------*/
	/*	
	/*			g_rel_form
	/*	
	/*	Format a relation on output logging file for
	/*	debugging.
	/*	
	/*----------------------------------------------------------*/

int
g_rel_form(name, dp)
char *name;					/* relation name of the field */
char *dp;					/* pointer to the data */
{
	register RELATION rel = *((RELATION *)dp); /* deref as relation */
	register TUPLE t;
	int count =0;
	char buffer[50];

       /*
        * Handle special case where relation is null
        */

	if (rel == NULL) {
		fprintf(gdb_log,"\nRELATION Name=%s is NULL\n===========================\n",name);
		return;
	}

	GDB_CHECK_REL(rel,"g_rel_form: format relation")

       /*
        * Print a header
        */

	fprintf(gdb_log,"\nRELATION at address: 0x%x Name=%s\n===========================\n",rel,name);

       /*
        * Now, for each field, print it
        */
	
	for (t=FIRST_TUPLE_IN_RELATION(rel); t != NULL; 
	     t = NEXT_TUPLE_IN_RELATION(rel,t)){
		(void) sprintf(buffer,"Number %d",++count);
		g_tdt_form(buffer,(char *)t);
	}
	
	fprintf(gdb_log,"END_OF_RELATION\n");
}


/************************************************************************/
/*	
/*	   DECLARE AND INITIALIZE THE SYSTEM TYPE DEFINITION 
/*			       TABLES
/*	
/*	This representation is clearly a real pain to keep up to date
/*	properly, mostly because C has such a lousy pre-processor.
/*	Probably this should be re-arranged so an initialization routine
/*	is called to set up the tables, but even that might be a nuissance.
/*	
/************************************************************************/

	/*----------------------------------------------------------*/
	/*	
	/*			gdb_i_stype
	/*	
	/*	Called at startup to initialize the type table with
	/*	the entries for the system types.
	/*	
	/*----------------------------------------------------------*/

#define ITYPE(inx,lp,ap,np,clp,ep,dp,fp,name) {\
	g_type_table[inx][LENGTH_PROPERTY].i = lp; \
	g_type_table[inx][ALIGNMENT_PROPERTY].i = ap; \
	g_type_table[inx][NULL_PROPERTY].f = np; \
	g_type_table[inx][CODED_LENGTH_PROPERTY].f = clp; \
	g_type_table[inx][ENCODE_PROPERTY].f = ep; \
	g_type_table[inx][DECODE_PROPERTY].f = dp; \
	g_type_table[inx][FORMAT_PROPERTY].f = fp; \
	g_type_table[inx][NAME_PROPERTY].cp = name; \
}

int
gdb_i_stype()
{
	gdb_n_types = SYSTEM_TYPE_COUNT;

	ITYPE(INTEGER_T,IN_LEN,IN_ALI,IN_NULL,IN_CDLEN,IN_ENC,IN_DEC,IN_FORM,
	      IN_NAME)
	ITYPE(STRING_T,ST_LEN,ST_ALI,ST_NULL,ST_CDLEN,ST_ENC,ST_DEC,ST_FORM,
	      ST_NAME)
	ITYPE(REAL_T,RL_LEN,RL_ALI,RL_NULL,RL_CDLEN,RL_ENC,RL_DEC,RL_FORM,
	      RL_NAME)
	ITYPE(DATE_T,DT_LEN,DT_ALI,DT_NULL,DT_CDLEN,DT_ENC,DT_DEC,DT_FORM,
	      DT_NAME)
	ITYPE(TUPLE_DESCRIPTOR_T,TPD_LEN,TPD_ALI,TPD_NULL,TPD_CDLEN,TPD_ENC,
	      TPD_DEC,TPD_FORM,TPD_NAME)
	ITYPE(TUPLE_T,TP_LEN,TP_ALI,TP_NULL,TP_CDLEN,TP_ENC,TP_DEC,TP_FORM,
	      TP_NAME)
	ITYPE(TUPLE_DATA_T,TDT_LEN,TDT_ALI,TDT_NULL,TDT_CDLEN,TDT_ENC,TDT_DEC,
	      TDT_FORM,TDT_NAME)
	ITYPE(RELATION_T,REL_LEN,REL_ALI,REL_NULL,REL_CDLEN,REL_ENC,REL_DEC,
	      REL_FORM,REL_NAME)
}
