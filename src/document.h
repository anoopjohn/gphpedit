/* This file is part of gPHPEdit, a GNOME2 PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009, 2010 José Rostagno (for vijona.com.ar) 

   For more information or to find the latest release, visit our 
   website at http://www.gphpedit.org/

   gPHPEdit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   gPHPEdit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with gPHPEdit. If not, see <http://www.gnu.org/licenses/>.

   The GNU General Public License is contained in the file COPYING.
*/
#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <gtk/gtk.h>
#include <gtkscintilla.h>

#define DOCUMENT_TYPE document_get_type()
#define DOCUMENT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), DOCUMENT_TYPE, Document))
#define DOCUMENT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), DOCUMENT_TYPE, DocumentClass))
#define OBJECT_IS_DOCUMENT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DOCUMENT_TYPE))
#define OBJECT_IS_DOCUMENT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOCUMENT_TYPE))
#define DOCUMENT_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), DOCUMENT_TYPE, DocumentClass))

typedef struct DocumentDetails DocumentDetails;

typedef struct
{
	GObject object;
	DocumentDetails *details;
} Document;

typedef struct
{
	GObjectClass parent_class;

	void (* load_complete) (Document *doc, gboolean result, gpointer user_data);
	void (* save_update) (Document *doc, gpointer user_data); /* emited when document save state change*/
	void (* type_changed) (Document *doc, gint type, gpointer user_data);

} DocumentClass;

/* document types enum */

enum {
TAB_FILE,
TAB_PHP,
TAB_CSS,
TAB_SQL,
TAB_HELP,
TAB_CXX,
TAB_PERL,
TAB_PYTHON,
TAB_COBOL,
TAB_PREVIEW
};

/* Basic GObject requirements. */
GType document_get_type (void);
Document *document_new (gint type, const gchar *filename, gint goto_line);
void document_load(Document *document);
void document_save(Document *doc);
void document_reload(Document *document);
void document_refresh_properties(Document *doc);
gchar *document_get_filename(Document *doc) G_GNUC_WARN_UNUSED_RESULT G_GNUC_MALLOC;
const gchar *document_get_help_function(Document *doc);
void document_set_mtime(Document *doc, GTimeVal value);
void document_grab_focus(Document *doc);
gchar *document_get_current_selected_text(Document *doc) G_GNUC_WARN_UNUSED_RESULT G_GNUC_MALLOC;
gboolean document_is_scintilla_based(Document *doc);
void document_selection_to_lower(Document *doc);
void document_selection_to_upper(Document *doc);
void document_select_all(Document *doc);
void document_copy(Document *doc, GtkClipboard* clipboard);
void document_cut(Document *doc, GtkClipboard* clipboard);
void document_paste(Document *doc, GtkClipboard* clipboard);
void document_undo(Document *doc);
void document_redo(Document *doc);
void document_zoom_in(Document *doc);
void document_zoom_out(Document *doc);
void document_zoom_restore(Document *doc);
void document_block_indent(Document *doc, gint indentation_size);
void document_block_unindent(Document *doc, gint indentation_size);
void document_marker_modify(Document *doc, gint line);
void document_modify_current_line_marker(Document *doc);
void document_find_next_marker(Document *doc);
void set_document_to_type(Document *document, gint type);
void tab_check_type_file(Document *document);
GtkScintilla *document_get_scintilla(Document *document);
void document_insert_text(Document *doc, gchar *data);
void document_replace_current_selection(Document *doc, gchar *data);
gchar *document_get_title(Document *doc);
gchar *document_get_text(Document *doc);
void document_clear_sintax_style(Document *doc);
void document_set_sintax_indicator(Document *doc);
void document_set_sintax_annotation(Document *doc);
void document_set_sintax_line(Document *doc, guint current_line_number);
void document_add_sintax_annotation(Document *doc, guint current_line_number, gchar *token, gint style);
void document_goto_pos(Document *doc, glong pos);
void document_goto_line(Document *doc, gint line);
void document_scroll_to_current_pos(Document *document);
gboolean document_search_text(Document *doc, const gchar *text, gboolean checkwholedoc, gboolean checkcase, gboolean checkwholeword, gboolean checkregex);
gboolean document_search_replace_text(Document *doc, const gchar *text, const gchar *replace, gboolean checkwholedoc, gboolean checkcase, gboolean checkwholeword, gboolean checkregex, gboolean ask_replace);
gboolean document_check_externally_modified(Document *doc);
void document_update_modified_mark(Document *doc);
void document_keyboard_macro_startstop(Document *document);
void document_keyboard_macro_playback(Document *document);
gchar *document_get_current_word(Document *doc);
gint document_get_current_position(Document *doc);
void document_show_calltip_at_current_pos(Document *document);
void document_add_text(Document *document, const gchar *text);
void document_force_autocomplete(Document *document);
void document_insert_template(Document *document, gchar *template);
void document_incremental_search(Document *document, gchar *current_text, gboolean advancing);
void check_externally_modified(Document *doc);
#endif /* DOCUMENT_H */

