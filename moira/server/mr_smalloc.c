#include <stdio.h>
botch(message)
	char *message;
{
	fprintf(stderr, "Malloc botch: %s\n", message);
	abort();
}	

#define rcheck
/****************************************************************
 *								*
 *		Storage Allocator for Foundation.		*
 *		Built from gnuemacs storage allocator		*
 *								*
 ****************************************************************/

/*   Copyright (C) 1985 Richard M. Stallman,
    based mostly on the public domain work of others.

This program is distributed in the hope that it will be useful,
but without any warranty.  No author or distributor
accepts responsibility to anyone for the consequences of using it
or for whether it serves any particular purpose or works at all,
unless he says so in writing.

   Permission is granted to anyone to distribute verbatim copies
   of this program's source code as received, in any medium, provided that
   the copyright notice, the nonwarraty notice above
   and this permission notice are preserved,
   and that the distributor grants the recipient all rights
   for further redistribution as permitted by this notice,
   and informs him of these rights.

   Permission is granted to distribute modified versions of this
   program's source code, or of portions of it, under the above
   conditions, plus the conditions that all changed files carry
   prominent notices stating who last changed them and that the
   derived material, including anything packaged together with it and
   conceptually functioning as a modification of it rather than an
   application of it, is in its entirety subject to a permission
   notice identical to this one.

   Permission is granted to distribute this program (verbatim or
   as modified) in compiled or executable form, provided verbatim
   redistribution is permitted as stated above for source code, and
    A.  it is accompanied by the corresponding machine-readable
      source code, under the above conditions, or
    B.  it is accompanied by a written offer, with no time limit,
      to distribute the corresponding machine-readable source code,
      under the above conditions, to any one, in return for reimbursement
      of the cost of distribution.   Verbatim redistribution of the
      written offer must be permitted.  Or,
    C.  it is distributed by someone who received only the
      compiled or executable form, and is accompanied by a copy of the
      written offer of source code which he received along with it.

   Permission is granted to distribute this program (verbatim or as modified)
   in executable form as part of a larger system provided that the source
   code for this program, including any modifications used,
   is also distributed or offered as stated in the preceding paragraph.

In other words, you are welcome to use, share and improve this program.
You are forbidden to forbid anyone else to use, share and improve
what you give them.   Help stamp out software-hoarding!  */

/****************************************************************
 *								*
 *		Helpful historical comments			*
 *								*
 ****************************************************************/

/*
 * @(#)nmalloc.c 1 (Caltech) 2/21/82
 *
 *	U of M Modified: 20 Jun 1983 ACT: strange hacks for Emacs
 *
 *	Nov 1983, Mike@BRL, Added support for 4.1C/4.2 BSD.
 *
 * This is a very fast storage allocator.  It allocates blocks of a small 
 * number of different sizes, and keeps free lists of each size.  Blocks
 * that don't exactly fit are passed up to the next larger size.  In this 
 * implementation, the available sizes are (2^n)-4 (or -16) bytes long.
 * This is designed for use in a program that uses vast quantities of
 * memory, but bombs when it runs out.  To make it a little better, it
 * warns the user when he starts to get near the end.
 *
 * June 84, ACT: modified rcheck code to check the range given to malloc,
 * rather than the range determined by the 2-power used.
 *
 * Jan 85, RMS: calls malloc_warning to issue warning on nearly full.
 * No longer Emacs-specific; can serve as all-purpose malloc for GNU.
 * You should call malloc_init to reinitialize after loading dumped Emacs.
 * Call malloc_stats to get info on memory stats if MSTATS turned on.
 * realloc knows how to return same block given, just changing its size,
 * if the power of 2 is correct.
 *
 * Jan 86, WDC: Removed Emacs specific stuff, and neatened a few comments.
 *
 * March 86 WDC: Added in code by Eichin for Scribble checking of blocks
 * Scribble check writes a known pattern into the free blocks, checks it
 * to see if it is still undamaged before allocating it.  It writes a
 * different pattern into the space beyond the end of an allocated block,
 * and tests it for damage when expanding the block's bounds in realloc.
 * Note, this check takes *TIME* and should not be compiled in by default.
 *
 * Berkeley UNIX 4.3 has a storage allocator that shares a common
 * ancestor with this one.  It handles realloc compatibly with the
 * archaic use of realloc on an already freed block to "compact"
 * storage.  It uses a pagesize system call rather than assuming the
 * page size is 1024 bytes.  Finally it guarantees that a freed block
 * is not munged by the allocator itself, incase someone wants to fiddle
 * with freed space after freeing it but before allocating more.
 *
 * This particular storage allocator would benefit from having a
 * non-hardwired pagesize.  But because of the scribble check it would
 * not be useful to keep the free pointer in the header.  SO: When you
 * free something allocated with this allocator, DONT TRY TO USE IT.
 * It is GUARANTEED to be damaged by the freeing process.
 *
 * For interfacing to systems that want to be able to ask the size of
 * the allocated block, rather than remembering it, the m_blocksize
 * function, rips open the block and tells you how big it is.  The size
 * returned is nbytes, the number of bytes asked for, NOT the actual
 * amount of space in the block.
 */

