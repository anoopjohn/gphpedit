/* This file is part of gPHPEdit, a GNOME PHP Editor.
 
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009, 2011 José Rostagno (for vijona.com.ar)
	  
   For more information or to find the latest release, visit our 
   website at http://www.gcssedit.org/
 
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
#include "language_css.h"
#include "preferences_manager.h"
#include "language_provider.h"
#include "symbol_manager.h"
#include "main_window.h"

/*
* language_css private struct
*/
struct Language_CSSDetails
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

#define LANGUAGE_CSS_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    LANGUAGE_CSS_TYPE,\
					    Language_CSSDetails))
enum
{
  PROP_0,
  PROP_DOCUMENT_SCINTILLA
};

static void language_css_language_provider_init(Language_ProviderInterface *iface, gpointer user_data);
static void language_css_trigger_completion (Language_Provider *lgcss, guint ch);
static void show_calltip (Language_Provider *lgcss);
static void language_css_setup_lexer(Language_Provider *lgcss);
static gchar *language_css_do_syntax_check(Language_Provider *lgcss);

G_DEFINE_TYPE_WITH_CODE(Language_CSS, language_css, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE (IFACE_TYPE_LANGUAGE_PROVIDER,
                                                 language_css_language_provider_init));

static void language_css_language_provider_init(Language_ProviderInterface *iface, gpointer user_data)
{
  iface->trigger_completion = language_css_trigger_completion;
  iface->show_calltip = show_calltip;
  iface->setup_lexer = language_css_setup_lexer;
  iface->do_syntax_check = language_css_do_syntax_check;
}

