/*
 *	$Source: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_debug.c,v $
 *	$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_debug.c,v 1.3 1997-01-29 23:16:45 danw Exp $
 */

#ifndef lint
static char *rcsid_gdb_debug_c = "$Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/gdb/gdb_debug.c,v 1.3 1997-01-29 23:16:45 danw Exp $";
#endif

/************************************************************************/
/*      
/*                      gdb_debug.c
/*      
/*      Debugging interfaces for gdb.  Some of these are
/*	designed to be called from dbx.
/*      
/*      routines included are:
/*	
/*		gdb_debug set the debugging status flags
/*      
/*              gd_types_print prints all the available types currently in
/*                      the type table
/*              
/*              gd_con_status prints all the status codes for connections
/*      
/*              gd_sum_cons summarizes all current connections detailing 
/*                      for each flags, half connections with their 
/*                      respective operation queues.
/*      
/*              gd_op_q (halfcon) prints the queue associated with a 
/*                      specified half connection (not directly callable) 
/*              
/*              gd_op_status a listing of the available states of an
/*                      operation
/*      
/*      Copyright 1986 MIT Project Athena
/*		For copying and distribution information, please see
/*	  	the file <mit-copyright.h>.
/*      
/************************************************************************/

#include <mit-copyright.h>
#include <stdio.h>
#include "gdb.h"

/************************************************************************/
/*	
/*			gdb_debug
/*	
/*	Toggle a debugging flag.  Warning:  the interface to this routine
/*	may change over time.
/*	
/************************************************************************/

int
gdb_debug(flag)
{
	gdb_Debug ^= flag;			/* toggle the flag */
}

/************************************************************************/
/*	
/*			print_relation
/*	
/************************************************************************/

int
print_relation(name, relation)
char *name;
RELATION relation;
{
	FCN_PROPERTY(RELATION_T,FORMAT_PROPERTY)(name, (char *)&(relation));
}
/************************************************************************/
/*	
/*			print_tuple
/*	
/************************************************************************/

int
print_tuple(name, tuple)
char *name;
TUPLE tuple;
{
	FCN_PROPERTY(TUPLE_T,FORMAT_PROPERTY)(name, (char *)&(tuple));
}

/************************************************************************/
/*	
/*			print_tuple_descriptor
/*	
/************************************************************************/

int
print_tuple_descriptor(name, tuple_descriptor)
char *name;
TUPLE_DESCRIPTOR tuple_descriptor;
{
	FCN_PROPERTY(TUPLE_DESCRIPTOR_T,FORMAT_PROPERTY)(name, 
					     (char *)&(tuple_descriptor));
}

/************************************************************************/
/*      
/*                      gd_types_print
/*
/*      This is a routine for printing all the available types and 
/*      their typecodes.
/*      
/************************************************************************/

int
gd_types_print ()
{
	register int i;

	printf ("\n\nTHE AVAILABLE TYPES WITH THEIR TYPE CODES ARE: \n\n");

	printf ("typecode     name\n");

	for (i = 0; i < gdb_n_types; i++) {
		printf ("%2d       %s \n", i, STR_PROPERTY(i,NAME_PROPERTY));
	}
}

/************************************************************************/
/*      
/*                      con_status
/*      
/*      This routine will print all the status codes for operations 
/*      This is just a listing of the status numbers located in gdb.h
/*      
/************************************************************************/

int
gd_con_status () 
{
        /*----------------------------------------------------------*/
        /*      
        /*      REMEMBER... these need to be fixed when the connection
        /*      status coded in gdb.h are redefined.
        /*      
        /*----------------------------------------------------------*/

        printf ("THE STATUS CODES ARE: \n\n");
        printf (" CODE     STATUS\n");
        printf ("   1      CON_STOPPED\n");
        printf ("   2      CON_UP\n");
        printf ("   3      CON_STARTING\n");
        printf ("   4      CON_STOPPING\n"); 

}


/************************************************************************/
/*      
/*                      summarize connections (gd_sum_con)
/*      
/************************************************************************/

gd_sum_con (index)
int index;
{
        if ((index > gdb_mcons) || (gdb_cons[index].status<1)) {
                printf ("gdb_cons[%d] is not a valid connection \n",index);
                return;
        }

        if (gdb_cons[index].status == CON_STOPPED) {
                printf ("connection gdb_cons[%d] is stopped\n",index);
                return;
        }

        /*----------------------------------------------------------*/
        /*      
        /*      REMEMBER this also must be changed when the def'n
        /*      of status fields in gdb.h is changed 
        /*      
        /*----------------------------------------------------------*/

                
        printf ("status of connection number %d is %2d \n",index,gdb_cons[index].status);
        printf ("The information for each half-connexn: \n\n");

        printf ("    the inbound half-connection\n");
        printf ("              status: %2d \n",gdb_cons[index].in.status);
        printf ("              flags : %2d \n",gdb_cons[index].in.status);
        printf ("              file descr: %2d \n",gdb_cons[index].in.fd);
        printf ("              The operation queue is:\n");
        gd_op_q (&(gdb_cons[index].in));

        printf ("    the outbound half-connection\n");
        printf ("              status: %2d \n",gdb_cons[index].out.status);
        printf ("              flags : %2d \n",gdb_cons[index].out.status);
        printf ("              file descr: %2d \n",gdb_cons[index].out.fd);
        printf ("              The operation queue is:\n");
        gd_op_q (&(gdb_cons[index].out));
      }


/************************************************************************/
/*
/*                      op_q (gd_op_q) 
/*      
/************************************************************************/


int 
gd_op_q (half_con)
HALF_CONNECTION half_con;

{
        int i;                                  /*counter for the ith
                                                  queued op */
        OPERATION current;
        
        current = half_con->op_q_first;  

        i = 0;
        
        if (current == NULL) {
                printf ("no operations in queue yet\n");
                return ;
        }


        printf ("OPERATION       STATUS\n\n");

        while (current != (OPERATION)half_con)  {
                printf ("%2d              %2d \n", i++ , current->status);
                current = current ->next;
        }
}

/************************************************************************/
/*      
/*                      op status
/*      this is a routine in which all the status codes and their 
/*      translations are printed.
/*      
/************************************************************************/
       
int 
gd_op_status ()
{
        /*----------------------------------------------------------*/
        /*      
        /*      REMEMBER these also need to be changed when 
        /*      states of an operation in gdb.h is redefined
        /*      
        /*----------------------------------------------------------*/

          printf ("CODE    OPERATION STATE\n\n");
          printf (" 1      OP_NOT_STARTED\n");
          printf (" 2      OP_QUEUED\n");
          printf (" 3      OP_RUNNING\n");
          printf (" 4      OP_COMPLETE\n");
          printf (" 5      OP_CANCELLING\n");
          printf (" 6      OP_CANCELLED\n");
          printf (" 7      OP_MARKED\n");
}
