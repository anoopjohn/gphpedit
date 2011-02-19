/* This file is part of gPHPEdit, a GNOME PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009, 2011 José Rostagno (for vijona.com.ar) 

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

#include <gdk/gdkkeysyms.h>
#include "debug.h"
#include "document.h"
#include "document_scintilla.h"
#include "document_saver.h"
#include "main_window.h"
#include "main_window_callbacks.h"
#include "gvfs_utils.h"
#include "gphpedit-statusbar.h"
#include "images.h"

/* lexer headers */
#include "tab_cobol.h"
#include "tab_css.h"
#include "tab_cxx.h"
#include "tab_php.h"
#include "tab_perl.h"
#include "tab_python.h"
#include "tab_sql.h"

#include "language_provider.h"
#include "language_css.h"
#include "language_cxx.h"
#include "language_cobol.h"
#include "language_sql.h"
#include "language_perl.h"

#include "search_infobar.h"
#include "goto_infobar.h"

/* object signal enumeration */
enum {
  LOAD_COMPLETE,
  SAVE_UPDATE,
  SAVE_START,
  TYPE_CHANGED,
  NEED_RELOAD,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

/*
* document_scintilla private struct
*/
struct Document_ScintillaDetails
{
  gint type;
  GtkWidget *scintilla;
  GtkWidget *label;
  GSList *keyboard_macro_list;
  gboolean is_macro_recording;
  gboolean is_pasting;
  gboolean converted_to_utf8;
  guint current_pos;
  guint current_line;
  gchar *contenttype;

  /* external modified check widget */
  GtkWidget *container;
  GtkWidget *infobar;
  GtkWidget *infolabel;
  /* incremental search widget */
  GtkWidget *searchbar;
  GtkWidget *gotobar;
  /* calltip stuff*/
//  guint calltip_timer_id;
//  gboolean calltip_timer_set;
  /*completion stuff*/
//  guint completion_timer_id;
//  gboolean completion_timer_set;

  DocumentSaver *saver;

  Language_Provider *lgcss;
};

typedef struct
{
	gint message;
	gulong wparam;
	glong lparam;
} MacroEvent;

#define DOCUMENT_SCINTILLA_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    DOCUMENT_SCINTILLA_TYPE,\
					    Document_ScintillaDetails))

static void document_scintilla_dispose (GObject *gobject);
static void char_added(GtkWidget *scintilla, guint ch, gpointer user_data);
static void process_drag_uri(GtkWidget *scintilla, gpointer data);
static void tab_set_general_scintilla_properties(Document_Scintilla *doc);
static void save_point_reached(GtkWidget *scintilla, gpointer user_data);
static void save_point_left(GtkWidget *scintilla, gpointer user_data);
static void update_ui(GtkWidget *scintilla);
static void tab_set_configured_scintilla_properties(GtkScintilla *scintilla);
static void margin_clicked (GtkWidget *scintilla, gint modifiers, gint position, gint margin, gpointer user_data);
static void fold_clicked(GtkWidget *scintilla, guint lineClick,guint bstate);
static void fold_expand(GtkWidget *scintilla, gint line, gboolean doExpand, gboolean force, gint visLevels, gint level);
static void fold_changed(GtkWidget *scintilla, int line,int levelNow,int levelPrev);
static void handle_modified(GtkWidget *scintilla, gint pos,gint mtype,gchar *text,gint len, gint added,gint line,gint foldNow,gint foldPrev);
static void macro_record (GtkWidget *scintilla, gint message, gulong wparam, glong lparam, gpointer user_data);
static void process_user_list_selection (GtkWidget *w, gint type, gchar *text, gpointer user_data);
static void scintilla_modified (GtkWidget *w);
static void document_scintilla_constructed (GObject *object);
void document_scintilla_saver_done_saving_cb (DocumentSaver *docsav, Document_Scintilla *document_scintilla, gpointer user_data);
static void document_scintilla_find_next_marker(Document_Scintilla *doc);
static void document_scintilla_marker_modify(Document_Scintilla *doc, gint line);
static void document_scintilla_modify_current_line_marker(Document_Scintilla *doc);
static void document_scintilla_show_calltip_at_current_pos(Document_Scintilla *document_scintilla);
static void document_scintilla_force_autocomplete(Document_Scintilla *document_scintilla);
static void tab_set_folding(Document_Scintilla *document_scintilla, gint folding);
static void document_scintilla_auto_detect_file_type(Documentable *doc);
static gchar *document_scintilla_get_title(Document_Scintilla *doc);

/*
 * Documentable iface stuff
*/
static void document_scintilla_documentable_init(DocumentableIface *iface, gpointer user_data);

G_DEFINE_TYPE_WITH_CODE(Document_Scintilla, document_scintilla, DOCUMENT_TYPE,
                        G_IMPLEMENT_INTERFACE (IFACE_TYPE_DOCUMENTABLE,
                                                 document_scintilla_documentable_init));

static void document_scintilla_zoom_in(Documentable *doc)
{
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(DOCUMENT_SCINTILLA(doc));
  gtk_scintilla_zoom_in(GTK_SCINTILLA(docdet->scintilla));
}

static void document_scintilla_zoom_out(Documentable *doc){
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gtk_scintilla_zoom_out(GTK_SCINTILLA(docdet->scintilla));
}

static void document_scintilla_zoom_restore(Documentable *doc){
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gtk_scintilla_set_zoom(GTK_SCINTILLA(docdet->scintilla), 0);
}

static void document_scintilla_undo(Documentable *doc){
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gtk_scintilla_undo(GTK_SCINTILLA(docdet->scintilla));
}

static void document_scintilla_redo(Documentable *doc){
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gtk_scintilla_redo(GTK_SCINTILLA(docdet->scintilla));
}

static void document_scintilla_select_all(Documentable *doc){
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gtk_scintilla_select_all(GTK_SCINTILLA(docdet->scintilla));
}

static void document_scintilla_selection_to_lower(Documentable *doc){
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gchar *buffer = documentable_get_current_selected_text(DOCUMENTABLE(doc));
  if (buffer) {
      /* buffer to lower */
      gchar *tmpbuffer=g_utf8_strdown (buffer,strlen(buffer));
      g_free(buffer);
      /* replace sel */
      gtk_scintilla_replace_sel(GTK_SCINTILLA(docdet->scintilla), tmpbuffer);
      g_free(tmpbuffer);
    }
}

static void document_scintilla_selection_to_upper(Documentable *doc)
{
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gchar *buffer = documentable_get_current_selected_text(DOCUMENTABLE(doc));
  if (buffer) {
      /* buffer to lower */
      gchar *tmpbuffer=g_utf8_strup (buffer,strlen(buffer));
      g_free(buffer);
      /* replace sel */
      gtk_scintilla_replace_sel(GTK_SCINTILLA(docdet->scintilla), tmpbuffer);
      g_free(tmpbuffer);
    }
}

static void document_scintilla_copy(Documentable *doc)
{
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gtk_scintilla_copy(GTK_SCINTILLA(docdet->scintilla));
  /* As copy doesn't change the buffer it doesn't get recorded, so do it manually */
  macro_record (docdet->scintilla, 2178, 0, 0, doc); 
}

static void document_scintilla_cut(Documentable *doc)
{
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gtk_scintilla_cut(GTK_SCINTILLA(docdet->scintilla));
}

static void on_paste_got_from_cliboard(GtkClipboard *clipboard, const gchar *text, gpointer data)
{
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(data);
  docdet->is_pasting = docdet->is_macro_recording;
  gtk_scintilla_replace_sel(GTK_SCINTILLA(docdet->scintilla), text);
  docdet->is_pasting = FALSE;
  if (docdet->type==TAB_FILE){
  /* if we type <?php then we are in a php file so force php syntax mode */
    if (strstr(text,"<?php")){
      documentable_set_type(DOCUMENTABLE(data), TAB_PHP);
      }
  }
  // Possible fix for rendering issues after pasting
  gtk_scintilla_colourise(GTK_SCINTILLA(docdet->scintilla), 0, -1);
}

static void document_scintilla_paste(Documentable *doc)
{
  g_return_if_fail(doc);
  GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  gtk_clipboard_request_text(clipboard, on_paste_got_from_cliboard, doc);
}

static void move_block(GtkScintilla *scintilla, gint indentation_size)
{
  gint startline;
  gint endline;
  gint line;
  gint indent;

  startline = gtk_scintilla_line_from_position(scintilla, gtk_scintilla_get_selection_start(scintilla));
  endline = gtk_scintilla_line_from_position(scintilla, gtk_scintilla_get_selection_end(scintilla));

  gtk_scintilla_begin_undo_action(scintilla);
  for (line = startline; line <= endline; line++) {
    indent = gtk_scintilla_get_line_indentation(scintilla, line);
    gtk_scintilla_set_line_indentation(scintilla, line, indent+indentation_size);
  }
  gtk_scintilla_end_undo_action(scintilla);
}

static void document_scintilla_block_indent(Documentable *doc){
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gint indent_size;
  g_object_get(main_window.prefmg, "indentation_size", &indent_size, NULL);
  move_block(GTK_SCINTILLA(docdet->scintilla), indent_size);
}

static void document_scintilla_block_unindent(Documentable *doc){
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gint indent_size;
  g_object_get(main_window.prefmg, "indentation_size", &indent_size, NULL);
  move_block(GTK_SCINTILLA(docdet->scintilla), 0-indent_size);
}

static gchar *document_scintilla_get_filename (Documentable  *doc) {
  if (!doc) return NULL;
  gboolean untitled;
  GFile *file = NULL;
  g_object_get(doc,"untitled", &untitled, "GFile", &file, NULL);
  if (untitled || !file) return g_strdup(_("Untitled"));
  return g_file_get_uri (file);
}

static void document_scintilla_set_type (Documentable  *doc, gint type)
{
  Document_Scintilla *document_scintilla = DOCUMENT_SCINTILLA(doc);
  if (!document_scintilla) return ;
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(document_scintilla);
  if (docdet->type != type) {
      docdet->type = type;
      switch (type) {
        case TAB_PHP:
//          if (docdet->lgcss) g_object_unref(docdet->lgcss);
//          docdet->lgcss = LANGUAGE_PROVIDER(language_php_new (DOCUMENT_SCINTILLA(doc)));
          tab_php_set_lexer(GTK_SCINTILLA(docdet->scintilla));
          tab_set_folding(document_scintilla, TRUE);
          break;
        case TAB_CSS:
          if (docdet->lgcss) g_object_unref(docdet->lgcss);
          docdet->lgcss = LANGUAGE_PROVIDER(language_css_new (DOCUMENT_SCINTILLA(doc)));
          tab_css_set_lexer(GTK_SCINTILLA(docdet->scintilla));
          tab_set_folding(document_scintilla, TRUE);
          break;
        case TAB_COBOL:
          if (docdet->lgcss) g_object_unref(docdet->lgcss);
          docdet->lgcss = LANGUAGE_PROVIDER(language_cobol_new (DOCUMENT_SCINTILLA(doc)));
          tab_cobol_set_lexer(GTK_SCINTILLA(docdet->scintilla));
          tab_set_folding(document_scintilla, TRUE);
          break;
        case TAB_CXX:
          if (docdet->lgcss) g_object_unref(docdet->lgcss);
          docdet->lgcss = LANGUAGE_PROVIDER(language_cxx_new (DOCUMENT_SCINTILLA(doc)));
          tab_cxx_set_lexer(GTK_SCINTILLA(docdet->scintilla));
          tab_set_folding(document_scintilla, TRUE);
          break;
        case TAB_PYTHON:
//          if (docdet->lgcss) g_object_unref(docdet->lgcss);
//          docdet->lgcss = LANGUAGE_PROVIDER(language_python_new (DOCUMENT_SCINTILLA(doc)));
          tab_python_set_lexer(GTK_SCINTILLA(docdet->scintilla));
          tab_set_folding(document_scintilla, TRUE);
          break;
        case TAB_SQL:
          if (docdet->lgcss) g_object_unref(docdet->lgcss);
          docdet->lgcss = LANGUAGE_PROVIDER(language_sql_new (DOCUMENT_SCINTILLA(doc)));
          tab_sql_set_lexer(GTK_SCINTILLA(docdet->scintilla));
          tab_set_folding(document_scintilla, TRUE);
          break;
        case TAB_PERL:
          if (docdet->lgcss) g_object_unref(docdet->lgcss);
          docdet->lgcss = LANGUAGE_PROVIDER(language_perl_new (DOCUMENT_SCINTILLA(doc)));
          tab_perl_set_lexer(GTK_SCINTILLA(docdet->scintilla));
          tab_set_folding(document_scintilla, TRUE);
          break;
        case TAB_FILE:
          /* SCLEX_NULL to select no lexing action */
          gtk_scintilla_set_lexer(GTK_SCINTILLA (docdet->scintilla), SCLEX_NULL); 
          tab_set_configured_scintilla_properties(GTK_SCINTILLA (docdet->scintilla));
          gtk_scintilla_colourise(GTK_SCINTILLA (docdet->scintilla), 0, -1);
          tab_set_folding(document_scintilla, FALSE);
          break;
        default:
          break;
      }
      g_signal_emit (G_OBJECT (document_scintilla), signals[TYPE_CHANGED], 0, type);
  }
}

