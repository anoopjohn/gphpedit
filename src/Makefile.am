## 
##  This file is part of gPHPEdit, a GNOME2 PHP Editor.
##
##  Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy@gphpedit.org>
##
##  This program is free software; you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation; either version 2 of the License, or
##  (at your option) any later version.
##
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with this program; if not, write to the Free Software
##  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
##

bin_PROGRAMS = gphpedit

INCLUDES = \
	$(GPHPEDIT_CFLAGS)	\
	-I$(top_srcdir)/src/gtkscintilla2 \
	-g -Wall -DGTK -DSCI_LEXER \
	-DPIXMAP_DIR=\""$(datadir)/pixmaps"\"

nodist_EXTRA_gphpedit_SOURCES = dummy.cxx
gphpedit_SOURCES = \
	calltip.c		\
	calltip.h		\
	classbrowser.c		\
	classbrowser.h		\
	classbrowser_parse.c	\
	classbrowser_parse.h	\
	edit_template.c		\
	edit_template.h		\
	find_replace.c		\
	find_replace.h		\
	gphpedit_ipc.c		\
	gphpedit_ipc.h		\
	grel2abs.c		\
	grel2abs.h		\
	plugin.c		\
	plugin.h		\
	main.c			\
	main.h			\
	main_window.c		\
	main_window.h		\
	main_window_callbacks.c	\
	main_window_callbacks.h	\
	main_window_menu.h	\
	preferences.c		\
	preferences.h		\
	preferences_dialog.c	\
	preferences_dialog.h	\
	syntax_check.c		\
	syntax_check.h		\
	tab.c			\
	tab.h			\
	tab_cxx.c		\
	tab_cxx.h		\
	tab_css.c		\
	tab_css.h		\
	tab_perl.c		\
	tab_perl.h		\
	tab_php.c		\
	tab_php.h		\
	tab_python.c		\
	tab_python.h		\
	tab_sql.c		\
	tab_sql.h		\
	tab_util.c		\
	tab_util.h		\
	templates.c		\
	templates.h

gtkscintilla2/libgtkscintilla2.a:
	cd gtkscintilla2 && $(MAKE) libgtkscintilla2.a && cd ..

gphpedit_LDADD = \
	$(GPHPEDIT_LIBS)\
	gtkscintilla2/libgtkscintilla2.a\
	gtkscintilla2/scintilla/bin/scintilla.a
