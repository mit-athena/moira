# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/Makefile,v 1.1 1991-05-28 21:04:49 mar Exp $

CFLAGS = -I/mit/motif/include -g
LIBS = -L/mit/motif/$(MACHINE)lib -lMu -lXm -lXt -lX11 \
	-lmoira -lmrgdb -lhesiod -lkrb -ldes -lcom_err

OBJS = form_data.o form_setup.o menu_data.o main.o formup.o \
	moira_cb.o help.o	

mmoira: $(OBJS)
	$(CC) -o mmoira $(OBJS) $(LIBS) 

form_data.c: forms.txt forms.awk
	awk -f forms.awk forms.txt > form_data.c

menu_data.c: menus.txt menus.awk
	awk -f menus.awk menus.txt > menu_data.c
