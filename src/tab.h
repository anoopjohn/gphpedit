/* This file is part of gPHPEdit, a GNOME2 PHP Editor.
 
   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
	  
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

#ifndef TAB_H
#define TAB_H

#include <sys/types.h>
#include <fcntl.h>
#include <gio/gio.h>
#include "main.h"
#include "classbrowser.h"
#include <webkit/webkit.h> 

#define TAB_FILE 1
#define TAB_PHP 2
#define TAB_CSS 3
#define TAB_SQL 4
#define TAB_HELP 5
#define TAB_CXX 6
#define TAB_PERL 7
#define TAB_PYTHON 8
#define TAB_COBOL 9
#define TAB_PREVIEW 10

/* annotations styles */
#define STYLE_ANNOTATION_ERROR 200
#define STYLE_ANNOTATION_WARNING 201
typedef struct
{
	gint type;
	GSList components;
	GtkWidget *scintilla;
	GtkWidget *help_scrolled_window;
	WebKitWebView *help_view;
	gint scintilla_id;
	GTimeVal file_mtime;
	GString *filename;
	gchar *short_filename;
	gboolean isreadonly;
	GdkPixbuf *file_icon;
	gchar *help_function;
	GString *opened_from;
	gint last_parsed_time; // TODO: Change to something more meaningful
	GSList functions_and_classes;
	gboolean saved;
	GtkWidget *label;
	GSList *keyboard_macro_list;
	gboolean is_macro_recording;
	gboolean is_pasting;
	gboolean converted_to_utf8;
	gboolean is_untitled;
	guint current_pos;
	guint current_line;
	guint file_size;
        const char *contenttype;
} Editor;

typedef struct
{
	gint message;
	gulong wparam;
	glong lparam;
} MacroEvent;

extern GtkWidget *label;
extern GSList *editors;

Editor *editor_find_from_scintilla(GtkWidget *scintilla);
Editor *editor_find_from_help(void *help);

gboolean tab_create_new(gint type, GString *filename);
void tab_check_php_file(Editor *editor);
void tab_check_css_file(Editor *editor);
void tab_check_cxx_file(Editor *editor);
void tab_check_perl_file(Editor *editor);
void tab_check_cobol_file(Editor *editor);
void tab_check_python_file(Editor *editor);
void tab_check_sql_file(Editor *editor);
void tab_load_file(Editor *editor);
gboolean is_php_file(Editor *editor);
gboolean is_php_file_from_filename(const gchar *filename);
gboolean switch_to_file_or_open(gchar *filename, gint line_number);
void tab_set_configured_scintilla_properties(GtkScintilla *scintilla, Preferences prefs);
GtkWidget *get_close_tab_widget(Editor *editor);
// Probably don't need all of these declared in the .h file, but I'll remove the unnecessary ones later - AJ
void fold_clicked(GtkWidget *scintilla, guint lineClick,guint bstate);
void fold_expand(GtkWidget *scintilla, gint line, gboolean doExpand, gboolean force, gint visLevels, gint level);
void fold_changed(GtkWidget *scintilla, int line,int levelNow,int levelPrev);
void handle_modified(GtkWidget *scintilla, gint pos,gint mtype,gchar *text,gint len,
				   gint added,gint line,gint foldNow,gint foldPrev);
void margin_clicked(GtkWidget *scintilla, gint position, gint modifiers, gint margin);
void macro_record (GtkWidget *scintilla, gint message, gulong wparam, glong lparam);
void keyboard_macro_empty_old(Editor *editor);
void update_ui(GtkWidget *scintilla);
gboolean auto_memberfunc_complete_callback(gpointer data);
void info_dialog (gchar *title, gchar *message);
void debug_dump_editors(void);
void register_file_opened(gchar *filename);
gchar * editor_convert_to_local(Editor *editor);
gboolean editor_is_local(Editor *editor);
gboolean uri_is_local_or_http(gchar *uri);
gchar *convert_to_full(gchar *filename);
void str_replace(char *Str, char ToRp, char WithC);
void tab_file_save_opened(Editor *editor,GFile *file);
char *macro_message_to_string(gint message);
gint yes_no_dialog (gchar *title, gchar *message);
void set_editor_to_php(Editor *editor);
void set_editor_to_css(Editor *editor);
void set_editor_to_sql(Editor *editor);
void set_editor_to_cxx(Editor *editor);
void set_editor_to_perl(Editor *editor);
void set_editor_to_cobol(Editor *editor);
void set_editor_to_python(Editor *editor);
#endif