/****************************************************************
 *								*
 *	 Includes, declarations, and definitions		*
 *								*
 ****************************************************************/

/* Determine which kind of system this is.  */
#include <signal.h>
#ifndef SIGTSTP
#define USG
#else /* SIGTSTP */
#ifdef SIGIO
#define BSD42
#endif /* SIGIO */
#endif /* SIGTSTP */

#ifndef BSD42
#ifndef USG
#include <sys/vlimit.h>		/* warn the user when near the end */
#endif
#else /* if BSD42 */
#include <sys/time.h>
#include <sys/resource.h>
#endif /* BSD42 */
#include <moira_site.h>

#ifdef scribblecheck
#define rcheck
#endif /* we need to have range data to use block boundary checking */

#ifdef rcheck
/*
 * To implement range checking, we write magic values in at the
 * beginning and end of each allocated block, and make sure they
 * are undisturbed whenever a free or a realloc occurs.
 */

/* Written in each of the 4 bytes following the block's real space */
#define MAGIC1 0x55
#define MAGICFREE 0x69		/* 0110 1001 Magic value for Free blocks */

/* Written in the 4 bytes before the block's real space */
#define MAGIC4 0x55555555
#define MAGICFREE4 0x69696969

#define ASSERT(p) if (!(p)) botch("p"); else
#define EXTRA  4		/* 4 bytes extra for MAGIC1s */
#else
#define ASSERT(p)
#define EXTRA  0
#endif /* rcheck */

#define ISALLOC ((char) 0xf7)	/* magic byte that implies allocation */
#define ISFREE ((char) 0x54)	/* magic byte that implies free block */
				/* this is for error checking only */

/* If range checking is not turned on, all we have is a flag
 * indicating whether memory is allocated, an index in nextf[],
 * and a field that tells how many bytes.
 * To realloc() memory we copy nbytes.
 * 16 bits of header space is unused.
 */
struct mhead {
	char     mh_alloc;	/* ISALLOC or ISFREE */
	char     mh_index;	/* index in nextf[] */
	unsigned short mh_extra;/* Currently wasted 16 bits */
/* Remainder are valid only when block is allocated */
	unsigned mh_nbytes;	/* number of bytes allocated */
#ifdef rcheck
	int      mh_magic4;	/* should be == MAGIC4 */
#endif /* rcheck */
};

/*
 * Access free-list pointer of a block.
 * It is stored at block + 4.
 * This is not a field in the mhead structure because we want
 * sizeof (struct mhead) to describe the overhead for when the
 * block is in use, and we do not want the free-list pointer
 * to count in that.
 */
#define CHAIN(a) \
  (*(struct mhead **) (sizeof (char *) + (char *) (a)))


/****************************************************************
 *								*
 *		Variable Creations				*
 *								*
 ****************************************************************/

extern char etext;
extern char *start_of_data ();  /* This seems necessary for USG */

#ifdef notdef

/* These two are for user programs to look at, when they are interested.  */

