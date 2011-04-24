/* This file is part of gPHPEdit, a GNOME PHP Editor.
 
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009, 2011 José Rostagno (for vijona.com.ar)
	  
   For more information or to find the latest release, visit our 
   website at http://www.gcxxedit.org/
 
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
#include "language_cxx.h"
#include "preferences_manager.h"
#include "language_provider.h"
#include "symbol_manager.h"
#include "main_window.h"

/*
* language_cxx private struct
*/
struct Language_CXXDetails
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

#define LANGUAGE_CXX_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    LANGUAGE_CXX_TYPE,\
					    Language_CXXDetails))
enum
{
  PROP_0,
  PROP_DOCUMENT_SCINTILLA
};

static void language_cxx_language_provider_init(Language_ProviderInterface *iface, gpointer user_data);
static void language_cxx_trigger_completion (Language_Provider *lgcxx, guint ch);
static void show_calltip (Language_Provider *lgcxx);
static void language_cxx_setup_lexer(Language_Provider *lgcxx);
static gchar *language_cxx_do_syntax_check(Language_Provider *lgcxx);

G_DEFINE_TYPE_WITH_CODE(Language_CXX, language_cxx, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE (IFACE_TYPE_LANGUAGE_PROVIDER,
                                                 language_cxx_language_provider_init));

static void language_cxx_language_provider_init(Language_ProviderInterface *iface, gpointer user_data)
{
  iface->trigger_completion = language_cxx_trigger_completion;
  iface->show_calltip = show_calltip;
  iface->setup_lexer = language_cxx_setup_lexer;
  iface->do_syntax_check = language_cxx_do_syntax_check;
}

