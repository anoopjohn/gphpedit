/* This file is part of gPHPEdit, a GNOME PHP Editor.
 
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009, 2011 José Rostagno (for vijona.com.ar)
	  
   For more information or to find the latest release, visit our 
   website at http://www.gsqledit.org/
 
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
#include "language_sql.h"
#include "preferences_manager.h"
#include "language_provider.h"
#include "symbol_manager.h"
#include "main_window.h"

/*
* language_sql private struct
*/
struct Language_SQLDetails
{
  Documentable *doc;
  GtkScintilla *sci;
  PreferencesManager *prefmg;
  SymbolManager *symbolmg;

  /*completion stuff*/
  guint completion_timer_id;
  gboolean completion_timer_set;
};

#define LANGUAGE_SQL_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    LANGUAGE_SQL_TYPE,\
					    Language_SQLDetails))
enum
{
  PROP_0,
  PROP_DOCUMENT_SCINTILLA
};

static void language_sql_language_provider_init(Language_ProviderInterface *iface, gpointer user_data);
static void language_sql_trigger_completion (Language_Provider *lgsql, guint ch);
static void language_sql_setup_lexer(Language_Provider *lgsql);
static gchar *language_sql_do_syntax_check(Language_Provider *lgsql);

G_DEFINE_TYPE_WITH_CODE(Language_SQL, language_sql, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE (IFACE_TYPE_LANGUAGE_PROVIDER,
                                                 language_sql_language_provider_init));

static void show_calltip (Language_Provider *lgcss)
{
 /* not implemented */
}

static void language_sql_language_provider_init(Language_ProviderInterface *iface, gpointer user_data)
{
  iface->trigger_completion = language_sql_trigger_completion;
  iface->show_calltip = show_calltip;
  iface->setup_lexer = language_sql_setup_lexer;
  iface->do_syntax_check = language_sql_do_syntax_check;
}

