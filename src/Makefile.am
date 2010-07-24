##
##  This file is part of gPHPEdit, a GNOME2 PHP Editor.
##
##  Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
##  Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
##
##  For more information or to find the latest release, visit our
##  website at http://www.gphpedit.org/
##
##  gPHPEdit is free software: you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  gPHPEdit is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with gPHPEdit.  If not, see <http://www.gnu.org/licenses/>.
##
##  The GNU General Public License is contained in the file COPYING.
##

bin_PROGRAMS = gphpedit

AM_CFLAGS = \
	$(WEBKIT_CFLAGS) $(GTK_CFLAGS) $(GCONF_CFLAGS) \
	-I$(top_srcdir)/src/gtkscintilla2 \
	-Wall -DGTK -DSCI_LEXER \
	-DPIXMAP_DIR=\""$(datadir)/pixmaps"\" 

nodist_EXTRA_gphpedit_SOURCES = dummy.cxx
gphpedit_SOURCES = \
	gvfs_utils.c		\
	gvfs_utils.h		\
	gphpedit-close-button.h		\
	gphpedit-close-button.c		\
	gphpedit-statusbar.h		\
	gphpedit-statusbar.c		\
	status-combo-box.h		\
	status-combo-box.c		\
	calltip.c		\
	calltip.h		\
	images.h		\
	classbrowser_ui.h		\
	classbrowser_ui.c		\
	classbrowser_backend.h		\
	classbrowser_backend.c		\
	classbrowser_parse.c	\
	classbrowser_parse.h	\
	history-entry.c		\
	history-entry.h		\
	filebrowser_backend.c		\
	filebrowser_backend.h		\
	filebrowser_ui.c		\
	filebrowser_ui.h		\
	edit_template.c		\
	edit_template.h		\
	find_dialog.h		\
	find_dialog.c		\
	replace_dialog.h		\
	replace_dialog.c		\
	gphpedit_ipc.c		\
	gphpedit_ipc.h		\
	plugins.c		\
	plugins.h		\
	pluginmanager.h		\
	pluginmanager.c		\
	pluginmenu.h		\
	pluginmenu.c		\
	main.c			\
	main.h			\
	menu.c			\
	menu.h			\
	toolbar.c		\
	toolbar.h		\
	main_window.c		\
	main_window.h		\
	main_window_callbacks.c	\
	main_window_callbacks.h	\
	preferences_manager.h		\
	preferences_manager.c		\
	preferences_dialog.c	\
	preferences_dialog.h	\
  syntax_check_manager.h \
  syntax_check_manager.c \
	syntax_check_window.h		\
	syntax_check_window.c		\
	tab.c			\
	tab.h			\
	tab_cxx.c		\
	tab_cxx.h		\
	tab_css.c		\
	tab_cobol.c		\
	tab_cobol.h		\
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
	$(WEBKIT_LIBS) $(GTK_LIBS) $(GCONF_LIBS)\
	gtkscintilla2/libgtkscintilla2.a\
	gtkscintilla2/scintilla/bin/scintilla.a
