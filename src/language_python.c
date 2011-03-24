/* This file is part of gPHPEdit, a GNOME PHP Editor.
 
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009, 2011 José Rostagno (for vijona.com.ar)
	  
   For more information or to find the latest release, visit our 
   website at http://www.gpythonedit.org/
 
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
#include "language_python.h"
#include "preferences_manager.h"
#include "language_provider.h"
#include "symbol_manager.h"
#include "main_window.h"

/*
* language_python private struct
*/
struct Language_PYTHONDetails
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

#define LANGUAGE_PYTHON_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    LANGUAGE_PYTHON_TYPE,\
					    Language_PYTHONDetails))
enum
{
  PROP_0,
  PROP_DOCUMENT_SCINTILLA
};

static void language_python_language_provider_init(Language_ProviderInterface *iface, gpointer user_data);
static void language_python_trigger_completion (Language_Provider *lgpython, guint ch);
static void show_calltip (Language_Provider *lgpython);
static void language_python_setup_lexer(Language_Provider *lgpython);
static gchar *language_python_do_syntax_check(Language_Provider *lgpython);

G_DEFINE_TYPE_WITH_CODE(Language_PYTHON, language_python, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE (IFACE_TYPE_LANGUAGE_PROVIDER,
                                                 language_python_language_provider_init));

static void language_python_language_provider_init(Language_ProviderInterface *iface, gpointer user_data)
{
  iface->trigger_completion = language_python_trigger_completion;
  iface->show_calltip = show_calltip;
  iface->setup_lexer = language_python_setup_lexer;
  iface->do_syntax_check = language_python_do_syntax_check;
}

