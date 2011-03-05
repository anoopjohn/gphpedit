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
#include "language_php.h"
#include "preferences_manager.h"
#include "language_provider.h"
#include "symbol_manager.h"

/*
* language_php private struct
*/
struct Language_PHPDetails
{
  Documentable *doc;
  GtkScintilla *sci;
  PreferencesManager *prefmg;
  SymbolManager *symbolmg;

  /* calltip stuff*/
  guint calltip_timer_id;
  gboolean calltip_timer_set;
  /*completion stuff*/
  guint completion_timer_id;
  gboolean completion_timer_set;
};

#define LANGUAGE_PHP_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    LANGUAGE_PHP_TYPE,\
					    Language_PHPDetails))
enum
{
  PROP_0,
  PROP_DOCUMENT_SCINTILLA
};

static void language_php_language_provider_init(Language_ProviderInterface *iface, gpointer user_data);
static void language_php_trigger_completion (Language_Provider *lgphp, guint ch);
static void show_calltip (Language_Provider *lgphp);

G_DEFINE_TYPE_WITH_CODE(Language_PHP, language_php, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE (IFACE_TYPE_LANGUAGE_PROVIDER,
                                                 language_php_language_provider_init));

static void language_php_language_provider_init(Language_ProviderInterface *iface, gpointer user_data)
{
  iface->trigger_completion = language_php_trigger_completion;
  iface->show_calltip = show_calltip;
}

