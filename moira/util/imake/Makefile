#
# WARNING! This is NOT an automatically generated Makefile!
# This is hand-crafted as a bootstrap, may need editing for your system.

          SHELL = /bin/sh
             RM = rm -f
           MAKE = make
         RM_CMD = $(RM) *.CKP *.ln *.BAK *.bak *.o core errs ,* *~ *.a \
			tags TAGS make.log
             LN = cp
        DESTDIR = 
         SRCDIR = /mit/kerberos/src
        SOURCES = ${SRCDIR}/util/imake/imake.c ${SRCDIR}/util/imake/ccflags.c ${SRCDIR}/util/imake/Makefile

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

config: $(SOURCES)
	-mkdir $(DESTDIR)/util/imake
	for i in $(SOURCES); do \
		$(RM) ${DESTDIR}/util/imake/`basename $$i`; \
		$(LN) $$i ${DESTDIR}/util/imake/`basename $$i`; done
