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

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "debug.h"
#include "language_php.h"
#include "preferences_manager.h"
#include "language_provider.h"
#include "symbol_manager.h"
#include "main_window.h"
#include "gvfs_utils.h"

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
static void language_php_setup_lexer(Language_Provider *lgphp);
static gchar *language_php_do_syntax_check(Language_Provider *lgphp);

G_DEFINE_TYPE_WITH_CODE(Language_PHP, language_php, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE (IFACE_TYPE_LANGUAGE_PROVIDER,
                                                 language_php_language_provider_init));

static void language_php_language_provider_init(Language_ProviderInterface *iface, gpointer user_data)
{
  iface->trigger_completion = language_php_trigger_completion;
  iface->show_calltip = show_calltip;
  iface->setup_lexer = language_php_setup_lexer;
  iface->do_syntax_check = language_php_do_syntax_check;
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
  current_pos = gtk_scintilla_get_current_pos(lgphpdet->sci);
  current_line = gtk_scintilla_line_from_position(lgphpdet->sci, current_pos);
  wordStart = gtk_scintilla_word_start_position(lgphpdet->sci, current_pos-1, TRUE);
  wordEnd = gtk_scintilla_word_end_position(lgphpdet->sci, current_pos-1, TRUE);
  current_word_length = wordEnd - wordStart;

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

/*
* process_php_lines
*/
static gchar *process_php_lines(gchar *output)
{
  gchar *copy;
  gchar *token;
  gchar *line_number;
  GString *result;
  copy = output;
  result = g_string_new (NULL);
  copy=g_strdup_printf("\n%s", output);
  while ((token = strtok(copy, "\n"))) {
      if (g_str_has_prefix(token, "PHP Warning:  ")){
        token+=14; /* skip 'PHP Warning:  '*/
        line_number = strstr(token, "line ");
        if (line_number){
        line_number+=5; /* len of 'line '*/
        }
        gint num=atoi(line_number);
        if (num>0) {
          g_string_append_printf (result, "%d W %s\n", num, token);
        } else {
          g_string_append_printf (result, "%s\n", token);
        }
      } else if (g_str_has_prefix(token,"PHP Parse error:  syntax error, ")){
        token+=32; /* skip 'PHP Parse error:  syntax error, ' */
        line_number = strrchr(token, ' ');
        line_number++;
        if (atoi(line_number)>0) {
          g_string_append_printf (result, "%d E %s\n", atoi(line_number), token);
        } else {
          g_string_append_printf (result, "%s\n", token);
        }
      } else {
         if (!g_str_has_prefix(token, "Content-type")){
           g_string_append_printf (result, "%s\n", token);
         } 
      }
      copy = NULL;
    }
  gphpedit_debug_message(DEBUG_SYNTAX, "result:%s\n", result->str);
  return g_string_free (result,FALSE);

}

/*
* save_as_temp_file (internal)
* save the content of an editor and return the filename of the temp file or NULL on error.
*/
static GString *save_as_temp_file(Documentable *document)
{
  gphpedit_debug(DEBUG_SYNTAX);
  gchar *write_buffer = documentable_get_text(document);
  GString *filename = text_save_as_temp_file(write_buffer);
  g_free(write_buffer);
  return filename;
}

static GString *get_syntax_filename(Documentable *document, gboolean *using_temp)
{
  GString *filename = NULL;
  gchar *docfilename = documentable_get_filename(document);
  gboolean untitled, saved;
  g_object_get(document, "untitled", &untitled, "saved", &saved, NULL);
  if (saved && filename_is_native(docfilename) && !untitled) {
    gchar *local_path = filename_get_scaped_path(docfilename);
    filename = g_string_new(local_path);
    g_free(local_path);
    *using_temp = FALSE;
  } else {
    filename = save_as_temp_file(document);
    *using_temp = TRUE;
  }
  g_free(docfilename);
  return filename;
}

static gchar *language_php_do_syntax_check(Language_Provider *lgphp)
{
  g_return_val_if_fail(lgphp, NULL);
  Language_PHPDetails *lgphpdet = LANGUAGE_PHP_GET_PRIVATE(lgphp);

  GString *command_line=NULL;
  gchar *output;
  gboolean using_temp;
  GString *filename = NULL;
  const gchar *php_binary_location;

  command_line = g_string_new(NULL);
  g_object_get(lgphpdet->prefmg, "php_binary_location", &php_binary_location, NULL);
  filename = get_syntax_filename(lgphpdet->doc, &using_temp);
  g_string_append_printf (command_line, "%s -q -l -d html_errors=Off -f '%s'", php_binary_location, filename->str);

  output = command_spawn_with_error (command_line->str);

//  gphpedit_debug_message(DEBUG_SYNTAX, "eject:%s\n", command_line->str); //FIXME
  if (using_temp) release_temp_file (filename->str);
  g_string_free(filename, TRUE);
  g_string_free(command_line, TRUE);
  gchar *result=NULL;
  if (output) {
    result = process_php_lines(output);
    g_free(output);
  } else {
    result = g_strdup(_("Error calling PHP CLI (is PHP command line binary installed? If so, check if it's in your path or set php_binary in Preferences)\n"));
  }
  return result;
}

static void language_php_setup_lexer(Language_Provider *lgphp)
{
  g_return_if_fail(lgphp);
  Language_PHPDetails *lgphpdet = LANGUAGE_PHP_GET_PRIVATE(lgphp);

  gtk_scintilla_clear_document_style (lgphpdet->sci);
  gtk_scintilla_set_lexer(lgphpdet->sci, SCLEX_HTML);
  gtk_scintilla_set_style_bits(lgphpdet->sci, 7);

  gtk_scintilla_set_keywords(lgphpdet->sci, 0, "__construct __destruct a abbr acronym address applet area b base basefont bdo big blockquote body br button caption center cite code col colgroup dd del dfn dir div dl dt em fieldset font form frame frameset h1 h2 h3 h4 h5 h6 head hr html i iframe img input ins isindex kbd label legend li link map menu meta noframes noscript object ol optgroup option p param pre q s samp script select small span strike strong style sub sup table tbody td textarea tfoot th thead title tr tt u ul var xml xmlns abbr accept-charset accept accesskey action align alink alt archive axis background bgcolor border cellpadding cellspacing char charoff charset checked cite class classid clear codebase codetype color cols colspan compact content coords data datafld dataformatas datapagesize datasrc datetime declare defer dir disabled enctype event face for frame frameborder headers height href hreflang hspace http-equiv id ismap label lang language leftmargin link longdesc marginwidth marginheight maxlength media method multiple name nohref noresize noshade nowrap object onblur onchange onclick ondblclick onfocus onkeydown onkeypress onkeyup onload onmousedown onmousemove onmouseover onmouseout onmouseup onreset onselect onsubmit onunload profile prompt readonly rel rev rows rowspan rules scheme scope selected shape size span src standby start style summary tabindex target text title topmargin type usemap valign value valuetype version vlink vspace width text password checkbox radio submit reset file hidden image public !doctype article aside calendar canvas card command commandset datagrid datatree footer gauge header m menubar menulabel nav progress section switch tabbox");
  gtk_scintilla_set_keywords(lgphpdet->sci, 1, "abstract boolean break byte case catch char class const continue debugger default delete do double else enum export extends final finally float for goto if implements import in include include_once instanceof int interface long native new package private protected public return require require_once short static super switch synchronized this throw throws transient try typeof var void volatile while with constructor prototype Infinity NaN Math NEGATIVE_INFINITY POSITIVE_INFINITY null undefined false true");

  gtk_scintilla_set_keywords(lgphpdet->sci, 3, "and assert break class continue def del elif else except exec finally for from global if import in is lambda None not or pass print raise return try while");
  gtk_scintilla_set_keywords(lgphpdet->sci, 4, "abstract boolean break byte case catch char class const continue debugger default delete do double else enum export extends final finally float for goto if implements import in include include_once instanceof int interface long native new package private protected public return require require_once short static super switch synchronized this throw throws transient try typeof var void volatile while withand assert break class continue def del elif else except exec finally for from global if import in is lambda None not or pass print raise return try while__construct __destruct __clone __get __set __call __autoload abstract interface implements namespace private protected public final const instanceof static try throw catch exception as extends break for foreach continue if else elseif endif do while switch include_once require_once mssql_field_length mssql_field_name mssql_field_type case function class new static global define __LINE__ __FILE__ PHP_VERSION PHP_OS TRUE FALSE E_ERROR E_WARNING E_PARSE E_NOTICE CASE_UPPER CASE_LOWER NULL int integer real double float string array object var abs acos acosh addcslashes addslashes apache_child_terminate apache_lookup_uri apache_note apache_setenv array array_change_key_case array_chunk array_count_values array_diff array_fill array_filter array_flip array_intersect array_key_exists array_keys array_map array_merge array_merge_recursive array_multisort array_pad array_pop array_push array_rand array_reduce array_reverse array_search array_shift array_slice array_splice array_sum array_unique array_unshift array_values array_walk arsort ascii2ebcdic asin asinh asort aspell_check aspell_check_raw aspell_new aspell_suggest assert assert_options atan atan2 atanh base64_decode base64_encode base_convert basename bcadd bccomp bcdiv bcmod bcmul bcpow bcscale bcsqrt bcsub bin2hex bind_textdomain_codeset bindec bindtextdomain bzclose bzcompress bzdecompress bzerrno bzerror bzerrstr bzflush bzopen bzread bzwrite cal_days_in_month cal_from_jd cal_info cal_to_jd call_user_func call_user_func_array call_user_method call_user_method_array ccvs_add ccvs_auth ccvs_command ccvs_count ccvs_delete ccvs_done ccvs_init ccvs_lookup ccvs_new ccvs_report ccvs_return ccvs_reverse ccvs_sale ccvs_status ccvs_textvalue ccvs_void ceil chdir checkdate checkdnsrr chgrp chmod chop chown chr chroot chunk_split class_exists clearstatcache closedir closelog com com_addref com_get com_invoke com_isenum com_load com_load_typelib com_propget com_propput com_propset com_release com_set compact connection_aborted connection_status connection_timeout constant convert_cyr_string copy cos cosh count count_chars cpdf_add_annotation cpdf_add_outline cpdf_arc cpdf_begin_text cpdf_circle cpdf_clip cpdf_close cpdf_closepath cpdf_closepath_fill_stroke cpdf_closepath_stroke cpdf_continue_text cpdf_curveto cpdf_end_text cpdf_fill cpdf_fill_stroke cpdf_finalize cpdf_finalize_page cpdf_global_set_document_limits cpdf_import_jpeg cpdf_lineto cpdf_moveto cpdf_newpath cpdf_open cpdf_output_buffer cpdf_page_init cpdf_place_inline_image cpdf_rect cpdf_restore cpdf_rlineto cpdf_rmoveto cpdf_rotate cpdf_rotate_text cpdf_save cpdf_save_to_file cpdf_scale cpdf_set_action_url cpdf_set_char_spacing cpdf_set_creator cpdf_set_current_page cpdf_set_font cpdf_set_font_directories cpdf_set_font_map_file cpdf_set_horiz_scaling cpdf_set_keywords cpdf_set_leading cpdf_set_page_animation cpdf_set_subject cpdf_set_text_matrix cpdf_set_text_pos cpdf_set_text_rendering cpdf_set_text_rise cpdf_set_title cpdf_set_viewer_preferences cpdf_set_word_spacing cpdf_setdash cpdf_setflat cpdf_setgray cpdf_setgray_fill cpdf_setgray_stroke cpdf_setlinecap cpdf_setlinejoin cpdf_setlinewidth cpdf_setmiterlimit cpdf_setrgbcolor cpdf_setrgbcolor_fill cpdf_setrgbcolor_stroke cpdf_show cpdf_show_xy cpdf_stringwidth cpdf_stroke cpdf_text cpdf_translate crack_check crack_closedict crack_getlastmessage crack_opendict crc32 create_function crypt ctype_alnum ctype_alpha ctype_cntrl ctype_digit ctype_graph ctype_lower ctype_print ctype_punct ctype_space ctype_upper ctype_xdigit curl_close curl_errno curl_error curl_exec curl_getinfo curl_init curl_setopt curl_version current cybercash_base64_decode cybercash_base64_encode cybercash_decr cybercash_encr cybermut_creerformulairecm cybermut_creerreponsecm cybermut_testmac cyrus_authenticate cyrus_bind cyrus_close cyrus_connect cyrus_query cyrus_unbind date dba_close dba_delete dba_exists dba_fetch dba_firstkey dba_insert dba_nextkey dba_open dba_optimize dba_popen dba_replace dba_sync dbase_add_record dbase_close dbase_create dbase_delete_record dbase_get_record dbase_get_record_with_names dbase_numfields dbase_numrecords dbase_open dbase_pack dbase_replace_record dblist dbmclose dbmdelete dbmexists dbmfetch dbmfirstkey dbminsert dbmnextkey dbmopen dbmreplace dbplus_add dbplus_aql dbplus_chdir dbplus_close dbplus_curr dbplus_errcode dbplus_errno dbplus_find dbplus_first dbplus_flush dbplus_freealllocks dbplus_freelock dbplus_freerlocks dbplus_getlock dbplus_getunique dbplus_info dbplus_last dbplus_lockrel dbplus_next dbplus_open dbplus_prev dbplus_rchperm dbplus_rcreate dbplus_rcrtexact dbplus_rcrtlike dbplus_resolve dbplus_restorepos dbplus_rkeys dbplus_ropen dbplus_rquery dbplus_rrename dbplus_rsecindex dbplus_runlink dbplus_rzap dbplus_savepos dbplus_setindex dbplus_setindexbynumber dbplus_sql dbplus_tcl dbplus_tremove dbplus_undo dbplus_undoprepare dbplus_unlockrel dbplus_unselect dbplus_update dbplus_xlockrel dbplus_xunlockrel dbx_close dbx_compare dbx_connect dbx_error dbx_query dbx_sort dcgettext dcngettext dcngettext debugger_off debugger_on decbin dechex decoct define define_syslog_variables defined deg2rad delete dgettext die dio_close dio_fcntl dio_open dio_read dio_seek dio_stat dio_truncate dio_write dir dirname disk_free_space disk_total_space diskfreespace dl domxml_add_root domxml_attributes domxml_children domxml_dumpmem domxml_get_attribute domxml_new_child domxml_new_xmldoc domxml_node domxml_node_set_content domxml_node_unlink_node domxml_root domxml_set_attribute domxml_version dotnet_load doubleval each easter_date easter_days ebcdic2ascii echo empty end ereg ereg_replace eregi eregi_replace error_log error_reporting escapeshellarg escapeshellcmd eval exec exit exp explode expm1 extension_loaded extract ezmlm_hash fbsql_affected_rows fbsql_autocommit fbsql_change_user fbsql_close fbsql_commit fbsql_connect fbsql_create_blob fbsql_create_clob fbsql_create_db fbsql_data_seek fbsql_database fbsql_database_password fbsql_db_query fbsql_db_status fbsql_drop_db fbsql_errno fbsql_error fbsql_fetch_array fbsql_fetch_assoc fbsql_fetch_field fbsql_fetch_lengths fbsql_fetch_object fbsql_fetch_row fbsql_field_flags fbsql_field_len fbsql_field_name fbsql_field_seek fbsql_field_table fbsql_field_type fbsql_free_result fbsql_get_autostart_info fbsql_hostname fbsql_insert_id fbsql_list_dbs fbsql_list_fields fbsql_list_tables fbsql_next_result fbsql_num_fields fbsql_num_rows fbsql_password fbsql_pconnect fbsql_query fbsql_read_blob fbsql_read_clob fbsql_result fbsql_rollback fbsql_select_db fbsql_set_lob_mode fbsql_set_transaction fbsql_start_db fbsql_stop_db fbsql_tablename fbsql_username fbsql_warnings fclose fdf_add_template fdf_close fdf_create fdf_get_file fdf_get_status fdf_get_value fdf_next_field_name fdf_open fdf_save fdf_set_ap fdf_set_encoding fdf_set_file fdf_set_flags fdf_set_javascript_action fdf_set_opt fdf_set_status fdf_set_submit_form_action fdf_set_value feof fflush fgetc fgetcsv fgets fgetss file file_exists fileatime filectime filegroup fileinode filemtime fileowner fileperms filepro filepro_fieldcount filepro_fieldname filepro_fieldtype filepro_fieldwidth filepro_retrieve filepro_rowcount filesize filetype floatval flock floor flush fopen fpassthru fputs fread frenchtojd fribidi_log2vis fscanf fseek fsockopen fstat ftell ftok ftp_cdup ftp_chdir ftp_connect ftp_delete ftp_exec ftp_fget ftp_fput ftp_get ftp_login ftp_mdtm ftp_mkdir ftp_nlist ftp_pasv ftp_put ftp_pwd ftp_quit ftp_rawlist ftp_rename ftp_rmdir ftp_site ftp_size ftp_systype ftruncate func_get_arg func_get_args func_num_args function_exists fwrite get_browser get_cfg_var get_class get_class_methods get_class_vars get_current_user get_declared_classes get_defined_constants get_defined_functions get_defined_vars get_extension_funcs get_html_translation_table get_included_files get_loaded_extensions get_magic_quotes_gpc get_magic_quotes_runtime get_meta_tags get_object_vars get_parent_class get_required_files get_resource_type getallheaders getcwd getdate getenv gethostbyaddr gethostbyname gethostbynamel getimagesize getlastmod getmxrr getmygid getmyinode getmypid getmyuid getprotobyname getprotobynumber getrandmax getrusage getservbyname getservbyport gettext gettimeofday gettype gmdate gmmktime gmp_abs gmp_add gmp_and gmp_clrbit gmp_cmp gmp_com gmp_div gmp_div_q gmp_div_qr gmp_div_r gmp_divexact gmp_fact gmp_gcd gmp_gcdext gmp_hamdist gmp_init gmp_intval gmp_invert gmp_jacobi gmp_legendre gmp_mod gmp_mul gmp_neg gmp_or gmp_perfect_square gmp_popcount gmp_pow gmp_powm gmp_prob_prime gmp_random gmp_scan0 gmp_scan1 gmp_setbit gmp_sign gmp_sqrt gmp_sqrtrm gmp_strval gmp_sub gmp_xor gmstrftime gregoriantojd gzclose gzcompress gzdeflate gzencode gzeof gzfile gzgetc gzgets gzgetss gzinflate gzopen gzpassthru gzputs gzread gzrewind gzseek gztell gzuncompress gzwrite header headers_sent hebrev hebrevc hexdec highlight_file highlight_string htmlentities htmlspecialchars hw_array2objrec hw_changeobject hw_children hw_childrenobj hw_close hw_connect hw_connection_info hw_cp hw_deleteobject hw_docbyanchor hw_docbyanchorobj hw_document_attributes hw_document_bodytag hw_document_content hw_document_setcontent hw_document_size hw_dummy hw_edittext hw_error hw_errormsg hw_free_document hw_getanchors hw_getanchorsobj hw_getandlock hw_getchildcoll hw_getchildcollobj hw_getchilddoccoll hw_getchilddoccollobj hw_getobject hw_getobjectbyquery hw_getobjectbyquerycoll hw_getobjectbyquerycollobj hw_getobjectbyqueryobj hw_getparents hw_getparentsobj hw_getrellink hw_getremote hw_getremotechildren hw_getsrcbydestobj hw_gettext hw_getusername hw_identify hw_incollections hw_info hw_inscoll hw_insdoc hw_insertanchors hw_insertdocument hw_insertobject hw_mapid hw_modifyobject hw_mv hw_new_document hw_objrec2array hw_output_document hw_pconnect hw_pipedocument hw_root hw_setlinkroot hw_stat hw_unlock hw_who hypot ibase_blob_add ibase_blob_cancel ibase_blob_close ibase_blob_create ibase_blob_echo ibase_blob_get ibase_blob_import ibase_blob_info ibase_blob_open ibase_close ibase_commit ibase_connect ibase_errmsg ibase_execute ibase_fetch_object ibase_fetch_row ibase_field_info ibase_free_query ibase_free_result ibase_num_fields ibase_pconnect ibase_prepare ibase_query ibase_rollback ibase_timefmt ibase_trans icap_close icap_create_calendar icap_delete_calendar icap_delete_event icap_fetch_event icap_list_alarms icap_list_events icap_open icap_rename_calendar icap_reopen icap_snooze icap_store_event iconv iconv_get_encoding iconv_set_encoding ifx_affected_rows ifx_blobinfile_mode ifx_byteasvarchar ifx_close ifx_connect ifx_copy_blob ifx_create_blob ifx_create_char ifx_do ifx_error ifx_errormsg ifx_fetch_row ifx_fieldproperties ifx_fieldtypes ifx_free_blob ifx_free_char ifx_free_result ifx_get_blob ifx_get_char ifx_getsqlca ifx_htmltbl_result ifx_nullformat ifx_num_fields ifx_num_rows ifx_pconnect ifx_prepare ifx_query ifx_textasvarchar ifx_update_blob ifx_update_char ifxus_close_slob ifxus_create_slob ifxus_free_slob ifxus_open_slob ifxus_read_slob ifxus_seek_slob ifxus_tell_slob ifxus_write_slob ignore_user_abort image2wbmp imagealphablending imagearc imagechar imagecharup imagecolorallocate imagecolorat imagecolorclosest imagecolorclosestalpha imagecolorclosestthwb imagecolordeallocate imagecolorexact imagecolorexactalpha imagecolorresolve imagecolorresolvealpha imagecolorset imagecolorsforindex imagecolorstotal imagecolortransparent imagecopy imagecopymerge imagecopymergegray imagecopyresampled imagecopyresized imagecreate imagecreatefromgd imagecreatefromgd2 imagecreatefromgd2part imagecreatefromgif imagecreatefromjpeg imagecreatefrompng imagecreatefromstring imagecreatefromwbmp imagecreatefromxbm imagecreatefromxpm imagecreatetruecolor imagedashedline imagedestroy imageellipse imagefill imagefilledarc imagefilledellipse imagefilledpolygon imagefilledrectangle imagefilltoborder imagefontheight imagefontwidth imageftbbox imagefttext imagegammacorrect imagegd imagegd2 imagegif imageinterlace imagejpeg imageline imageloadfont imagepalettecopy imagepng imagepolygon imagepsbbox imagepsencodefont imagepsextendfont imagepsfreefont imagepsloadfont imagepsslantfont imagepstext imagerectangle imagesetbrush imagesetpixel imagesetstyle imagesetthickness imagesettile imagestring imagestringup imagesx imagesy imagetruecolortopalette imagettfbbox imagettftext imagetypes imagewbmp imap_8bit imap_alerts imap_append imap_base64 imap_binary imap_body imap_bodystruct imap_check imap_clearflag_full imap_close imap_createmailbox imap_delete imap_deletemailbox imap_errors imap_expunge imap_fetch_overview imap_fetchbody imap_fetchheader imap_fetchstructure imap_get_quota imap_getmailboxes imap_getsubscribed imap_header imap_headerinfo imap_headers imap_last_error imap_listmailbox imap_listsubscribed imap_mail imap_mail_compose imap_mail_copy imap_mail_move imap_mailboxmsginfo imap_mime_header_decode imap_msgno imap_num_msg imap_num_recent imap_open imap_ping imap_popen imap_qprint imap_renamemailbox imap_reopen imap_rfc822_parse_adrlist imap_rfc822_parse_headers imap_rfc822_write_address imap_scanmailbox imap_search imap_set_quota imap_setacl imap_setflag_full imap_sort imap_status imap_subscribe imap_thread imap_uid imap_undelete imap_unsubscribe imap_utf7_decode imap_utf7_encode imap_utf8 implode import_request_variables in_array ingres_autocommit ingres_close ingres_commit ingres_connect ingres_fetch_array ingres_fetch_object ingres_fetch_row ingres_field_length ingres_field_name ingres_field_nullable ingres_field_precision ingres_field_scale ingres_field_type ingres_num_fields ingres_num_rows ingres_pconnect ingres_query ingres_rollback ini_alter ini_get ini_get_all ini_restore ini_set intval ip2long iptcparse ircg_channel_mode ircg_disconnect ircg_fetch_error_msg ircg_get_username ircg_html_encode ircg_ignore_add ircg_ignore_del ircg_is_conn_alive ircg_join ircg_kick ircg_lookup_format_messages ircg_msg ircg_nick ircg_nickname_escape ircg_nickname_escape ircg_notice ircg_part ircg_pconnect ircg_register_format_messages ircg_set_current ircg_set_file ircg_set_on_die ircg_topic ircg_whois is_array is_bool is_dir is_double is_executable is_file is_float is_int is_integer is_link is_long is_null is_numeric is_object is_readable is_real is_resource is_scalar is_string is_subclass_of is_uploaded_file is_writable is_writeable isset java_last_exception_clear java_last_exception_get jddayofweek jdmonthname jdtofrench jdtogregorian jdtojewish jdtojulian jdtounix jewishtojd join jpeg2wbmp juliantojd key krsort ksort lcg_value ldap_8859_to_t61 ldap_add ldap_bind ldap_close ldap_compare ldap_connect ldap_count_entries ldap_delete ldap_dn2ufn ldap_err2str ldap_errno ldap_error ldap_explode_dn ldap_first_attribute ldap_first_entry ldap_first_reference ldap_free_result ldap_get_attributes ldap_get_dn ldap_get_entries ldap_get_option ldap_get_values ldap_get_values_len ldap_list ldap_mod_add ldap_mod_del ldap_mod_replace ldap_modify ldap_next_attribute ldap_next_entry ldap_next_reference ldap_parse_reference ldap_parse_result ldap_read ldap_rename ldap_search ldap_set_option ldap_set_rebind_proc ldap_sort ldap_start_tls ldap_t61_to_8859 ldap_unbind leak levenshtein link linkinfo list localeconv localtime log log10 log1p long2ip lstat ltrim mail mailparse_determine_best_xfer_encoding mailparse_msg_create mailparse_msg_extract_part mailparse_msg_extract_part_file mailparse_msg_free mailparse_msg_get_part mailparse_msg_get_part_data mailparse_msg_get_structure mailparse_msg_parse mailparse_msg_parse_file mailparse_rfc822_parse_addresses mailparse_stream_encode mailparse_uudecode_all max mb_convert_encoding mb_convert_kana mb_convert_variables mb_decode_mimeheader mb_decode_numericentity mb_detect_encoding mb_detect_order mb_encode_mimeheader mb_encode_numericentity mb_http_input mb_http_output mb_internal_encoding mb_language mb_output_handler mb_parse_str mb_preferred_mime_name mb_send_mail mb_strcut mb_strimwidth mb_strlen mb_strpos mb_strrpos mb_strwidth mb_substitute_character mb_substr mcal_append_event mcal_close mcal_create_calendar mcal_date_compare mcal_date_valid mcal_day_of_week mcal_day_of_year mcal_days_in_month mcal_delete_calendar mcal_delete_event mcal_event_add_attribute mcal_event_init mcal_event_set_alarm mcal_event_set_category mcal_event_set_class mcal_event_set_description mcal_event_set_end mcal_event_set_recur_daily mcal_event_set_recur_monthly_mday mcal_event_set_recur_monthly_wday mcal_event_set_recur_none mcal_event_set_recur_weekly mcal_event_set_recur_yearly mcal_event_set_start mcal_event_set_title mcal_expunge mcal_fetch_current_stream_event mcal_fetch_event mcal_is_leap_year mcal_list_alarms mcal_list_events mcal_next_recurrence mcal_open mcal_popen mcal_rename_calendar mcal_reopen mcal_snooze mcal_store_event mcal_time_valid mcal_week_of_year mcrypt_cbc mcrypt_cfb mcrypt_create_iv mcrypt_decrypt mcrypt_ecb mcrypt_enc_get_algorithms_name mcrypt_enc_get_block_size mcrypt_enc_get_iv_size mcrypt_enc_get_key_size mcrypt_enc_get_modes_name mcrypt_enc_get_supported_key_sizes mcrypt_enc_is_block_algorithm mcrypt_enc_is_block_algorithm_mode mcrypt_enc_is_block_mode mcrypt_enc_self_test mcrypt_encrypt mcrypt_generic mcrypt_generic_deinit mcrypt_generic_end mcrypt_generic_init mcrypt_get_block_size mcrypt_get_cipher_name mcrypt_get_iv_size mcrypt_get_key_size mcrypt_list_algorithms mcrypt_list_modes mcrypt_module_close mcrypt_module_get_algo_block_size mcrypt_module_get_algo_key_size mcrypt_module_get_supported_key_sizes mcrypt_module_is_block_algorithm mcrypt_module_is_block_algorithm_mode mcrypt_module_is_block_mode mcrypt_module_open mcrypt_module_self_test mcrypt_ofb md5 md5_file mdecrypt_generic metaphone method_exists mhash mhash_count mhash_get_block_size mhash_get_hash_name mhash_keygen_s2k microtime min ming_setcubicthreshold ming_setscale ming_useswfversion mkdir mktime move_uploaded_file msession_connect msession_count msession_create msession_destroy msession_disconnect msession_find msession_get msession_get_array msession_getdata msession_inc msession_list msession_listvar msession_lock msession_plugin msession_randstr msession_set msession_set_array msession_setdata msession_timeout msession_uniq msession_unlock msql msql_affected_rows msql_close msql_connect msql_create_db msql_createdb msql_data_seek msql_dbname msql_drop_db msql_dropdb msql_error msql_fetch_array msql_fetch_field msql_fetch_object msql_fetch_row msql_field_seek msql_fieldflags msql_fieldlen msql_fieldname msql_fieldtable msql_fieldtype msql_free_result msql_freeresult msql_list_dbs msql_list_fields msql_list_tables msql_listdbs msql_listfields msql_listtables msql_num_fields msql_num_rows msql_numfields msql_numrows msql_pconnect msql_query msql_regcase msql_result msql_select_db msql_selectdb msql_tablename mssql_bind mssql_close mssql_connect mssql_data_seek mssql_execute mssql_fetch_array mssql_fetch_assoc mssql_fetch_batch mssql_fetch_field mssql_fetch_object mssql_fetch_row mssql_field_length mssql_field_name mssql_field_seek mssql_field_type mssql_free_result mssql_get_last_message mssql_guid_string mssql_init mssql_min_error_severity mssql_min_message_severity mssql_next_result mssql_num_fields mssql_num_rows mssql_pconnect mssql_query mssql_result mssql_rows_affected mssql_select_db mt_getrandmax mt_rand mt_srand muscat_close muscat_get muscat_give muscat_setup muscat_setup_net mysql_affected_rows mysql_change_user mysql_close mysql_connect mysql_create_db mysql_data_seek mysql_db_name mysql_db_query mysql_drop_db mysql_errno mysql_error mysql_escape_string mysql_fetch_array mysql_fetch_assoc mysql_fetch_field mysql_fetch_lengths mysql_fetch_object mysql_fetch_row mysql_field_flags mysql_field_len mysql_field_name mysql_field_seek mysql_field_table mysql_field_type mysql_free_result mysql_get_client_info mysql_get_host_info mysql_get_proto_info mysql_get_server_info mysql_insert_id mysql_list_dbs mysql_list_fields mysql_list_tables mysql_num_fields mysql_num_rows mysql_pconnect mysql_query mysql_result mysql_select_db mysql_tablename mysql_unbuffered_query natcasesort natsort ncurses_addch ncurses_addchnstr ncurses_addchstr ncurses_addnstr ncurses_addstr ncurses_assume_default_colors ncurses_attroff ncurses_attron ncurses_attrset ncurses_baudrate ncurses_beep ncurses_bkgd ncurses_bkgdset ncurses_border ncurses_can_change_color ncurses_cbreak ncurses_clear ncurses_clrtobot ncurses_clrtoeol ncurses_color_set ncurses_curs_set ncurses_def_prog_mode ncurses_def_shell_mode ncurses_define_key ncurses_delay_output ncurses_delch ncurses_deleteln ncurses_delwin ncurses_doupdate ncurses_echo ncurses_echochar ncurses_end ncurses_erase ncurses_erasechar ncurses_filter ncurses_flash ncurses_flushinp ncurses_getch ncurses_halfdelay ncurses_has_colors ncurses_has_ic ncurses_has_il ncurses_has_key ncurses_hline ncurses_inch ncurses_init ncurses_init_color ncurses_init_pair ncurses_insch ncurses_insdelln ncurses_insertln ncurses_insstr ncurses_instr ncurses_isendwin ncurses_keyok ncurses_killchar ncurses_mouseinterval ncurses_move ncurses_mvaddch ncurses_mvaddchnstr ncurses_mvaddchstr ncurses_mvaddnstr ncurses_mvaddstr ncurses_mvcur ncurses_mvdelch ncurses_mvgetch ncurses_mvhline ncurses_mvinch ncurses_mvvline ncurses_mvwaddstr ncurses_napms ncurses_newwin ncurses_nl ncurses_nocbreak ncurses_noecho ncurses_nonl ncurses_noqiflush ncurses_noraw ncurses_putp ncurses_qiflush ncurses_raw ncurses_refresh ncurses_resetty ncurses_savetty ncurses_scr_dump ncurses_scr_init ncurses_scr_restore ncurses_scr_set ncurses_scrl ncurses_slk_attr ncurses_slk_attroff ncurses_slk_attron ncurses_slk_attrset ncurses_slk_clear ncurses_slk_color ncurses_slk_init ncurses_slk_noutrefresh ncurses_slk_refresh ncurses_slk_restore ncurses_slk_touch ncurses_standend ncurses_standout ncurses_start_color ncurses_termattrs ncurses_timeout ncurses_typeahead ncurses_ungetch ncurses_use_default_colors ncurses_use_env ncurses_use_extended_names ncurses_vidattr ncurses_vline ncurses_wrefresh next ngettext nl2br nl_langinfo notes_body notes_copy_db notes_create_db notes_create_note notes_drop_db notes_find_note notes_header_info notes_list_msgs notes_mark_read notes_mark_unread notes_nav_create notes_search notes_unread notes_version number_format ob_clean ob_end_clean ob_end_flush ob_flush ob_get_contents ob_get_length ob_get_level ob_gzhandler ob_iconv_handler ob_implicit_flush ob_start ocibindbyname ocicancel ocicollappend ocicollassign ocicollassignelem ocicollgetelem ocicollmax ocicollsize ocicolltrim ocicolumnisnull ocicolumnname ocicolumnprecision ocicolumnscale ocicolumnsize ocicolumntype ocicolumntyperaw ocicommit ocidefinebyname ocierror ociexecute ocifetch ocifetchinto ocifetchstatement ocifreecollection ocifreecursor ocifreedesc ocifreestatement ociinternaldebug ociloadlob ocilogoff ocilogon ocinewcollection ocinewcursor ocinewdescriptor ocinlogon ocinumcols ociparse ociplogon ociresult ocirollback ocirowcount ocisavelob ocisavelobfile ociserverversion ocisetprefetch ocistatementtype ociwritelobtofile octdec odbc_autocommit odbc_binmode odbc_close odbc_close_all odbc_columnprivileges odbc_columns odbc_commit odbc_connect odbc_cursor odbc_do odbc_error odbc_errormsg odbc_exec odbc_execute odbc_fetch_array odbc_fetch_into odbc_fetch_object odbc_fetch_row odbc_field_len odbc_field_name odbc_field_num odbc_field_precision odbc_field_scale odbc_field_type odbc_foreignkeys odbc_free_result odbc_gettypeinfo odbc_longreadlen odbc_next_result odbc_num_fields odbc_num_rows odbc_pconnect odbc_prepare odbc_primarykeys odbc_procedurecolumns odbc_procedures odbc_result odbc_result_all odbc_rollback odbc_setoption odbc_specialcolumns odbc_statistics odbc_tableprivileges odbc_tables opendir openlog openssl_csr_export openssl_csr_export_to_file openssl_csr_new openssl_csr_sign openssl_error_string openssl_free_key openssl_get_privatekey openssl_get_publickey openssl_open openssl_pkcs7_decrypt openssl_pkcs7_encrypt openssl_pkcs7_sign openssl_pkcs7_verify openssl_pkey_export openssl_pkey_export_to_file openssl_pkey_new openssl_private_decrypt openssl_private_encrypt openssl_public_decrypt openssl_public_encrypt openssl_seal openssl_sign openssl_verify openssl_x509_check_private_key openssl_x509_checkpurpose openssl_x509_export openssl_x509_export_to_file openssl_x509_free openssl_x509_parse openssl_x509_read ora_bind ora_close ora_columnname ora_columnsize ora_columntype ora_commit ora_commitoff ora_commiton ora_do ora_error ora_errorcode ora_exec ora_fetch ora_fetch_into ora_getcolumn ora_logoff ora_logon ora_numcols ora_numrows ora_open ora_parse ora_plogon ora_rollback orbitenum orbitobject orbitstruct ord overload ovrimos_close ovrimos_commit ovrimos_connect ovrimos_cursor ovrimos_exec ovrimos_execute ovrimos_fetch_into ovrimos_fetch_row ovrimos_field_len ovrimos_field_name ovrimos_field_num ovrimos_field_type ovrimos_free_result ovrimos_longreadlen ovrimos_num_fields ovrimos_num_rows ovrimos_prepare ovrimos_result ovrimos_result_all ovrimos_rollback pack parse_ini_file parse_str parse_url passthru pathinfo pattern modifiers pattern syntax pclose pdf_add_annotation pdf_add_bookmark pdf_add_launchlink pdf_add_locallink pdf_add_note pdf_add_outline pdf_add_pdflink pdf_add_thumbnail pdf_add_weblink pdf_arc pdf_arcn pdf_attach_file pdf_begin_page pdf_begin_pattern pdf_begin_template pdf_circle pdf_clip pdf_close pdf_close_image pdf_close_pdi pdf_close_pdi_page pdf_closepath pdf_closepath_fill_stroke pdf_closepath_stroke pdf_concat pdf_continue_text pdf_curveto pdf_delete pdf_end_page pdf_end_pattern pdf_end_template pdf_endpath pdf_fill pdf_fill_stroke pdf_findfont pdf_get_buffer pdf_get_font pdf_get_fontname pdf_get_fontsize pdf_get_image_height pdf_get_image_width pdf_get_majorversion pdf_get_minorversion pdf_get_parameter pdf_get_pdi_parameter pdf_get_pdi_value pdf_get_value pdf_initgraphics pdf_lineto pdf_makespotcolor pdf_moveto pdf_new pdf_open pdf_open_ccitt pdf_open_file pdf_open_gif pdf_open_image pdf_open_image_file pdf_open_jpeg pdf_open_memory_image pdf_open_pdi pdf_open_pdi_page pdf_open_png pdf_open_tiff pdf_place_image pdf_place_pdi_page pdf_rect pdf_restore pdf_rotate pdf_save pdf_scale pdf_set_border_color pdf_set_border_dash pdf_set_border_style pdf_set_char_spacing pdf_set_duration pdf_set_font pdf_set_horiz_scaling pdf_set_info pdf_set_info_author pdf_set_info_creator pdf_set_info_keywords pdf_set_info_subject pdf_set_info_title pdf_set_leading pdf_set_parameter pdf_set_text_matrix pdf_set_text_pos pdf_set_text_rendering pdf_set_text_rise pdf_set_value pdf_set_word_spacing pdf_setcolor pdf_setdash pdf_setflat pdf_setfont pdf_setgray pdf_setgray_fill pdf_setgray_stroke pdf_setlinecap pdf_setlinejoin pdf_setlinewidth pdf_setmatrix pdf_setmiterlimit pdf_setpolydash pdf_setrgbcolor pdf_setrgbcolor_fill pdf_setrgbcolor_stroke pdf_show pdf_show_boxed pdf_show_xy pdf_skew pdf_stringwidth pdf_stroke pdf_translate pfpro_cleanup pfpro_init pfpro_process pfpro_process_raw pfpro_version pfsockopen pg_cancel_query pg_client_encoding pg_close pg_cmdtuples pg_connect pg_connection_busy pg_connection_reset pg_connection_status pg_copy_from pg_copy_to pg_dbname pg_end_copy pg_errormessage pg_escape_bytea pg_escape_string pg_exec pg_fetch_array pg_fetch_object pg_fetch_row pg_fieldisnull pg_fieldname pg_fieldnum pg_fieldprtlen pg_fieldsize pg_fieldtype pg_freeresult pg_get_result pg_getlastoid pg_host pg_is_busy pg_last_notice pg_lo_close pg_lo_seek pg_lo_tell pg_loclose pg_locreate pg_loexport pg_loimport pg_loopen pg_loread pg_loreadall pg_lounlink pg_lowrite pg_numfields pg_numrows pg_options pg_pconnect pg_port pg_put_line pg_request_cancel pg_result pg_result_error pg_result_status pg_send_query pg_set_client_encoding pg_trace pg_tty pg_untrace php_logo_guid php_sapi_name php_uname phpcredits phpinfo phpversion pi png2wbmp popen pos posix_ctermid posix_getcwd posix_getegid posix_geteuid posix_getgid posix_getgrgid posix_getgrnam posix_getgroups posix_getlogin posix_getpgid posix_getpgrp posix_getpid posix_getppid posix_getpwnam posix_getpwuid posix_getrlimit posix_getsid posix_getuid posix_isatty posix_kill posix_mkfifo posix_setegid posix_seteuid posix_setgid posix_setpgid posix_setsid posix_setuid posix_times posix_ttyname posix_uname pow preg_grep preg_match preg_match_all preg_quote preg_replace preg_replace_callback preg_split prev print print_r printer_abort printer_close printer_create_brush printer_create_dc printer_create_font printer_create_pen printer_delete_brush printer_delete_dc printer_delete_font printer_delete_pen printer_draw_bmp printer_draw_chord printer_draw_elipse printer_draw_line printer_draw_pie printer_draw_rectangle printer_draw_roundrect printer_draw_text printer_end_doc printer_end_page printer_get_option printer_list printer_logical_fontheight printer_open printer_select_brush printer_select_font printer_select_pen printer_set_option printer_start_doc printer_start_page printer_write printf pspell_add_to_personal pspell_add_to_session pspell_check pspell_clear_session pspell_config_create pspell_config_ignore pspell_config_mode pspell_config_personal pspell_config_repl pspell_config_runtogether pspell_config_save_repl pspell_new pspell_new_config pspell_new_personal pspell_save_wordlist pspell_store_replacement pspell_suggest putenv qdom_error qdom_tree quoted_printable_decode quotemeta rad2deg rand range rawurldecode rawurlencode read_exif_data readdir readfile readgzfile readline readline_add_history readline_clear_history readline_completion_function readline_info readline_list_history readline_read_history readline_write_history readlink realpath recode recode_file recode_string register_shutdown_function register_tick_function rename reset restore_error_handler rewind rewinddir rmdir round rsort rtrim satellite_caught_exception satellite_exception_id satellite_exception_value satellite_get_repository_id satellite_load_idl satellite_object_to_string sem_acquire sem_get sem_release sem_remove serialize sesam_affected_rows sesam_commit sesam_connect sesam_diagnostic sesam_disconnect sesam_errormsg sesam_execimm sesam_fetch_array sesam_fetch_result sesam_fetch_row sesam_field_array sesam_field_name sesam_free_result sesam_num_fields sesam_query sesam_rollback sesam_seek_row sesam_settransaction session_cache_expire session_cache_limiter session_decode session_destroy session_encode session_get_cookie_params session_id session_is_registered session_module_name session_name session_register session_save_path session_set_cookie_params session_set_save_handler session_start session_unregister session_unset session_write_close set_error_handler set_file_buffer set_magic_quotes_runtime set_time_limit setcookie setlocale settype shell_exec shm_attach shm_detach shm_get_var shm_put_var shm_remove shm_remove_var shmop_close shmop_delete shmop_open shmop_read shmop_size shmop_write show_source shuffle similar_text sin sinh sizeof sleep snmp_get_quick_print snmp_set_quick_print snmpget snmprealwalk snmpset snmpwalk snmpwalkoid socket_accept socket_bind socket_close socket_connect socket_create socket_create_listen socket_create_pair socket_fd_alloc socket_fd_clear socket_fd_free socket_fd_isset socket_fd_set socket_fd_zero socket_get_status socket_getopt socket_getpeername socket_getsockname socket_iovec_add socket_iovec_alloc socket_iovec_delete socket_iovec_fetch socket_iovec_free socket_iovec_set socket_last_error socket_listen socket_read socket_readv socket_recv socket_recvfrom socket_recvmsg socket_select socket_send socket_sendmsg socket_sendto socket_set_blocking socket_set_nonblock socket_set_timeout socket_setopt socket_shutdown socket_strerror socket_write socket_writev sort soundex split spliti sprintf sql_regcase sqrt srand sscanf stat str_pad str_repeat str_replace str_rot13 strcasecmp strchr strcmp strcoll strcspn strftime strip_tags stripcslashes stripslashes stristr strlen strnatcasecmp strnatcmp strncasecmp strncmp strpos strrchr strrev strrpos strspn strstr strtok strtolower strtotime strtoupper strtr strval substr substr_count substr_replace swf_actiongeturl swf_actiongotoframe swf_actiongotolabel swf_actionnextframe swf_actionplay swf_actionprevframe swf_actionsettarget swf_actionstop swf_actiontogglequality swf_actionwaitforframe swf_addbuttonrecord swf_addcolor swf_closefile swf_definebitmap swf_definefont swf_defineline swf_definepoly swf_definerect swf_definetext swf_endbutton swf_enddoaction swf_endshape swf_endsymbol swf_fontsize swf_fontslant swf_fonttracking swf_getbitmapinfo swf_getfontinfo swf_getframe swf_labelframe swf_lookat swf_modifyobject swf_mulcolor swf_nextid swf_oncondition swf_openfile swf_ortho swf_ortho2 swf_perspective swf_placeobject swf_polarview swf_popmatrix swf_posround swf_pushmatrix swf_removeobject swf_rotate swf_scale swf_setfont swf_setframe swf_shapearc swf_shapecurveto swf_shapecurveto3 swf_shapefillbitmapclip swf_shapefillbitmaptile swf_shapefilloff swf_shapefillsolid swf_shapelinesolid swf_shapelineto swf_shapemoveto swf_showframe swf_startbutton swf_startdoaction swf_startshape swf_startsymbol swf_textwidth swf_translate swf_viewport swfaction swfbitmap swfbutton swfbutton_keypress swfdisplayitem swffill swffont swfgradient swfmorph swfmovie swfshape swfsprite swftext swftextfield sybase_affected_rows sybase_close sybase_connect sybase_data_seek sybase_fetch_array sybase_fetch_field sybase_fetch_object sybase_fetch_row sybase_field_seek sybase_free_result sybase_get_last_message sybase_min_client_severity sybase_min_error_severity sybase_min_message_severity sybase_min_server_severity sybase_num_fields sybase_num_rows sybase_pconnect sybase_query sybase_result sybase_select_db symlink syslog system tan tanh tempnam textdomain time tmpfile touch trigger_error trim uasort ucfirst ucwords udm_add_search_limit udm_alloc_agent udm_api_version udm_cat_list udm_cat_path udm_check_charset udm_check_stored udm_clear_search_limits udm_close_stored udm_crc32 udm_errno udm_error udm_find udm_free_agent udm_free_ispell_data udm_free_res udm_get_doc_count udm_get_res_field udm_get_res_param udm_load_ispell_data udm_open_stored udm_set_agent_param uksort umask uniqid unixtojd unlink unpack unregister_tick_function unserialize unset urldecode urlencode user_error usleep usort utf8_decode utf8_encode var_dump var_export variant version_compare virtual vpopmail_add_alias_domain vpopmail_add_alias_domain_ex vpopmail_add_domain vpopmail_add_domain_ex vpopmail_add_user vpopmail_alias_add vpopmail_alias_del vpopmail_alias_del_domain vpopmail_alias_get vpopmail_alias_get_all vpopmail_auth_user vpopmail_del_domain vpopmail_del_domain_ex vpopmail_del_user vpopmail_error vpopmail_passwd vpopmail_set_user_quota vprintf vsprintf w32api_deftype w32api_init_dtype w32api_invoke_function w32api_register_function w32api_set_call_method wddx_add_vars wddx_deserialize wddx_packet_end wddx_packet_start wddx_serialize_value wddx_serialize_vars wordwrap xml_error_string xml_get_current_byte_index xml_get_current_column_number xml_get_current_line_number xml_get_error_code xml_parse xml_parse_into_struct xml_parser_create xml_parser_create_ns xml_parser_free xml_parser_get_option xml_parser_set_option xml_set_character_data_handler xml_set_default_handler xml_set_element_handler xml_set_end_namespace_decl_handler xml_set_external_entity_ref_handler xml_set_notation_decl_handler xml_set_object xml_set_processing_instruction_handler xml_set_start_namespace_decl_handler xml_set_unparsed_entity_decl_handler xmldoc xmldocfile xmlrpc_decode xmlrpc_decode_request xmlrpc_encode xmlrpc_encode_request xmlrpc_get_type xmlrpc_parse_method_descriptions xmlrpc_server_add_introspection_data xmlrpc_server_call_method xmlrpc_server_create xmlrpc_server_destroy xmlrpc_server_register_introspection_callback xmlrpc_server_register_method xmlrpc_set_type xmltree xpath_eval xpath_eval_expression xpath_new_context xptr_eval xptr_new_context xslt_create xslt_errno xslt_error xslt_free xslt_process xslt_set_base xslt_set_encoding xslt_set_error_handler xslt_set_log xslt_set_sax_handler xslt_set_sax_handlers xslt_set_scheme_handler xslt_set_scheme_handlers yaz_addinfo yaz_ccl_conf yaz_ccl_parse yaz_close yaz_connect yaz_database yaz_element yaz_errno yaz_error yaz_hits yaz_itemorder yaz_present yaz_range yaz_record yaz_scan yaz_scan_result yaz_search yaz_sort yaz_syntax yaz_wait yp_all yp_cat yp_err_string yp_errno yp_first yp_get_default_domain yp_master yp_match yp_next yp_order zend_logo_guid zend_version zip_close zip_entry_close zip_entry_compressedsize zip_entry_compressionmethod zip_entry_filesize zip_entry_name zip_entry_open zip_entry_read zip_open zip_read");

  gchar *font;
  guint size;
  g_object_get(lgphpdet->prefmg, "style_font_name", &font,"font_size", &size, NULL);
  gchar *style_name;
  g_object_get(lgphpdet->prefmg, "style_name", &style_name, NULL);

  GtkSourceStyleScheme	*scheme = gtk_source_style_scheme_manager_get_scheme (main_window.stylemg, style_name);
  /* PHP LEXER STYLE */
  set_scintilla_lexer_default_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_HPHP_DEFAULT, font, size);
  set_scintilla_lexer_keyword_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_HPHP_WORD, font, size);
  set_scintilla_lexer_variable_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_HPHP_VARIABLE, font, size);
  set_scintilla_lexer_variable_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_HPHP_COMPLEX_VARIABLE, font, size);
  set_scintilla_lexer_variable_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_HPHP_HSTRING_VARIABLE, font, size);
  set_scintilla_lexer_string_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_HPHP_HSTRING, font, size);
  set_scintilla_lexer_simple_string_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_HPHP_SIMPLESTRING, font, size);
  set_scintilla_lexer_operator_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_HPHP_OPERATOR, font, size);
  set_scintilla_lexer_number_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_HPHP_NUMBER, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(lgphpdet->sci), scheme, SCE_HPHP_COMMENT, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(lgphpdet->sci), scheme, SCE_HPHP_COMMENTLINE, font, size);
  set_scintilla_lexer_special_constant_style (GTK_WIDGET(lgphpdet->sci), scheme, SCE_H_QUESTION, font, size); //FIXME: LEXER BUG??
  set_scintilla_lexer_error_style (GTK_WIDGET(lgphpdet->sci), scheme, SCE_ERR_PHP, font, size);
  /* JAVASCRIPT LEXER STYLE */
  set_scintilla_lexer_default_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_HJ_DEFAULT, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(lgphpdet->sci), scheme, SCE_HJ_COMMENT, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(lgphpdet->sci), scheme, SCE_HJ_COMMENTLINE, font, size);
  set_scintilla_lexer_number_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_HJ_NUMBER, font, size);
  set_scintilla_lexer_keyword_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_HJ_WORD, font, size);
  set_scintilla_lexer_string_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_HJ_DOUBLESTRING, font, size);
  set_scintilla_lexer_simple_string_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_HJ_SINGLESTRING, font, size);
  set_scintilla_lexer_variable_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_HJ_KEYWORD, font, size);
  set_scintilla_lexer_special_constant_style (GTK_WIDGET(lgphpdet->sci), scheme, SCE_HJ_START, font, size);
  set_scintilla_lexer_special_constant_style (GTK_WIDGET(lgphpdet->sci), scheme, SCE_HJ_STRINGEOL, font, size);
  set_scintilla_lexer_special_constant_style (GTK_WIDGET(lgphpdet->sci), scheme, SCE_HJ_REGEX, font, size);
  set_scintilla_lexer_operator_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_HJ_SYMBOLS, font, size);
  set_scintilla_lexer_doc_comment_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_HJ_COMMENTDOC, font, size);

  /* HTML LEXER STYLE */
  set_scintilla_lexer_default_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_H_DEFAULT, font, size);
  set_scintilla_lexer_number_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_H_NUMBER, font, size);
  set_scintilla_lexer_string_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_H_DOUBLESTRING, font, size);
  set_scintilla_lexer_simple_string_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_H_SINGLESTRING, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(lgphpdet->sci), scheme, SCE_H_COMMENT, font, size);
  set_scintilla_lexer_operator_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_H_VALUE, font, size);
  set_scintilla_lexer_variable_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_H_TAGUNKNOWN, font, size);
  set_scintilla_lexer_xml_element_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_H_TAG, font, size);
  set_scintilla_lexer_xml_element_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_H_TAGEND, font, size);
  set_scintilla_lexer_special_constant_style (GTK_WIDGET(lgphpdet->sci), scheme, SCE_H_SCRIPT, font, size);
  set_scintilla_lexer_special_constant_style (GTK_WIDGET(lgphpdet->sci), scheme, SCE_H_CDATA, font, size);
  set_scintilla_lexer_xml_atribute_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_H_ATTRIBUTE, font, size);
  set_scintilla_lexer_xml_entity_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_H_ENTITY, font, size);
  set_scintilla_lexer_special_constant_style (GTK_WIDGET(lgphpdet->sci), scheme, SCE_H_XMLSTART, font, size);
  set_scintilla_lexer_special_constant_style (GTK_WIDGET(lgphpdet->sci), scheme, SCE_H_XMLEND, font, size);
  set_scintilla_lexer_special_constant_style (GTK_WIDGET(lgphpdet->sci), scheme, SCE_H_OTHER, font, size);
  set_scintilla_lexer_xml_instruction_style(GTK_WIDGET(lgphpdet->sci), scheme, SCE_H_ATTRIBUTEUNKNOWN, font, size);

  gtk_scintilla_set_property(lgphpdet->sci, "fold.html", "1");
  gtk_scintilla_set_property(lgphpdet->sci, "fold", "1");
  gtk_scintilla_set_property(lgphpdet->sci, "fold.hypertext.comment", "1");
  gtk_scintilla_set_property(lgphpdet->sci, "fold.hypertext.heredoc", "1");
  gtk_scintilla_set_property(lgphpdet->sci, "lexer.html.mako", "1");
  gtk_scintilla_set_property(lgphpdet->sci, "lexer.html.django", "1");

  g_free(font);
  g_free(style_name);

  gtk_scintilla_colourise(lgphpdet->sci, 0, -1);

  gtk_scintilla_set_word_chars(lgphpdet->sci, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_$");
  gtk_scintilla_autoc_set_fill_ups(lgphpdet->sci, "( .");
}