int malloc_sbrk_used;       /* amount of data space used now */
int malloc_sbrk_unused;     /* amount more we can have */
#endif notdef
/* start of data space; can be changed by calling init_malloc */
static char *data_space_start;

#ifdef MSTATS
/*
 * nmalloc[i] is the difference between the number of mallocs and frees
 * for a given block size.
 */
static int nmalloc[30];
static int nmal, nfre;
#endif /* MSTATS */

/*
 * nextf[i] is the pointer to the next free block of size 2^(i+3).  The
 * smallest allocatable block is 8 bytes.  The overhead information will
 * go in the first int of the block, and the returned pointer will point
 * to the second.
 */
static struct mhead *nextf[30];

/* Number of bytes of writable memory we can expect to be able to get */
static int lim_data;
/* Level number of warnings already issued.
 * 0 -- no warnings issued.
 * 1 -- 75% warning already issued.
 * 2 -- 85% warning already issued.
 */
static int warnlevel;

#ifdef notdef
/* nonzero once initial bunch of free blocks made */
static int gotpool;
#endif notdef

/****************************************************************
 *								*
 *		Start of procedures				*
 *								*
 *	malloc_init, m_blocksize				*
 *								*
 ****************************************************************/

/*
 * Cause reinitialization based on job parameters;
 * also declare where the end of pure storage is.
 */
malloc_init (start)
     char *start;
{
  data_space_start = start;
  lim_data = 0;
  warnlevel = 0;
}

int m_blocksize(a_block)
     char *a_block;
{
  return(((struct mhead *)a_block-1)->mh_nbytes);
}

#if INGRESVER == 5 && defined(vax)
/* This is here to pull in our private version of meinitlst.o
 * so that we don't try to get the buggy Ingres 5.0 one.
 */
extern int MEinitLists();

static int (*foo)() = MEinitLists;
#endif
	

/****************************************************************
 *								*
 *    morecore - Ask the system for more memory			*
 *								*
 ****************************************************************/

static
morecore (nu)			/* ask system for more memory */
     register int nu;		/* size index to get more of  */
{
  char *sbrk ();
  register char *cp;
  register int nblks;
  register int siz;

#ifdef notdef
  if (!data_space_start)
    {
#if defined(USG)
      data_space_start = start_of_data ();
#else /* not USG */
      data_space_start = &etext;
#endif /* not USG */
    }

  if (lim_data == 0)
    get_lim_data ();

  /* On initial startup, get two blocks of each size up to 1k bytes */
  if (!gotpool)
    getpool (), getpool (), gotpool = 1;

  /* Find current end of memory and issue warning if getting near max */

  cp = sbrk (0);
  siz = cp - data_space_start;
  malloc_sbrk_used = siz;
  malloc_sbrk_unused = lim_data - siz;

  switch (warnlevel)
    {
    case 0: 
      if (siz > (lim_data / 4) * 3)
	{
	  warnlevel++;
	  malloc_warning ("Warning: past 75% of memory limit");
	}
      break;
    case 1: 
      if (siz > (lim_data / 20) * 17)
	{
	  warnlevel++;
	  malloc_warning ("Warning: past 85% of memory limit");
	}
      break;
    case 2: 
      if (siz > (lim_data / 20) * 19)
	{
	  warnlevel++;
	  malloc_warning ("Warning: past 95% of memory limit");
	}
      break;
    }

  if ((int) cp & 0x3ff)	/* land on 1K boundaries */
    sbrk (1024 - ((int) cp & 0x3ff));
#endif notdef
  
  /* Take at least 2k, and figure out how many blocks of the desired size
    we're about to get */
  nblks = 1;
  if ((siz = nu) < 8)
    nblks = 1 << ((siz = 8) - nu);
#if INGRESVER == 5 && defined(vax)
  {
      char *tcp;	  
      if (MEalloc(1, 1 << (siz+3), &tcp))
	return;			/* No more room! */
      cp = tcp;
  }
#else /* INGRESVER == 5 && defined(vax) */
  if ((cp = sbrk (1 << (siz + 3))) == (char *) -1)
    return;			/* no more room! */
#endif /* INGRESVER == 5 && defined(vax) */
  if ((int) cp & 7)
    {		/* shouldn't happen, but just in case */
      cp = (char *) (((int) cp + 8) & ~7);
      nblks--;
    }

  /* save new header and link the nblks blocks together */
  nextf[nu] = (struct mhead *) cp;
  siz = 1 << (nu + 3);
  while (1)
    {
      ((struct mhead *) cp) -> mh_alloc = ISFREE;
      ((struct mhead *) cp) -> mh_index = nu;
#ifdef rcheck
      ((struct mhead *) cp) -> mh_magic4 = MAGICFREE4;
#endif /* rcheck */
#ifdef scribblecheck
    {
      /* Check that upper stuff was still MAGIC1 */
      register char *m = (char *)((struct mhead *)cp+1);
      register char *en = (8<<nu) + cp;
      /* Fill whole block with MAGICFREE */
      while (m<en) *m++ = MAGICFREE;
    }
#endif /* scribblecheck */

      /* Clear newly allocated blocks, to match free ones */
      if (--nblks <= 0) break;
      CHAIN ((struct mhead *) cp) = (struct mhead *) (cp + siz);
      cp += siz;
    }
  CHAIN ((struct mhead *) cp) = 0;
}

