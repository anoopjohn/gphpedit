/* This file is part of gPHPEdit, a GNOME PHP Editor.
 
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
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with gPHPEdit.  If not, see <http://www.gnu.org/licenses/>.
 
   The GNU General Public License is contained in the file COPYING.
*/

#include <string.h>
#include "debug.h"
#include "language_cobol.h"
#include "preferences_manager.h"
#include "language_provider.h"
#include "symbol_manager.h"

/*
* language_cobol private struct
*/
struct Language_COBOLDetails
{
  Documentable *doc;
  GtkScintilla *sci;
  PreferencesManager *prefmg;
  SymbolManager *symbolmg;

  /*completion stuff*/
  guint completion_timer_id;
  gboolean completion_timer_set;
};

#define LANGUAGE_COBOL_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    LANGUAGE_COBOL_TYPE,\
					    Language_COBOLDetails))
enum
{
  PROP_0,
  PROP_DOCUMENT_SCINTILLA
};

static void language_cobol_language_provider_init(Language_ProviderInterface *iface, gpointer user_data);
static void language_cobol_trigger_completion (Language_Provider *lgcobol, guint ch);

G_DEFINE_TYPE_WITH_CODE(Language_COBOL, language_cobol, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE (IFACE_TYPE_LANGUAGE_PROVIDER,
                                                 language_cobol_language_provider_init));

static void show_calltip (Language_Provider *lgcss)
{
 /* not implemented */
}

static void language_cobol_language_provider_init(Language_ProviderInterface *iface, gpointer user_data)
{
  iface->trigger_completion = language_cobol_trigger_completion;
  iface->show_calltip = show_calltip;
}