static void
language_python_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
  Language_PYTHONDetails *lgpythondet;
  lgpythondet = LANGUAGE_PYTHON_GET_PRIVATE(object);

  switch (prop_id)
  {
    case PROP_DOCUMENT_SCINTILLA:
      if (lgpythondet->doc) g_object_unref(lgpythondet->doc);
      lgpythondet->doc = g_value_dup_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
language_python_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
  Language_PYTHONDetails *lgpythondet;
  lgpythondet = LANGUAGE_PYTHON_GET_PRIVATE(object);

  
  switch (prop_id)
  {
    case PROP_DOCUMENT_SCINTILLA:
      g_value_set_object (value, lgpythondet->doc);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}


/*
* disposes the Gobject
*/
static void language_python_dispose (GObject *object)
{
  Language_PYTHONDetails *lgpythondet;
  lgpythondet = LANGUAGE_PYTHON_GET_PRIVATE(object);
  g_object_unref(lgpythondet->prefmg);
  g_object_unref(lgpythondet->doc);
  g_object_unref(lgpythondet->symbolmg);
  /* Chain up to the parent class */
  G_OBJECT_CLASS (language_python_parent_class)->dispose (object);
}

static void language_python_constructed (GObject *object)
{
  Language_PYTHONDetails *lgpythondet;
  lgpythondet = LANGUAGE_PYTHON_GET_PRIVATE(object);
  GtkScintilla *sci;
  g_object_get(lgpythondet->doc, "scintilla", &sci, NULL);
  lgpythondet->sci = sci;
}

static void
language_python_class_init (Language_PYTHONClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = language_python_dispose;
  object_class->set_property = language_python_set_property;
  object_class->get_property = language_python_get_property;
  object_class->constructed = language_python_constructed;
  g_object_class_install_property (object_class,
                              PROP_DOCUMENT_SCINTILLA,
                              g_param_spec_object ("document_scintilla",
                              NULL, NULL,
                              DOCUMENT_SCINTILLA_TYPE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_type_class_add_private (klass, sizeof (Language_PYTHONDetails));
}

static void
language_python_init (Language_PYTHON * object)
{
  Language_PYTHONDetails *lgpythondet;
  lgpythondet = LANGUAGE_PYTHON_GET_PRIVATE(object);
  lgpythondet->prefmg = preferences_manager_new ();
  lgpythondet->symbolmg = symbol_manager_new ();
}

Language_PYTHON *language_python_new (Document_Scintilla *doc)
{
  Language_PYTHON *lgpython;
  lgpython = g_object_new (LANGUAGE_PYTHON_TYPE, "document_scintilla", doc, NULL);
  return lgpython; /* return new object */
}

typedef struct {
 Language_PYTHON *lgpython;
 gint old_pos;
} Calltip_Data;

static gboolean auto_complete_callback(gpointer data)
{
  Calltip_Data *dat = (Calltip_Data *) data;
  Language_PYTHONDetails *lgpythondet = LANGUAGE_PYTHON_GET_PRIVATE(dat->lgpython);
  gint current_pos;
  current_pos = documentable_get_current_position(lgpythondet->doc);
  GtkScintilla *scintilla = lgpythondet->sci;
  if (current_pos == dat->old_pos) {
    gchar *prefix = documentable_get_current_word(lgpythondet->doc);
    gchar *calltip = symbol_manager_get_symbols_matches (lgpythondet->symbolmg, prefix, SYMBOL_FUNCTION | SYMBOL_CLASS | SYMBOL_VAR, TAB_PYTHON);
    if (calltip && strlen(calltip)!=0) gtk_scintilla_user_list_show(GTK_SCINTILLA(scintilla), 1, calltip);
    g_free(prefix);
    g_free(calltip);
  }
  g_slice_free(Calltip_Data, dat);
  lgpythondet->completion_timer_set=FALSE;
  return FALSE;
}

static void show_autocompletion (Language_PYTHON *lgpython, gint pos)
{
  Language_PYTHONDetails *lgpythondet = LANGUAGE_PYTHON_GET_PRIVATE(lgpython);
  if (!lgpythondet->completion_timer_set) {
    gint delay;
    g_object_get(lgpythondet->prefmg, "autocomplete_delay", &delay, NULL);
    Calltip_Data *dat = g_slice_new(Calltip_Data);
    dat->lgpython = lgpython;
    dat->old_pos = pos;
    lgpythondet->completion_timer_id = g_timeout_add(delay, auto_complete_callback, dat);
    lgpythondet->completion_timer_set=TRUE;
  }
}

static gboolean calltip_callback(gpointer data)
{
  Calltip_Data *dat = (Calltip_Data *) data;
  Language_PYTHONDetails *lgpythondet = LANGUAGE_PYTHON_GET_PRIVATE(dat->lgpython);
  gint current_pos;
  current_pos = documentable_get_current_position(lgpythondet->doc);
  GtkScintilla *scintilla = lgpythondet->sci;
  if (current_pos == dat->old_pos) {
    gchar *prefix = documentable_get_current_word(lgpythondet->doc);
    gchar *calltip = symbol_manager_get_calltip (lgpythondet->symbolmg, prefix, TAB_PYTHON);
    if (calltip){
      gtk_scintilla_call_tip_show(GTK_SCINTILLA(scintilla), current_pos, calltip);
      g_free(calltip);
    }
    g_free(prefix);
  }
  g_slice_free(Calltip_Data, dat);
  lgpythondet->calltip_timer_set = FALSE;
  return FALSE;
}

static void show_calltip (Language_Provider *lgpython)
{
  Language_PYTHONDetails *lgpythondet = LANGUAGE_PYTHON_GET_PRIVATE(lgpython);
  if (!lgpythondet->calltip_timer_set) {
    gint delay;
    g_object_get(lgpythondet->prefmg, "calltip_delay", &delay, NULL);
    Calltip_Data *dat = g_slice_new(Calltip_Data);
    dat->lgpython = LANGUAGE_PYTHON(lgpython);
    dat->old_pos = documentable_get_current_position(lgpythondet->doc);
    lgpythondet->calltip_timer_id = g_timeout_add(delay, calltip_callback, dat);
    lgpythondet->calltip_timer_set = TRUE;
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

static gboolean is_python_char_autoindent(char ch)
{
  return (ch==':' || ch== 92 /* / */);
}

static gboolean is_python_char_autounindent(char ch)
{
//FIXME
  return FALSE;
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
    if (is_python_char_autoindent(*previous_char_buffer)) {
      gint indentation_size;
      g_object_get(pref, "indentation_size", &indentation_size, NULL);
      previous_line_indentation+=indentation_size;
    } else if (is_python_char_autounindent(*previous_char_buffer)) {
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
        if (!(g_ascii_iscntrl(c) || g_ascii_isspace(c) || is_python_char_autounindent(c))) {
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

static void language_python_trigger_completion (Language_Provider *lgpython, guint ch)
{
  g_return_if_fail(lgpython);
  Language_PYTHONDetails *lgpythondet = LANGUAGE_PYTHON_GET_PRIVATE(lgpython);
  gint current_pos;
  gchar *member_function_buffer = NULL;

  current_pos = gtk_scintilla_get_current_pos(lgpythondet->sci);
  gboolean auto_brace;
  g_object_get(lgpythondet->prefmg, "auto_complete_braces", &auto_brace, NULL);

  if (IsOpenBrace(ch) && auto_brace) {
    InsertCloseBrace (lgpythondet->sci, current_pos, ch);
    return;
  }

  switch(ch) {
    case ('\r'):
    case ('\n'):
        autoindent_brace_code (lgpythondet->sci, lgpythondet->prefmg);
        break;
    case (')'):
        cancel_calltip (lgpythondet->sci);
        break;
    case ('('):
        show_calltip (lgpython);
        break;
    default:
        member_function_buffer = documentable_get_current_word(lgpythondet->doc);
        if (member_function_buffer && strlen(member_function_buffer)>=3) show_autocompletion (LANGUAGE_PYTHON(lgpython), current_pos);
        g_free(member_function_buffer);
  }
}

static gchar *language_python_do_syntax_check(Language_Provider *lgpython)
{
  return NULL;
}

static void language_python_setup_lexer(Language_Provider *lgpython)
{
  g_return_if_fail(lgpython);
  Language_PYTHONDetails *lgpythondet = LANGUAGE_PYTHON_GET_PRIVATE(lgpython);

  gtk_scintilla_clear_document_style (lgpythondet->sci);
  gtk_scintilla_set_lexer(lgpythondet->sci, SCLEX_PYTHON);
  gtk_scintilla_set_style_bits(lgpythondet->sci, 8);

  // Python keywords
  gtk_scintilla_set_keywords(lgpythondet->sci, 0, "abs all any apply and assert break class continue def del dict dir elif else except exec finally for from global if import  intern in is lambda len None not or pass print raise return try while yield from as with open __import__ self");

  gtk_scintilla_set_keywords(lgpythondet->sci, 1, "Ellipsis NotImplemented ArithmeticError AssertionError AttributeError EnvironmentError EOFError Exception FloatingPointError ImportError IndentationError IndexError IOError KeyboardInterrupt KeyError LookupError MemoryError NameError NotImplementedError OSError OverflowError ReferenceError RuntimeError StandardError StopIteration SyntaxError SystemError SystemExit TabError TypeError UnboundLocalError UnicodeDecodeError UnicodeEncodeError UnicodeError UnicodeTranslateError ValueError WindowsError ZeroDivisionError Warning UserWarning DeprecationWarning PendingDeprecationWarning SyntaxWarning OverflowWarning RuntimeWarning FutureWarning basestring bool buffer callable chr classmethod cmp coerce compile complex delattr divmod enumerate eval execfile file filter float frozenset getattr globals hasattr hash hex id input int  isinstance issubclass iter  list locals long map max min object oct ord pow property range raw_input reduce reload repr reversed round setattr set slice sorted staticmethod str sum super tuple type unichr unicode vars xrange zip");

  const gchar *style_name;
  g_object_get(lgpythondet->prefmg, "style_name", &style_name, NULL);

  GtkSourceStyleScheme	*scheme = gtk_source_style_scheme_manager_get_scheme (main_window.stylemg, style_name);
  /* PYTHON LEXER STYLE */
  const gchar *font;
  guint size;
  g_object_get(lgpythondet->prefmg, "style_font_name", &font,"font_size", &size, NULL);

  set_scintilla_lexer_default_style(GTK_WIDGET(lgpythondet->sci), scheme, SCE_P_DEFAULT, font, size);

  set_scintilla_lexer_keyword_style(GTK_WIDGET(lgpythondet->sci), scheme, SCE_P_WORD, font, size);
  set_scintilla_lexer_type_style(GTK_WIDGET(lgpythondet->sci), scheme, SCE_P_WORD2, font, size);
  set_scintilla_lexer_variable_style(GTK_WIDGET(lgpythondet->sci), scheme, SCE_P_IDENTIFIER, font, size);
  set_scintilla_lexer_string_style(GTK_WIDGET(lgpythondet->sci), scheme, SCE_P_STRING, font, size);
  set_scintilla_lexer_simple_string_style(GTK_WIDGET(lgpythondet->sci), scheme, SCE_P_TRIPLE, font, size);
  set_scintilla_lexer_string_style(GTK_WIDGET(lgpythondet->sci), scheme, SCE_P_TRIPLEDOUBLE, font, size);
  set_scintilla_lexer_simple_string_style(GTK_WIDGET(lgpythondet->sci), scheme, SCE_P_CHARACTER, font, size);
  set_scintilla_lexer_operator_style(GTK_WIDGET(lgpythondet->sci), scheme, SCE_P_OPERATOR, font, size);
  set_scintilla_lexer_number_style(GTK_WIDGET(lgpythondet->sci), scheme, SCE_P_NUMBER, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(lgpythondet->sci), scheme, SCE_P_COMMENTBLOCK, font, size);
  set_scintilla_lexer_special_constant_style (GTK_WIDGET(lgpythondet->sci), scheme, SCE_P_STRINGEOL, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(lgpythondet->sci), scheme, SCE_P_COMMENTLINE, font, size);
  set_scintilla_lexer_xml_entity_style(GTK_WIDGET(lgpythondet->sci), scheme, SCE_P_DECORATOR, font, size);
  set_scintilla_lexer_preprocessor_style(GTK_WIDGET(lgpythondet->sci), scheme, SCE_P_DEFNAME, font, size);
  set_scintilla_lexer_xml_instruction_style(GTK_WIDGET(lgpythondet->sci), scheme, SCE_P_CLASSNAME, font, size);

  gtk_scintilla_set_property(lgpythondet->sci, "fold", "1");
  gtk_scintilla_set_property(lgpythondet->sci, "lexer.python.strings.over.newline", "1");
  gtk_scintilla_colourise(lgpythondet->sci, 0, -1);

  gtk_scintilla_set_word_chars(lgpythondet->sci, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-");
}