/****************************************************************
 *								*
 *	getpool - Get initial pools of small blocks		*
 *								*
 ****************************************************************/
#ifdef notdef
static
getpool ()
{
  register int nu;
  register char *cp = sbrk (0);

  if ((int) cp & 0x3ff)	/* land on 1K boundaries */
    sbrk (1024 - ((int) cp & 0x3ff));

  /* Get 2k of storage */

  cp = sbrk (04000);
  if (cp == (char *) -1)
    return;

  /* Divide it into an initial 8-word block
     plus one block of size 2**nu for nu = 3 ... 10.  */

  CHAIN (cp) = nextf[0];
  nextf[0] = (struct mhead *) cp;
  ((struct mhead *) cp) -> mh_alloc = ISFREE;
  ((struct mhead *) cp) -> mh_index = 0;
#ifdef rcheck
      ((struct mhead *) cp) -> mh_magic4 = MAGICFREE4;
#endif /* rcheck */
  cp += 8;

  for (nu = 0; nu < 7; nu++)
    {
      CHAIN (cp) = nextf[nu];
      nextf[nu] = (struct mhead *) cp;
      ((struct mhead *) cp) -> mh_alloc = ISFREE;
      ((struct mhead *) cp) -> mh_index = nu;
#ifdef rcheck
      ((struct mhead *) cp) -> mh_magic4 = MAGICFREE4;
#endif /* rcheck */
#ifdef scribblecheck
    {
      register char *m = (char *)((struct mhead *)cp+1);
      register char *en = (8<<nu) + cp;
      /* Fill whole block with MAGICFREE */
      while (m<en) *m++ = MAGICFREE;
    }
#endif /* scribblecheck */
      cp += 8 << nu;
    }
}
#endif notdef

/****************************************************************
 *								*
 *	malloc - get a block of space from a pool		*
 *								*
 ****************************************************************/

