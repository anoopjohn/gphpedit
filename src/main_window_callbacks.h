/* This file is part of gPHPEdit, a GNOME2 PHP Editor.
 
   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2010 José Rostagno
	  
   For more information or to find the latest release, visit our 
   website at http://www.gphpedit.org/
 
   gPHPEdit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   gPHPEdit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with gPHPEdit.  If not, see <http://www.gnu.org/licenses/>.
 
   The GNU General Public License is contained in the file COPYING.
*/

#ifndef MAIN_WINDOW_CALLBACKS_H
#define MAIN_WINDOW_CALLBACKS_H
#include "main.h"
#include "main_window.h"

void update_status_combobox(Document *document);
void main_window_destroy_event(GtkWidget *widget, gpointer data);
gboolean main_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
gint main_window_key_press_event(GtkWidget   *widget, GdkEventKey *event,gpointer user_data);
void on_new1_activate(GtkWidget *widget);
void open_file_ok(GtkFileChooser *file_selection);
void reopen_recent(GtkRecentChooser *chooser, gpointer data);
void on_open1_activate(GtkWidget *widget);
void on_openselected1_activate(GtkWidget *widget);
void on_save1_activate(GtkWidget *widget);
void on_save_as1_activate(GtkWidget *widget);
void on_saveall1_activate(GtkWidget *widget);
void close_page(Document *document);
void on_close1_activate(GtkWidget *widget);
void on_quit1_activate(GtkWidget *widget);
void on_cut1_activate(GtkWidget *widget);
void on_copy1_activate(GtkWidget *widget);
void on_paste1_activate(GtkWidget *widget);
void on_selectall1_activate(GtkWidget *widget);
void on_find1_activate(GtkWidget *widget);
void on_replace1_activate(GtkWidget *widget);
void on_properties1_activate(GtkWidget *widget);
void on_undo1_activate(GtkWidget *widget);
void on_redo1_activate(GtkWidget *widget);
void on_preferences1_activate(GtkWidget *widget);
void context_help(GtkWidget *widget);
void on_about1_activate(GtkWidget *widget);
void force_php(GtkWidget *widget);
void force_css(GtkWidget *widget);
void force_sql(GtkWidget *widget);
void force_cxx(GtkWidget *widget);
void force_perl(GtkWidget *widget);
void force_cobol(GtkWidget *widget);
void force_python(GtkWidget *widget);
void zoom_in(GtkWidget *widget);
void zoom_out(GtkWidget *widget);
void zoom_100(GtkWidget *widget);
void update_zoom_level(void);
void can_undo(GtkWidget *widget);
void on_notebook_switch_page (GtkNotebook *notebook, GtkNotebookPage *page, gint page_num, gpointer user_data);
void block_indent(GtkWidget *widget);
void block_unindent(GtkWidget *widget);
void syntax_check(GtkWidget *widget);
void syntax_check_clear(GtkWidget *widget);
void goto_line_int(gint line);
void goto_line(gchar *text);
void selectiontoupper(void);
void selectiontolower(void);
gboolean on_notebook_focus_tab(GtkNotebook *notebook, GtkNotebookTab arg1, gpointer user_data);
void on_reload1_activate(GtkWidget *widget);
void keyboard_macro_startstop(GtkWidget *widget);
void keyboard_macro_playback(GtkWidget *widget);
void classbrowser_show(void);
void classbrowser_hide(void);
void classbrowser_show_hide(GtkWidget *widget);
void close_all_tabs(void);
void on_tab_close_activate(GtkWidget *widget, Document *document);
gboolean main_window_activate_focus (GtkWidget *widget,GdkEventFocus *event,gpointer       user_data);
void document_manager_new_document_cb (DocumentManager *docmg, Document *doc, gpointer user_data);
void document_manager_change_document_cb (DocumentManager *docmg, Document *doc, gpointer user_data);
#endif