static void document_scintilla_goto_pos(Documentable *doc, glong pos)
{
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gtk_scintilla_goto_pos(GTK_SCINTILLA(docdet->scintilla), pos);
}

static void document_scintilla_goto_line(Documentable *doc, gint line)
{
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gtk_scintilla_grab_focus(GTK_SCINTILLA(docdet->scintilla));
  gtk_scintilla_goto_line(GTK_SCINTILLA(docdet->scintilla), line + gtk_scintilla_lines_on_screen(GTK_SCINTILLA(docdet->scintilla))); //move some lines to center in screen
  gtk_scintilla_goto_line(GTK_SCINTILLA(docdet->scintilla), line-1); // seems to be off by one...
}

static void document_scintilla_scroll_to_current_pos (Documentable  *document_scintilla)
{
  g_return_if_fail(document_scintilla);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(document_scintilla);
  GtkScintilla *sci = GTK_SCINTILLA(docdet->scintilla);
  gtk_scintilla_grab_focus(sci);
  gtk_scintilla_scroll_caret(sci);
  gtk_scintilla_grab_focus(sci);
}

static gchar *document_scintilla_get_current_selected_text (Documentable  *doc)
{
  if (!doc) return NULL;
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  return gtk_scintilla_get_current_selected_text(GTK_SCINTILLA(docdet->scintilla));
}

static gchar *document_scintilla_get_session_entry (Documentable  *doc)
{
  gphpedit_debug(DEBUG_DOCUMENT);
  gchar *result;
  Document_ScintillaDetails *docdet;
  gboolean untitled;
  gchar *docfilename;

  if (!doc) return NULL;
  docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  g_object_get(doc, "untitled", &untitled, NULL);
  if (untitled) return NULL;
  docfilename = documentable_get_filename(DOCUMENTABLE(doc));
  result = g_strdup_printf ("%s\n",docfilename);
  g_free(docfilename);
  return result;
}

static void document_scintilla_reload (Documentable *document_scintilla)
{
  gphpedit_debug (DEBUG_DOCUMENT);
  g_return_if_fail(document_scintilla);
  g_signal_emit (G_OBJECT (document_scintilla), signals[NEED_RELOAD], 0);
}

static gint document_scintilla_get_current_position(Documentable *doc)
{
  g_return_val_if_fail(doc, -1);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  return gtk_scintilla_get_current_pos(GTK_SCINTILLA(docdet->scintilla));
}

static gboolean document_scintilla_search_text (Documentable  *doc, const gchar *text, gboolean checkwholedoc, 
                                              gboolean checkcase, gboolean checkwholeword, gboolean checkregex)
{
  g_return_val_if_fail(doc, FALSE);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gint search_flags = 0;
  glong length_of_document_scintilla;
  glong current_pos;
  glong last_found = -1;
  glong start_found;
  glong end_found;
  glong result;

  length_of_document_scintilla = gtk_scintilla_get_length(GTK_SCINTILLA(docdet->scintilla));
  current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(docdet->scintilla));

  if (checkcase) {
    search_flags += SCFIND_MATCHCASE;
  }

  if (checkwholeword) {
    search_flags += SCFIND_WHOLEWORD;
  }

  if (checkregex) {
    search_flags += SCFIND_REGEXP;
  }
  result = gtk_scintilla_find_text (GTK_SCINTILLA(docdet->scintilla),
                                    search_flags, (gchar *) text, current_pos, length_of_document_scintilla, &start_found, &end_found);
  if (result == -1) {
    return FALSE;
  } else {
    if (start_found == last_found) return FALSE;
    last_found = start_found;
    gtk_scintilla_goto_pos(GTK_SCINTILLA(docdet->scintilla), start_found);
    gtk_scintilla_scroll_caret(GTK_SCINTILLA(docdet->scintilla));
    gtk_scintilla_set_selection_start(GTK_SCINTILLA(docdet->scintilla), start_found);
    gtk_scintilla_set_selection_end(GTK_SCINTILLA(docdet->scintilla), end_found);
  } 
  return TRUE;
}

static gboolean document_scintilla_search_replace_text(Documentable  *doc, const gchar *text, const gchar *replace, 
                gboolean checkwholedoc, gboolean checkcase, gboolean checkwholeword, gboolean checkregex, gboolean ask_replace)
{
  g_return_val_if_fail(doc, FALSE);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gint search_flags = 0;
  glong length_of_document_scintilla;
  glong current_pos;
  glong last_found = -1;
  glong start_found;
  glong end_found;
  glong result;

  length_of_document_scintilla = gtk_scintilla_get_length(GTK_SCINTILLA(docdet->scintilla));
  current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(docdet->scintilla));

  if (checkcase) {
    search_flags += SCFIND_MATCHCASE;
  }

  if (checkwholeword) {
    search_flags += SCFIND_WHOLEWORD;
  }

  if (checkregex) {
    search_flags += SCFIND_REGEXP;
  }
  result = gtk_scintilla_find_text (GTK_SCINTILLA(docdet->scintilla),
                                    search_flags, (gchar *) text, current_pos, length_of_document_scintilla, &start_found, &end_found);
  if (result == -1) {
    return FALSE;
  } else {
    if (start_found == last_found) return FALSE;
    last_found = start_found;
    gtk_scintilla_set_selection_start(GTK_SCINTILLA(docdet->scintilla), start_found);
    gtk_scintilla_set_selection_end(GTK_SCINTILLA(docdet->scintilla), end_found);
    if (ask_replace) {
      // Prompt for replace?
      GtkWidget *replace_prompt_dialog;
      replace_prompt_dialog = gtk_message_dialog_new(GTK_WINDOW(main_window.window), GTK_DIALOG_DESTROY_WITH_PARENT,
      GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,
            _("Do you want to replace this occurence?"));
      gtk_window_set_title(GTK_WINDOW(replace_prompt_dialog), _("Question"));
      gtk_window_set_transient_for (GTK_WINDOW(replace_prompt_dialog),GTK_WINDOW(main_window.window));
      gint result = gtk_dialog_run (GTK_DIALOG (replace_prompt_dialog));
      gint selection_start;
      if (result==GTK_RESPONSE_YES) {
        selection_start = gtk_scintilla_get_selection_start(GTK_SCINTILLA(docdet->scintilla));
        gtk_scintilla_replace_sel(GTK_SCINTILLA(docdet->scintilla), replace);
        gtk_scintilla_set_selection_start(GTK_SCINTILLA(docdet->scintilla), selection_start);
        gtk_scintilla_set_selection_end(GTK_SCINTILLA(docdet->scintilla), selection_start + strlen(replace));
        }
        gtk_widget_destroy(replace_prompt_dialog);
        last_found++;
    } else {
    gtk_scintilla_replace_sel(GTK_SCINTILLA(docdet->scintilla), replace);
    gtk_scintilla_set_selection_start(GTK_SCINTILLA(docdet->scintilla), start_found);
    gtk_scintilla_set_selection_end(GTK_SCINTILLA(docdet->scintilla), start_found + strlen(replace));
    }
  } 
  return TRUE;
}

static void document_scintilla_incremental_search (Documentable  *document_scintilla, gchar *current_text, gboolean advancing)
{
  g_return_if_fail(document_scintilla);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(document_scintilla);
  gint found_pos;
  glong text_min, text_max;
  glong current_pos = 0;
  if (advancing){
    current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(docdet->scintilla));
  }
  found_pos = gtk_scintilla_find_text(GTK_SCINTILLA(docdet->scintilla), 0, current_text, current_pos, gtk_scintilla_get_length(GTK_SCINTILLA(docdet->scintilla)), &text_min, &text_max);
  if (found_pos != -1) {
    gtk_scintilla_set_sel(GTK_SCINTILLA(docdet->scintilla), text_min, text_max);
  } else {
    if (advancing) {
      document_scintilla_incremental_search(document_scintilla, current_text, FALSE);
    }
  }
}

static void document_scintilla_activate_incremental_search(Documentable *doc)
{
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gtk_widget_show(docdet->searchbar);
  gtk_widget_grab_focus(docdet->searchbar);
}

/*
* document_scintilla_get_text
* Return a newly allocate string with current document_scintilla text. 
* must be free with g_free when no longer needed.
*/

static gchar *document_scintilla_get_text (Documentable  *doc)
{
  if (!doc) return NULL;
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  return gtk_scintilla_get_full_text (GTK_SCINTILLA(docdet->scintilla));
}

static gboolean document_scintilla_check_externally_modified_t(Document_Scintilla *doc)
{
  if (!doc) return FALSE;
  gboolean untitled;
  gint64 mtime;
  GFile *file = NULL;
  g_object_get(doc, "untitled", &untitled, NULL);
  if (untitled) return FALSE;
  g_object_get(doc, "GFile", &file, "mtime", &mtime, NULL);
  /* verify if file has been externally modified */
  return GFile_get_is_modified(file, &mtime, FALSE);
}

static void document_scintilla_check_externally_modified (Documentable  *doc)
{
  if(!doc) return ;
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  if (document_scintilla_check_externally_modified_t(DOCUMENT_SCINTILLA(doc))){
    gchar *filename = documentable_get_filename(DOCUMENTABLE(doc));
    gchar *path = filename_get_relative_path(filename);
    gchar *message= g_strdup_printf(_("<b>The file \"%s\" has been externally modified.</b>"), path);
    g_free(filename);
    g_free(path);
    gtk_label_set_markup (GTK_LABEL (docdet->infolabel), message);
    g_free(message);
    gtk_widget_show (docdet->infobar);
    return;
  } else {
    gtk_widget_hide (docdet->infobar);
  }
}

static void document_scintilla_save(Documentable *doc)
{
  gphpedit_debug (DEBUG_DOCUMENT);
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  g_signal_emit (G_OBJECT (doc), signals[SAVE_START], 0);
  document_saver_save_document(docdet->saver, DOCUMENT(doc));
}

static void document_scintilla_save_as(Documentable *doc, GFile *file)
{
  gphpedit_debug (DEBUG_DOCUMENT);
  g_return_if_fail(doc);
  g_object_set(doc, "GFile", file, NULL);
  gchar *basename = g_file_get_basename (file);
  g_object_set(doc, "untitled", FALSE, "short_filename", basename, NULL);
  /* Call Save method to actually save it now it has a filename */
  documentable_save(DOCUMENTABLE(doc));
}

static void tab_reset_scintilla_after_open(GtkScintilla *scintilla, guint current_line)
{
  gtk_scintilla_set_undo_collection(scintilla, FALSE);
  gtk_scintilla_empty_undo_buffer(scintilla);
  gtk_scintilla_set_save_point(scintilla);
  gtk_scintilla_goto_line(scintilla, current_line);
  gtk_scintilla_scroll_caret(scintilla);
  gtk_scintilla_grab_focus(scintilla);
  gtk_scintilla_set_undo_collection(scintilla, TRUE);
}

static void document_scintilla_replace_text (Documentable  *document_scintilla, gchar *new_text)
{
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(document_scintilla);
  gsize size;
  if (!new_text) size = 0;
  else size = strlen(new_text);
  if (size!=0){
    // Clear scintilla buffer
    gtk_scintilla_clear_all(GTK_SCINTILLA (docdet->scintilla));
    gtk_scintilla_add_text(GTK_SCINTILLA (docdet->scintilla), size, new_text);
  }
  tab_reset_scintilla_after_open(GTK_SCINTILLA (docdet->scintilla), 0);
  document_scintilla_auto_detect_file_type(DOCUMENTABLE(document_scintilla));
//  gtk_scintilla_set_save_point(GTK_SCINTILLA(docdet->scintilla));
}

static void document_scintilla_insert_text (Documentable  *doc, gchar *data)
{
  g_return_if_fail(doc && data);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gtk_scintilla_insert_text(GTK_SCINTILLA(docdet->scintilla), gtk_scintilla_get_current_pos(GTK_SCINTILLA(docdet->scintilla)), data);
}

static gchar *document_scintilla_get_current_word (Documentable  *doc)
{
  g_return_val_if_fail(doc, NULL);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gint wordStart;
  gint wordEnd;
  gchar *buffer = NULL;
  GtkScintilla *sci = GTK_SCINTILLA(docdet->scintilla);
  gint length;
  gint current_pos = gtk_scintilla_get_current_pos(sci);
  wordStart = gtk_scintilla_word_start_position(sci, current_pos-1, TRUE);
  wordEnd = gtk_scintilla_word_end_position(sci, current_pos-1, TRUE);
  buffer = gtk_scintilla_get_text_range (sci, wordStart, wordEnd, &length);
  return buffer;
}

