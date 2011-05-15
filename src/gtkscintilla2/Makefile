# Makefile for the GtkScintilla2 widget

NAME = GtkScintilla2
LIB_NAME = gtkscintilla2
VERSION = 0.2.6
PREFIX = $(shell pkg-config --variable=prefix gtk+-3.0)

SCINTILLA_DIR = ./scintilla
SCINTILLA_INCLUDE = $(SCINTILLA_DIR)/include
SCINTILLA_LIB = $(SCINTILLA_DIR)/bin/scintilla.a
SCINTILLA_IFACE = $(SCINTILLA_INCLUDE)/Scintilla.iface
LEXERS = $(wildcard $(SCINTILLA_DIR)/gtk/Lex*.o)
CFLAGS = $(shell pkg-config --cflags gtk+-3.0) -g -Wall -DGTK -DGTK3 -DSCI_LEXER -fPIC
LDFLAGS_PRE = $(shell pkg-config --libs gtk+-3.0) -DGTK3
LDFLAGS_POST =  $(LEXERS)  $(SCINTILLA_LIB) -lstdc++

ARCH=-arch i386 
CC = gcc
LD = gcc
AR = ar

STATIC_LIB = lib$(LIB_NAME).a
SHARED_LIB = lib$(LIB_NAME).so.$(VERSION)

LIB_DIR = $(PREFIX)/lib
INCLUDE_DIR = $(PREFIX)/include/$(LIB_NAME)
PKG_CONFIG_DIR = $(PREFIX)/lib/pkgconfig

all: $(STATIC_LIB) $(SHARED_LIB)

$(STATIC_LIB): $(SCINTILLA_LIB) gtkscintilla.o marshal.o
	$(AR) -rs $@ $^

$(SHARED_LIB): $(SCINTILLA_LIB) gtkscintilla.o marshal.o
	$(LD)  $(LDFLAGS_PRE) gtkscintilla.o marshal.o -shared $(LDFLAGS_POST) -o $@

# GObject Signal Marshallers
marshal.o: marshal.c marshal.h
marshal.h: marshal.list
	glib-genmarshal --header --prefix="scintilla_marshal" marshal.list > $(@F)
marshal.c: marshal.h marshal.list
	glib-genmarshal --body --prefix="scintilla_marshal" marshal.list > $(@F)

gtkscintilla.o: gtkscintilla.h gtkscintilla.c marshal.c marshal.h

$(SCINTILLA_LIB):
	cd $(SCINTILLA_DIR)/gtk ; $(MAKE) GTK3=1

gtkscintilla.h gtkscintilla.c: scigen.py $(SCINTILLA_IFACE)
	python scigen.py

install: all
	install -D --mode=755 $(SHARED_LIB) $(DESTDIR)/$(LIB_DIR)/$(SHARED_LIB)
	install -D --mode=755 $(STATIC_LIB) $(DESTDIR)/$(LIB_DIR)/$(STATIC_LIB)
	rm -f $(DESTDIR)/$(LIB_DIR)lib$(LIB_NAME).so
	workdir=$$(pwd) ; cd $(DESTDIR)/$(LIB_DIR); \
	ln -s $(SHARED_LIB) lib$(LIB_NAME).so ; \
	cd $$workdir
	install -D --mode=644 gtkscintilla.h $(DESTDIR)/$(INCLUDE_DIR)/gtkscintilla.h
	install -D --mode=644 GtkScintilla-2.0.pc $(DESTDIR)/$(PKG_CONFIG_DIR)/GtkScintilla-2.0.pc

uninstall:
	rm -f $(DESTDIR)/$(LIB_DIR)$(SHARED_LIB)
	rm -f $(DESTDIR)/$(LIB_DIR)$(STATIC_LIB)
	rm -f $(DESTDIR)/$(LIB_DIR)/lib$(LIB_NAME).so
	rm -f $(DESTDIR)/$(INCLUDE_DIR)gtkscintilla.h

clean:
	rm -f *.o *.so *~ core $(SHARED_LIB) $(STATIC_LIB)
	rm -f $(SCINTILLA_DIR)/bin/scintilla.a
	cd $(SCINTILLA_DIR)/gtk ; $(MAKE) clean
#	rm -f *.c *.h

realclean: clean
	rm -f gtkscintilla.h gtkscintilla.c

dist: clean
	if test -d "$(NAME)-$(VERSION)"; then rm -rf $(NAME)-$(VERSION); fi
	if test -f "$(NAME)-$(VERSION).tar.gz"; then rm -f $(NAME)-$(VERSION).tar.gz; fi
	mkdir $(NAME)-$(VERSION)
	cp -r scintilla $(NAME)-$(VERSION)
	cp *.c $(NAME)-$(VERSION)
	cp *.h $(NAME)-$(VERSION)
	cp Makefile $(NAME)-$(VERSION)
	cp README $(NAME)-$(VERSION)
	cp COPYING $(NAME)-$(VERSION)
	cp GtkScintilla-2.0.pc $(NAME)-$(VERSION)
	cp GtkScintilla.spec $(NAME)-$(VERSION)
	cp scigen.py $(NAME)-$(VERSION)
	cp gtkscintilla.h.in $(NAME)-$(VERSION)
	cp gtkscintilla.c.in $(NAME)-$(VERSION)
	(GZIP=--best ; tar cvzf $(NAME)-$(VERSION).tar.gz $(NAME)-$(VERSION))
	rm -rf $(NAME)-$(VERSION)