static void
language_cobol_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
  Language_COBOLDetails *lgcoboldet;
  lgcoboldet = LANGUAGE_COBOL_GET_PRIVATE(object);

  switch (prop_id)
  {
    case PROP_DOCUMENT_SCINTILLA:
      if (lgcoboldet->doc) g_object_unref(lgcoboldet->doc);
      lgcoboldet->doc = g_value_dup_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
language_cobol_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
  Language_COBOLDetails *lgcoboldet;
  lgcoboldet = LANGUAGE_COBOL_GET_PRIVATE(object);

  
  switch (prop_id)
  {
    case PROP_DOCUMENT_SCINTILLA:
      g_value_set_object (value, lgcoboldet->doc);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}


/*
* disposes the Gobject
*/
static void language_cobol_dispose (GObject *object)
{
  Language_COBOLDetails *lgcoboldet;
  lgcoboldet = LANGUAGE_COBOL_GET_PRIVATE(object);
  g_object_unref(lgcoboldet->prefmg);
  g_object_unref(lgcoboldet->doc);
  g_object_unref(lgcoboldet->symbolmg);
  /* Chain up to the parent class */
  G_OBJECT_CLASS (language_cobol_parent_class)->dispose (object);
}

static void language_cobol_constructed (GObject *object)
{
  Language_COBOLDetails *lgcoboldet;
  lgcoboldet = LANGUAGE_COBOL_GET_PRIVATE(object);
  GtkScintilla *sci;
  g_object_get(lgcoboldet->doc, "scintilla", &sci, NULL);
  lgcoboldet->sci = sci;
}

static void
language_cobol_class_init (Language_COBOLClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = language_cobol_dispose;
  object_class->set_property = language_cobol_set_property;
  object_class->get_property = language_cobol_get_property;
  object_class->constructed = language_cobol_constructed;
  g_object_class_install_property (object_class,
                              PROP_DOCUMENT_SCINTILLA,
                              g_param_spec_object ("document_scintilla",
                              NULL, NULL,
                              DOCUMENT_SCINTILLA_TYPE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_type_class_add_private (klass, sizeof (Language_COBOLDetails));
}

static void
language_cobol_init (Language_COBOL * object)
{
  Language_COBOLDetails *lgcoboldet;
  lgcoboldet = LANGUAGE_COBOL_GET_PRIVATE(object);
  lgcoboldet->prefmg = preferences_manager_new ();
  lgcoboldet->symbolmg = symbol_manager_new ();
}

Language_COBOL *language_cobol_new (Document_Scintilla *doc)
{
  Language_COBOL *lgcobol;
  lgcobol = g_object_new (LANGUAGE_COBOL_TYPE, "document_scintilla", doc, NULL);
  return lgcobol; /* return new object */
}

typedef struct {
 Language_COBOL *lgcobol;
 gint old_pos;
} Calltip_Data;

static gboolean auto_complete_callback(gpointer data)
{
  Calltip_Data *dat = (Calltip_Data *) data;
  Language_COBOLDetails *lgcoboldet = LANGUAGE_COBOL_GET_PRIVATE(dat->lgcobol);
  gint current_pos;
  current_pos = documentable_get_current_position(lgcoboldet->doc);
  GtkScintilla *scintilla = lgcoboldet->sci;
  if (current_pos == dat->old_pos) {
    gchar *prefix = documentable_get_current_word(lgcoboldet->doc);
    gchar *calltip = symbol_manager_get_symbols_matches (lgcoboldet->symbolmg, prefix, SYMBOL_FUNCTION | SYMBOL_CLASS | SYMBOL_VAR, TAB_COBOL);
    if (calltip && strlen(calltip)!=0) gtk_scintilla_user_list_show(GTK_SCINTILLA(scintilla), 1, calltip);
    g_free(prefix);
    g_free(calltip);
  }
  g_slice_free(Calltip_Data, dat);
  lgcoboldet->completion_timer_set=FALSE;
  return FALSE;
}

static void show_autocompletion (Language_COBOL *lgcobol, gint pos)
{
  Language_COBOLDetails *lgcoboldet = LANGUAGE_COBOL_GET_PRIVATE(lgcobol);
  if (!lgcoboldet->completion_timer_set) {
    gint delay;
    g_object_get(lgcoboldet->prefmg, "autocomplete_delay", &delay, NULL);
    Calltip_Data *dat = g_slice_new(Calltip_Data);
    dat->lgcobol = lgcobol;
    dat->old_pos = pos;
    lgcoboldet->completion_timer_id = g_timeout_add(delay, auto_complete_callback, dat);
    lgcoboldet->completion_timer_set=TRUE;
  }
}

static gboolean IsOpenBrace(gchar ch)
{
	return ch == '[' || ch == '(' || ch == '{';
}

static void InsertCloseBrace (GtkScintilla *scintilla, gint current_pos, gchar ch)
{
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

static void autoindent_brace_code (GtkScintilla *sci, PreferencesManager *pref)
{
  gint current_pos;
  gint current_line;
  gint previous_line;
  gint previous_line_indentation;
  gint previous_line_end;

  current_pos = gtk_scintilla_get_current_pos(sci);
  current_line = gtk_scintilla_line_from_position(sci, current_pos);

  gphpedit_debug (DEBUG_DOCUMENT);

  if (current_line>0) {
    gtk_scintilla_begin_undo_action(sci);
    previous_line = current_line-1;
    previous_line_indentation = gtk_scintilla_get_line_indentation(sci, previous_line);

    previous_line_end = gtk_scintilla_get_line_end_position(sci, previous_line);
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

static void language_cobol_trigger_completion (Language_Provider *lgcobol, guint ch)
{
  g_return_if_fail(lgcobol);
  Language_COBOLDetails *lgcoboldet = LANGUAGE_COBOL_GET_PRIVATE(lgcobol);
  gint current_pos;
  gchar *member_function_buffer = NULL;

  current_pos = gtk_scintilla_get_current_pos(lgcoboldet->sci);
  gboolean auto_brace;
  g_object_get(lgcoboldet->prefmg, "auto_complete_braces", &auto_brace, NULL);

  if (IsOpenBrace(ch) && auto_brace) {
    InsertCloseBrace (lgcoboldet->sci, current_pos, ch);
    return;
  }

  switch(ch) {
    case ('\r'):
    case ('\n'):
        autoindent_brace_code (lgcoboldet->sci, lgcoboldet->prefmg);
        break;
    default:
        member_function_buffer = documentable_get_current_word(lgcoboldet->doc);
        if (member_function_buffer && strlen(member_function_buffer)>=3) show_autocompletion (LANGUAGE_COBOL(lgcobol), current_pos);
        g_free(member_function_buffer);
  }
}