static void document_scintilla_replace_current_selection (Documentable *doc, gchar *data)
{
  g_return_if_fail(doc && data);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gtk_scintilla_replace_sel(GTK_SCINTILLA(docdet->scintilla), data);
}

static void document_scintilla_apply_preferences (Documentable *doc)
{
  gphpedit_debug (DEBUG_DOCUMENT);
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  tab_set_configured_scintilla_properties(GTK_SCINTILLA(docdet->scintilla));
  gint type;
  /* reset type */
  g_object_get(doc, "type", &type, NULL);
  //FIXME: we set TAB_PHP style and then TAB_FILE style due to a bug, causing no style update.
  if (type==TAB_FILE) {
     documentable_set_type(doc, TAB_PHP);
  } else {
     documentable_set_type(doc, TAB_FILE);
  }
  documentable_set_type(doc, type);
}

static void document_scintilla_grab_focus (Documentable *doc)
{
  gphpedit_debug (DEBUG_DOCUMENT);
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gtk_widget_grab_focus(docdet->scintilla);
}

static void document_scintilla_documentable_init(DocumentableIface *iface, gpointer user_data)
{
  iface->zoom_in = document_scintilla_zoom_in;
  iface->zoom_out = document_scintilla_zoom_out;
  iface->zoom_restore = document_scintilla_zoom_restore;
  iface->undo = document_scintilla_undo;
  iface->redo = document_scintilla_redo;
  iface->select_all = document_scintilla_select_all;
  iface->selection_to_upper = document_scintilla_selection_to_upper;
  iface->selection_to_lower = document_scintilla_selection_to_lower;
  iface->copy = document_scintilla_copy;
  iface->cut = document_scintilla_cut;
  iface->paste = document_scintilla_paste;
  iface->block_indent = document_scintilla_block_indent;
  iface->block_unindent = document_scintilla_block_unindent;
  iface->get_filename = document_scintilla_get_filename;
  iface->set_type = document_scintilla_set_type;
  iface->goto_pos = document_scintilla_goto_pos;
  iface->goto_line = document_scintilla_goto_line;
  iface->scroll_to_current_pos = document_scintilla_scroll_to_current_pos;
  iface->get_current_selected_text = document_scintilla_get_current_selected_text;
  iface->get_session_entry = document_scintilla_get_session_entry;
  iface->reload = document_scintilla_reload;
  iface->get_current_position = document_scintilla_get_current_position;
  iface->search_text = document_scintilla_search_text;
  iface->search_replace_text = document_scintilla_search_replace_text;
  iface->incremental_search = document_scintilla_incremental_search;
  iface->activate_incremental_search = document_scintilla_activate_incremental_search;
  iface->get_text = document_scintilla_get_text;
  iface->check_externally_modified = document_scintilla_check_externally_modified;
  iface->save = document_scintilla_save;
  iface->save_as = document_scintilla_save_as;
  iface->replace_text = document_scintilla_replace_text;
  iface->get_current_word = document_scintilla_get_current_word;
  iface->insert_text = document_scintilla_insert_text;
  iface->replace_current_selection = document_scintilla_replace_current_selection;
  iface->apply_preferences = document_scintilla_apply_preferences;
  iface->grab_focus = document_scintilla_grab_focus;
}

enum
{
  PROP_0,
  PROP_CAN_MODIFY,
  PROP_CONVERTED_TO_UTF8,
  PROP_IS_EMPTY,
  PROP_SAVED,
  PROP_CAN_PREVIEW,
  PROP_ZOOM_LEVEL,
  PROP_TYPE,
  PROP_LABEL,
  PROP_WIDGET,
  PROP_TITLE,
  PROP_SCINTILLA
};

