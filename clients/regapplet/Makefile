# $Id$

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
