BINDIR=/usr/bin
all: winlaunch 
CFLAGS=-DWIN64

VERSION=0.2c

winlaunch: main.o cygwin.o
	gcc -o winlaunch main.o cygwin.o 

main.o: main.c defs.h 
	gcc $(CFLAGS) -DVERSION=\"$(VERSION)\" -Wall -o main.o -c main.c

cygwin.o: cygwin.c defs.h 
	gcc $(CFLAGS) -DVERSION=\"$(VERSION)\" -Wall -o cygwin.o -c cygwin.c

clean:
	rm -f winlaunch *.o dump *.stackdump 

install:
	cp -p winlaunch $(BINDIR)

dist: clean
	cd ..;  tar cvfz /tmp/winlaunch-0.2.tar.gz winlaunch 

