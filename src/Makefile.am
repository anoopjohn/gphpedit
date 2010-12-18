##
##  This file is part of gPHPEdit, a GNOME2 PHP Editor.
##
##  Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
##  Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
##  Copyright (C) 2010 - Jose Rostagno
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

uidir = $(pkgdatadir)/ui
dist_ui_DATA = preferences_dialog.ui \
  classbrowser.ui \
  filebrowser.ui

AM_CFLAGS = \
	$(WEBKIT_CFLAGS) $(GTK_CFLAGS) $(GCONF_CFLAGS) $(LIBXML_CFLAGS)\
	-I$(top_srcdir)/src/gtkscintilla2 \
	-Wall -DGTK -DSCI_LEXER \
	-DPIXMAP_DIR=\""$(datadir)/pixmaps"\" \
  -DGPHPEDIT_UI_DIR=\""$(uidir)"\"

nodist_EXTRA_gphpedit_SOURCES = dummy.cxx
gphpedit_SOURCES = \
  calltip.c \
  calltip.h \
  classbrowser_backend.c \
  classbrowser_backend.h \
  classbrowser_parse.c \
  classbrowser_parse.h \
  classbrowser_ui.c \
  classbrowser_ui.h \
  debug.c \
  debug.h \
  documentable.c \
  documentable.h \
  document_webkit.c \
  document_webkit.h \
  document_scintilla.c \
  document_scintilla.h \
  document.c \
  document.h \
  document_loader.c \
  document_loader.h \
  document_saver.c \
  document_saver.h \
  document_manager.c \
  document_manager.h \
  edit_template.c \
  edit_template.h \
  filebrowser_backend.c \
  filebrowser_backend.h \
  filebrowser_ui.c \
  filebrowser_ui.h \
  find_dialog.c \
  find_dialog.h \
  gphpedit-close-button.c \
  gphpedit-close-button.h \
  gphpedit-session.c \
  gphpedit-session.h \
  gphpedit-statusbar.c \
  gphpedit-statusbar.h \
  gphpedit_ipc.c \
  gphpedit_ipc.h \
  gvfs_utils.c \
  gvfs_utils.h \
  goto_infobar.c \
  goto_infobar.h \
  history-entry.c \
  history-entry.h \
  images.h \
  main.c \
  main.h \
  main_window.c \
  main_window.h \
  main_window_callbacks.c \
  main_window_callbacks.h \
  menubar.c \
  menubar.h \
  plugindialog.c \
  plugindialog.h \
  pluginmanager.c \
  pluginmanager.h \
  pluginmenu.c \
  pluginmenu.h \
  plugins.c \
  plugins.h \
  preferences_dialog.c \
  preferences_dialog.h \
  preferences_manager.c \
  preferences_manager.h \
  replace_dialog.c \
  replace_dialog.h \
  status-combo-box.c \
  status-combo-box.h \
  search_infobar.c \
  search_infobar.h \
  syntax_check_manager.c \
  syntax_check_manager.h \
  syntax_check_window.c \
  syntax_check_window.h \
  tab.c \
  tab.h \
  tab_cobol.c \
  tab_cobol.h \
  tab_css.c \
  tab_css.h \
  tab_cxx.c \
  tab_cxx.h \
  tab_perl.c \
  tab_perl.h \
  tab_php.c \
  tab_php.h \
  tab_python.c \
  tab_python.h \
  tab_sql.c \
  tab_sql.h \
  tab_util.c \
  tab_util.h \
  templates.c \
  templates.h \
  templates_editor.c \
  templates_editor.h \
  gtksourcestylescheme.c \
  gtksourcestylescheme.h \
  gtksourcestyleschememanager.c \
  gtksourcestyleschememanager.h \
  gtksourcestyle.c \
  gtksourcestyle.h \
  gtksourcestyle-private.h \
  gtksourceview-utils.c \
  gtksourceview-utils.h \
  toolbar.c \
  toolbar.h


gtkscintilla2/libgtkscintilla2.a:
	cd gtkscintilla2 && $(MAKE) libgtkscintilla2.a && cd ..

gphpedit_LDADD = \
	$(WEBKIT_LIBS) $(GTK_LIBS) $(GCONF_LIBS) $(LIBXML_LIBS)\
	gtkscintilla2/libgtkscintilla2.a\
	gtkscintilla2/scintilla/bin/scintilla.a