static void
language_sql_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
  Language_SQLDetails *lgsqldet;
  lgsqldet = LANGUAGE_SQL_GET_PRIVATE(object);

  switch (prop_id)
  {
    case PROP_DOCUMENT_SCINTILLA:
      if (lgsqldet->doc) g_object_unref(lgsqldet->doc);
      lgsqldet->doc = g_value_dup_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
language_sql_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
  Language_SQLDetails *lgsqldet;
  lgsqldet = LANGUAGE_SQL_GET_PRIVATE(object);

  
  switch (prop_id)
  {
    case PROP_DOCUMENT_SCINTILLA:
      g_value_set_object (value, lgsqldet->doc);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}


/*
* disposes the Gobject
*/
static void language_sql_dispose (GObject *object)
{
  Language_SQLDetails *lgsqldet;
  lgsqldet = LANGUAGE_SQL_GET_PRIVATE(object);
  g_object_unref(lgsqldet->prefmg);
  g_object_unref(lgsqldet->doc);
  g_object_unref(lgsqldet->symbolmg);
  /* Chain up to the parent class */
  G_OBJECT_CLASS (language_sql_parent_class)->dispose (object);
}

static void language_sql_constructed (GObject *object)
{
  Language_SQLDetails *lgsqldet;
  lgsqldet = LANGUAGE_SQL_GET_PRIVATE(object);
  GtkScintilla *sci;
  g_object_get(lgsqldet->doc, "scintilla", &sci, NULL);
  lgsqldet->sci = sci;
}

static void
language_sql_class_init (Language_SQLClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = language_sql_dispose;
  object_class->set_property = language_sql_set_property;
  object_class->get_property = language_sql_get_property;
  object_class->constructed = language_sql_constructed;
  g_object_class_install_property (object_class,
                              PROP_DOCUMENT_SCINTILLA,
                              g_param_spec_object ("document_scintilla",
                              NULL, NULL,
                              DOCUMENT_SCINTILLA_TYPE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_type_class_add_private (klass, sizeof (Language_SQLDetails));
}

static void
language_sql_init (Language_SQL * object)
{
  Language_SQLDetails *lgsqldet;
  lgsqldet = LANGUAGE_SQL_GET_PRIVATE(object);
  lgsqldet->prefmg = preferences_manager_new ();
  lgsqldet->symbolmg = symbol_manager_new ();
}

Language_SQL *language_sql_new (Document_Scintilla *doc)
{
  Language_SQL *lgsql;
  lgsql = g_object_new (LANGUAGE_SQL_TYPE, "document_scintilla", doc, NULL);
  return lgsql; /* return new object */
}

typedef struct {
 Language_SQL *lgsql;
 gint old_pos;
} Calltip_Data;

static gboolean auto_complete_callback(gpointer data)
{
  Calltip_Data *dat = (Calltip_Data *) data;
  Language_SQLDetails *lgsqldet = LANGUAGE_SQL_GET_PRIVATE(dat->lgsql);
  gint current_pos;
  current_pos = documentable_get_current_position(lgsqldet->doc);
  GtkScintilla *scintilla = lgsqldet->sci;
  if (current_pos == dat->old_pos) {
    gchar *prefix = documentable_get_current_word(lgsqldet->doc);
    gchar *calltip = symbol_manager_get_symbols_matches (lgsqldet->symbolmg, prefix, SYMBOL_FUNCTION | SYMBOL_CLASS | SYMBOL_VAR, TAB_SQL);
    if (calltip && strlen(calltip)!=0) gtk_scintilla_user_list_show(GTK_SCINTILLA(scintilla), 1, calltip);
    g_free(prefix);
    g_free(calltip);
  }
  g_slice_free(Calltip_Data, dat);
  lgsqldet->completion_timer_set=FALSE;
  return FALSE;
}

static void show_autocompletion (Language_SQL *lgsql, gint pos)
{
  Language_SQLDetails *lgsqldet = LANGUAGE_SQL_GET_PRIVATE(lgsql);
  if (!lgsqldet->completion_timer_set) {
    gint delay;
    g_object_get(lgsqldet->prefmg, "autocomplete_delay", &delay, NULL);
    Calltip_Data *dat = g_slice_new(Calltip_Data);
    dat->lgsql = lgsql;
    dat->old_pos = pos;
    lgsqldet->completion_timer_id = g_timeout_add(delay, auto_complete_callback, dat);
    lgsqldet->completion_timer_set=TRUE;
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

  current_pos = gtk_scintilla_get_current_pos(sci);
  current_line = gtk_scintilla_line_from_position(sci, current_pos);

  gphpedit_debug (DEBUG_DOCUMENT);

  if (current_line>0) {
    gtk_scintilla_begin_undo_action(sci);
    previous_line = current_line-1;
    previous_line_indentation = gtk_scintilla_get_line_indentation(sci, previous_line);

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

static void language_sql_trigger_completion (Language_Provider *lgsql, guint ch)
{
  g_return_if_fail(lgsql);
  Language_SQLDetails *lgsqldet = LANGUAGE_SQL_GET_PRIVATE(lgsql);
  gint current_pos;
  gchar *member_function_buffer = NULL;

  current_pos = gtk_scintilla_get_current_pos(lgsqldet->sci);
  gboolean auto_brace;
  g_object_get(lgsqldet->prefmg, "auto_complete_braces", &auto_brace, NULL);

  if (IsOpenBrace(ch) && auto_brace) {
    InsertCloseBrace (lgsqldet->sci, current_pos, ch);
    return;
  }

  switch(ch) {
    case ('\r'):
    case ('\n'):
        autoindent_brace_code (lgsqldet->sci, lgsqldet->prefmg);
        break;
    default:
        member_function_buffer = documentable_get_current_word(lgsqldet->doc);
        if (member_function_buffer && strlen(member_function_buffer)>=3) show_autocompletion (LANGUAGE_SQL(lgsql), current_pos);
        g_free(member_function_buffer);
  }
}

static gchar *language_sql_do_syntax_check(Language_Provider *lgsql)
{
  return NULL;
}

static void language_sql_setup_lexer(Language_Provider *lgsql)
{
  g_return_if_fail(lgsql);
  Language_SQLDetails *lgsqldet = LANGUAGE_SQL_GET_PRIVATE(lgsql);

  gtk_scintilla_clear_document_style (lgsqldet->sci);
  gtk_scintilla_set_lexer(lgsqldet->sci, SCLEX_SQL);
  gtk_scintilla_set_style_bits(lgsqldet->sci, 5);

  gtk_scintilla_set_keywords(lgsqldet->sci, 0, "absolute action add admin after aggregate alias all allocate alter and any are array as asc assertion at authorization before begin binary bit blob boolean both breadth by call cascade cascaded case cast catalog char character check class clob close collate collation column commit completion connect connection constraint constraints constructor continue corresponding create cross cube current current_date current_path current_role current_time current_timestamp current_user cursor cycle data date day deallocate dec decimal declare default deferrable deferred delete depth deref desc describe descriptor destroy destructor deterministic dictionary diagnostics disconnect distinct domain double drop dynamic each else end end-exec equals escape every except exception exec execute external false fetch first float for foreign found from free full function general get global go goto grant group grouping having host hour identity if ignore immediate in indicator initialize initially inner inout input insert int integer intersect interval into is isolation iterate join key language large last lateral leading left less level like limit local localtime localtimestamp locator map match minute modifies modify module month names national natural nchar nclob new next no none not null numeric object of off old on only open operation option or order ordinality out outer output pad parameter parameters partial path postfix precision prefix preorder prepare preserve primary prior privileges procedure public read reads real recursive ref references referencing relative restrict result return returns revoke right role rollback rollup routine row rows savepoint schema scroll scope search second section select sequence session session_user set sets size smallint some| space specific specifictype sql sqlexception sqlstate sqlwarning start state statement static structure system_user table temporary terminate than then time timestamp timezone_hour timezone_minute to trailing transaction translation treat trigger true under union unique unknown unnest update usage user using value values varchar variable varying view when whenever where with without work write year zone");

gtk_scintilla_set_keywords(lgsqldet->sci, 1, "all alter and any array as asc at authid avg begin between binary_integer body boolean bulk by char char_base check close cluster collect comment commit compress connect constant create current currval cursor date day declare decimal default delete desc distinct do drop else elsif end exception exclusive execute exists exit extends false fetch float for forall from function goto group having heap hour if immediate in index indicator insert integer interface intersect interval into is isolation java level like limited lock long loop max min minus minute mlslabel mod mode month natural naturaln new nextval nocopy not nowait null number number_base ocirowid of on opaque open operator option or order organization others out package partition pctfree pls_integer positive positiven pragma prior private procedure public raise range raw real record ref release return reverse rollback row rowid rownum rowtype savepoint second select separate set share smallint space sql sqlcode sqlerrm start stddev subtype successful sum synonym sysdate table then time timestamp to trigger true type uid union unique update use user validate values varchar varchar2 variance view when whenever where while with work write year zone");

  gchar *font;
  guint size;
  g_object_get(lgsqldet->prefmg, "style_font_name", &font, "font_size", &size, NULL);
  gchar *style_name;
  g_object_get(lgsqldet->prefmg, "style_name", &style_name, NULL);

  GtkSourceStyleSchemeManager *stylemg = gtk_source_style_scheme_manager_new();
  GtkSourceStyleScheme	*scheme = gtk_source_style_scheme_manager_get_scheme (stylemg, style_name);

  /* SQL LEXER STYLE */
  set_scintilla_lexer_default_style(GTK_WIDGET(lgsqldet->sci), scheme, SCE_SQL_DEFAULT, font, size);
  set_scintilla_lexer_keyword_style(GTK_WIDGET(lgsqldet->sci), scheme, SCE_SQL_WORD, font, size);
  set_scintilla_lexer_xml_element_style(GTK_WIDGET(lgsqldet->sci), scheme, SCE_SQL_WORD2, font, size);
  set_scintilla_lexer_keyword_style(GTK_WIDGET(lgsqldet->sci), scheme, SCE_SQL_SQLPLUS, font, size);
  set_scintilla_lexer_keyword_style(GTK_WIDGET(lgsqldet->sci), scheme, SCE_SQL_SQLPLUS_PROMPT, font, size);
  set_scintilla_lexer_string_style(GTK_WIDGET(lgsqldet->sci), scheme, SCE_SQL_STRING, font, size);
  set_scintilla_lexer_simple_string_style(GTK_WIDGET(lgsqldet->sci), scheme, SCE_SQL_CHARACTER, font, size);
  set_scintilla_lexer_operator_style(GTK_WIDGET(lgsqldet->sci), scheme, SCE_SQL_OPERATOR, font, size);
  set_scintilla_lexer_number_style(GTK_WIDGET(lgsqldet->sci), scheme, SCE_SQL_NUMBER, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(lgsqldet->sci), scheme, SCE_SQL_COMMENT, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(lgsqldet->sci), scheme, SCE_SQL_COMMENTLINE, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(lgsqldet->sci), scheme, SCE_SQL_SQLPLUS_COMMENT, font, size);
  set_scintilla_lexer_doc_comment_style(GTK_WIDGET(lgsqldet->sci), scheme, SCE_SQL_COMMENTDOC, font, size);
  set_scintilla_lexer_doc_comment_style(GTK_WIDGET(lgsqldet->sci), scheme, SCE_SQL_COMMENTDOCKEYWORD, font, size);
  set_scintilla_lexer_doc_comment_style(GTK_WIDGET(lgsqldet->sci), scheme, SCE_SQL_COMMENTLINEDOC, font, size);
  set_scintilla_lexer_variable_style(GTK_WIDGET(lgsqldet->sci), scheme, SCE_SQL_IDENTIFIER, font, size);
  set_scintilla_lexer_special_constant_style (GTK_WIDGET(lgsqldet->sci), scheme, SCE_SQL_QUOTEDIDENTIFIER, font, size);
  set_scintilla_lexer_default_style(GTK_WIDGET(lgsqldet->sci), scheme, SCE_SQL_USER1, font, size);
  set_scintilla_lexer_default_style(GTK_WIDGET(lgsqldet->sci), scheme, SCE_SQL_USER2, font, size);
  set_scintilla_lexer_default_style(GTK_WIDGET(lgsqldet->sci), scheme, SCE_SQL_USER3, font, size);
  set_scintilla_lexer_default_style(GTK_WIDGET(lgsqldet->sci), scheme, SCE_SQL_USER4, font, size);

  gtk_scintilla_colourise(lgsqldet->sci, 0, -1);
  g_free(font);
  g_free(style_name);
}
