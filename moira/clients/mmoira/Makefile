# $Header: /afs/.athena.mit.edu/astaff/project/moiradev/repository/moira/clients/mmoira/Makefile,v 1.2 1991-06-05 11:10:56 mar Exp $

CFLAGS = -I/mit/motif/include -g -D_NO_PROTO
LIBS = -L/mit/motif/$(MACHINE)lib -lXm -lXt -lX11 \
	-lmoira -lmrgdb -lhesiod -lkrb -ldes -lcom_err

OBJS = form_data.o form_setup.o menu_data.o main.o formup.o \
	queries.o help.o stubs.o display.o

mmoira: $(OBJS)
	$(CC) -o mmoira $(OBJS) $(LIBS) 

form_data.c: forms.txt forms.awk
	awk -f forms.awk forms.txt > form_data.c

menu_data.c: menus.txt menus.awk
	awk -f menus.awk menus.txt > menu_data.c

clean:
	rm -f *.o mmoira form_data.c menu_data.c
