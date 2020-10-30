
prefix = /usr/local
includedir = $(prefix)/include
libdir = $(prefix)/lib


MAJOR=2
MINOR=1
REVISION=1

SHELL=/bin/sh

CFLAGS = -O2 -Wall
CC = gcc

LIBSONAME = libdcgi.so.$(MAJOR).$(MINOR).$(REVISION)

all: $(LIBSONAME)

static: libdcgi.a

$(LIBSONAME): dcgi.c dcgi.h
	$(CC) $(CFLAGS) -fPIC -c dcgi.c
	ld -shared -soname libdcgi.so.$(MAJOR) -o $(LIBSONAME) -lc dcgi.o

libdcgi.a: dcgi.c dcgi.h
	$(CC) $(CFLAGS) -c dcgi.c
	ar rcs libdcgi.a dcgi.o

install: $(LIBSONAME)
	-mkdir -p $(DESTDIR)$(includedir)
	-mkdir -p $(DESTDIR)$(libdir)
	cp dcgi.h $(DESTDIR)$(includedir)
	chmod 644 $(DESTDIR)$(includedir)/dcgi.h
	cp $(LIBSONAME) $(DESTDIR)$(libdir)
	chmod 755 $(DESTDIR)$(libdir)/$(LIBSONAME)
	cd $(DESTDIR)$(libdir) ; ln -sf $(LIBSONAME) libdcgi.so
	cd $(DESTDIR)$(libdir) ; ln -sf $(LIBSONAME) libdcgi.so.$(MAJOR)
	cd $(DESTDIR)$(libdir) ; ln -sf $(LIBSONAME) libdcgi.so.$(MAJOR).$(MINOR)

install-static: libdcgi.a
	-mkdir -p $(DESTDIR)$(includedir)
	-mkdir -p $(DESTDIR)$(libdir)
	cp dcgi.h $(DESTDIR)$(includedir)
	chmod 644 $(DESTDIR)$(includedir)/dcgi.h
	cp libdcgi.a $(DESTDIR)$(libdir)
	chmod 644 $(DESTDIR)$(libdir)/libdcgi.a


.PHONY: clean
clean:
	-rm *.o libdcgi.*