static void
document_scintilla_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(object);

  switch (prop_id)
  {
    case PROP_CAN_MODIFY:
      gtk_scintilla_set_read_only(GTK_SCINTILLA(docdet->scintilla), 
          !g_value_get_boolean (value));
      break;
    case PROP_CONVERTED_TO_UTF8:
      docdet->converted_to_utf8 = g_value_get_boolean (value);
      break;
    case PROP_TYPE:
      docdet->type = g_value_get_int (value);
      documentable_set_type(DOCUMENTABLE(object), docdet->type);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
document_scintilla_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(object);
  
  gboolean temp;
  gboolean untitled;
  gchar *content;
  gint p;
  switch (prop_id)
  {
   case PROP_CAN_MODIFY:
        g_value_set_boolean (value, !gtk_scintilla_get_read_only(GTK_SCINTILLA(docdet->scintilla)));
        break;
   case PROP_CONVERTED_TO_UTF8:
        g_value_set_boolean (value, docdet->converted_to_utf8);
        break;
   case PROP_TYPE:
         g_value_set_int (value, docdet->type);
         break;
     case PROP_IS_EMPTY:
         g_object_get(object, "untitled", &untitled, NULL);
         temp = (untitled && gtk_scintilla_get_text_length(GTK_SCINTILLA(docdet->scintilla))==0);
         g_value_set_boolean (value, temp);
         break;
    case PROP_SAVED:
      /* http://www.scintilla.org/ScintillaDoc.html#SCI_GETMODIFY */
       temp = !gtk_scintilla_get_modify(GTK_SCINTILLA(docdet->scintilla));
       g_value_set_boolean (value, temp);
       break;
    case PROP_CAN_PREVIEW:
	g_object_get(object,"content_type", &content, NULL);
        g_value_set_boolean (value, (g_strcmp0(content,"text/html")==0));
      break;
    case PROP_ZOOM_LEVEL:
      p= gtk_scintilla_get_zoom(GTK_SCINTILLA(docdet->scintilla));
      p= (p*10) + 100;
      g_value_set_uint (value, p);
      break;
    case PROP_TITLE:
      g_value_set_string (value, document_scintilla_get_title(DOCUMENT_SCINTILLA(object)));
      break;
    case PROP_LABEL:
      g_value_set_object (value, docdet->label);
      break;
    case PROP_WIDGET:
      g_value_set_object (value, docdet->container);
      break;
    case PROP_SCINTILLA:
      g_value_set_object (value, docdet->scintilla);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
document_scintilla_class_init (Document_ScintillaClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = document_scintilla_dispose;
  object_class->set_property = document_scintilla_set_property;
  object_class->get_property = document_scintilla_get_property;
  object_class->constructed = document_scintilla_constructed;

	signals[LOAD_COMPLETE] =
		g_signal_new ("load_complete",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (Document_ScintillaClass, load_complete),
		              NULL, NULL,
		               g_cclosure_marshal_VOID__BOOLEAN,
		               G_TYPE_NONE, 1, G_TYPE_BOOLEAN, NULL);

	signals[SAVE_UPDATE] =
		g_signal_new ("save_update",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (Document_ScintillaClass, save_update),
		              NULL, NULL,
		               g_cclosure_marshal_VOID__VOID,
		               G_TYPE_NONE, 0);

	signals[SAVE_START] =
		g_signal_new ("save_start",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (Document_ScintillaClass, save_update),
		              NULL, NULL,
		               g_cclosure_marshal_VOID__VOID,
		               G_TYPE_NONE, 0);

	signals[TYPE_CHANGED] =
		g_signal_new ("type_changed",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (Document_ScintillaClass, save_update),
		              NULL, NULL,
		               g_cclosure_marshal_VOID__INT,
		               G_TYPE_NONE, 1, G_TYPE_INT, NULL);

	signals[NEED_RELOAD] =
		g_signal_new ("need_reload",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (Document_ScintillaClass, save_update),
		              NULL, NULL,
		               g_cclosure_marshal_VOID__VOID,
		               G_TYPE_NONE, 0);

  /*DOCUMENT_SCINTILLA PROPERTIES*/
  /* CAN_MODIFY PROPERTY: When a document_scintilla can be modified */
  g_object_class_install_property (object_class,
                              PROP_CAN_MODIFY,
                              g_param_spec_boolean ("can_modify",
                              NULL, NULL,
                              TRUE, G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_CONVERTED_TO_UTF8,
                              g_param_spec_boolean ("converted_to_utf8",
                              NULL, NULL,
                              FALSE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
                              PROP_IS_EMPTY,
                              g_param_spec_boolean ("is_empty",
                              NULL, NULL,
                              FALSE, G_PARAM_READABLE));

  g_object_class_install_property (object_class,
                              PROP_SAVED,
                              g_param_spec_boolean ("saved",
                              NULL, NULL,
                              FALSE, G_PARAM_READABLE));

  g_object_class_install_property (object_class,
                              PROP_CAN_PREVIEW,
                              g_param_spec_boolean ("can_preview",
                              NULL, NULL,
                              FALSE, G_PARAM_READABLE));

  g_object_class_install_property (object_class,
                              PROP_ZOOM_LEVEL,
                              g_param_spec_uint ("zoom_level",
                              NULL, NULL, 0, G_MAXUINT, 
                              100, G_PARAM_READABLE));

  g_object_class_install_property (object_class,
                              PROP_TYPE,
                              g_param_spec_int ("type",
                              NULL, NULL, 0, G_MAXINT, 
                              TAB_FILE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
                              PROP_TITLE,
                              g_param_spec_string ("title",
                              NULL, NULL,
                              "", G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_LABEL,
                              g_param_spec_object ("editor_label",
                              NULL, NULL,
                              GTK_TYPE_WIDGET, G_PARAM_READABLE));

  g_object_class_install_property (object_class,
                              PROP_WIDGET,
                              g_param_spec_object ("editor_widget",
                              NULL, NULL,
                              GTK_TYPE_WIDGET, G_PARAM_READABLE));

  g_object_class_install_property (object_class,
                              PROP_SCINTILLA,
                              g_param_spec_object ("scintilla",
                              NULL, NULL,
                              GTK_TYPE_SCINTILLA, G_PARAM_READABLE));

  g_type_class_add_private (klass, sizeof (Document_ScintillaDetails));
}

static void process_external (GtkInfoBar *info_bar, gint response_id, Document_Scintilla *doc)
{
  if (response_id!=GTK_RESPONSE_CANCEL){
    g_signal_emit (G_OBJECT (doc), signals[NEED_RELOAD], 0);
  } else { 
  GTimeVal file_mtime;
  g_get_current_time (&file_mtime); /*set current time*/
  gint64 mtime = (((gint64) file_mtime.tv_sec) * G_USEC_PER_SEC) + file_mtime.tv_usec;
  g_object_set(doc, "mtime", mtime, NULL);
  }
  gtk_widget_hide (GTK_WIDGET(info_bar));
}

static void create_infobar(Document_Scintilla *doc) {
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  GtkWidget *image;
  GtkWidget *vbox;
  gchar *primary_markup;
  gchar *secondary_markup;
  GtkWidget *secondary_label; 

  /* set up info bar */
  docdet->infobar= gtk_info_bar_new_with_buttons(GTK_STOCK_REFRESH, 1, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
  gtk_info_bar_set_message_type (GTK_INFO_BAR(docdet->infobar), GTK_MESSAGE_WARNING);
  GtkWidget *content_area = gtk_info_bar_get_content_area(GTK_INFO_BAR (docdet->infobar));
  gtk_box_set_spacing(GTK_BOX (content_area), 0);

  image = gtk_image_new_from_stock ("gtk-dialog-warning", GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start (GTK_BOX (content_area), image, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (image), 0.5, 0);
  
  vbox = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (content_area), vbox, TRUE, TRUE, 0);
	
  primary_markup = g_strdup_printf (_("<b>The file \"%s\" has been externally modified.</b>"),".");
  docdet->infolabel = gtk_label_new (primary_markup);
  g_free (primary_markup);
  gtk_box_pack_start (GTK_BOX (vbox), docdet->infolabel, TRUE, TRUE, 0);
  gtk_label_set_use_markup (GTK_LABEL (docdet->infolabel), TRUE);
  gtk_label_set_line_wrap (GTK_LABEL (docdet->infolabel), TRUE);
  gtk_misc_set_alignment (GTK_MISC (docdet->infolabel), 0, 0.5);
  gtk_label_set_selectable (GTK_LABEL (docdet->infolabel), TRUE);

  secondary_markup = g_strdup_printf ("<small>%s</small>", _("Do you want reload it?"));
  secondary_label = gtk_label_new (secondary_markup);
  g_free (secondary_markup);
  gtk_box_pack_start (GTK_BOX (vbox), secondary_label, TRUE, TRUE, 0);
  gtk_widget_set_can_focus (secondary_label, TRUE);
  gtk_label_set_use_markup (GTK_LABEL (secondary_label), TRUE);
  gtk_label_set_line_wrap (GTK_LABEL (secondary_label), TRUE);
  gtk_label_set_selectable (GTK_LABEL (secondary_label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (secondary_label), 0, 0.5);
  gtk_widget_show_all (content_area);

  gtk_widget_show(docdet->infolabel);
  g_signal_connect(docdet->infobar, "response", G_CALLBACK (process_external), doc);
  gtk_box_pack_start(GTK_BOX(docdet->container), docdet->infobar, FALSE, FALSE, 0);
}

static gboolean scintilla_key_press (GtkWidget *widget, GdkEventKey *event, gpointer user_data) 
{
  if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && ((event->keyval == GDK_F2)))  {
    document_scintilla_modify_current_line_marker(user_data);
    return TRUE;
  }
  else if ((event->keyval == GDK_F2))  {
      document_scintilla_find_next_marker(user_data);
      return TRUE;
  }  
  else if (((event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK))==(GDK_CONTROL_MASK | GDK_SHIFT_MASK)) && (event->keyval == GDK_space)) {
    document_scintilla_show_calltip_at_current_pos(user_data);
    return TRUE;
  }
  else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && ((event->keyval == GDK_j) || (event->keyval == GDK_J)))  {
    template_find_and_insert(main_window.tempmg, user_data);
    return TRUE;
  }
  else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && (event->keyval == GDK_space)) { 
    document_scintilla_force_autocomplete(user_data);
    return TRUE;
  }
  return FALSE;
}

static void
document_scintilla_init (Document_Scintilla * object)
{
  Document_ScintillaDetails *docdet;
  docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(object);
  docdet->saver = document_saver_new ();
  g_signal_connect(G_OBJECT(docdet->saver), "done_saving", G_CALLBACK(document_scintilla_saver_done_saving_cb), NULL);

  docdet->container = gtk_vbox_new (FALSE, 0);
  create_infobar(object);

  /* create incremental search widget */
  docdet->searchbar = search_infobar_new();
  gtk_box_pack_start(GTK_BOX(docdet->container), docdet->searchbar, FALSE, FALSE, 0);

  docdet->gotobar = goto_infobar_new();
  gtk_box_pack_start(GTK_BOX(docdet->container), docdet->gotobar, FALSE, FALSE, 0);

  docdet->scintilla = gtk_scintilla_new();
  g_signal_connect (G_OBJECT (docdet->scintilla), "char_added", G_CALLBACK (char_added), object);
  g_signal_connect (G_OBJECT (docdet->scintilla), "update_ui", G_CALLBACK (update_ui), NULL);
  g_signal_connect (G_OBJECT (docdet->scintilla), "uri_dropped", G_CALLBACK (process_drag_uri), NULL);
  g_signal_connect (G_OBJECT (docdet->scintilla), "user_list_selection", G_CALLBACK (process_user_list_selection), object);
  g_signal_connect (G_OBJECT (docdet->scintilla), "painted", G_CALLBACK (scintilla_modified), NULL);
  g_signal_connect (G_OBJECT (docdet->scintilla), "key-press-event", G_CALLBACK (scintilla_key_press), object);
  g_signal_connect (G_OBJECT (docdet->scintilla), "save_point_reached", G_CALLBACK (save_point_reached), object);
  g_signal_connect (G_OBJECT (docdet->scintilla), "save_point_left", G_CALLBACK (save_point_left), object);
  g_signal_connect (G_OBJECT (docdet->scintilla), "macro_record", G_CALLBACK (macro_record), object);

  gtk_box_pack_end(GTK_BOX(docdet->container), docdet->scintilla, TRUE, TRUE, 0);
  tab_set_general_scintilla_properties(object);
  docdet->label = gtk_label_new ("");
  gtk_widget_show (docdet->label);
  gtk_widget_show (docdet->scintilla);
}

/*
* disposes the Gobject
*/
static void document_scintilla_dispose (GObject *object)
{
  Document_Scintilla *doc = DOCUMENT_SCINTILLA(object);
  Document_ScintillaDetails *docdet;
  docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  /* free object resources*/
  g_object_unref(docdet->saver);
  /* Chain up to the parent class */
  G_OBJECT_CLASS (document_scintilla_parent_class)->dispose (object);
}

static void document_scintilla_add_recent(Document_Scintilla *document_scintilla)
{
  g_return_if_fail(document_scintilla);
  gchar *full_filename = documentable_get_filename(DOCUMENTABLE(document_scintilla));
  GtkRecentManager *manager;
  manager = gtk_recent_manager_get_default ();
  gtk_recent_manager_add_item (manager, full_filename);
  g_free(full_filename);
}

static void document_scintilla_constructed (GObject *object)
{
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(object);
  gchar *short_filename;
  gboolean untitled;
  GFile *file;
  g_object_get(object, "untitled", &untitled, NULL);
  if (untitled){
    short_filename = g_strdup(_("Untitled"));
  } else {
    g_object_get(object, "GFile", &file, NULL);
    short_filename = g_file_get_basename (file);
    document_scintilla_add_recent(DOCUMENT_SCINTILLA(object));
  }
  g_object_set(object, "short_filename", short_filename, NULL);
  gtk_label_set_text(GTK_LABEL(docdet->label), short_filename);
  gtk_widget_show (docdet->label);

}

Document_Scintilla *document_scintilla_new (gint type, GFile *file, gint goto_line, gchar *contents)
{
  Document_Scintilla *doc;
  doc = g_object_new (DOCUMENT_SCINTILLA_TYPE, "type", type, "GFile", file, "untitled", !file, NULL);
  documentable_replace_text(DOCUMENTABLE(doc), contents);

  return doc; /* return new object */
}

static void register_autoc_images(GtkScintilla *sci) {
  gtk_scintilla_register_image(sci, 1, (const gchar *) function_xpm);
  gtk_scintilla_register_image(sci, 2, (const gchar *) bullet_blue_xpm);
  gtk_scintilla_register_image(sci, 3, (const gchar *) variable_xpm);
  gtk_scintilla_register_image(sci, 4, (const gchar *) class_xpm);
}

static void tab_set_general_scintilla_properties(Document_Scintilla *doc)
{
  Document_ScintillaDetails *docdet;
  docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  GtkScintilla *scintilla = GTK_SCINTILLA(docdet->scintilla);
  gtk_scintilla_set_backspace_unindents(scintilla, 1);
  gtk_scintilla_autoc_set_choose_single(scintilla, FALSE);
  gtk_scintilla_autoc_set_ignore_case(scintilla, TRUE);
  register_autoc_images(scintilla);
  gtk_scintilla_autoc_set_drop_rest_of_word(scintilla, FALSE);
  gtk_scintilla_set_scroll_width_tracking(scintilla, TRUE);
  gtk_scintilla_set_code_page(scintilla, SC_CP_UTF8);

  gint width;
  width = gtk_scintilla_text_width(scintilla, STYLE_LINENUMBER, "_99999");
  gtk_scintilla_set_margin_width_n(scintilla, 0, width);
  gtk_scintilla_set_margin_width_n (scintilla, 2, 0);
  //makers margin settings
  gtk_scintilla_set_margin_type_n(scintilla, 1, SC_MARGIN_SYMBOL);
  gtk_scintilla_set_margin_width_n (scintilla, 1, 14);
  gtk_scintilla_set_margin_sensitive_n(scintilla, 1, 1);
  g_signal_connect (G_OBJECT (docdet->scintilla), "margin_click", G_CALLBACK (margin_clicked), doc);
  
  tab_set_configured_scintilla_properties(scintilla);
  gtk_widget_show (docdet->scintilla);
}

static void tab_set_configured_scintilla_properties(GtkScintilla *scintilla)
{
  PreferencesManager *pref = preferences_manager_new ();
  gboolean edge_mode, show_indent_guides, higthlight_caret_line, line_wrapping, tabs_instead_spaces;
  gint edge_column, font_quality;
  gchar *font;
  g_object_get(pref, "edge_mode", &edge_mode,"edge_column", &edge_column, "show_indentation_guides", &show_indent_guides,
       "higthlight_caret_line", &higthlight_caret_line, "line_wrapping",&line_wrapping,
        "tabs_instead_spaces", &tabs_instead_spaces,"font_quality", &font_quality, "style_font_name", &font, NULL);
  
  gtk_scintilla_set_wrap_mode(scintilla, line_wrapping);
  gtk_scintilla_set_h_scroll_bar(scintilla, !line_wrapping); /* if line wrapping is ON disable hscrollbar*/
  /* reset styles */
  gtk_scintilla_style_clear_all(scintilla);

  guint size;
  gchar *style_name;
  gint indentation_size, tab_size;
  g_object_get(pref, "style_name", &style_name, "indentation_size", &indentation_size, 
    "font_size", &size, "tab_size", &tab_size, NULL);
  GtkSourceStyleScheme	*scheme = gtk_source_style_scheme_manager_get_scheme (main_window.stylemg, style_name);
  g_free(style_name);
  gtk_source_style_scheme_apply (scheme, GTK_WIDGET(scintilla), font, size);
  g_free(font);
  /* set font quality */
  gtk_scintilla_set_font_quality(scintilla, font_quality);
  gtk_scintilla_set_caret_line_visible(scintilla, higthlight_caret_line);

  gtk_scintilla_set_indentation_guides (scintilla, show_indent_guides);
  gtk_scintilla_set_edge_mode (scintilla, edge_mode);
  gtk_scintilla_set_edge_column (scintilla, edge_column);

  gtk_scintilla_set_caret_width (scintilla, 2);
  gtk_scintilla_set_caret_period (scintilla, 250);

  gtk_scintilla_autoc_set_choose_single (scintilla, FALSE);
  gtk_scintilla_set_use_tabs (scintilla, tabs_instead_spaces);
  gtk_scintilla_set_tab_indents (scintilla, 1);
  gtk_scintilla_set_backspace_unindents (scintilla, 1);
  gtk_scintilla_set_tab_width (scintilla, tab_size);
  gtk_scintilla_set_indent (scintilla, indentation_size);

  //annotation styles
  gtk_scintilla_style_set_size (scintilla,  STYLE_ANNOTATION_ERROR, 8);
  gtk_scintilla_style_set_italic (scintilla,  STYLE_ANNOTATION_ERROR, FALSE);
  gtk_scintilla_style_set_bold (scintilla,  STYLE_ANNOTATION_ERROR, FALSE);
  gtk_scintilla_style_set_fore (scintilla,  STYLE_ANNOTATION_ERROR, 3946645);
  gtk_scintilla_style_set_back (scintilla,  STYLE_ANNOTATION_ERROR, 13355513);

  gtk_scintilla_style_set_size (scintilla,  STYLE_ANNOTATION_WARNING, 8);
  gtk_scintilla_style_set_italic (scintilla,  STYLE_ANNOTATION_WARNING, FALSE);
  gtk_scintilla_style_set_bold (scintilla,  STYLE_ANNOTATION_WARNING, FALSE);
  gtk_scintilla_style_set_fore (scintilla,  STYLE_ANNOTATION_WARNING, 2859424);
  gtk_scintilla_style_set_back (scintilla,  STYLE_ANNOTATION_WARNING, 10813438);
  
  /* syntax check indicator setup */
  gtk_scintilla_set_indicator_current(scintilla, 20);
  gtk_scintilla_indic_set_style(scintilla, 20, INDIC_SQUIGGLE);
  gtk_scintilla_indic_set_fore(scintilla, 20, 0x0000ff);

  g_object_unref(pref);
}

static void save_point_reached(GtkWidget *scintilla, gpointer user_data)
{
  Document_Scintilla *doc = DOCUMENT_SCINTILLA(user_data);
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gchar *short_filename;
  g_object_get(doc, "short_filename", &short_filename, NULL);
  if (short_filename != NULL) {
    gtk_label_set_text(GTK_LABEL (docdet->label), short_filename);
    /*emit save update signal*/
    g_signal_emit (G_OBJECT (doc), signals[SAVE_UPDATE], 0);
  }
}

static void save_point_left(GtkWidget *scintilla, gpointer user_data)
{
  Document_Scintilla *doc = DOCUMENT_SCINTILLA(user_data);
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gchar *caption;
  gchar *short_filename;
  g_object_get(doc, "short_filename", &short_filename, NULL);
  if (short_filename != NULL) {
    caption= g_strdup_printf("*%s",short_filename);
    gtk_label_set_text(GTK_LABEL (docdet->label), caption);
    g_free(caption);

    /*emit save update signal*/
    g_signal_emit (G_OBJECT (doc), signals[SAVE_UPDATE], 0); 
  }
}

static void scintilla_modified (GtkWidget *scintilla){
  GtkScintilla *sci = GTK_SCINTILLA(scintilla);
  gint current_pos = gtk_scintilla_get_current_pos(sci);
  gphpedit_statusbar_set_cursor_position (GPHPEDIT_STATUSBAR(main_window.appbar), 
  gtk_scintilla_line_from_position(sci, current_pos), 
  gtk_scintilla_get_column(sci, current_pos));
  gphpedit_statusbar_set_overwrite (GPHPEDIT_STATUSBAR(main_window.appbar), gtk_scintilla_get_overtype(sci));
}

static void update_ui(GtkWidget *scintilla)
{
  // ----------------------------------------------------
  // This code is based on that found in SciTE
  // Converted by AJ 2004-03-04
  // ----------------------------------------------------
  
  int current_brace_pos = -1;
  int matching_brace_pos = -1;
  int current_brace_column = -1;
  int matching_brace_column = -1;
  int current_line;
  int current_pos;
  char character_before = '\0';
  char character_after = '\0';

  GtkScintilla *sci = GTK_SCINTILLA(scintilla);

  if (gtk_scintilla_call_tip_active(sci)) {
    gtk_scintilla_call_tip_cancel(sci);
  }
    
  current_pos = gtk_scintilla_get_current_pos(sci);
  current_line = gtk_scintilla_line_from_position(sci, gtk_scintilla_get_current_pos(sci));
  
  //Check if the character before the cursor is a brace.
  if (current_pos > 0) {
    character_before = gtk_scintilla_get_char_at(sci, current_pos - 1);
    // style_before = gtk_scintilla_get_style_at(sci, current_pos - 1);
    if (character_before && strchr("[](){}", character_before)) {
      current_brace_pos = current_pos - 1;
    }
  }
  //If the character before the cursor is not a brace then
  //check if the character at the cursor is a brace.
  if (current_brace_pos < 0) {
    character_after = gtk_scintilla_get_char_at(sci, current_pos);
    // style_before = gtk_scintilla_get_style_at(sci, current_pos);
    if (character_after && strchr("[](){}", character_after)) {
      current_brace_pos = current_pos;
    }
  }
  //find the matching brace  
  if (current_brace_pos>=0) {
    matching_brace_pos = gtk_scintilla_brace_match(sci, current_brace_pos);
  }
  
  
  // If no brace has been found or we aren't editing PHP code
  if ((current_brace_pos==-1)) {
    gtk_scintilla_brace_bad_light(sci, -1);// Remove any existing highlight
    return;
  }
  
  // A brace has been found, but there isn't a matching one ...
  if (current_brace_pos!=1 && matching_brace_pos==-1) {
    // ... therefore send the bad_list message so it highlights the brace in red
    gtk_scintilla_brace_bad_light(sci, current_brace_pos);
  }
  else {
    // a brace has been found and a matching one, so highlight them both
    gtk_scintilla_brace_highlight(sci, current_brace_pos, matching_brace_pos);
    
    // and highlight the indentation marker
    current_brace_column = gtk_scintilla_get_column(sci, current_brace_pos);
    matching_brace_column = gtk_scintilla_get_column(sci, matching_brace_pos);
    
    gtk_scintilla_set_highlight_guide(sci, MIN(current_brace_column, matching_brace_pos));
  }
}
#if 0
static gboolean auto_memberfunc_complete_callback(gpointer data)
{
  Documentable *document = document_manager_get_current_documentable(main_window.docmg);
  Document_Scintilla *doc = DOCUMENT_SCINTILLA(document);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gint current_pos;
  current_pos = documentable_get_current_position(document);
  if (current_pos == GPOINTER_TO_INT(data)) {
    gchar *prefix = documentable_get_current_word(document);
    gchar *calltip = symbol_manager_get_symbols_matches (main_window.symbolmg, prefix, SYMBOL_MEMBER, docdet->type);
    if (calltip && strlen(calltip)!=0) gtk_scintilla_user_list_show(GTK_SCINTILLA(docdet->scintilla), 1, calltip);
//    gtk_scintilla_autoc_show(GTK_SCINTILLA(docdet->scintilla), current_pos, calltip);
    g_free(prefix);
  }
  docdet->completion_timer_set=FALSE;
  return FALSE;
}

static gboolean auto_complete_callback(gpointer data)
{
  Documentable *document = document_manager_get_current_documentable(main_window.docmg);
  Document_Scintilla *doc = DOCUMENT_SCINTILLA(document);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gint current_pos;
  current_pos = documentable_get_current_position(document);
  if (current_pos == GPOINTER_TO_INT(data)) {
    gchar *prefix = documentable_get_current_word(document);
    gchar *calltip = symbol_manager_get_symbols_matches (main_window.symbolmg, prefix, SYMBOL_FUNCTION | SYMBOL_CLASS | SYMBOL_VAR, docdet->type);
    if (calltip && strlen(calltip)!=0) gtk_scintilla_user_list_show(GTK_SCINTILLA(docdet->scintilla), 1, calltip);
    g_free(prefix);
    g_free(calltip);
  }
  docdet->completion_timer_set=FALSE;
  return FALSE;
}

static gboolean calltip_callback(gpointer data)
{
  Documentable *document = document_manager_get_current_documentable(main_window.docmg);
  Document_Scintilla *doc = DOCUMENT_SCINTILLA(document);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gint current_pos;
  current_pos = documentable_get_current_position(document);
  if (current_pos == GPOINTER_TO_INT(data)) {
    gchar *prefix = documentable_get_current_word(document);
    gchar *calltip = symbol_manager_get_calltip (main_window.symbolmg, prefix, docdet->type);
    if (calltip){
      gtk_scintilla_call_tip_show(GTK_SCINTILLA(docdet->scintilla), current_pos, calltip);
      g_free(calltip);
    }
    g_free(prefix);
  }
  docdet->calltip_timer_set = FALSE;
  return FALSE;
}
#endif
static void document_scintilla_show_calltip_at_current_pos(Document_Scintilla *doc)
{
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  language_provider_show_calltip (docdet->lgcss);
//  calltip_callback(GINT_TO_POINTER(documentable_get_current_position(DOCUMENTABLE(document_scintilla))));
}

/*
* autocomplete current word
*/
static void process_user_list_selection (GtkWidget *w, gint type, gchar *text, gpointer user_data){
  GtkScintilla *sci = GTK_SCINTILLA(w);
  Document_Scintilla *doc = DOCUMENT_SCINTILLA(user_data);
  gint current_pos = gtk_scintilla_get_current_pos(sci);
  if (type==1){
  gchar *current = documentable_get_current_word(DOCUMENTABLE(doc));
  gtk_scintilla_insert_text(sci, current_pos, text + strlen (current));
  gtk_scintilla_goto_pos(sci, current_pos + strlen (text) - strlen (current));
  g_free(current);
  } else {
    gtk_scintilla_insert_text(sci, current_pos, text);
    gtk_scintilla_goto_pos(sci, current_pos + strlen (text));
  }
}
#if 0
static gboolean IsOpenBrace(gchar ch) {
	return ch == '[' || ch == '(' || ch == '{';
}

static void InsertCloseBrace (GtkScintilla *scintilla, gint current_pos, gchar ch) {
  switch (ch) {
    case '[':
      gtk_scintilla_insert_text(scintilla, current_pos,"]");
      gtk_scintilla_goto_pos(scintilla, current_pos);
      break;
    case '{':
      gtk_scintilla_insert_text(scintilla, current_pos,"}");
      gtk_scintilla_goto_pos(scintilla, current_pos);
      break;
    case '(':
      gtk_scintilla_insert_text(scintilla, current_pos,")");
      gtk_scintilla_goto_pos(scintilla, current_pos);
      break;
  }
}

static void indent_line(GtkScintilla *sci, gint line, gint indent)
{
  gint selStart;
  gint selEnd;
  gint posBefore;
  gint posAfter;
  gint posDifference;

  selStart = gtk_scintilla_get_selection_start(sci);
  selEnd = gtk_scintilla_get_selection_end(sci);
  posBefore = gtk_scintilla_get_line_indentation(sci, line);
  gtk_scintilla_set_line_indentation(sci, line, indent);
  posAfter = gtk_scintilla_get_line_indentation(sci, line);
  posDifference =  posAfter - posBefore;

  if (posAfter > posBefore) {
    // Move selection on
    if (selStart >= posBefore) {
      selStart += posDifference;
    }
    if (selEnd >= posBefore) {
      selEnd += posDifference;
    }
  }
  else if (posAfter < posBefore) {
    // Move selection back
    if (selStart >= posAfter) {
      if (selStart >= posBefore)
        selStart += posDifference;
      else
        selStart = posAfter;
    }
    if (selEnd >= posAfter) {
      if (selEnd >= posBefore)
        selEnd += posDifference;
      else
        selEnd = posAfter;
    }
  }
  gtk_scintilla_set_selection_start(sci, selStart);
  gtk_scintilla_set_selection_end(sci, selEnd);
}

static void autoindent_brace_code (GtkScintilla *sci, gboolean (*indent_char_func)(char ch), gboolean (*unindent_char_func)(char ch))
{
  gint current_pos;
  gint current_line;
  gint previous_line;
  gint previous_line_indentation;
  gint previous_line_start;
  gint previous_line_end;
  gchar *previous_char_buffer;
  gint previous_char_buffer_length;
  gchar *previous_line_buffer;
  gint previous_line_buffer_length;

  PreferencesManager *pref = preferences_manager_new ();
  current_pos = gtk_scintilla_get_current_pos(sci);
  current_line = gtk_scintilla_line_from_position(sci, current_pos);

  gphpedit_debug (DEBUG_DOCUMENT);

  if (current_line>0) {
    gtk_scintilla_begin_undo_action(sci);
    previous_line = current_line-1;
    previous_line_indentation = gtk_scintilla_get_line_indentation(sci, previous_line);

    previous_line_end = gtk_scintilla_get_line_end_position(sci, previous_line);
    previous_char_buffer = gtk_scintilla_get_text_range (sci, previous_line_end-1, previous_line_end, &previous_char_buffer_length);
    if (indent_char_func(*previous_char_buffer)) {
      gint indentation_size;
      g_object_get(pref, "indentation_size", &indentation_size, NULL);
      previous_line_indentation+=indentation_size;
    } else if (unindent_char_func(*previous_char_buffer)) {
      gint indentation_size;
      g_object_get(pref, "indentation_size", &indentation_size, NULL);
      previous_line_indentation-=indentation_size;
      if (previous_line_indentation < 0) previous_line_indentation = 0;
      previous_line_start = gtk_scintilla_position_from_line(sci, previous_line);
      previous_line_buffer = gtk_scintilla_get_text_range (sci, previous_line_start, previous_line_end, &previous_line_buffer_length);
      gboolean unindent = TRUE;
      gint char_act = 0;
      while (char_act <= previous_line_buffer_length)
      {
        char c = previous_line_buffer[char_act];
        if (!(g_ascii_iscntrl(c) || g_ascii_isspace(c) || unindent_char_func(c))) {
          unindent = FALSE;
          break;
        }
        char_act++;
      }
      if (unindent) gtk_scintilla_set_line_indentation(sci, previous_line, previous_line_indentation);
      g_free(previous_line_buffer);
    }
    g_free(previous_char_buffer);
    indent_line(sci, current_line, previous_line_indentation);
    gphpedit_debug_message (DEBUG_DOCUMENT, "previous_line=%d, previous_indent=%d\n", previous_line, previous_line_indentation);
    gint pos;
    gboolean tabs_instead_spaces;
    g_object_get(pref,"tabs_instead_spaces", &tabs_instead_spaces, NULL);
    if(tabs_instead_spaces){
      pos = gtk_scintilla_position_from_line(sci, current_line)+(previous_line_indentation/gtk_scintilla_get_tab_width(sci));
    } else {
      pos = gtk_scintilla_position_from_line(sci, current_line)+(previous_line_indentation);
    }
    gtk_scintilla_goto_pos(sci, pos);
    gtk_scintilla_end_undo_action(sci);
  }
}

static void cancel_calltip (GtkScintilla *sci)
{
  if (gtk_scintilla_call_tip_active(sci)) {
    gtk_scintilla_call_tip_cancel(sci);
  }
}

static void show_calltip (Document_ScintillaDetails *docdet, gint pos)
{
  if (!docdet->calltip_timer_set) {
    gint delay;
    PreferencesManager *pref = preferences_manager_new ();
    g_object_get(pref, "calltip_delay", &delay, NULL);
    g_object_unref(pref);
    docdet->calltip_timer_id = g_timeout_add(delay, calltip_callback, GINT_TO_POINTER(pos));
    docdet->calltip_timer_set = TRUE;
  }
}

static void show_autocompletion (Document_ScintillaDetails *docdet, gint pos)
{
  if (!docdet->completion_timer_set) {
    PreferencesManager *pref = preferences_manager_new ();
    gint delay;
    g_object_get(pref, "autocomplete_delay", &delay, NULL);
    g_object_unref(pref);
    docdet->completion_timer_id = g_timeout_add(delay, auto_complete_callback, GINT_TO_POINTER(pos));
    docdet->completion_timer_set=TRUE;
  }
}

static gboolean auto_complete_classes_callback(gpointer data)
{
  Documentable *document = document_manager_get_current_documentable(main_window.docmg);
  Document_Scintilla *doc =  DOCUMENT_SCINTILLA(document);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gint current_pos;
  current_pos = documentable_get_current_position(document);
  if (current_pos == GPOINTER_TO_INT(data)) {
    gchar *autocomp = symbol_manager_get_classes (main_window.symbolmg, docdet->type);
    if (autocomp) gtk_scintilla_user_list_show(GTK_SCINTILLA(docdet->scintilla), 2, autocomp);
    g_free(autocomp); /*release resources*/
  }
  docdet->completion_timer_set=FALSE;
  return FALSE;
}

static gboolean auto_complete_php_variables_callback(gpointer data)
{
  Documentable *document = document_manager_get_current_documentable(main_window.docmg);
  Document_Scintilla *doc =  DOCUMENT_SCINTILLA(document);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gint current_pos;
  current_pos = documentable_get_current_position(document);
  if (current_pos == GPOINTER_TO_INT(data)) {
    gchar *prefix = documentable_get_current_word(document);
    gchar *result = symbol_manager_get_symbols_matches (main_window.symbolmg, prefix, SYMBOL_VAR, docdet->type);
    if (result) gtk_scintilla_user_list_show(GTK_SCINTILLA(docdet->scintilla), 1, result);
    g_free(prefix);
  }
  docdet->completion_timer_set=FALSE;
  return FALSE;
}
#endif

static void char_added(GtkWidget *scintilla, guint ch, gpointer user_data)
{
  gphpedit_debug_message (DEBUG_DOCUMENT, "char added:%d",ch);
  Document_Scintilla *doc = DOCUMENT_SCINTILLA(user_data);
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  if (docdet->lgcss) language_provider_trigger_completion (docdet->lgcss, ch);
#if 0
  gphpedit_debug_message (DEBUG_DOCUMENT, "char added:%d",ch);
  Document_Scintilla *doc = DOCUMENT_SCINTILLA(user_data);
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);

  GtkScintilla *sci = GTK_SCINTILLA(scintilla);

  gint current_pos;
  gint wordStart;
  gint wordEnd;
  gint current_word_length;
  gint current_line;
  gchar *ac_buffer = NULL;
  gint ac_length;
  gchar *member_function_buffer = NULL;
  gint member_function_length;
  guint style;
  PreferencesManager *pref = preferences_manager_new ();
  current_pos = gtk_scintilla_get_current_pos(sci);
  current_line = gtk_scintilla_line_from_position(sci, current_pos);
  wordStart = gtk_scintilla_word_start_position(sci, current_pos-1, TRUE);
  wordEnd = gtk_scintilla_word_end_position(sci, current_pos-1, TRUE);
  current_word_length = wordEnd - wordStart;
  style = gtk_scintilla_get_style_at(sci, current_pos);
  gboolean auto_brace;
  g_object_get(pref, "auto_complete_braces", &auto_brace, NULL);
  if (IsOpenBrace(ch) && auto_brace) {
      InsertCloseBrace (sci, current_pos, ch);
      return;
    }
  if (gtk_scintilla_autoc_active(sci)==1) {
    style = 0; // Hack to get around the drop-down not showing in comments, but if it's been forced...  
  }
  gint prev_char;
  switch(docdet->type) {
    case(TAB_PHP):
      if ((style != SCE_HPHP_SIMPLESTRING) && (style != SCE_HPHP_HSTRING) && (style != SCE_HPHP_COMMENTLINE) && (style !=SCE_HPHP_COMMENT)) {
      switch(ch) {
        case ('\r'):
        case ('\n'):
          autoindent_brace_code (sci, is_php_char_autoindent, is_php_char_autounindent);
          break;
        case (')'):
          cancel_calltip (sci);
          break;
        case ('('):
          show_calltip (docdet, current_pos);
          break;
        case ('>'):
        case (':'):
          prev_char = gtk_scintilla_get_char_at(sci, current_pos-2);
          if ((prev_char=='-' && ch =='>') || (prev_char==':' && ch ==':')) {
            /*search back for a '$' in that line */
            gint initial_pos= gtk_scintilla_position_from_line(sci, current_line);
            gint line_size;
            gchar *line_text = gtk_scintilla_get_text_range (sci, initial_pos, wordStart-1, &line_size);
            if (!check_php_variable_before(line_text)) break;
            if (!docdet->completion_timer_set) {
              gint delay;
              g_object_get(pref, "autocomplete_delay", &delay, NULL);
              docdet->completion_timer_id = g_timeout_add(delay, auto_memberfunc_complete_callback, GINT_TO_POINTER(current_pos));
              docdet->completion_timer_set=TRUE;
            }
          }
          break;
        default:
        member_function_buffer = gtk_scintilla_get_text_range (sci, wordStart, wordEnd, &member_function_length);
        if (g_str_has_prefix(member_function_buffer,"$") || g_str_has_prefix(member_function_buffer,"__")) {
            if (!docdet->completion_timer_set) {
              gint delay;
              g_object_get(pref, "autocomplete_delay", &delay, NULL);
              docdet->completion_timer_id = g_timeout_add(delay, auto_complete_php_variables_callback, GINT_TO_POINTER(current_pos));
              docdet->completion_timer_set=TRUE;
            }
        } else if (g_strcmp0(member_function_buffer,"instanceof")==0 || g_strcmp0(member_function_buffer,"is_subclass_of")==0) {
          gtk_scintilla_insert_text(sci, current_pos," ");
          gtk_scintilla_goto_pos(sci, current_pos + 1);
          docdet->completion_timer_set=TRUE;
          auto_complete_classes_callback(GINT_TO_POINTER(current_pos + 1));
        } else if (current_word_length>=3) {
        // check to see if they've typed <?php and if so do nothing
        if (wordStart>1) {
          ac_buffer = gtk_scintilla_get_text_range (sci, wordStart-2, wordEnd, &ac_length);
          if (g_strcmp0(ac_buffer,"<?php")==0) {
            g_free(ac_buffer);
            break;
          }
          g_free(ac_buffer);
        }
        show_autocompletion (docdet, current_pos);
        }
        g_free(member_function_buffer);
        }
        break;
      case(TAB_CXX):
      case (TAB_PERL):
        switch(ch) {
            case ('\r'):
            case ('\n'):
              autoindent_brace_code (sci, is_cxx_char_autoindent, is_cxx_char_autounindent);
            break;
            case (')'):
              cancel_calltip (sci);
            break;
            case ('('):
              show_calltip (docdet, current_pos);
            break;
            case ('>'):
            case (':'):
              prev_char = gtk_scintilla_get_char_at(sci, current_pos-2);
              if ((prev_char=='-' && ch =='>') || (prev_char==':' && ch ==':')) {
                /*search back for a '$' in that line */
                gint initial_pos= gtk_scintilla_position_from_line(sci, current_line);
                gint line_size;
                gchar *line_text = gtk_scintilla_get_text_range (sci, initial_pos, wordStart-1, &line_size);
                if (!check_php_variable_before(line_text)) break;
                if (!docdet->completion_timer_set) {
                  gint delay;
                  g_object_get(pref, "autocomplete_delay", &delay, NULL);
                  docdet->completion_timer_id = g_timeout_add(delay, auto_memberfunc_complete_callback, GINT_TO_POINTER(current_pos));
                  docdet->completion_timer_set=TRUE;
                }
              }
              break;
            default:
            member_function_buffer = gtk_scintilla_get_text_range (sci, wordStart-2, wordStart, &member_function_length);
            if(current_word_length>=3) show_autocompletion (docdet, current_pos);
            }
            g_free(member_function_buffer);
        break;
      case (TAB_PYTHON):
        switch(ch) {
            case ('\r'):
            case ('\n'):
              autoindent_brace_code (sci, is_python_char_autoindent, is_python_char_autounindent);
            break;
            case (')'):
              cancel_calltip (sci);
            break;
            case ('('):
              show_calltip (docdet, current_pos);
            break;
            default:
              member_function_buffer = gtk_scintilla_get_text_range (sci, wordStart-2, wordStart, &member_function_length);
              if(current_word_length>=3){
                show_autocompletion (docdet, current_pos);
              }
              g_free(member_function_buffer);
        }
        break;
      case(TAB_CSS):
      switch(ch) {
          case ('\r'):
          case ('\n'):
            autoindent_brace_code (sci, is_css_char_autoindent, is_css_char_autounindent);
          break;
        case (';'):
            cancel_calltip (sci);
            break;
          case (':'):
            show_calltip (docdet, current_pos);
            break;
          default:
        member_function_buffer = gtk_scintilla_get_text_range (sci, wordStart-2, wordStart, &member_function_length);
        if(current_word_length>=3){
          show_autocompletion (docdet, current_pos);
        }
        g_free(member_function_buffer);
        }
        break;
      case(TAB_COBOL):
      case(TAB_SQL):
        switch(ch) {
            case ('\r'):
            case ('\n'):
              autoindent_brace_code (sci, is_sql_char_autoindent, is_sql_char_autounindent);
            break;
            default:
        member_function_buffer = gtk_scintilla_get_text_range (sci, wordStart-2, wordStart, &member_function_length);
        if(current_word_length>=3){
          show_autocompletion (docdet, current_pos);
        }
        g_free(member_function_buffer);
        break;
        }
      }
     default:
            member_function_buffer = gtk_scintilla_get_text_range (sci, wordStart-2, wordEnd, &member_function_length);
            /* if we type <?php then we are in a php file so force php syntax mode */
            if (g_strcmp0(member_function_buffer,"<?php")==0){
                documentable_set_type(DOCUMENTABLE(doc), TAB_PHP);
               }
            g_free(member_function_buffer);
            break;
    }
  g_object_unref(pref);
#endif
}

/*
* process_drag_uri
* send open signal for uris dropped in scintilla widget
*/
static void process_drag_uri(GtkWidget *scintilla, gpointer data)
{
  if (data){
    gchar **uris= g_strsplit (data,"\n",0);
    int i=0;
    while (uris[i]!=0){
        int k=strlen(uris[i]);
        if (k!=0){
          gchar *uri=g_malloc(k);
          strncpy(uri,uris[i],k); /* skip \n */
          uri[k-1]=0;
          document_manager_switch_to_file_or_open(main_window.docmg, uri, 0);
          g_free(uri);
        }
      i++;
    }
    g_strfreev (uris);
  }
}

static void tab_set_folding(Document_Scintilla *document_scintilla, gint folding)
{
  if (folding) {
    Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(document_scintilla);
    GtkScintilla *sci = GTK_SCINTILLA(docdet->scintilla);
    gint modeventmask;

    modeventmask = gtk_scintilla_get_mod_event_mask(sci);
    gtk_scintilla_set_mod_event_mask(sci, modeventmask | SC_MOD_CHANGEFOLD);
    gtk_scintilla_set_fold_flags(sci, SC_FOLDFLAG_LINEAFTER_CONTRACTED);
    
    gtk_scintilla_set_margin_type_n(sci, 2, SC_MARGIN_SYMBOL);
    gtk_scintilla_set_margin_mask_n(sci, 2, SC_MASK_FOLDERS);
    gtk_scintilla_set_margin_sensitive_n(sci, 2, 1);
    
    gtk_scintilla_marker_define(sci, SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS);
    gtk_scintilla_marker_set_fore(sci, SC_MARKNUM_FOLDEROPEN,16777215);
    gtk_scintilla_marker_set_back(sci, SC_MARKNUM_FOLDEROPEN,0);
    gtk_scintilla_marker_define(sci, SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS);
    gtk_scintilla_marker_set_fore(sci, SC_MARKNUM_FOLDER,16777215);
    gtk_scintilla_marker_set_back(sci, SC_MARKNUM_FOLDER,0);
    gtk_scintilla_marker_define(sci, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
    gtk_scintilla_marker_set_fore(sci, SC_MARKNUM_FOLDERSUB,16777215);
    gtk_scintilla_marker_set_back(sci, SC_MARKNUM_FOLDERSUB,0);
    gtk_scintilla_marker_define(sci, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER);
    gtk_scintilla_marker_set_fore(sci, SC_MARKNUM_FOLDERTAIL,16777215);
    gtk_scintilla_marker_set_back(sci, SC_MARKNUM_FOLDERTAIL,0);
    gtk_scintilla_marker_define(sci, SC_MARKNUM_FOLDEREND, SC_MARK_BOXPLUSCONNECTED);
    gtk_scintilla_marker_set_fore(sci, SC_MARKNUM_FOLDEREND,16777215);
    gtk_scintilla_marker_set_back(sci, SC_MARKNUM_FOLDEREND,0);
    gtk_scintilla_marker_define(sci, SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED);
    gtk_scintilla_marker_set_fore(sci, SC_MARKNUM_FOLDEROPENMID,16777215);
    gtk_scintilla_marker_set_back(sci, SC_MARKNUM_FOLDEROPENMID,0);
    gtk_scintilla_marker_define(sci, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER);
    gtk_scintilla_marker_set_fore(sci, SC_MARKNUM_FOLDERMIDTAIL,16777215);
    gtk_scintilla_marker_set_back(sci, SC_MARKNUM_FOLDERMIDTAIL,0);
    
    gtk_scintilla_set_margin_width_n (sci, 2, 14);
    
    g_signal_connect (G_OBJECT (docdet->scintilla), "modified", G_CALLBACK (handle_modified), NULL);
  }
}

static void margin_clicked (GtkWidget *scintilla, gint modifiers, gint position, gint margin, gpointer user_data)
{
  if(margin!=1){
    gint line;
    gboolean show_folding;
    line = gtk_scintilla_line_from_position(GTK_SCINTILLA(scintilla), position);
    PreferencesManager *pref = preferences_manager_new ();
    g_object_get(pref,"show_folding", &show_folding, NULL);
    if (show_folding && margin == 2) {
      fold_clicked(scintilla, line, modifiers);
    }
    g_object_unref(pref);
  }else{
    gint line;
    line = gtk_scintilla_line_from_position(GTK_SCINTILLA(scintilla), position);
    document_scintilla_marker_modify(DOCUMENT_SCINTILLA(user_data), line);
  }
}

// All the folding functions are converted from QScintilla, released under the GPLv2 by
// Riverbank Computing Limited <info@riverbankcomputing.co.uk> and Copyright (c) 2003 by them.
static void fold_clicked(GtkWidget *scintilla, guint lineClick, guint bstate)
{
  gint levelClick;
  GtkScintilla *sci = GTK_SCINTILLA(scintilla);

  levelClick = gtk_scintilla_get_fold_level(GTK_SCINTILLA(scintilla), lineClick);

  if (levelClick & SC_FOLDLEVELHEADERFLAG)
  {
    if (bstate & SCMOD_SHIFT) {
      // Ensure all children are visible.
      gtk_scintilla_set_fold_expanded(sci, lineClick, 1);
      fold_expand(scintilla, lineClick, TRUE, TRUE, 100, levelClick);
    }
    else if (bstate & SCMOD_CTRL) {
      if (gtk_scintilla_get_fold_expanded(sci, lineClick)) {
        // Contract this line and all its children.
        gtk_scintilla_set_fold_expanded(sci, lineClick, 0);
        fold_expand(scintilla, lineClick, FALSE, TRUE, 0, levelClick);
      }
      else {
        // Expand this line and all its children.
        gtk_scintilla_set_fold_expanded(sci, lineClick, 1);
        fold_expand(scintilla, lineClick, TRUE, TRUE, 100, levelClick);
      }
    }
    else {
      // Toggle this line.
      gtk_scintilla_toggle_fold(sci, lineClick);
    }
  }
}

// All the folding functions are converted from QScintilla, released under the GPLv2 by
// Riverbank Computing Limited <info@riverbankcomputing.co.uk> and Copyright (c) 2003 by them.
static void fold_expand(GtkWidget *scintilla, gint line, gboolean doExpand, gboolean force, gint visLevels, gint level)
{
  gint lineMaxSubord;
  gint levelLine;

  GtkScintilla *sci = GTK_SCINTILLA(scintilla);

  lineMaxSubord = gtk_scintilla_get_last_child(sci, line, level & SC_FOLDLEVELNUMBERMASK);

  while (line <= lineMaxSubord) {
    line++;
    if (force) {
      if (visLevels > 0) {
        gtk_scintilla_show_lines(sci, line, line);
      }
      else {
        gtk_scintilla_hide_lines(sci, line, line);
      }
    }
    else if (doExpand) {
      gtk_scintilla_show_lines(sci, line, line);
    }

    levelLine = level;

    if (levelLine == -1) {
      levelLine = gtk_scintilla_get_fold_level(sci, line);
    }

    if (levelLine & SC_FOLDLEVELHEADERFLAG) {
      if (force) {
        if (visLevels > 1) {
          gtk_scintilla_set_fold_expanded(sci, line, 1);
        }
        else {
          gtk_scintilla_set_fold_expanded(sci, line, 0);
        }

        fold_expand(scintilla, line, doExpand, force, visLevels - 1, 0); // Added last 0 param - AJ
      }
      else if (doExpand) {
        if (!gtk_scintilla_get_fold_expanded(sci, line)) {
          gtk_scintilla_set_fold_expanded(sci, line, 1);
        }

        fold_expand(scintilla, line, TRUE, force, visLevels - 1, 0); // Added last 0 param - AJ
      }
      else {
        fold_expand(scintilla, line, FALSE, force, visLevels - 1, 0); // Added last 0 param - AJ
      }
    }
    else {
      line++;
    }
  }
}

// All the folding functions are converted from QScintilla, released under the GPLv2 by
// Riverbank Computing Limited <info@riverbankcomputing.co.uk> and Copyright (c) 2003 by them.
static void fold_changed(GtkWidget *scintilla, int line,int levelNow,int levelPrev)
{
  if (levelNow & SC_FOLDLEVELHEADERFLAG) {
    if (!(levelPrev & SC_FOLDLEVELHEADERFLAG))
      gtk_scintilla_set_fold_expanded(GTK_SCINTILLA(scintilla), line, 1);
        }
  else if (levelPrev & SC_FOLDLEVELHEADERFLAG) {
    if (!gtk_scintilla_get_fold_expanded(GTK_SCINTILLA(scintilla), line)) {
      // Removing the fold from one that has been contracted
      // so should expand.  Otherwise lines are left
      // invisible with no way to make them visible.
      fold_expand(scintilla, line, TRUE, FALSE, 0, levelPrev);
    }
  }
}

// All the folding functions are converted from QScintilla, released under the GPLv2 by
// Riverbank Computing Limited <info@riverbankcomputing.co.uk> and Copyright (c) 2003 by them.
static void handle_modified(GtkWidget *scintilla, gint pos,gint mtype,gchar *text,gint len,
           gint added,gint line,gint foldNow,gint foldPrev)
{
  PreferencesManager *pref = preferences_manager_new ();
  gboolean show_folding;
  g_object_get(pref,"show_folding", &show_folding, NULL);
  if (show_folding && (mtype & SC_MOD_CHANGEFOLD)) {
    fold_changed(scintilla, line, foldNow, foldPrev);
  }
  g_object_unref(pref);
}

static void document_scintilla_marker_add(GtkScintilla *scintilla, gint line){
    gtk_scintilla_marker_define(scintilla, 1, SC_MARK_SHORTARROW);
    gtk_scintilla_marker_set_back(scintilla, 1, 101);
    gtk_scintilla_marker_set_fore(scintilla, 1, 101);
    gtk_scintilla_marker_add(scintilla, line, 1);
}

static void document_scintilla_marker_delete(GtkScintilla *scintilla, gint line){
   gtk_scintilla_marker_delete(scintilla, line, 1);
}

static void document_scintilla_marker_modify(Document_Scintilla *doc, gint line)
{
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  if (gtk_scintilla_marker_get(GTK_SCINTILLA(docdet->scintilla),line)!= SC_MARK_ARROW){
    document_scintilla_marker_add(GTK_SCINTILLA(docdet->scintilla), line);
  }else{
    document_scintilla_marker_delete(GTK_SCINTILLA(docdet->scintilla), line);
  }
}

static void document_scintilla_modify_current_line_marker(Document_Scintilla *doc)
{
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  guint current_pos;
  guint current_line;
  current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(docdet->scintilla));
  current_line = gtk_scintilla_line_from_position(GTK_SCINTILLA(docdet->scintilla), current_pos);
  document_scintilla_marker_modify(doc, current_line);
}

//circle markers
static void document_scintilla_find_next_marker(Document_Scintilla *doc){
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  guint current_pos;
  guint current_line;
  current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(docdet->scintilla));
  current_line = gtk_scintilla_line_from_position(GTK_SCINTILLA(docdet->scintilla), current_pos);
  gint line;
  //skip the current line
  line= gtk_scintilla_marker_next(GTK_SCINTILLA(docdet->scintilla), current_line + 1, 2);
  if (line==-1){
    //no markers in that direccion, we should go back to the first line
    line= gtk_scintilla_marker_next(GTK_SCINTILLA(docdet->scintilla),0, 2);
    if (line!=-1){
      //go back to the first marker
      //bugfix the maker is in the next line
      documentable_goto_line(DOCUMENTABLE(doc), line + 1);
    }else{  
      gphpedit_statusbar_flash_message (GPHPEDIT_STATUSBAR(main_window.appbar),0,"%s",_("No marker found"));
    }
  }else{
    //goto the marker posicion
      documentable_goto_line(DOCUMENTABLE(doc), line + 1);
  }
}

static void document_scintilla_auto_detect_file_type(Documentable *doc) {
  if (!doc) return;
  gchar *filename = documentable_get_filename(doc);
  /* reset type */
  g_object_set(doc, "type", TAB_FILE, NULL);
  gint type = TAB_FILE;
  if (is_perl_file(filename)) {
    type = TAB_PERL;
  } else if (is_css_file(filename)) {
    type = TAB_CSS;
  } else if (is_cxx_file(filename)) {
    type = TAB_CXX;
  } else if (is_cobol_file(filename)) {
    type = TAB_COBOL;
  } else if (is_python_file(filename)) {
    type = TAB_PYTHON;
  } else if (is_sql_file(filename)) {
    type = TAB_SQL;
  } else if (is_php_file_from_filename(filename)){
    type = TAB_PHP;
  } else {
    gchar *content;
    content = documentable_get_text(doc);
    if (is_php_file_from_content(content)){
      type = TAB_PHP;
    }
    g_free(content);
  }
  documentable_set_type(doc, type);
  g_free(filename);
}

void document_scintilla_clear_sintax_style(Document_Scintilla *doc)
{
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  if (GTK_IS_SCINTILLA(docdet->scintilla)){
  /* clear document_scintilla before start any styling action */
  gtk_scintilla_indicator_clear_range(GTK_SCINTILLA(docdet->scintilla), 0, gtk_scintilla_get_text_length(GTK_SCINTILLA(docdet->scintilla)));
  }
}

void document_scintilla_set_sintax_annotation(Document_Scintilla *doc)
{
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gtk_scintilla_annotation_clear_all(GTK_SCINTILLA(docdet->scintilla));
  gtk_scintilla_annotation_set_visible(GTK_SCINTILLA(docdet->scintilla), 2);
}

void document_scintilla_set_sintax_line(Document_Scintilla *doc, guint current_line_number)
{
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gint line_start;
  gint line_end;
  gint indent;
  gint indentation_size;
  PreferencesManager *pref = preferences_manager_new ();
  g_object_get(pref, "indentation_size", &indentation_size, NULL);
  g_object_unref(pref);
  indent = gtk_scintilla_get_line_indentation(GTK_SCINTILLA(docdet->scintilla), current_line_number);
  line_start = gtk_scintilla_position_from_line(GTK_SCINTILLA(docdet->scintilla), current_line_number);
  line_start += (indent/indentation_size);
  line_end = gtk_scintilla_get_line_end_position(GTK_SCINTILLA(docdet->scintilla), current_line_number);
  gtk_scintilla_indicator_fill_range(GTK_SCINTILLA(docdet->scintilla), line_start, line_end-line_start);  
}

void document_scintilla_add_sintax_annotation(Document_Scintilla *doc, guint current_line_number, gchar *token, gint style)
{
  g_return_if_fail(doc);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gtk_scintilla_annotation_set_style(GTK_SCINTILLA(docdet->scintilla), current_line_number, style);
  gtk_scintilla_annotation_set_text(GTK_SCINTILLA(docdet->scintilla), current_line_number, token);
}

void document_scintilla_saver_done_saving_cb (DocumentSaver *docsav, Document_Scintilla *document_scintilla, gpointer user_data)
{
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(document_scintilla);
  gtk_scintilla_set_save_point (GTK_SCINTILLA(docdet->scintilla));
  document_scintilla_auto_detect_file_type(DOCUMENTABLE(document_scintilla));
  gchar *filename = documentable_get_filename(DOCUMENTABLE(document_scintilla));
  register_file_opened(filename);
  g_free(filename);
}

static char *macro_message_to_string(gint message)
{
  switch (message) {
    case (2170) : return "REPLACE SELECTION";
    case (2177) : return "CLIPBOARD CUT";
    case (2178) : return "CLIPBOARD COPY";
    case (2179) : return "CLIPBOARD PASTE";
    case (2180) : return "CLEAR";
    case (2300) : return "LINE DOWN";
    case (2301) : return "LINE DOWN EXTEND";
    case (2302) : return "LINE UP";
    case (2303) : return "LINE UP EXTEND";
    case (2304) : return "CHAR LEFT";
    case (2305) : return "CHAR LEFT EXTEND";
    case (2306) : return "CHAR RIGHT";
    case (2307) : return "CHAR RIGHT EXTEND";
    case (2308) : return "WORD LEFT";
    case (2309) : return "WORD LEFT EXTEND";
    case (2310) : return "WORD RIGHT";
    case (2311) : return "WORD RIGHT EXTEND";
    case (2312) : return "HOME";
    case (2313) : return "HOME EXTEND";
    case (2314) : return "LINE END";
    case (2315) : return "LINE END EXTEND";
    case (2316) : return "DOCUMENT_SCINTILLA START";
    case (2317) : return "DOCUMENT_SCINTILLA START EXTEND";
    case (2318) : return "DOCUMENT_SCINTILLA END";
    case (2319) : return "DOCUMENT_SCINTILLA END EXTEND";
    case (2320) : return "PAGE UP";
    case (2321) : return "PAGE UP EXTEND";
    case (2322) : return "PAGE DOWN";
    case (2323) : return "PAGE DOWN EXTEND";
    default:
      return "-- UNKNOWN --";
  }
}

static void macro_record (GtkWidget *scintilla, gint message, gulong wparam, glong lparam, gpointer user_data)
{
  Document_Scintilla *document_scintilla = DOCUMENT_SCINTILLA(user_data);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(document_scintilla);
  MacroEvent *event;
  
  if (docdet->is_macro_recording) {
    event = g_new0(MacroEvent, 1);
    event->message = message;
    event->wparam = wparam;
    // Special handling for text inserting, duplicate inserted string
    if (event->message == 2170 && docdet->is_pasting) {
      event->message = 2179;
    }
    else if (event->message == 2170) {
      event->lparam = (glong) g_strdup((gchar *)lparam);
    }
    else {
      event->lparam = lparam;
    }
    gphpedit_debug_message (DEBUG_DOCUMENT,"Message: %d (%s)\n", event->message, macro_message_to_string(event->message));
    docdet->keyboard_macro_list = g_slist_append(docdet->keyboard_macro_list, event);
  }
}

static void keyboard_macro_empty_old(Document_Scintilla *document_scintilla)
{
  g_return_if_fail(document_scintilla);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(document_scintilla);
  GSList *current;
  MacroEvent *event;

  for (current = docdet->keyboard_macro_list; current; current = g_slist_next(current)) {
    event = current->data;
    if (event->message == 2170) {
      // Special handling for text inserting, free inserted string
      g_free((gchar *)event->lparam);
    }
    g_free(event);
  }
  g_slist_free(docdet->keyboard_macro_list);
  docdet->keyboard_macro_list = NULL;
}

void document_scintilla_keyboard_macro_startstop(Document_Scintilla *document_scintilla)
{
  g_return_if_fail(document_scintilla);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(document_scintilla);
    if (docdet->is_macro_recording) {
      gtk_scintilla_stop_record(GTK_SCINTILLA(docdet->scintilla));
      docdet->is_macro_recording = FALSE;
    } else {
      if (docdet->keyboard_macro_list) {
        keyboard_macro_empty_old(document_scintilla);
      }
      gtk_scintilla_start_record(GTK_SCINTILLA(docdet->scintilla));
      docdet->is_macro_recording = TRUE;
    }
}

void document_scintilla_keyboard_macro_playback(Document_Scintilla *document_scintilla)
{
  GSList *current;
  MacroEvent *event;
  g_return_if_fail(document_scintilla);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(document_scintilla);
  gtk_scintilla_begin_undo_action(GTK_SCINTILLA(docdet->scintilla));
  if (docdet->keyboard_macro_list) {
    for (current = docdet->keyboard_macro_list; current; current = g_slist_next(current)) {
      event = current->data;
      gphpedit_debug_message (DEBUG_DOCUMENT,"Message: %d (%s)\n", event->message, macro_message_to_string(event->message));
      switch (event->message) {
          case (2170) : gtk_scintilla_replace_sel(GTK_SCINTILLA(docdet->scintilla), (gchar *)event->lparam); break;
          case (2177) : gtk_scintilla_cut(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2178) : gtk_scintilla_copy(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2179) : gtk_scintilla_paste(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2180) : gtk_scintilla_clear(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2300) : gtk_scintilla_line_down(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2301) : gtk_scintilla_line_down_extend(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2302) : gtk_scintilla_line_up(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2303) : gtk_scintilla_line_up_extend(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2304) : gtk_scintilla_char_left(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2305) : gtk_scintilla_char_left_extend(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2306) : gtk_scintilla_char_right(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2307) : gtk_scintilla_char_right_extend(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2308) : gtk_scintilla_word_left(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2309) : gtk_scintilla_word_left_extend(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2310) : gtk_scintilla_word_right(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2311) : gtk_scintilla_word_right_extend(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2312) : gtk_scintilla_home(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2313) : gtk_scintilla_home_extend(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2314) : gtk_scintilla_line_end(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2315) : gtk_scintilla_line_end_extend(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2316) : gtk_scintilla_document_start(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2317) : gtk_scintilla_document_start_extend(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2318) : gtk_scintilla_document_end(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2319) : gtk_scintilla_document_end_extend(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2320) : gtk_scintilla_page_up(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2321) : gtk_scintilla_page_up_extend(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2322) : gtk_scintilla_page_down(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2323) : gtk_scintilla_page_down_extend(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2324) : gtk_scintilla_edit_toggle_overtype(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2325) : gtk_scintilla_cancel(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2326) : gtk_scintilla_delete_back(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2327) : gtk_scintilla_tab(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2328) : gtk_scintilla_back_tab(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2329) : gtk_scintilla_new_line(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2330) : gtk_scintilla_form_feed(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2331) : gtk_scintilla_v_c_home(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2332) : gtk_scintilla_v_c_home_extend(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2333) : gtk_scintilla_zoom_in(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2334) : gtk_scintilla_zoom_out(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2335) : gtk_scintilla_del_word_left(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2336) : gtk_scintilla_del_word_right(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2337) : gtk_scintilla_line_cut(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2338) : gtk_scintilla_line_delete(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2339) : gtk_scintilla_line_transpose(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2340) : gtk_scintilla_lower_case(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2341) : gtk_scintilla_upper_case(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2342) : gtk_scintilla_line_scroll_down(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2343) : gtk_scintilla_line_scroll_up(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2344) : gtk_scintilla_delete_back_not_line(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2345) : gtk_scintilla_home_display(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2346) : gtk_scintilla_home_display_extend(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2347) : gtk_scintilla_line_end_display(GTK_SCINTILLA(docdet->scintilla)); break;
          case (2348) : gtk_scintilla_line_end_display_extend(GTK_SCINTILLA(docdet->scintilla)); break;
          default:
            g_print(_("Unhandle keyboard macro function %d, please report\n"), event->message);
      }
    }
  }
  gtk_scintilla_end_undo_action(GTK_SCINTILLA(docdet->scintilla));
}


static void document_scintilla_force_autocomplete(Document_Scintilla *document_scintilla)
{
  g_return_if_fail(document_scintilla);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(document_scintilla);
  if (docdet->lgcss) language_provider_trigger_completion (docdet->lgcss, 0);
#if 0
  guint current_pos;
  guint current_line;
  gchar *member_function_buffer;
  gint member_function_length;
  gint wordStart;
  gint wordEnd;

  g_return_if_fail(document_scintilla);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(document_scintilla);
  GtkScintilla *sci = GTK_SCINTILLA(docdet->scintilla);
  current_pos = gtk_scintilla_get_current_pos(sci);
  wordStart = gtk_scintilla_word_start_position(sci, current_pos-1, TRUE);
  wordEnd = gtk_scintilla_word_end_position(sci, current_pos-1, TRUE);
  member_function_buffer = gtk_scintilla_get_text_range (sci, wordEnd-1, wordEnd +1, &member_function_length);
  if (g_strcmp0(member_function_buffer, "->")==0 || g_strcmp0(member_function_buffer, "::")==0) {
    gint line_size;
    gint initial_pos;
    gchar *line_text;
    /*search back for a '$' in that line */
    current_line = gtk_scintilla_line_from_position(sci, current_pos);
    initial_pos= gtk_scintilla_position_from_line(sci, current_line);
    line_text= gtk_scintilla_get_text_range (sci, initial_pos, wordStart-1, &line_size);
    if (!check_php_variable_before(line_text)) return;
    auto_memberfunc_complete_callback(GINT_TO_POINTER(current_pos));
  } else {
    auto_complete_callback(GINT_TO_POINTER(current_pos));
  }
  g_free(member_function_buffer);
#endif
}

#define BACKSLASH 92
#define NEWLINE 10
#define TAB 9

void document_scintilla_insert_template(Document_Scintilla *document_scintilla, gchar *template)
{
  gphpedit_debug_message (DEBUG_DOCUMENT, "%s",template);
  gint current_pos;
  gint wordStart;
  gint wordEnd;
  char buf[16384];
  gint indentation;
  gint current_line;
  gint new_cursor_pos = 0;
  gint buffer_pos;

  g_return_if_fail(document_scintilla);
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(document_scintilla);
  if (GTK_IS_SCINTILLA(docdet->scintilla)){
  GtkScintilla *sci = GTK_SCINTILLA(docdet->scintilla);

  current_pos = gtk_scintilla_get_current_pos(sci);
  wordStart = gtk_scintilla_word_start_position(sci, current_pos-1, TRUE);
  wordEnd = gtk_scintilla_word_end_position(sci, current_pos-1, TRUE);

  strncpy(buf, template, 16383);
  gtk_scintilla_begin_undo_action(sci);
    
  // Remove template-key
  gtk_scintilla_set_selection_start(sci, wordStart);
  gtk_scintilla_set_selection_end(sci, wordEnd);
  gtk_scintilla_replace_sel(sci, "");

  // Get current indentation level
  current_line = gtk_scintilla_line_from_position(sci, current_pos);
  indentation = gtk_scintilla_get_line_indentation(sci, current_line);
   
  // Insert template, line by line, taking in to account indentation
  buffer_pos = 0;
    while (buf[buffer_pos] && buffer_pos<16380) {// 16384 - a few to account for lookaheads
      if (buf[buffer_pos] == BACKSLASH && buf[buffer_pos+1] == 'n') { 
        gtk_scintilla_add_text(sci, 1, "\n");      
        current_pos = gtk_scintilla_get_current_pos(sci);
        current_line = gtk_scintilla_line_from_position(sci, current_pos);
        gtk_scintilla_set_line_indentation(sci, current_line, indentation);
        gtk_scintilla_set_current_pos(sci, gtk_scintilla_get_line_end_position(sci, current_line));
        buffer_pos++; buffer_pos++;
      }
      else if (buf[buffer_pos] == '|') { // Current choice of cursor pos character
        new_cursor_pos = gtk_scintilla_get_current_pos(sci);
        buffer_pos++;
      }
      else if (buf[buffer_pos] == BACKSLASH && buf[buffer_pos+1] == 't') { 
        gtk_scintilla_add_text(sci, 1, "\t");      
        buffer_pos++; buffer_pos++;
      }
      else {
        gtk_scintilla_add_text(sci, 1, &buf[buffer_pos]);      
        buffer_pos++;
      }
    }
    
    // If there was a cursor pos character in there, put the cursor there
    if (new_cursor_pos) {
      gtk_scintilla_set_current_pos(sci, new_cursor_pos);
      gtk_scintilla_set_selection_start(sci, new_cursor_pos);
      gtk_scintilla_set_selection_end(sci, new_cursor_pos);
    }
    gtk_scintilla_end_undo_action(sci);
  }
}

static gchar *document_scintilla_get_title(Document_Scintilla *doc)
{
  gphpedit_debug (DEBUG_DOCUMENT);
  if (!doc) return NULL;
  GString *title= NULL;
  gchar *dir;
  char *str = NULL;
  title = g_string_new(str);
  gchar *filename = documentable_get_filename(DOCUMENTABLE(doc));
  gchar *tmp = filename_parent_uri(filename);
  g_free(filename);
  dir = filename_get_relative_path(tmp);
  g_free(tmp);
  gchar *short_filename;
  g_object_get(doc, "short_filename", &short_filename, NULL);
  g_string_printf (title,"%s (%s)", short_filename, dir);
  g_free(dir);
  g_string_append(title, _(" - gPHPEdit"));
  gboolean saved_status;
  g_object_get(doc, "saved", &saved_status, NULL);
  if (!saved_status){
  //If the content is not saved then add a * to the begining of the title
    g_string_prepend(title, "*");
  }
  if (title) return g_string_free(title, FALSE);
  return NULL;
}

void document_scintilla_activate_goto_line(Document_Scintilla *doc)
{
  Document_ScintillaDetails *docdet = DOCUMENT_SCINTILLA_GET_PRIVATE(doc);
  gtk_widget_show(docdet->gotobar);
  gtk_widget_grab_focus(docdet->gotobar);
}
