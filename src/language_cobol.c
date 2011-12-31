/* This file is part of gPHPEdit, a GNOME PHP Editor.
 
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009, 2011 José Rostagno (for vijona.com.ar)
	  
   For more information or to find the latest release, visit our 
   website at http://www.gcoboledit.org/
 
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
#include "main_window.h"

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
static void language_cobol_setup_lexer(Language_Provider *lgcobol);
static gchar *language_cobol_do_syntax_check(Language_Provider *lgcobol);

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
  iface->setup_lexer = language_cobol_setup_lexer;
  iface->do_syntax_check = language_cobol_do_syntax_check;
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

static gchar *language_cobol_do_syntax_check(Language_Provider *lgcobol)
{
  return NULL;
}

static void language_cobol_setup_lexer(Language_Provider *lgcobol)
{
  g_return_if_fail(lgcobol);
  Language_COBOLDetails *lgcoboldet = LANGUAGE_COBOL_GET_PRIVATE(lgcobol);

  gtk_scintilla_clear_document_style (lgcoboldet->sci);
  gtk_scintilla_set_lexer(lgcoboldet->sci, SCLEX_COBOL);
  gtk_scintilla_set_style_bits(lgcoboldet->sci, 7);

  gtk_scintilla_set_keywords(lgcoboldet->sci, 0, "configuration data declaratives division environment environment-division file file-control function i-o i-o-control identification input input-output linkage local-storage output procedure program program-id receive-control section special-names working-storage");
  gtk_scintilla_set_keywords(lgcoboldet->sci, 1, "accept add alter apply assign call chain close compute continue control convert copy count delete display divide draw drop eject else enable end-accept end-add end-call end-chain end-compute end-delete end-display end-divide end-evaluate end-if end-invoke end-multiply end-perform end-read end-receive end-return end-rewrite end-search end-start end-string end-subtract end-unstring end-write erase evaluate examine exec execute exit go goback generate if ignore initialize initiate insert inspect invoke leave merge move multiply open otherwise perform print read receive release reload replace report reread rerun reserve reset return rewind rewrite rollback run search seek select send set sort start stop store string subtract sum suppress terminate then transform unlock unstring update use wait when wrap write");
  gtk_scintilla_set_keywords(lgcoboldet->sci, 2, "a access acquire actual address advancing after all allowing alphabet alphabetic alphabetic-lower alphabetic-upper alphanumeric alphanumeric-edited also alternate and any are area areas as ascending at attribute author auto auto-hyphen-skip auto-skip automatic autoterminate background-color background-colour backward basis beep before beginning bell binary blank blink blinking block bold bottom box boxed by c01 c02 c03 c04 c05 c06 c07 c08 c09 c10 c11 c12 cancel cbl cd centered cf ch chaining changed character characters chart class clock-units cobol code code-set col collating color colour column com-reg comma command-line commit commitment common communication comp comp-0 comp-1 comp-2 comp-3 comp-4 comp-5 comp-6 comp-x compression computational computational-1 computational-2 computational-3 computational-4 computational-5 computational-6 computational-x computational console contains content control-area controls conversion converting core-index corr corresponding crt crt-under csp currency current-date cursor cycle cyl-index cyl-overflow date date-compiled date-written day day-of-week dbcs de debug debug-contents debug-item debug-line debug-name debug-sub-1 debug-sub-2 debug-sub-3 debugging decimal-point default delimited delimiter depending descending destination detail disable disk disp display-1 display-st down duplicates dynamic echo egcs egi emi empty-check encryption end end-of-page ending enter entry eol eop eos equal equals error escape esi every exceeds exception excess-3 exclusive exhibit extend extended-search external externally-described-key factory false fd fh--fcd fh--keydef file-id file-limit file-limits file-prefix filler final first fixed footing for foreground-color foreground-colour footing format from full giving global greater grid group heading high high-value high-values highlight id in index indexed indic indicate indicator indicators inheriting initial installation into invalid invoked is japanese just justified kanji kept key keyboard label last leading left left-justify leftline length length-check less limit limits lin linage linage-counter line line-counter lines lock lock-holding locking low low-value low-values lower lowlight manual mass-update master-index memory message method mode modified modules more-labels multiple name named national national-edited native nchar negative next no no-echo nominal not note nstd-reels null nulls number numeric numeric-edited numeric-fill o-fill object object-computer object-storage occurs of off omitted on oostackptr optional or order organization other others overflow overline packed-decimal padding page page-counter packed-decimal paragraph password pf ph pic picture plus pointer pop-up pos position positioning positive previous print-control print-switch printer printer-1 printing prior private procedure-pointer procedures proceed process processing prompt protected public purge queue quote quotes random range rd readers ready record record-overflow recording records redefines reel reference references relative remainder remarks removal renames reorg-criteria repeated replacing reporting reports required resident return-code returning reverse reverse-video reversed rf rh right right-justify rolling rounded s01 s02 s03 s04 s05 same screen scroll sd secure security segment segment-limit selective self selfclass sentence separate sequence sequential service setshadow shift-in shift-out sign size skip1 skip2 skip3 sort-control sort-core-size sort-file-size sort-merge sort-message sort-mode-size sort-option sort-return source source-computer space spaces space-fill spaces standard standard-1 standard-2 starting status sub-queue-1 sub-queue-2 sub-queue-3 subfile super symbolic sync synchronized sysin sysipt syslst sysout syspch syspunch system-info tab tallying tape terminal terminal-info test text than through thru time time-of-day time-out timeout times title to top totaled totaling trace track-area track-limit tracks trailing trailing-sign transaction true type typedef underline underlined unequal unit until up updaters upon upper upsi-0 upsi-1 upsi-2 upsi-3 upsi-4 upsi-5 upsi-6 upsi-7 usage user using value values variable varying when-compiled window with words write-only write-verify writerszero x zero zero-fill zeros zeroes");

  gchar *font;
  guint size;
  g_object_get(lgcoboldet->prefmg, "style_font_name", &font,"font_size", &size, NULL);

  gchar *style_name;
  g_object_get(lgcoboldet->prefmg, "style_name", &style_name, NULL);

  GtkSourceStyleScheme	*scheme = gtk_source_style_scheme_manager_get_scheme (main_window.stylemg, style_name);
  /* COBOL LEXER STYLE */
  set_scintilla_lexer_default_style(GTK_WIDGET(lgcoboldet->sci), scheme, SCE_C_DEFAULT, font, size);
  set_scintilla_lexer_variable_style(GTK_WIDGET(lgcoboldet->sci), scheme, SCE_C_WORD, font, size);
  set_scintilla_lexer_variable_style(GTK_WIDGET(lgcoboldet->sci), scheme, SCE_C_WORD2, font, size);
  set_scintilla_lexer_string_style(GTK_WIDGET(lgcoboldet->sci), scheme, SCE_C_STRING, font, size);
  set_scintilla_lexer_simple_string_style(GTK_WIDGET(lgcoboldet->sci), scheme, SCE_C_CHARACTER, font, size);
  set_scintilla_lexer_operator_style(GTK_WIDGET(lgcoboldet->sci), scheme, SCE_C_OPERATOR, font, size);
  set_scintilla_lexer_number_style(GTK_WIDGET(lgcoboldet->sci), scheme, SCE_C_NUMBER, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(lgcoboldet->sci), scheme, SCE_C_COMMENT, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(lgcoboldet->sci), scheme, SCE_C_COMMENTLINE, font, size);
  set_scintilla_lexer_doc_comment_style(GTK_WIDGET(lgcoboldet->sci), scheme, SCE_C_COMMENTDOC, font, size);
  set_scintilla_lexer_type_style(GTK_WIDGET(lgcoboldet->sci), scheme, SCE_C_IDENTIFIER, font, size);
  set_scintilla_lexer_preprocessor_style(GTK_WIDGET(lgcoboldet->sci), scheme, SCE_C_PREPROCESSOR, font, size);
  set_scintilla_lexer_xml_element_style(GTK_WIDGET(lgcoboldet->sci), scheme, SCE_C_UUID, font, size);

  g_free(font);
  g_free(style_name);

  gtk_scintilla_set_property(lgcoboldet->sci, "fold.compact", "1");
  gtk_scintilla_set_property(lgcoboldet->sci, "fold", "1");
  gtk_scintilla_colourise(lgcoboldet->sci, 0, -1);

  gtk_scintilla_set_word_chars(GTK_SCINTILLA(lgcoboldet->sci), "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-");  
}