static void
language_css_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
  Language_CSSDetails *lgcssdet;
  lgcssdet = LANGUAGE_CSS_GET_PRIVATE(object);

  switch (prop_id)
  {
    case PROP_DOCUMENT_SCINTILLA:
      if (lgcssdet->doc) g_object_unref(lgcssdet->doc);
      lgcssdet->doc = g_value_dup_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
language_css_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
  Language_CSSDetails *lgcssdet;
  lgcssdet = LANGUAGE_CSS_GET_PRIVATE(object);

  
  switch (prop_id)
  {
    case PROP_DOCUMENT_SCINTILLA:
      g_value_set_object (value, lgcssdet->doc);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}


/*
* disposes the Gobject
*/
static void language_css_dispose (GObject *object)
{
  Language_CSSDetails *lgcssdet;
  lgcssdet = LANGUAGE_CSS_GET_PRIVATE(object);
  g_object_unref(lgcssdet->prefmg);
  g_object_unref(lgcssdet->doc);
  g_object_unref(lgcssdet->symbolmg);
  /* Chain up to the parent class */
  G_OBJECT_CLASS (language_css_parent_class)->dispose (object);
}

static void language_css_constructed (GObject *object)
{
  Language_CSSDetails *lgcssdet;
  lgcssdet = LANGUAGE_CSS_GET_PRIVATE(object);
  GtkScintilla *sci;
  g_object_get(lgcssdet->doc, "scintilla", &sci, NULL);
  lgcssdet->sci = sci;
}

static void
language_css_class_init (Language_CSSClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = language_css_dispose;
  object_class->set_property = language_css_set_property;
  object_class->get_property = language_css_get_property;
  object_class->constructed = language_css_constructed;
  g_object_class_install_property (object_class,
                              PROP_DOCUMENT_SCINTILLA,
                              g_param_spec_object ("document_scintilla",
                              NULL, NULL,
                              DOCUMENT_SCINTILLA_TYPE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_type_class_add_private (klass, sizeof (Language_CSSDetails));
}

static void
language_css_init (Language_CSS * object)
{
  Language_CSSDetails *lgcssdet;
  lgcssdet = LANGUAGE_CSS_GET_PRIVATE(object);
  lgcssdet->prefmg = preferences_manager_new ();
  lgcssdet->symbolmg = symbol_manager_new ();
}

Language_CSS *language_css_new (Document_Scintilla *doc)
{
  Language_CSS *lgcss;
  lgcss = g_object_new (LANGUAGE_CSS_TYPE, "document_scintilla", doc, NULL);
  return lgcss; /* return new object */
}

typedef struct {
 Language_CSS *lgcss;
 gint old_pos;
} Calltip_Data;

static gboolean auto_complete_callback(gpointer data)
{
  Calltip_Data *dat = (Calltip_Data *) data;
  Language_CSSDetails *lgcssdet = LANGUAGE_CSS_GET_PRIVATE(dat->lgcss);
  gint current_pos;
  current_pos = documentable_get_current_position(lgcssdet->doc);
  GtkScintilla *scintilla = lgcssdet->sci;
  if (current_pos == dat->old_pos) {
    gchar *prefix = documentable_get_current_word(lgcssdet->doc);
    gchar *calltip = symbol_manager_get_symbols_matches (lgcssdet->symbolmg, prefix, SYMBOL_FUNCTION | SYMBOL_CLASS | SYMBOL_VAR, TAB_CSS);
    if (calltip && strlen(calltip)!=0) gtk_scintilla_user_list_show(GTK_SCINTILLA(scintilla), 1, calltip);
    g_free(prefix);
    g_free(calltip);
  }
  g_slice_free(Calltip_Data, dat);
  lgcssdet->completion_timer_set=FALSE;
  return FALSE;
}

static void show_autocompletion (Language_CSS *lgcss, gint pos)
{
  Language_CSSDetails *lgcssdet = LANGUAGE_CSS_GET_PRIVATE(lgcss);
  if (!lgcssdet->completion_timer_set) {
    gint delay;
    g_object_get(lgcssdet->prefmg, "autocomplete_delay", &delay, NULL);
    Calltip_Data *dat = g_slice_new(Calltip_Data);
    dat->lgcss = lgcss;
    dat->old_pos = pos;
    lgcssdet->completion_timer_id = g_timeout_add(delay, auto_complete_callback, dat);
    lgcssdet->completion_timer_set=TRUE;
  }
}

static gboolean calltip_callback(gpointer data)
{
  Calltip_Data *dat = (Calltip_Data *) data;
  Language_CSSDetails *lgcssdet = LANGUAGE_CSS_GET_PRIVATE(dat->lgcss);
  gint current_pos;
  current_pos = documentable_get_current_position(lgcssdet->doc);
  GtkScintilla *scintilla = lgcssdet->sci;
  if (current_pos == dat->old_pos) {
    gchar *prefix = documentable_get_current_word(lgcssdet->doc);
    gchar *calltip = symbol_manager_get_calltip (lgcssdet->symbolmg, prefix, TAB_CSS);
    if (calltip){
      gtk_scintilla_call_tip_show(GTK_SCINTILLA(scintilla), current_pos, calltip);
      g_free(calltip);
    }
    g_free(prefix);
  }
  g_slice_free(Calltip_Data, dat);
  lgcssdet->calltip_timer_set = FALSE;
  return FALSE;
}

static void show_calltip (Language_Provider *lgcss)
{
  Language_CSSDetails *lgcssdet = LANGUAGE_CSS_GET_PRIVATE(lgcss);
  if (!lgcssdet->calltip_timer_set) {
    gint delay;
    g_object_get(lgcssdet->prefmg, "calltip_delay", &delay, NULL);
    Calltip_Data *dat = g_slice_new(Calltip_Data);
    dat->lgcss = LANGUAGE_CSS(lgcss);
    dat->old_pos = documentable_get_current_position(lgcssdet->doc);
    lgcssdet->calltip_timer_id = g_timeout_add(delay, calltip_callback, dat);
    lgcssdet->calltip_timer_set = TRUE;
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

static gboolean is_css_char_autoindent(char ch)
{
  return (ch == '{');
}

static gboolean is_css_char_autounindent(char ch)
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
    if (is_css_char_autoindent(*previous_char_buffer)) {
      gint indentation_size;
      g_object_get(pref, "indentation_size", &indentation_size, NULL);
      previous_line_indentation+=indentation_size;
    } else if (is_css_char_autounindent(*previous_char_buffer)) {
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
        if (!(g_ascii_iscntrl(c) || g_ascii_isspace(c) || is_css_char_autounindent(c))) {
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

static void language_css_trigger_completion (Language_Provider *lgcss, guint ch)
{
  g_return_if_fail(lgcss);
  Language_CSSDetails *lgcssdet = LANGUAGE_CSS_GET_PRIVATE(lgcss);
  gint current_pos;
  gchar *member_function_buffer = NULL;

  current_pos = gtk_scintilla_get_current_pos(lgcssdet->sci);
  gboolean auto_brace;
  g_object_get(lgcssdet->prefmg, "auto_complete_braces", &auto_brace, NULL);

  if (IsOpenBrace(ch) && auto_brace) {
    InsertCloseBrace (lgcssdet->sci, current_pos, ch);
    return;
  }

  switch(ch) {
    case ('\r'):
    case ('\n'):
        autoindent_brace_code (lgcssdet->sci, lgcssdet->prefmg);
        break;
    case (';'):
        cancel_calltip (lgcssdet->sci);
        break;
    case (':'):
        show_calltip (lgcss);
        break;
    default:
        member_function_buffer = documentable_get_current_word(lgcssdet->doc);
        if (member_function_buffer && strlen(member_function_buffer)>=3) show_autocompletion (LANGUAGE_CSS(lgcss), current_pos);
        g_free(member_function_buffer);
  }
}

static gchar *language_css_do_syntax_check(Language_Provider *lgcss)
{
  return NULL;
}

static void language_css_setup_lexer(Language_Provider *lgcss)
{
  g_return_if_fail(lgcss);
  Language_CSSDetails *lgcssdet = LANGUAGE_CSS_GET_PRIVATE(lgcss);

  gtk_scintilla_clear_document_style (lgcssdet->sci);
  gtk_scintilla_set_lexer(lgcssdet->sci, SCLEX_CSS);
  gtk_scintilla_set_style_bits(lgcssdet->sci, 5);

  gtk_scintilla_set_keywords(lgcssdet->sci, 0, "aqua auto background background-attachment background-color background-image background-position background-repeat baseline black blink block blue bold bolder border border-bottom border-bottom-width border-color border-left border-left-width border-right border-right-width border-style border-top border-top-width border-width both bottom capitalize center circle clear color dashed decimal disc display dotted double fixed float font font-family font-size font-style font-variant font-weight fuchsia gray green groove height inline inset inside italic justify large larger left letter-spacing lighter lime line-height line-through list-item list-style list-style-image list-style-position list-style-type lower-alpha lower-roman lowercase margin margin-bottom margin-left margin-right margin-top maroon medium medium middle navy no-repeat no-wrap none normal oblique olive outset outside overline padding padding-bottom padding-left padding-right padding-top pre purple red repeat repeat-x repeat-y ridge right scroll silver small small-caps smaller solid square sub super teal text-align text-bottom text-decoration text-indent text-top text-transform thick thin top transparent underline upper-alpha upper-roman uppercase vertical-align white white-space width word-spacing x-large x-small xx-large xx-small yellow");
  gtk_scintilla_set_keywords(lgcssdet->sci, 1, "active after before first first-child first-letter first-line focus hover lang left link right visited");
  gtk_scintilla_set_keywords(lgcssdet->sci, 2, "ActiveBorder ActiveCaption AppWorkspace Background ButtonFace ButtonHighlight ButtonShadow ButtonText CaptionText GrayText Highlight HighlightText InactiveBorder InactiveCaption InactiveCaptionText InfoBackground InfoText Menu MenuText Scrollbar ThreeDDarkShadow ThreeDFace ThreeDHighlight ThreeDLightShadow ThreeDShadow Window WindowFrame WindowText above absolute always armenian ascent avoid azimuth baseline bbox behind below bidi-override border-bottom-color border-bottom-style border-collapse border-color border-left-color border-left-style border-right-color border-right-style border-spacing border-style border-top-color border-top-style bottom cap-height caption caption-side center-left center-right centerline child cjk-ideographic clip close-quote code collapse compact condensed content continous counter-increment counter-reset crop cross crosshair cue cue-after cue-before cursor decimal-leading-zero default definition-src descent digits direction e-resize elevation embed empty-cells expanded extra-condensed extra-expanded far-left far-right fast faster female fixed font-size-adjust font-stretch georgian hebrew help hidden hide high higher hiragana hiragana-iroha icon inherit inline-table katakana katakana-iroha landscape left left-side leftwards level loud low lower lower-greek lower-latin ltr male marker marker-offset marks mathline max-height max-width medium medium menu message-box min-height min-width mix move n-resize narrower ne-resize no-close-quote no-open-quote nw-resize once open-quote orphans outline outline-color outline-style outline-width overflow page page-break-after page-break-before page-break-inside panose-1 pause pause-after pause-before pitch pitch-range play-during pointer portrait position quotes relative richness right right-side rightwards rtl run-in s-resize scroll se-resize semi-condensed semi-expanded separate show silent size slope slow slower small-caption soft speak speak-header speak-numeral speak-punctuation speech-rate spell-out src static status-bar stemh stemv stress sw-resize table table-caption table-cell table-column table-column-group table-footer-group table-header-group table-layout table-row table-row-group text text-shadow top topline ultra-condensed ultra-expanded unicode-bidi unicode-range units-per-em upper-latin visibility visible voice-family volume w-resize wait wider widows widths x-fast x-height x-high x-loud x-low x-slow x-soft z-index");
  gtk_scintilla_set_keywords(lgcssdet->sci, 4, "@charset @font-face @import @media @page");

  const gchar *font;
  guint size;
  g_object_get(lgcssdet->prefmg, "style_font_name", &font,"font_size", &size, NULL);

  const gchar *style_name;
  g_object_get(lgcssdet->prefmg, "style_name", &style_name, NULL);

  GtkSourceStyleScheme	*scheme = gtk_source_style_scheme_manager_get_scheme (main_window.stylemg, style_name);
  /* COBOL LEXER STYLE */
  set_scintilla_lexer_default_style(GTK_WIDGET(lgcssdet->sci), scheme, SCE_CSS_DEFAULT, font, size);
  set_scintilla_lexer_keyword_style(GTK_WIDGET(lgcssdet->sci), scheme, SCE_CSS_TAG, font, size);
  set_scintilla_lexer_variable_style(GTK_WIDGET(lgcssdet->sci), scheme, SCE_CSS_CLASS, font, size);
  set_scintilla_lexer_preprocessor_style(GTK_WIDGET(lgcssdet->sci), scheme, SCE_CSS_PSEUDOCLASS, font, size);
  set_scintilla_lexer_xml_element_style(GTK_WIDGET(lgcssdet->sci), scheme, SCE_CSS_UNKNOWN_PSEUDOCLASS, font, size);
  set_scintilla_lexer_operator_style(GTK_WIDGET(lgcssdet->sci), scheme, SCE_CSS_OPERATOR, font, size);
  set_scintilla_lexer_keyword_style(GTK_WIDGET(lgcssdet->sci), scheme, SCE_CSS_IDENTIFIER, font, size);
  set_scintilla_lexer_xml_element_style(GTK_WIDGET(lgcssdet->sci), scheme, SCE_CSS_UNKNOWN_IDENTIFIER, font, size);
  set_scintilla_lexer_string_style(GTK_WIDGET(lgcssdet->sci), scheme, SCE_C_STRING, font, size);
  set_scintilla_lexer_simple_string_style(GTK_WIDGET(lgcssdet->sci), scheme, SCE_CSS_ID, font, size);
  set_scintilla_lexer_number_style(GTK_WIDGET(lgcssdet->sci), scheme, SCE_CSS_VALUE, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(lgcssdet->sci), scheme, SCE_CSS_COMMENT, font, size);
  set_scintilla_lexer_special_constant_style (GTK_WIDGET(lgcssdet->sci), scheme, SCE_CSS_IMPORTANT, font, size);
  set_scintilla_lexer_special_constant_style (GTK_WIDGET(lgcssdet->sci), scheme, SCE_CSS_MEDIA, font, size);
  set_scintilla_lexer_xml_instruction_style(GTK_WIDGET(lgcssdet->sci), scheme, SCE_CSS_DIRECTIVE, font, size);

  gtk_scintilla_colourise(lgcssdet->sci, 0, -1);

  gtk_scintilla_set_property(lgcssdet->sci, "fold.comment", "1");
  gtk_scintilla_set_property(lgcssdet->sci, "fold.css", "1");
  gtk_scintilla_set_property(lgcssdet->sci, "fold", "1");
  
  gtk_scintilla_autoc_set_fill_ups(lgcssdet->sci, ":");
  gtk_scintilla_set_word_chars(lgcssdet->sci, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-");
}
