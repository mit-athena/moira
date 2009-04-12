# $Id: Makefile,v 1.1 2005-06-22 06:03:38 zacheiss Exp $

SHELL=/bin/sh
TARGET=regclasses.zip
JAVAC=javac

srcdir=.

all: $(TARGET)

$(TARGET): 
	$(JAVAC) -classpath $(srcdir) mit/*/*.java regapplet/*.java
	zip -0 $@ mit/*/*.class regapplet/*.class
clean:
	rm -f mit/*/*.class regapplet/*.class regclasses.zip