char *
malloc (n)		/* get a block */
     unsigned n;
{
  register struct mhead *p;
  register unsigned int nbytes;
  register int nunits = 0;

  /* Figure out how many bytes are required, rounding up to the nearest
     multiple of 4, then figure out which nextf[] area to use */
  nbytes = (n + sizeof *p + EXTRA + 3) & ~3;
  {
    register unsigned int   shiftr = (nbytes - 1) >> 2;

    while (shiftr >>= 1)
      nunits++;
  }

  /* If there are no blocks of the appropriate size, go get some */
  /* COULD SPLIT UP A LARGER BLOCK HERE ... ACT */
  if (nextf[nunits] == 0)
    morecore (nunits);

  /* Get one block off the list, and set the new list head */
  if ((p = nextf[nunits]) == 0)
    return 0;
  nextf[nunits] = CHAIN (p);

  /* Check for free block clobbered */
  /* If not for this check, we would gobble a clobbered free chain ptr */
  /* and bomb out on the NEXT allocate of this size block */
  if (p -> mh_alloc != ISFREE || p -> mh_index != nunits)
#ifdef rcheck
    botch ("block on free list clobbered");
#else /* not rcheck */
    abort ();
#endif /* not rcheck */
#ifdef rcheck
  if (p -> mh_magic4 != MAGICFREE4)
    botch ("Magic in block on free list clobbered");
#endif /* rcheck */
#ifdef scribblecheck
  /* Check for block filled with magic numbers, then change to zeros */
  {
    register char  *m = (char *) (p + 1);
    register char *en = (8<<p->mh_index) + (char *) p;
    register int  block_valid = 0;
    while(m<en && (block_valid=(*m==MAGICFREE)))
      *m++=(char)0;
    /* so, status comes out as 1 if ok, 0 if terminated */
    if (!block_valid) botch ("data on free list damaged");
  }
#endif /* scribblecheck */
  /* Fill in the info, and if range checking, set up the magic numbers */
  p -> mh_alloc = ISALLOC;
  p -> mh_nbytes = n;
#ifdef rcheck
  p -> mh_magic4 = MAGIC4;
  {
    register char  *m = (char *) (p + 1) + n;
#ifdef scribblecheck
    register char *en = (8<<p->mh_index)+(char *)p;
    /* point to end of block */
    while (m<en) *m++ = MAGIC1;
#else /* scribblecheck */
    *m++ = MAGIC1, *m++ = MAGIC1, *m++ = MAGIC1, *m = MAGIC1; 
#endif /* scribblecheck */
  }
#endif /* not rcheck */
#ifdef MSTATS
  nmalloc[nunits]++;
  nmal++;
#endif /* MSTATS */
  return (char *) (p + 1);
}

/****************************************************************
 *								*
 *	free - Free a block of space				*
 *								*
 ****************************************************************/

free (mem)
     char *mem;
{
  register struct mhead *p;
  {
    register char *ap = mem;

    ASSERT (ap != 0);
    p = (struct mhead *) ap - 1;
    ASSERT (p -> mh_alloc == ISALLOC);
#ifdef rcheck
    ASSERT (p -> mh_magic4 == MAGIC4);
    ap += p -> mh_nbytes;
    p->mh_magic4 = MAGICFREE4;
    ASSERT (*ap++ == MAGIC1); ASSERT (*ap++ == MAGIC1);
    ASSERT (*ap++ == MAGIC1); ASSERT (*ap   == MAGIC1);
#endif /* rcheck */
  }
  {
    register int nunits = p -> mh_index;

    ASSERT (nunits <= 29);
#ifdef scribblecheck
    {
      /* Check that upper stuff was still MAGIC1 */
      register char  *m = (char *) (p + 1) + p->mh_nbytes;
      register char *en = (8<<p->mh_index) + (char *) p;
      register int  block_valid = 0;
      while(m<en && (block_valid=(*m++==MAGIC1)));
      if (!block_valid) botch ("block freed with data out of bounds");
      /* Fill whole block with MAGICFREE */
      m = (char *) (p + 1);
      while (m<en) *m++ = MAGICFREE;
    }
#endif /* scribblecheck */
    p -> mh_alloc = ISFREE;
    CHAIN (p) = nextf[nunits];
    nextf[nunits] = p;
#ifdef MSTATS
    nmalloc[nunits]--;
    nfre++;
#endif /* MSTATS */
  }
}

/****************************************************************
 *								*
 *	realloc - resize a block, copy if necessary		*
 *								*
 ****************************************************************/

