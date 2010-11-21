/* This file is part of gPHPEdit, a GNOME2 PHP Editor.
 
   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009 José Rostagno(for vijona.com.ar)
    
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

#include "tab_css.h"
#include "tab_util.h"
#include "preferences_manager.h"
#include "main_window.h"
#include "gtksourcestyleschememanager.h"


void tab_css_set_lexer(GtkScintilla *scintilla)
{

  PreferencesManager *prefmg = preferences_manager_new ();
  gtk_scintilla_clear_document_style (scintilla);
  gtk_scintilla_set_lexer(scintilla, SCLEX_CSS);
  gtk_scintilla_set_style_bits(scintilla, 5);

  gtk_scintilla_set_keywords(scintilla, 0, "aqua auto background background-attachment background-color background-image background-position background-repeat baseline black blink block blue bold bolder border border-bottom border-bottom-width border-color border-left border-left-width border-right border-right-width border-style border-top border-top-width border-width both bottom capitalize center circle clear color dashed decimal disc display dotted double fixed float font font-family font-size font-style font-variant font-weight fuchsia gray green groove height inline inset inside italic justify large larger left letter-spacing lighter lime line-height line-through list-item list-style list-style-image list-style-position list-style-type lower-alpha lower-roman lowercase margin margin-bottom margin-left margin-right margin-top maroon medium medium middle navy no-repeat no-wrap none normal oblique olive outset outside overline padding padding-bottom padding-left padding-right padding-top pre purple red repeat repeat-x repeat-y ridge right scroll silver small small-caps smaller solid square sub super teal text-align text-bottom text-decoration text-indent text-top text-transform thick thin top transparent underline upper-alpha upper-roman uppercase vertical-align white white-space width word-spacing x-large x-small xx-large xx-small yellow");
  gtk_scintilla_set_keywords(scintilla, 1, "active after before first first-child first-letter first-line focus hover lang left link right visited");
  gtk_scintilla_set_keywords(scintilla, 2, "ActiveBorder ActiveCaption AppWorkspace Background ButtonFace ButtonHighlight ButtonShadow ButtonText CaptionText GrayText Highlight HighlightText InactiveBorder InactiveCaption InactiveCaptionText InfoBackground InfoText Menu MenuText Scrollbar ThreeDDarkShadow ThreeDFace ThreeDHighlight ThreeDLightShadow ThreeDShadow Window WindowFrame WindowText above absolute always armenian ascent avoid azimuth baseline bbox behind below bidi-override border-bottom-color border-bottom-style border-collapse border-color border-left-color border-left-style border-right-color border-right-style border-spacing border-style border-top-color border-top-style bottom cap-height caption caption-side center-left center-right centerline child cjk-ideographic clip close-quote code collapse compact condensed content continous counter-increment counter-reset crop cross crosshair cue cue-after cue-before cursor decimal-leading-zero default definition-src descent digits direction e-resize elevation embed empty-cells expanded extra-condensed extra-expanded far-left far-right fast faster female fixed font-size-adjust font-stretch georgian hebrew help hidden hide high higher hiragana hiragana-iroha icon inherit inline-table katakana katakana-iroha landscape left left-side leftwards level loud low lower lower-greek lower-latin ltr male marker marker-offset marks mathline max-height max-width medium medium menu message-box min-height min-width mix move n-resize narrower ne-resize no-close-quote no-open-quote nw-resize once open-quote orphans outline outline-color outline-style outline-width overflow page page-break-after page-break-before page-break-inside panose-1 pause pause-after pause-before pitch pitch-range play-during pointer portrait position quotes relative richness right right-side rightwards rtl run-in s-resize scroll se-resize semi-condensed semi-expanded separate show silent size slope slow slower small-caption soft speak speak-header speak-numeral speak-punctuation speech-rate spell-out src static status-bar stemh stemv stress sw-resize table table-caption table-cell table-column table-column-group table-footer-group table-header-group table-layout table-row table-row-group text text-shadow top topline ultra-condensed ultra-expanded unicode-bidi unicode-range units-per-em upper-latin visibility visible voice-family volume w-resize wait wider widows widths x-fast x-height x-high x-loud x-low x-slow x-soft z-index");
  gtk_scintilla_set_keywords(scintilla, 4, "@charset @font-face @import @media @page");

  const gchar *font;
  guint size;
  g_object_get(prefmg, "style_font_name", &font,"font_size", &size, NULL);

  const gchar *style_name;
  g_object_get(prefmg, "style_name", &style_name, NULL);

  GtkSourceStyleScheme	*scheme = gtk_source_style_scheme_manager_get_scheme (main_window.stylemg, style_name);
  /* COBOL LEXER STYLE */
  set_scintilla_lexer_default_style(GTK_WIDGET(scintilla), scheme, SCE_CSS_DEFAULT, font, size);
  set_scintilla_lexer_keyword_style(GTK_WIDGET(scintilla), scheme, SCE_CSS_TAG, font, size);
  set_scintilla_lexer_variable_style(GTK_WIDGET(scintilla), scheme, SCE_CSS_CLASS, font, size);
  set_scintilla_lexer_preprocessor_style(GTK_WIDGET(scintilla), scheme, SCE_CSS_PSEUDOCLASS, font, size);
  set_scintilla_lexer_xml_element_style(GTK_WIDGET(scintilla), scheme, SCE_CSS_UNKNOWN_PSEUDOCLASS, font, size);
  set_scintilla_lexer_operator_style(GTK_WIDGET(scintilla), scheme, SCE_CSS_OPERATOR, font, size);
  set_scintilla_lexer_keyword_style(GTK_WIDGET(scintilla), scheme, SCE_CSS_IDENTIFIER, font, size);
  set_scintilla_lexer_xml_element_style(GTK_WIDGET(scintilla), scheme, SCE_CSS_UNKNOWN_IDENTIFIER, font, size);
  set_scintilla_lexer_string_style(GTK_WIDGET(scintilla), scheme, SCE_C_STRING, font, size);
  set_scintilla_lexer_simple_string_style(GTK_WIDGET(scintilla), scheme, SCE_CSS_ID, font, size);
  set_scintilla_lexer_number_style(GTK_WIDGET(scintilla), scheme, SCE_CSS_VALUE, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(scintilla), scheme, SCE_CSS_COMMENT, font, size);
  set_scintilla_lexer_special_constant_style (GTK_WIDGET(scintilla), scheme, SCE_CSS_IMPORTANT, font, size);
  set_scintilla_lexer_special_constant_style (GTK_WIDGET(scintilla), scheme, SCE_CSS_MEDIA, font, size);
  set_scintilla_lexer_xml_instruction_style(GTK_WIDGET(scintilla), scheme, SCE_CSS_DIRECTIVE, font, size);

  gtk_scintilla_colourise(scintilla, 0, -1);

  gtk_scintilla_set_property(scintilla, "fold.comment", "1");
  gtk_scintilla_set_property(scintilla, "fold.css", "1");
  gtk_scintilla_set_property(scintilla, "fold", "1");
  
  gtk_scintilla_autoc_set_fill_ups(scintilla, ":");
  gtk_scintilla_set_word_chars(scintilla, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-");

  g_object_unref(prefmg);
}

gboolean is_css_file(const gchar *filename)
{
  if (g_str_has_suffix(filename,".css"))
      return TRUE;
  return FALSE;
}

gboolean is_css_char_autoindent(char ch)
{
  return (ch == '{');
}

gboolean is_css_char_autounindent(char ch)
{
  return (ch == '}');
}

