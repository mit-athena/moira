#ifdef hpux
#define ccflags "-Wc,-Nd4000,-Ns3000 -DSYSV"
#else
#ifdef _AIX
#define ccflags "-D_BSD -D_BSD_INCLUDES -O"
#else
#define ccflags "-O"
#endif
#endif

main()
{
	write(1, ccflags, sizeof(ccflags) - 1);
	exit(0);
}