char *
realloc (mem, n)
     char *mem;
     register unsigned n;
{
  register struct mhead *p;
  register unsigned int tocopy;
  register int nbytes;
  register int nunits;

  if ((p = (struct mhead *) mem) == 0)
    return malloc (n);
  p--;
  nunits = p -> mh_index;
  ASSERT (p -> mh_alloc == ISALLOC);
  tocopy = p -> mh_nbytes;
#ifdef rcheck
  ASSERT (p -> mh_magic4 == MAGIC4);
  {
    register char *m = mem + tocopy;
#ifdef scribblecheck
    register char *en = (8<<p->mh_index) + (char *)p;
    register int block_valid = 0;
    while(m<en && (block_valid=(*m++==MAGIC1)));
    if (!block_valid) botch ("out of bounds data on realloc");
#else /* scribblecheck */
    ASSERT (*m++ == MAGIC1); ASSERT (*m++ == MAGIC1);
    ASSERT (*m++ == MAGIC1); ASSERT (*m   == MAGIC1);
#endif /* scribblecheck */
  }
#endif /* not rcheck */

  /* See if desired size rounds to same power of 2 as actual size. */
  nbytes = (n + sizeof *p + EXTRA + 7) & ~7;

  /* If ok, use the same block, just marking its size as changed.  */
  if (nbytes > (4 << nunits) && nbytes <= (8 << nunits))
    {
      /* Here we check on realloc if we are grabbing unused space */
#ifdef rcheck
      register char *m = mem + tocopy;
#ifdef scribblecheck
      register char *en = (8<<p->mh_index) + (char *) p;
      while (m<en) *m++=(char)0;
#else /* scribblecheck */
      *m++ = 0;  *m++ = 0;  *m++ = 0;  *m++ = 0;
#endif /* scribblecheck */
      m = mem + n;
#ifdef scribblecheck
      while(m<en) *m++ = MAGIC1;
#else /* scribblecheck */
      *m++ = MAGIC1;  *m++ = MAGIC1;  *m++ = MAGIC1;  *m++ = MAGIC1;
#endif /* scribblecheck */
#endif /* not rcheck */
      p-> mh_nbytes = n;
      return mem;
    }

  if (n < tocopy)
    tocopy = n;
  {
    register char *new;

    if ((new = malloc (n)) == 0)
      return 0;
    bcopy (mem, new, tocopy);
    free (mem);
    return new;
  }
}

/****************************************************************
 *								*
 *	Memory Statistics stuff					*
 *								*
 ****************************************************************/

#ifdef MSTATS
/* Return statistics describing allocation of blocks of size 2**n. */

struct mstats_value
  {
    int blocksize;
    int nfree;
    int nused;
  };

struct mstats_value
malloc_stats (size)
     int size;
{
  struct mstats_value v;
  register int i;
  register struct mhead *p;

  v.nfree = 0;

  if (size < 0 || size >= 30)
    {
      v.blocksize = 0;
      v.nused = 0;
      return v;
    }

  v.blocksize = 1 << (size + 3);
  v.nused = nmalloc[size];

  for (p = nextf[size]; p; p = CHAIN (p))
    v.nfree++;

  return v;
}
#endif /* MSTATS */

#ifdef notdef
/****************************************************************
 *								*
 *	Stuff having to do with determining memory limits	*
 *								*
 ****************************************************************/

/*
 *	This function returns the total number of bytes that the process
 *	will be allowed to allocate via the sbrk(2) system call.  On
 *	BSD systems this is the total space allocatable to stack and
 *	data.  On USG systems this is the data space only.
 */

#ifdef USG

get_lim_data ()
{
  extern long ulimit ();
    
  lim_data = ulimit (3, 0);
  lim_data -= (long) data_space_start;
}

#else /* not USG */
#ifndef BSD42

get_lim_data ()
{
  lim_data = vlimit (LIM_DATA, -1);
}

#else /* BSD42 */

get_lim_data ()
{
  struct rlimit XXrlimit;

  getrlimit (RLIMIT_DATA, &XXrlimit);
  lim_data = XXrlimit.rlim_cur;		/* soft limit */
}

#endif /* BSD42 */
#endif /* not USG */
#endif notdef

/*
 * Calloc - allocate and clear memory block
 */
char *
calloc(num, size)
        register unsigned num, size;
{
        extern char *malloc();
        register char *p;

        size *= num;
        if (p = malloc(size))
                bzero(p, size);
        return (p);
}

cfree(p, num, size)
        char *p;
        unsigned num;
        unsigned size;
{
        free(p);
}