static void
language_cxx_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
  Language_CXXDetails *lgcxxdet;
  lgcxxdet = LANGUAGE_CXX_GET_PRIVATE(object);

  switch (prop_id)
  {
    case PROP_DOCUMENT_SCINTILLA:
      if (lgcxxdet->doc) g_object_unref(lgcxxdet->doc);
      lgcxxdet->doc = g_value_dup_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
language_cxx_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
  Language_CXXDetails *lgcxxdet;
  lgcxxdet = LANGUAGE_CXX_GET_PRIVATE(object);

  
  switch (prop_id)
  {
    case PROP_DOCUMENT_SCINTILLA:
      g_value_set_object (value, lgcxxdet->doc);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}


/*
* disposes the Gobject
*/
static void language_cxx_dispose (GObject *object)
{
  Language_CXXDetails *lgcxxdet;
  lgcxxdet = LANGUAGE_CXX_GET_PRIVATE(object);
  g_object_unref(lgcxxdet->prefmg);
  g_object_unref(lgcxxdet->doc);
  g_object_unref(lgcxxdet->symbolmg);
  /* Chain up to the parent class */
  G_OBJECT_CLASS (language_cxx_parent_class)->dispose (object);
}

static void language_cxx_constructed (GObject *object)
{
  Language_CXXDetails *lgcxxdet;
  lgcxxdet = LANGUAGE_CXX_GET_PRIVATE(object);
  GtkScintilla *sci;
  g_object_get(lgcxxdet->doc, "scintilla", &sci, NULL);
  lgcxxdet->sci = sci;
}

static void
language_cxx_class_init (Language_CXXClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = language_cxx_dispose;
  object_class->set_property = language_cxx_set_property;
  object_class->get_property = language_cxx_get_property;
  object_class->constructed = language_cxx_constructed;
  g_object_class_install_property (object_class,
                              PROP_DOCUMENT_SCINTILLA,
                              g_param_spec_object ("document_scintilla",
                              NULL, NULL,
                              DOCUMENT_SCINTILLA_TYPE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_type_class_add_private (klass, sizeof (Language_CXXDetails));
}

static void
language_cxx_init (Language_CXX * object)
{
  Language_CXXDetails *lgcxxdet;
  lgcxxdet = LANGUAGE_CXX_GET_PRIVATE(object);
  lgcxxdet->prefmg = preferences_manager_new ();
  lgcxxdet->symbolmg = symbol_manager_new ();
}

Language_CXX *language_cxx_new (Document_Scintilla *doc)
{
  Language_CXX *lgcxx;
  lgcxx = g_object_new (LANGUAGE_CXX_TYPE, "document_scintilla", doc, NULL);
  return lgcxx; /* return new object */
}

typedef struct {
 Language_CXX *lgcxx;
 gint old_pos;
} Calltip_Data;

static gboolean auto_complete_callback(gpointer data)
{
  Calltip_Data *dat = (Calltip_Data *) data;
  Language_CXXDetails *lgcxxdet = LANGUAGE_CXX_GET_PRIVATE(dat->lgcxx);
  gint current_pos;
  current_pos = documentable_get_current_position(lgcxxdet->doc);
  GtkScintilla *scintilla = lgcxxdet->sci;
  if (current_pos == dat->old_pos) {
    gchar *prefix = documentable_get_current_word(lgcxxdet->doc);
    gchar *calltip = symbol_manager_get_symbols_matches (lgcxxdet->symbolmg, prefix, SYMBOL_FUNCTION | SYMBOL_CLASS | SYMBOL_VAR, TAB_CXX);
    if (calltip && strlen(calltip)!=0) gtk_scintilla_user_list_show(GTK_SCINTILLA(scintilla), 1, calltip);
    g_free(prefix);
    g_free(calltip);
  }
  g_slice_free(Calltip_Data, dat);
  lgcxxdet->completion_timer_set=FALSE;
  return FALSE;
}

static void show_autocompletion (Language_CXX *lgcxx, gint pos)
{
  Language_CXXDetails *lgcxxdet = LANGUAGE_CXX_GET_PRIVATE(lgcxx);
  if (!lgcxxdet->completion_timer_set) {
    gint delay;
    g_object_get(lgcxxdet->prefmg, "autocomplete_delay", &delay, NULL);
    Calltip_Data *dat = g_slice_new(Calltip_Data);
    dat->lgcxx = lgcxx;
    dat->old_pos = pos;
    lgcxxdet->completion_timer_id = g_timeout_add(delay, auto_complete_callback, dat);
    lgcxxdet->completion_timer_set=TRUE;
  }
}

static gboolean auto_memberfunc_complete_callback(gpointer data)
{
  Calltip_Data *dat = (Calltip_Data *) data;
  Language_CXXDetails *lgcxxdet = LANGUAGE_CXX_GET_PRIVATE(dat->lgcxx);
  gint current_pos;
  current_pos = documentable_get_current_position(lgcxxdet->doc);
  GtkScintilla *scintilla = lgcxxdet->sci;
  if (current_pos == dat->old_pos) {
    gchar *prefix = documentable_get_current_word(lgcxxdet->doc);
    gchar *calltip = symbol_manager_get_symbols_matches (lgcxxdet->symbolmg, prefix, SYMBOL_MEMBER, TAB_CXX);
    if (calltip && strlen(calltip)!=0) gtk_scintilla_user_list_show(GTK_SCINTILLA(scintilla), 1, calltip);
//    gtk_scintilla_autoc_show(GTK_SCINTILLA(docdet->scintilla), current_pos, calltip);
    g_free(prefix);
  }
  g_slice_free(Calltip_Data, dat);
  lgcxxdet->completion_timer_set=FALSE;
  return FALSE;
}

static void autocomplete_member(Language_CXX *lgcxx, gint pos)
{
  Language_CXXDetails *lgcxxdet = LANGUAGE_CXX_GET_PRIVATE(lgcxx);
  if (!lgcxxdet->completion_timer_set) {
    gint delay;
    g_object_get(lgcxxdet->prefmg, "autocomplete_delay", &delay, NULL);
    Calltip_Data *dat = g_slice_new(Calltip_Data);
    dat->lgcxx = lgcxx;
    dat->old_pos = pos;
    lgcxxdet->completion_timer_id = g_timeout_add(delay, auto_memberfunc_complete_callback, dat);
    lgcxxdet->completion_timer_set=TRUE;
  }
}

static gboolean calltip_callback(gpointer data)
{
  Calltip_Data *dat = (Calltip_Data *) data;
  Language_CXXDetails *lgcxxdet = LANGUAGE_CXX_GET_PRIVATE(dat->lgcxx);
  gint current_pos;
  current_pos = documentable_get_current_position(lgcxxdet->doc);
  GtkScintilla *scintilla = lgcxxdet->sci;
  if (current_pos == dat->old_pos) {
    gchar *prefix = documentable_get_current_word(lgcxxdet->doc);
    gchar *calltip = symbol_manager_get_calltip (lgcxxdet->symbolmg, prefix, TAB_CXX);
    if (calltip){
      gtk_scintilla_call_tip_show(GTK_SCINTILLA(scintilla), current_pos, calltip);
      g_free(calltip);
    }
    g_free(prefix);
  }
  g_slice_free(Calltip_Data, dat);
  lgcxxdet->calltip_timer_set = FALSE;
  return FALSE;
}

static void show_calltip (Language_Provider *lgcxx)
{
  Language_CXXDetails *lgcxxdet = LANGUAGE_CXX_GET_PRIVATE(lgcxx);
  if (!lgcxxdet->calltip_timer_set) {
    gint delay;
    g_object_get(lgcxxdet->prefmg, "calltip_delay", &delay, NULL);
    Calltip_Data *dat = g_slice_new(Calltip_Data);
    dat->lgcxx = LANGUAGE_CXX(lgcxx);
    dat->old_pos = documentable_get_current_position(lgcxxdet->doc);
    lgcxxdet->calltip_timer_id = g_timeout_add(delay, calltip_callback, dat);
    lgcxxdet->calltip_timer_set = TRUE;
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

static gboolean is_cxx_char_autoindent(char ch)
{
  return (ch == '{');
}

static gboolean is_cxx_char_autounindent(char ch)
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
    if (is_cxx_char_autoindent(*previous_char_buffer)) {
      gint indentation_size;
      g_object_get(pref, "indentation_size", &indentation_size, NULL);
      previous_line_indentation+=indentation_size;
    } else if (is_cxx_char_autounindent(*previous_char_buffer)) {
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
        if (!(g_ascii_iscntrl(c) || g_ascii_isspace(c) || is_cxx_char_autounindent(c))) {
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

static void language_cxx_trigger_completion (Language_Provider *lgcxx, guint ch)
{
  g_return_if_fail(lgcxx);
  Language_CXXDetails *lgcxxdet = LANGUAGE_CXX_GET_PRIVATE(lgcxx);
  gint current_pos;
  gchar *member_function_buffer = NULL;
  guint prev_char;
  current_pos = gtk_scintilla_get_current_pos(lgcxxdet->sci);
  gboolean auto_brace;
  g_object_get(lgcxxdet->prefmg, "auto_complete_braces", &auto_brace, NULL);

  if (IsOpenBrace(ch) && auto_brace) {
    InsertCloseBrace (lgcxxdet->sci, current_pos, ch);
    return;
  }

  switch(ch) {
    case ('\r'):
    case ('\n'):
        autoindent_brace_code (lgcxxdet->sci, lgcxxdet->prefmg);
        break;
    case (')'):
        cancel_calltip (lgcxxdet->sci);
        break;
    case ('('):
        show_calltip (lgcxx);
        break;
     case ('>'):
     case (':'):
       prev_char = gtk_scintilla_get_char_at(lgcxxdet->sci, current_pos-2);
       if ((prev_char=='-' && ch =='>') || (prev_char==':' && ch ==':')) {
         autocomplete_member(LANGUAGE_CXX(lgcxx), current_pos);
       }
       break;
    default:
        member_function_buffer = documentable_get_current_word(lgcxxdet->doc);
        if (member_function_buffer && strlen(member_function_buffer)>=3) show_autocompletion (LANGUAGE_CXX(lgcxx), current_pos);
        g_free(member_function_buffer);
  }
}

static gchar *language_cxx_do_syntax_check(Language_Provider *lgcxx)
{
  return NULL;
}

static void language_cxx_setup_lexer(Language_Provider *lgcxx)
{
  g_return_if_fail(lgcxx);
  Language_CXXDetails *lgcxxdet = LANGUAGE_CXX_GET_PRIVATE(lgcxx);

  gtk_scintilla_clear_document_style (lgcxxdet->sci);
  gtk_scintilla_set_lexer(lgcxxdet->sci, SCLEX_CPP);
  gtk_scintilla_set_style_bits(lgcxxdet->sci, 5);

  gtk_scintilla_set_keywords(lgcxxdet->sci, 0, "asm auto bool break case catch char class const const_cast continue default delete do double dynamic_cast else enum explicit export extern false float for friend goto if inline int long mutable namespace new operator private protected public register reinterpret_cast return short signed sizeof static static_cast struct switch template this throw true try typedef typeid typename union unsigned using virtual void volatile wchar_t while addindex addtogroup anchor arg attention author b brief bug c class code date def defgroup deprecated dontinclude e em endcode endhtmlonly endif endlatexonly endlink endverbatim enum example exception file hideinitializer htmlinclude htmlonly if image include ingroup internal invariant interface latexonly li line link mainpage name namespace nosubgrouping note overload p page par param post pre ref relates remarks return retval sa section see showinitializer since skip skipline struct subsection test throw todo typedef union until var verbatim verbinclude version warning weakgroup printf scanf");
  
  gtk_scintilla_set_keywords(lgcxxdet->sci, 1, "strstr strlen strcmp clrscr gotoXY FILE stat memcpy memmove memccpy memset strncpy strcpy strdup strndup fclose fopen freopen fdopen remove rename rewind tmpfile clearerr feof ferror fflush fflush fgetpos fgetc fgets fputc fputs ftell fseek fsetpos fread fwrite getc getchar gets fprintf sprintf vprintf perror putc putchar fputchar fscanf sscanf setbuf setvbuf tmpnam ungetc puts atof atoi atol strtod strtol strtoul rand srand malloc calloc realloc free abort atexit exit getenv system bsearch qsort abs div ldiv");

  gchar *font;
  guint size;
  g_object_get(lgcxxdet->prefmg, "style_font_name", &font,"font_size", &size, NULL);

  gchar *style_name;
  g_object_get(lgcxxdet->prefmg, "style_name", &style_name, NULL);

  GtkSourceStyleScheme	*scheme = gtk_source_style_scheme_manager_get_scheme (main_window.stylemg, style_name);
  /* PYTHON LEXER STYLE */
  set_scintilla_lexer_default_style(GTK_WIDGET(lgcxxdet->sci), scheme, SCE_C_DEFAULT, font, size);
  set_scintilla_lexer_keyword_style(GTK_WIDGET(lgcxxdet->sci), scheme, SCE_C_WORD, font, size);
  set_scintilla_lexer_keyword_style(GTK_WIDGET(lgcxxdet->sci), scheme, SCE_C_WORD2, font, size);
  set_scintilla_lexer_type_style(GTK_WIDGET(lgcxxdet->sci), scheme, SCE_C_IDENTIFIER, font, size);
  set_scintilla_lexer_string_style(GTK_WIDGET(lgcxxdet->sci), scheme, SCE_C_STRING, font, size);
  set_scintilla_lexer_simple_string_style(GTK_WIDGET(lgcxxdet->sci), scheme, SCE_C_CHARACTER, font, size);
  set_scintilla_lexer_operator_style(GTK_WIDGET(lgcxxdet->sci), scheme, SCE_C_OPERATOR, font, size);
  set_scintilla_lexer_number_style(GTK_WIDGET(lgcxxdet->sci), scheme, SCE_C_NUMBER, font, size);
  set_scintilla_lexer_doc_comment_style(GTK_WIDGET(lgcxxdet->sci), scheme, SCE_C_COMMENTDOC, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(lgcxxdet->sci), scheme, SCE_C_COMMENT, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(lgcxxdet->sci), scheme, SCE_C_COMMENTLINE, font, size);
  set_scintilla_lexer_preprocessor_style(GTK_WIDGET(lgcxxdet->sci), scheme, SCE_C_PREPROCESSOR, font, size);
  set_scintilla_lexer_xml_element_style(GTK_WIDGET(lgcxxdet->sci), scheme, SCE_C_UUID, font, size);
  set_scintilla_lexer_special_constant_style (GTK_WIDGET(lgcxxdet->sci), scheme, SCE_C_REGEX, font, size);
  set_scintilla_lexer_xml_atribute_style(GTK_WIDGET(lgcxxdet->sci), scheme, SCE_C_VERBATIM, font, size);
  set_scintilla_lexer_xml_instruction_style(GTK_WIDGET(lgcxxdet->sci), scheme, SCE_C_GLOBALCLASS, font, size);

  g_free(font);
  g_free(style_name);

  gtk_scintilla_set_property(lgcxxdet->sci, "lexer.cpp.allow.dollars", "1");
  gtk_scintilla_set_property(lgcxxdet->sci, "fold.comment", "1");
  gtk_scintilla_set_property(lgcxxdet->sci, "fold.preprocessor", "1");
  gtk_scintilla_set_property(lgcxxdet->sci, "fold.compact", "1");
  gtk_scintilla_set_property(lgcxxdet->sci, "fold.at.else", "1");
  gtk_scintilla_set_property(lgcxxdet->sci, "fold", "1");
  gtk_scintilla_colourise(lgcxxdet->sci, 0, -1);
}
