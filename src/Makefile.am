##
##  This file is part of gPHPEdit, a GNOME PHP Editor.
##
##  Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
##  Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
##  Copyright (C) 2010, 2011 - Jose Rostagno
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
  gphpedit.ui \
  classbrowser.ui \
  filebrowser.ui  \
  toolbar.ui \
  find_dialog.ui \
  replace_dialog.ui

AM_CFLAGS = \
	$(WEBKIT_CFLAGS) $(GTK_CFLAGS) $(GLIB_CFLAGS) $(LIBXML_CFLAGS)\
	-I$(top_srcdir)/src/gtkscintilla2 \
	-Wall -DGTK -DSCI_LEXER\
	-DPIXMAP_DIR=\""$(datadir)/pixmaps"\" \
  -DGPHPEDIT_UI_DIR=\""$(uidir)"\"

nodist_EXTRA_gphpedit_SOURCES = dummy.cxx

BUILT_SOURCES = 			\
  gphpedit-marshal.c			\
  gphpedit-marshal.h

gphpedit_SOURCES = \
  $(BUILT_SOURCES) \
  classbrowser.c \
  classbrowser.h \
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
  document_types.c \
  document_types.h \
  language_provider.c \
  language_provider.h \
  language_css.c \
  language_css.h \
  language_cxx.c \
  language_cxx.h \
  language_cobol.c \
  language_cobol.h \
  language_sql.c \
  language_sql.h \
  language_perl.c \
  language_perl.h \
  language_python.c \
  language_python.h \
  language_php.c \
  language_php.h \
  language_untitled.c \
  language_untitled.h \
  languages.h \
  edit_template.c \
  edit_template.h \
  filebrowser_backend.c \
  filebrowser_backend.h \
  filebrowser_ui.c \
  filebrowser_ui.h \
  find_dialog.c \
  find_dialog.h \
  gedit-close-button.c \
  gedit-close-button.h \
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
  gedit-history-entry.c \
  gedit-history-entry.h \
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
  gedit-status-combo-box.c \
  gedit-status-combo-box.h \
  search_infobar.c \
  search_infobar.h \
  symbolizable.c \
  symbolizable.h \
  symbol_bd_utils.c \
  symbol_bd_utils.h \
  symbol_bd_php.c \
  symbol_bd_php.h \
  symbol_bd_php_parse.c \
  symbol_bd_php_parse.h \
  symbol_bd_cobol.c \
  symbol_bd_cobol.h \
  symbol_bd_css.c \
  symbol_bd_css.h \
  symbol_bd_cxx.c \
  symbol_bd_cxx.h \
  symbol_bd_python.c \
  symbol_bd_python.h \
  symbol_bd_perl.c \
  symbol_bd_perl.h \
  symbol_bd_sql.c \
  symbol_bd_sql.h \
  symbol_manager.c \
  symbol_manager.h \
  syntax_check_window.c \
  syntax_check_window.h \
  tab.c \
  tab.h \
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

gphpedit-marshal.h: gphpedit-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) $(GLIB_GENMARSHAL) $< --header --prefix=gphpedit_marshal > $@

gphpedit-marshal.c: gphpedit-marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) echo "#include \"gphpedit-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=gphpedit_marshal >> $@

gtkscintilla2/libgtkscintilla2.a:
	cd gtkscintilla2 && $(MAKE) libgtkscintilla2.a && cd ..

gphpedit_LDADD = \
	$(WEBKIT_LIBS) $(GTK_LIBS) $(GLIB_LIBS) $(LIBXML_LIBS)\
	gtkscintilla2/libgtkscintilla2.a\
	gtkscintilla2/scintilla/bin/scintilla.a
