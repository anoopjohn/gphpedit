/* This file is part of gPHPEdit, a GNOME2 PHP Editor.
 
   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
    
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
#include "main_window.h"

void tab_css_set_lexer(Document *document)
{
  GtkScintilla *scintilla = document_get_scintilla(document);
  gtk_scintilla_clear_document_style (scintilla);
  gtk_scintilla_set_lexer(scintilla, SCLEX_CSS);
  gtk_scintilla_set_style_bits(scintilla, 5);

  gchar *font =NULL;
  gint size, fore, back;
  gboolean italic, bold;

  get_preferences_manager_style_settings(main_window.prefmg, "default_style", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_font (scintilla, SCE_CSS_DEFAULT, font);
  gtk_scintilla_style_set_fore (scintilla, SCE_CSS_DEFAULT, fore);
  gtk_scintilla_style_set_back (scintilla, SCE_CSS_DEFAULT, back);
  gtk_scintilla_style_set_size (scintilla, SCE_CSS_DEFAULT, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_CSS_DEFAULT, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_CSS_DEFAULT, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "php_default", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_font (scintilla, STYLE_BRACELIGHT, font);
  gtk_scintilla_style_set_fore (scintilla, STYLE_BRACELIGHT, 16711680);// Matching bracket
  if(get_preferences_manager_higthlight_caret_line (main_window.prefmg))
    gtk_scintilla_style_set_back (scintilla, STYLE_BRACELIGHT, get_preferences_manager_higthlight_caret_line_color(main_window.prefmg));
  else
    gtk_scintilla_style_set_back (scintilla, STYLE_BRACELIGHT, back);
  gtk_scintilla_style_set_size (scintilla, STYLE_BRACELIGHT, size);
  gtk_scintilla_style_set_italic (scintilla, STYLE_BRACELIGHT, italic);
  gtk_scintilla_style_set_bold (scintilla, STYLE_BRACELIGHT, TRUE);
  gtk_scintilla_style_set_font (scintilla, STYLE_BRACEBAD, font);
  gtk_scintilla_style_set_fore (scintilla, STYLE_BRACEBAD, 255);
  if(get_preferences_manager_higthlight_caret_line (main_window.prefmg))
    gtk_scintilla_style_set_back (scintilla, STYLE_BRACEBAD, get_preferences_manager_higthlight_caret_line_color(main_window.prefmg));
  else
    gtk_scintilla_style_set_back (scintilla, STYLE_BRACEBAD, back);
  gtk_scintilla_style_set_size (scintilla, STYLE_BRACEBAD, size);
  gtk_scintilla_style_set_italic (scintilla, STYLE_BRACEBAD, italic);
  gtk_scintilla_style_set_bold (scintilla, STYLE_BRACEBAD, TRUE);

  gtk_scintilla_set_keywords(scintilla, 0, "aqua auto background background-attachment background-color background-image background-position background-repeat baseline black blink block blue bold bolder border border-bottom border-bottom-width border-color border-left border-left-width border-right border-right-width border-style border-top border-top-width border-width both bottom capitalize center circle clear color dashed decimal disc display dotted double fixed float font font-family font-size font-style font-variant font-weight fuchsia gray green groove height inline inset inside italic justify large larger left letter-spacing lighter lime line-height line-through list-item list-style list-style-image list-style-position list-style-type lower-alpha lower-roman lowercase margin margin-bottom margin-left margin-right margin-top maroon medium medium middle navy no-repeat no-wrap none normal oblique olive outset outside overline padding padding-bottom padding-left padding-right padding-top pre purple red repeat repeat-x repeat-y ridge right scroll silver small small-caps smaller solid square sub super teal text-align text-bottom text-decoration text-indent text-top text-transform thick thin top transparent underline upper-alpha upper-roman uppercase vertical-align white white-space width word-spacing x-large x-small xx-large xx-small yellow");
  gtk_scintilla_set_keywords(scintilla, 1, "active after before first first-child first-letter first-line focus hover lang left link right visited");
  gtk_scintilla_set_keywords(scintilla, 2, "ActiveBorder ActiveCaption AppWorkspace Background ButtonFace ButtonHighlight ButtonShadow ButtonText CaptionText GrayText Highlight HighlightText InactiveBorder InactiveCaption InactiveCaptionText InfoBackground InfoText Menu MenuText Scrollbar ThreeDDarkShadow ThreeDFace ThreeDHighlight ThreeDLightShadow ThreeDShadow Window WindowFrame WindowText above absolute always armenian ascent avoid azimuth baseline bbox behind below bidi-override border-bottom-color border-bottom-style border-collapse border-color border-left-color border-left-style border-right-color border-right-style border-spacing border-style border-top-color border-top-style bottom cap-height caption caption-side center-left center-right centerline child cjk-ideographic clip close-quote code collapse compact condensed content continous counter-increment counter-reset crop cross crosshair cue cue-after cue-before cursor decimal-leading-zero default definition-src descent digits direction e-resize elevation embed empty-cells expanded extra-condensed extra-expanded far-left far-right fast faster female fixed font-size-adjust font-stretch georgian hebrew help hidden hide high higher hiragana hiragana-iroha icon inherit inline-table katakana katakana-iroha landscape left left-side leftwards level loud low lower lower-greek lower-latin ltr male marker marker-offset marks mathline max-height max-width medium medium menu message-box min-height min-width mix move n-resize narrower ne-resize no-close-quote no-open-quote nw-resize once open-quote orphans outline outline-color outline-style outline-width overflow page page-break-after page-break-before page-break-inside panose-1 pause pause-after pause-before pitch pitch-range play-during pointer portrait position quotes relative richness right right-side rightwards rtl run-in s-resize scroll se-resize semi-condensed semi-expanded separate show silent size slope slow slower small-caption soft speak speak-header speak-numeral speak-punctuation speech-rate spell-out src static status-bar stemh stemv stress sw-resize table table-caption table-cell table-column table-column-group table-footer-group table-header-group table-layout table-row table-row-group text text-shadow top topline ultra-condensed ultra-expanded unicode-bidi unicode-range units-per-em upper-latin visibility visible voice-family volume w-resize wait wider widows widths x-fast x-height x-high x-loud x-low x-slow x-soft z-index");
  gtk_scintilla_set_keywords(scintilla, 4, "@charset @font-face @import @media @page");

  get_preferences_manager_style_settings(main_window.prefmg, "css_tag", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_back (scintilla, SCE_CSS_TAG, back);
  gtk_scintilla_style_set_font (scintilla, SCE_CSS_TAG, font);
  gtk_scintilla_style_set_fore (scintilla, SCE_CSS_TAG, fore);
  gtk_scintilla_style_set_size (scintilla, SCE_CSS_TAG, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_CSS_TAG, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_CSS_TAG, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "css_class", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_CSS_CLASS, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_CSS_CLASS, font);
  gtk_scintilla_style_set_back (scintilla, SCE_CSS_CLASS, back);
  gtk_scintilla_style_set_size (scintilla, SCE_CSS_CLASS, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_CSS_CLASS, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_CSS_CLASS, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "css_pseudoclass", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_back (scintilla, SCE_CSS_PSEUDOCLASS, back);
  gtk_scintilla_style_set_font (scintilla, SCE_CSS_PSEUDOCLASS, font);
  gtk_scintilla_style_set_fore (scintilla, SCE_CSS_PSEUDOCLASS, fore);
  gtk_scintilla_style_set_size (scintilla, SCE_CSS_PSEUDOCLASS, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_CSS_PSEUDOCLASS, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_CSS_PSEUDOCLASS, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "css_unknown_pseudoclass", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_CSS_UNKNOWN_PSEUDOCLASS, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_CSS_UNKNOWN_PSEUDOCLASS, font);
  gtk_scintilla_style_set_back (scintilla, SCE_CSS_UNKNOWN_PSEUDOCLASS, back);
  gtk_scintilla_style_set_size (scintilla, SCE_CSS_UNKNOWN_PSEUDOCLASS, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_CSS_UNKNOWN_PSEUDOCLASS, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_CSS_UNKNOWN_PSEUDOCLASS, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "css_operator", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_CSS_OPERATOR, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_CSS_OPERATOR, font);
  gtk_scintilla_style_set_back (scintilla, SCE_CSS_OPERATOR, back);
  gtk_scintilla_style_set_size (scintilla, SCE_CSS_OPERATOR, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_CSS_OPERATOR, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_CSS_OPERATOR, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "css_identifier", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_CSS_IDENTIFIER, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_CSS_IDENTIFIER, font);
  gtk_scintilla_style_set_back (scintilla, SCE_CSS_IDENTIFIER, back);
  gtk_scintilla_style_set_size (scintilla, SCE_CSS_IDENTIFIER, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_CSS_IDENTIFIER, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_CSS_IDENTIFIER, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "css_unknown_identifier", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_CSS_UNKNOWN_IDENTIFIER, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_CSS_UNKNOWN_IDENTIFIER, font);
  gtk_scintilla_style_set_back (scintilla, SCE_CSS_UNKNOWN_IDENTIFIER, back);
  gtk_scintilla_style_set_size (scintilla, SCE_CSS_UNKNOWN_IDENTIFIER, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_CSS_UNKNOWN_IDENTIFIER, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_CSS_UNKNOWN_IDENTIFIER, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "css_value", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_CSS_VALUE, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_CSS_VALUE, font);
  gtk_scintilla_style_set_back (scintilla, SCE_CSS_VALUE, back);
  gtk_scintilla_style_set_size (scintilla, SCE_CSS_VALUE, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_CSS_VALUE, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_CSS_VALUE, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "css_comment", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_CSS_COMMENT, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_CSS_COMMENT, font);
  gtk_scintilla_style_set_back (scintilla, SCE_CSS_COMMENT, back);
  gtk_scintilla_style_set_size (scintilla, SCE_CSS_COMMENT, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_CSS_COMMENT, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_CSS_COMMENT, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "css_id", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_font (scintilla, SCE_CSS_ID, font);
  gtk_scintilla_style_set_fore (scintilla, SCE_CSS_ID, fore);
  gtk_scintilla_style_set_back (scintilla, SCE_CSS_ID, back);
  gtk_scintilla_style_set_size (scintilla, SCE_CSS_ID, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_CSS_ID, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_CSS_ID, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "css_important", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_font (scintilla, SCE_CSS_IMPORTANT, font);
  gtk_scintilla_style_set_fore (scintilla, SCE_CSS_IMPORTANT, fore);
  gtk_scintilla_style_set_back (scintilla, SCE_CSS_IMPORTANT, back);
  gtk_scintilla_style_set_size (scintilla, SCE_CSS_IMPORTANT, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_CSS_IMPORTANT, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_CSS_IMPORTANT, bold);

  /* same style*/
  gtk_scintilla_style_set_font (scintilla, SCE_CSS_MEDIA, font);
  gtk_scintilla_style_set_fore (scintilla, SCE_CSS_MEDIA, fore);
  gtk_scintilla_style_set_back (scintilla, SCE_CSS_MEDIA, back);
  gtk_scintilla_style_set_size (scintilla, SCE_CSS_MEDIA, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_CSS_MEDIA, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_CSS_MEDIA, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "css_directive", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_CSS_DIRECTIVE, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_CSS_DIRECTIVE, font);
  gtk_scintilla_style_set_back (scintilla, SCE_CSS_DIRECTIVE, back);
  gtk_scintilla_style_set_size (scintilla, SCE_CSS_DIRECTIVE, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_CSS_DIRECTIVE, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_CSS_DIRECTIVE, bold);


  gtk_scintilla_colourise(scintilla, 0, -1);

  gtk_scintilla_set_property(scintilla, "fold.comment", "1");
  gtk_scintilla_set_property(scintilla, "fold.css", "1");
  gtk_scintilla_set_property(scintilla, "fold", "1");
}