static void
language_php_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
  Language_PHPDetails *lgphpdet;
  lgphpdet = LANGUAGE_PHP_GET_PRIVATE(object);

  switch (prop_id)
  {
    case PROP_DOCUMENT_SCINTILLA:
      if (lgphpdet->doc) g_object_unref(lgphpdet->doc);
      lgphpdet->doc = g_value_dup_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
language_php_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
  Language_PHPDetails *lgphpdet;
  lgphpdet = LANGUAGE_PHP_GET_PRIVATE(object);

  
  switch (prop_id)
  {
    case PROP_DOCUMENT_SCINTILLA:
      g_value_set_object (value, lgphpdet->doc);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}


/*
* disposes the Gobject
*/
static void language_php_dispose (GObject *object)
{
  Language_PHPDetails *lgphpdet;
  lgphpdet = LANGUAGE_PHP_GET_PRIVATE(object);
  g_object_unref(lgphpdet->prefmg);
  g_object_unref(lgphpdet->doc);
  g_object_unref(lgphpdet->symbolmg);
  /* Chain up to the parent class */
  G_OBJECT_CLASS (language_php_parent_class)->dispose (object);
}

static void language_php_constructed (GObject *object)
{
  Language_PHPDetails *lgphpdet;
  lgphpdet = LANGUAGE_PHP_GET_PRIVATE(object);
  GtkScintilla *sci;
  g_object_get(lgphpdet->doc, "scintilla", &sci, NULL);
  lgphpdet->sci = sci;
}

static void
language_php_class_init (Language_PHPClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = language_php_dispose;
  object_class->set_property = language_php_set_property;
  object_class->get_property = language_php_get_property;
  object_class->constructed = language_php_constructed;
  g_object_class_install_property (object_class,
                              PROP_DOCUMENT_SCINTILLA,
                              g_param_spec_object ("document_scintilla",
                              NULL, NULL,
                              DOCUMENT_SCINTILLA_TYPE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_type_class_add_private (klass, sizeof (Language_PHPDetails));
}

static void
language_php_init (Language_PHP * object)
{
  Language_PHPDetails *lgphpdet;
  lgphpdet = LANGUAGE_PHP_GET_PRIVATE(object);
  lgphpdet->prefmg = preferences_manager_new ();
  lgphpdet->symbolmg = symbol_manager_new ();
}

Language_PHP *language_php_new (Document_Scintilla *doc)
{
  Language_PHP *lgphp;
  lgphp = g_object_new (LANGUAGE_PHP_TYPE, "document_scintilla", doc, NULL);
  return lgphp; /* return new object */
}

typedef struct {
 Language_PHP *lgphp;
 gint old_pos;
} Calltip_Data;

static gboolean auto_complete_callback(gpointer data)
{
  Calltip_Data *dat = (Calltip_Data *) data;
  Language_PHPDetails *lgphpdet = LANGUAGE_PHP_GET_PRIVATE(dat->lgphp);
  gint current_pos;
  current_pos = documentable_get_current_position(lgphpdet->doc);
  GtkScintilla *scintilla = lgphpdet->sci;
  if (current_pos == dat->old_pos) {
    gchar *prefix = documentable_get_current_word(lgphpdet->doc);
    gchar *calltip = symbol_manager_get_symbols_matches (lgphpdet->symbolmg, prefix, SYMBOL_FUNCTION | SYMBOL_CLASS | SYMBOL_VAR, TAB_PHP);
    if (calltip && strlen(calltip)!=0) gtk_scintilla_user_list_show(GTK_SCINTILLA(scintilla), 1, calltip);
    g_free(prefix);
    g_free(calltip);
  }
  g_slice_free(Calltip_Data, dat);
  lgphpdet->completion_timer_set=FALSE;
  return FALSE;
}

static void show_autocompletion (Language_PHP *lgphp, gint pos)
{
  Language_PHPDetails *lgphpdet = LANGUAGE_PHP_GET_PRIVATE(lgphp);
  if (!lgphpdet->completion_timer_set) {
    gint delay;
    g_object_get(lgphpdet->prefmg, "autocomplete_delay", &delay, NULL);
    Calltip_Data *dat = g_slice_new(Calltip_Data);
    dat->lgphp = lgphp;
    dat->old_pos = pos;
    lgphpdet->completion_timer_id = g_timeout_add(delay, auto_complete_callback, dat);
    lgphpdet->completion_timer_set=TRUE;
  }
}

static gboolean auto_memberfunc_complete_callback(gpointer data)
{
  Calltip_Data *dat = (Calltip_Data *) data;
  Language_PHPDetails *lgphpdet = LANGUAGE_PHP_GET_PRIVATE(dat->lgphp);
  gint current_pos;
  current_pos = documentable_get_current_position(lgphpdet->doc);
  GtkScintilla *scintilla = lgphpdet->sci;
  if (current_pos == dat->old_pos) {
    gchar *prefix = documentable_get_current_word(lgphpdet->doc);
    gchar *calltip = symbol_manager_get_symbols_matches (lgphpdet->symbolmg, prefix, SYMBOL_MEMBER, TAB_PHP);
    if (calltip && strlen(calltip)!=0) gtk_scintilla_user_list_show(GTK_SCINTILLA(scintilla), 1, calltip);
//    gtk_scintilla_autoc_show(GTK_SCINTILLA(docdet->scintilla), current_pos, calltip);
    g_free(prefix);
  }
  g_slice_free(Calltip_Data, dat);
  lgphpdet->completion_timer_set=FALSE;
  return FALSE;
}

static void autocomplete_member(Language_PHP *lgphp, gint pos)
{
  Language_PHPDetails *lgphpdet = LANGUAGE_PHP_GET_PRIVATE(lgphp);
  if (!lgphpdet->completion_timer_set) {
    gint delay;
    g_object_get(lgphpdet->prefmg, "autocomplete_delay", &delay, NULL);
    Calltip_Data *dat = g_slice_new(Calltip_Data);
    dat->lgphp = lgphp;
    dat->old_pos = pos;
    lgphpdet->completion_timer_id = g_timeout_add(delay, auto_memberfunc_complete_callback, dat);
    lgphpdet->completion_timer_set=TRUE;
  }
}

static gboolean auto_complete_php_variables_callback(gpointer data)
{
  Calltip_Data *dat = (Calltip_Data *) data;
  Language_PHPDetails *lgphpdet = LANGUAGE_PHP_GET_PRIVATE(dat->lgphp);
  gint current_pos;
  current_pos = documentable_get_current_position(lgphpdet->doc);
  GtkScintilla *scintilla = lgphpdet->sci;
  if (current_pos == dat->old_pos) {
    gchar *prefix = documentable_get_current_word(lgphpdet->doc);
    gchar *result = symbol_manager_get_symbols_matches (lgphpdet->symbolmg, prefix, SYMBOL_VAR, TAB_PHP);
    if (result) gtk_scintilla_user_list_show(GTK_SCINTILLA(scintilla), 1, result);
    g_free(prefix);
  }
  g_slice_free(Calltip_Data, dat);
  lgphpdet->completion_timer_set=FALSE;
  return FALSE;
}


static void autocomplete_var(Language_PHP *lgphp, gint pos)
{
  Language_PHPDetails *lgphpdet = LANGUAGE_PHP_GET_PRIVATE(lgphp);
  if (!lgphpdet->completion_timer_set) {
    gint delay;
    g_object_get(lgphpdet->prefmg, "autocomplete_delay", &delay, NULL);
    Calltip_Data *dat = g_slice_new(Calltip_Data);
    dat->lgphp = lgphp;
    dat->old_pos = pos;
    lgphpdet->completion_timer_id = g_timeout_add(delay, auto_complete_php_variables_callback, dat);
    lgphpdet->completion_timer_set=TRUE;
  }
}

static gboolean auto_complete_classes_callback(gpointer data)
{
  Calltip_Data *dat = (Calltip_Data *) data;
  Language_PHPDetails *lgphpdet = LANGUAGE_PHP_GET_PRIVATE(dat->lgphp);
  gint current_pos;
  current_pos = documentable_get_current_position(lgphpdet->doc);
  GtkScintilla *scintilla = lgphpdet->sci;
  if (current_pos == dat->old_pos) {
    gchar *autocomp = symbol_manager_get_classes (lgphpdet->symbolmg, TAB_PHP);
    if (autocomp) gtk_scintilla_user_list_show(GTK_SCINTILLA(scintilla), 2, autocomp);
    g_free(autocomp); /*release resources*/
  }
  g_slice_free(Calltip_Data, dat);
  lgphpdet->completion_timer_set = FALSE;
  return FALSE;
}

static void autocomplete_class(Language_PHP *lgphp, gint pos)
{
  Language_PHPDetails *lgphpdet = LANGUAGE_PHP_GET_PRIVATE(lgphp);
  gtk_scintilla_insert_text(lgphpdet->sci, pos," ");
  gtk_scintilla_goto_pos(lgphpdet->sci, pos + 1);
  lgphpdet->completion_timer_set = TRUE;
  Calltip_Data *dat = g_slice_new(Calltip_Data);
  dat->lgphp = lgphp;
  dat->old_pos = pos + 1;
  auto_complete_classes_callback(dat);
}

static gboolean calltip_callback(gpointer data)
{
  Calltip_Data *dat = (Calltip_Data *) data;
  Language_PHPDetails *lgphpdet = LANGUAGE_PHP_GET_PRIVATE(dat->lgphp);
  gint current_pos;
  current_pos = documentable_get_current_position(lgphpdet->doc);
  GtkScintilla *scintilla = lgphpdet->sci;
  if (current_pos == dat->old_pos) {
    gchar *prefix = documentable_get_current_word(lgphpdet->doc);
    gchar *calltip = symbol_manager_get_calltip (lgphpdet->symbolmg, prefix, TAB_PHP);
    if (calltip){
      gtk_scintilla_call_tip_show(GTK_SCINTILLA(scintilla), current_pos, calltip);
      g_free(calltip);
    }
    g_free(prefix);
  }
  g_slice_free(Calltip_Data, dat);
  lgphpdet->calltip_timer_set = FALSE;
  return FALSE;
}

static void show_calltip (Language_Provider *lgphp)
{
  Language_PHPDetails *lgphpdet = LANGUAGE_PHP_GET_PRIVATE(lgphp);
  if (!lgphpdet->calltip_timer_set) {
    gint delay;
    g_object_get(lgphpdet->prefmg, "calltip_delay", &delay, NULL);
    Calltip_Data *dat = g_slice_new(Calltip_Data);
    dat->lgphp = LANGUAGE_PHP(lgphp);
    dat->old_pos = documentable_get_current_position(lgphpdet->doc);
    lgphpdet->calltip_timer_id = g_timeout_add(delay, calltip_callback, dat);
    lgphpdet->calltip_timer_set = TRUE;
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

static gboolean is_php_char_autoindent(char ch)
{
  return (ch == '{');
}

static gboolean is_php_char_autounindent(char ch)
{
  return (ch == '}');
}

static void autoindent_brace_code (GtkScintilla *sci, PreferencesManager *pref)
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

  current_pos = gtk_scintilla_get_current_pos(sci);
  current_line = gtk_scintilla_line_from_position(sci, current_pos);

  gphpedit_debug (DEBUG_DOCUMENT);

  if (current_line>0) {
    gtk_scintilla_begin_undo_action(sci);
    previous_line = current_line-1;
    previous_line_indentation = gtk_scintilla_get_line_indentation(sci, previous_line);

    previous_line_end = gtk_scintilla_get_line_end_position(sci, previous_line);
    previous_char_buffer = gtk_scintilla_get_text_range (sci, previous_line_end-1, previous_line_end, &previous_char_buffer_length);
    if (is_php_char_autoindent(*previous_char_buffer)) {
      gint indentation_size;
      g_object_get(pref, "indentation_size", &indentation_size, NULL);
      previous_line_indentation+=indentation_size;
    } else if (is_php_char_autounindent(*previous_char_buffer)) {
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
        if (!(g_ascii_iscntrl(c) || g_ascii_isspace(c) || is_php_char_autounindent(c))) {
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

/*
 * gboolean check_variable_before(const gchar *line_text)
 * check if there is a valid php variable has suffix in the gchar input
 * something like this "p$sk->" return FALSE
 * $this-> return TRUE
 * $var($this-> return TRUE
 * $var[$this-> return TRUE
 */
static gboolean check_php_variable_before(const gchar *line_text)
{
  gboolean r=FALSE;
  int i;
  if (!strchr(line_text,'$')) return r;
  for (i=strlen(line_text)-1;i>=0;i--){
    if (*(line_text+i)==';') break;
    if (*(line_text+i)==' ') break;
    if (*(line_text+i)=='$' && (*(line_text+i-1)==' ' || *(line_text+i-1)=='(' || *(line_text+i-1)=='[' || i==0)){  
    r=TRUE; 
    break;
    }
  }
  return r;
}

static void language_php_trigger_completion (Language_Provider *lgphp, guint ch)
{
  g_return_if_fail(lgphp);
  Language_PHPDetails *lgphpdet = LANGUAGE_PHP_GET_PRIVATE(lgphp);
  gint current_pos;
  gint wordStart;
  gint wordEnd;
  gint current_word_length;
  gint current_line;
  gchar *ac_buffer = NULL;
  gint ac_length;
  gint prev_char;
  gchar *member_function_buffer = NULL;
  guint style;
  current_pos = gtk_scintilla_get_current_pos(lgphpdet->sci);
  current_line = gtk_scintilla_line_from_position(lgphpdet->sci, current_pos);
  wordStart = gtk_scintilla_word_start_position(lgphpdet->sci, current_pos-1, TRUE);
  wordEnd = gtk_scintilla_word_end_position(lgphpdet->sci, current_pos-1, TRUE);
  current_word_length = wordEnd - wordStart;
  style = gtk_scintilla_get_style_at(lgphpdet->sci, current_pos);

  gboolean auto_brace;
  g_object_get(lgphpdet->prefmg, "auto_complete_braces", &auto_brace, NULL);

  if (IsOpenBrace(ch) && auto_brace) {
    InsertCloseBrace (lgphpdet->sci, current_pos, ch);
    return;
  }

  switch(ch) {
    case ('\r'):
    case ('\n'):
        autoindent_brace_code (lgphpdet->sci, lgphpdet->prefmg);
        break;
    case (')'):
        cancel_calltip (lgphpdet->sci);
        break;
    case ('('):
        show_calltip (lgphp);
        break;
     case ('>'):
     case (':'):
       prev_char = gtk_scintilla_get_char_at(lgphpdet->sci, current_pos-2);
       if ((prev_char=='-' && ch =='>') || (prev_char==':' && ch ==':')) {
         /*search back for a '$' in that line */
         gint initial_pos= gtk_scintilla_position_from_line(lgphpdet->sci, current_line);
         gint line_size;
         gchar *line_text = gtk_scintilla_get_text_range (lgphpdet->sci, initial_pos, wordStart-1, &line_size);
         if (!check_php_variable_before(line_text)) break;
         autocomplete_member(LANGUAGE_PHP(lgphp), current_pos);
       }
       break;
    default:
        member_function_buffer = documentable_get_current_word(lgphpdet->doc);
        if (g_str_has_prefix(member_function_buffer,"$") || g_str_has_prefix(member_function_buffer,"__")) {
          autocomplete_var(LANGUAGE_PHP(lgphp), current_pos);
        } else if (g_strcmp0(member_function_buffer,"instanceof")==0 || g_strcmp0(member_function_buffer,"is_subclass_of")==0) {
          autocomplete_class(LANGUAGE_PHP(lgphp), current_pos);
        } else if (current_word_length>=3) {
        // check to see if they've typed <?php and if so do nothing
        if (wordStart>1) {
          ac_buffer = gtk_scintilla_get_text_range (lgphpdet->sci, wordStart-2, wordEnd, &ac_length);
          if (g_strcmp0(ac_buffer,"<?php")==0) {
            g_free(ac_buffer);
            break;
          }
          g_free(ac_buffer);
        }
        show_autocompletion (LANGUAGE_PHP(lgphp), current_pos);
        }
        g_free(member_function_buffer);
  }
}
