#
# WARNING! This is NOT an automatically generated Makefile!
# This is hand-crafted as a bootstrap, may need editing for your system.

          SHELL = /bin/sh
             RM = rm -f
           MAKE = make
         RM_CMD = $(RM) *.CKP *.ln *.BAK *.bak *.o core errs ,* *~ *.a \
			tags TAGS make.log

all: imake

imake: ccflags imake.c
	$(CC) -o imake imake.c `./ccflags`

ccflags: ccflags.c
	$(CC) -o ccflags ccflags.c

relink:
	$(RM) imake
	$(MAKE) $(MFLAGS) imake

clean:
	$(RM) ccflags imake
	$(RM_CMD) \#*

depend:

Makefile:

tags:

install:

Makefiles:
