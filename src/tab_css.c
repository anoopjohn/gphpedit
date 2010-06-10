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

#include "tab_php.h"
#include "tab_css.h"
#include "tab_util.h"
#include "preferences.h"

void tab_css_set_lexer(Editor *editor)
{
  gtk_scintilla_clear_document_style (GTK_SCINTILLA(editor->scintilla));
  gtk_scintilla_set_lexer(GTK_SCINTILLA (editor->scintilla), SCLEX_CSS);
  gtk_scintilla_set_style_bits(GTK_SCINTILLA(editor->scintilla), 5);

  /* Example style setting up code! */
  gtk_scintilla_autoc_set_choose_single (GTK_SCINTILLA(editor->scintilla), TRUE);
  gtk_scintilla_set_use_tabs (GTK_SCINTILLA(editor->scintilla), 1);
  gtk_scintilla_set_tab_indents (GTK_SCINTILLA(editor->scintilla), 1);
  gtk_scintilla_set_backspace_unindents (GTK_SCINTILLA(editor->scintilla), 1);
  gtk_scintilla_set_tab_width (GTK_SCINTILLA(editor->scintilla), preferences.indentation_size);
  gtk_scintilla_set_indent (GTK_SCINTILLA(editor->scintilla), preferences.tab_size);

  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), STYLE_DEFAULT, preferences.default_font);
  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), STYLE_DEFAULT, preferences.default_fore);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), STYLE_DEFAULT, preferences.default_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), STYLE_DEFAULT, preferences.default_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), STYLE_DEFAULT, preferences.default_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), STYLE_DEFAULT, preferences.default_bold);

  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_CSS_DEFAULT, preferences.default_font);
  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_CSS_DEFAULT, preferences.default_fore);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_CSS_DEFAULT, preferences.default_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_CSS_DEFAULT, preferences.default_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_CSS_DEFAULT, preferences.default_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_CSS_DEFAULT, preferences.default_bold);

  //gtk_scintilla_set_sel_back (GTK_SCINTILLA(editor->scintilla), TRUE, preferences.set_sel_back);
  //gtk_scintilla_marker_set_back (GTK_SCINTILLA(editor->scintilla), TRUE, preferences.marker_back);

  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), STYLE_BRACELIGHT, preferences.php_default_font);
  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), STYLE_BRACELIGHT, 16711680);// Matching bracket
  if(preferences.higthlightcaretline)
    gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), STYLE_BRACELIGHT, preferences.higthlightcaretline_color);
  else
    gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), STYLE_BRACELIGHT, preferences.php_default_back);

  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), STYLE_BRACELIGHT, preferences.php_default_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), STYLE_BRACELIGHT, preferences.php_default_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), STYLE_BRACELIGHT, TRUE);
  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), STYLE_BRACEBAD, preferences.php_default_font);
  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), STYLE_BRACEBAD, 255);
  if(preferences.higthlightcaretline)
    gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), STYLE_BRACEBAD, preferences.higthlightcaretline_color);
  else
    gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), STYLE_BRACEBAD, preferences.php_default_back);

  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), STYLE_BRACEBAD, preferences.php_default_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), STYLE_BRACEBAD, preferences.php_default_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), STYLE_BRACEBAD, TRUE);
  
  gtk_scintilla_set_indentation_guides (GTK_SCINTILLA(editor->scintilla), preferences.show_indentation_guides);
  gtk_scintilla_set_edge_mode (GTK_SCINTILLA(editor->scintilla), preferences.edge_mode);
  gtk_scintilla_set_edge_column (GTK_SCINTILLA(editor->scintilla), preferences.edge_column);
  gtk_scintilla_set_edge_colour (GTK_SCINTILLA(editor->scintilla), preferences.edge_colour);

  gtk_scintilla_set_keywords(GTK_SCINTILLA(editor->scintilla), 0, "aqua auto background background-attachment background-color background-image background-position background-repeat baseline black blink block blue bold bolder border border-bottom border-bottom-width border-color border-left border-left-width border-right border-right-width border-style border-top border-top-width border-width both bottom capitalize center circle clear color dashed decimal disc display dotted double fixed float font font-family font-size font-style font-variant font-weight fuchsia gray green groove height inline inset inside italic justify large larger left letter-spacing lighter lime line-height line-through list-item list-style list-style-image list-style-position list-style-type lower-alpha lower-roman lowercase margin margin-bottom margin-left margin-right margin-top maroon medium medium middle navy no-repeat no-wrap none normal oblique olive outset outside overline padding padding-bottom padding-left padding-right padding-top pre purple red repeat repeat-x repeat-y ridge right scroll silver small small-caps smaller solid square sub super teal text-align text-bottom text-decoration text-indent text-top text-transform thick thin top transparent underline upper-alpha upper-roman uppercase vertical-align white white-space width word-spacing x-large x-small xx-large xx-small yellow");
  gtk_scintilla_set_keywords(GTK_SCINTILLA(editor->scintilla), 1, "active after before first first-child first-letter first-line focus hover lang left link right visited");
  gtk_scintilla_set_keywords(GTK_SCINTILLA(editor->scintilla), 2, "ActiveBorder ActiveCaption AppWorkspace Background ButtonFace ButtonHighlight ButtonShadow ButtonText CaptionText GrayText Highlight HighlightText InactiveBorder InactiveCaption InactiveCaptionText InfoBackground InfoText Menu MenuText Scrollbar ThreeDDarkShadow ThreeDFace ThreeDHighlight ThreeDLightShadow ThreeDShadow Window WindowFrame WindowText above absolute always armenian ascent avoid azimuth baseline bbox behind below bidi-override border-bottom-color border-bottom-style border-collapse border-color border-left-color border-left-style border-right-color border-right-style border-spacing border-style border-top-color border-top-style bottom cap-height caption caption-side center-left center-right centerline child cjk-ideographic clip close-quote code collapse compact condensed content continous counter-increment counter-reset crop cross crosshair cue cue-after cue-before cursor decimal-leading-zero default definition-src descent digits direction e-resize elevation embed empty-cells expanded extra-condensed extra-expanded far-left far-right fast faster female fixed font-size-adjust font-stretch georgian hebrew help hidden hide high higher hiragana hiragana-iroha icon inherit inline-table katakana katakana-iroha landscape left left-side leftwards level loud low lower lower-greek lower-latin ltr male marker marker-offset marks mathline max-height max-width medium medium menu message-box min-height min-width mix move n-resize narrower ne-resize no-close-quote no-open-quote nw-resize once open-quote orphans outline outline-color outline-style outline-width overflow page page-break-after page-break-before page-break-inside panose-1 pause pause-after pause-before pitch pitch-range play-during pointer portrait position quotes relative richness right right-side rightwards rtl run-in s-resize scroll se-resize semi-condensed semi-expanded separate show silent size slope slow slower small-caption soft speak speak-header speak-numeral speak-punctuation speech-rate spell-out src static status-bar stemh stemv stress sw-resize table table-caption table-cell table-column table-column-group table-footer-group table-header-group table-layout table-row table-row-group text text-shadow top topline ultra-condensed ultra-expanded unicode-bidi unicode-range units-per-em upper-latin visibility visible voice-family volume w-resize wait wider widows widths x-fast x-height x-high x-loud x-low x-slow x-soft z-index");
  gtk_scintilla_set_keywords(GTK_SCINTILLA(editor->scintilla), 4, "@charset @font-face @import @media @page");

  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_CSS_TAG, preferences.css_tag_back);
  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_CSS_TAG, preferences.css_tag_font);
  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_CSS_TAG, preferences.css_tag_fore);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_CSS_TAG, preferences.css_tag_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_CSS_TAG, preferences.css_tag_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_CSS_TAG, preferences.css_tag_bold);

  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_CSS_CLASS, preferences.css_class_fore);
  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_CSS_CLASS, preferences.css_class_font);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_CSS_CLASS, preferences.css_class_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_CSS_CLASS, preferences.css_class_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_CSS_CLASS, preferences.css_class_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_CSS_CLASS, preferences.css_class_bold);

  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_CSS_PSEUDOCLASS, preferences.css_pseudoclass_back);
  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_CSS_PSEUDOCLASS, preferences.css_pseudoclass_font);
  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_CSS_PSEUDOCLASS, preferences.css_pseudoclass_fore);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_CSS_PSEUDOCLASS, preferences.css_pseudoclass_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_CSS_PSEUDOCLASS, preferences.css_pseudoclass_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_CSS_PSEUDOCLASS, preferences.css_pseudoclass_bold);

  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_CSS_UNKNOWN_PSEUDOCLASS, preferences.css_unknown_pseudoclass_fore);
  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_CSS_UNKNOWN_PSEUDOCLASS, preferences.css_unknown_pseudoclass_font);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_CSS_UNKNOWN_PSEUDOCLASS, preferences.css_unknown_pseudoclass_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_CSS_UNKNOWN_PSEUDOCLASS, preferences.css_unknown_pseudoclass_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_CSS_UNKNOWN_PSEUDOCLASS, preferences.css_unknown_pseudoclass_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_CSS_UNKNOWN_PSEUDOCLASS, preferences.css_unknown_pseudoclass_bold);

  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_CSS_OPERATOR, preferences.css_operator_fore);
  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_CSS_OPERATOR, preferences.css_operator_font);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_CSS_OPERATOR, preferences.css_operator_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_CSS_OPERATOR, preferences.css_operator_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_CSS_OPERATOR, preferences.css_operator_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_CSS_OPERATOR, preferences.css_operator_bold);

  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_CSS_IDENTIFIER, preferences.css_identifier_fore);
  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_CSS_IDENTIFIER, preferences.css_identifier_font);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_CSS_IDENTIFIER, preferences.css_identifier_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_CSS_IDENTIFIER, preferences.css_identifier_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_CSS_IDENTIFIER, preferences.css_identifier_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_CSS_IDENTIFIER, preferences.css_identifier_bold);

  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_CSS_UNKNOWN_IDENTIFIER, preferences.css_unknown_identifier_fore);
  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_CSS_UNKNOWN_IDENTIFIER, preferences.css_unknown_identifier_font);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_CSS_UNKNOWN_IDENTIFIER, preferences.css_unknown_identifier_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_CSS_UNKNOWN_IDENTIFIER, preferences.css_unknown_identifier_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_CSS_UNKNOWN_IDENTIFIER, preferences.css_unknown_identifier_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_CSS_UNKNOWN_IDENTIFIER, preferences.css_unknown_identifier_bold);

  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_CSS_VALUE, preferences.css_value_fore);
  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_CSS_VALUE, preferences.css_value_font);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_CSS_VALUE, preferences.css_value_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_CSS_VALUE, preferences.css_value_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_CSS_VALUE, preferences.css_value_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_CSS_VALUE, preferences.css_value_bold);

  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_CSS_COMMENT, preferences.css_comment_fore);
  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_CSS_COMMENT, preferences.css_comment_font);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_CSS_COMMENT, preferences.css_comment_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_CSS_COMMENT, preferences.css_comment_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_CSS_COMMENT, preferences.css_comment_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_CSS_COMMENT, preferences.css_comment_bold);

  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_CSS_ID, preferences.css_id_font);
  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_CSS_ID, preferences.css_id_fore);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_CSS_ID, preferences.css_id_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_CSS_ID, preferences.css_id_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_CSS_ID, preferences.css_id_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_CSS_ID, preferences.css_id_bold);

  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_CSS_IMPORTANT, preferences.css_important_font);
  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_CSS_IMPORTANT, preferences.css_important_fore);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_CSS_IMPORTANT, preferences.css_important_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_CSS_IMPORTANT, preferences.css_important_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_CSS_IMPORTANT, preferences.css_important_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_CSS_IMPORTANT, preferences.css_important_bold);

  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_CSS_MEDIA, preferences.css_important_font);
  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_CSS_MEDIA, preferences.css_important_fore);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_CSS_MEDIA, preferences.css_important_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_CSS_MEDIA, preferences.css_important_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_CSS_MEDIA, preferences.css_important_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_CSS_MEDIA, preferences.css_important_bold);

  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_CSS_DIRECTIVE, preferences.css_directive_fore);
  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_CSS_DIRECTIVE, preferences.css_directive_font);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_CSS_DIRECTIVE, preferences.css_directive_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_CSS_DIRECTIVE, preferences.css_directive_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_CSS_DIRECTIVE, preferences.css_directive_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_CSS_DIRECTIVE, preferences.css_directive_bold);


  gtk_scintilla_colourise(GTK_SCINTILLA (editor->scintilla), 0, -1);

  gtk_scintilla_set_property(GTK_SCINTILLA (editor->scintilla), "fold.comment", "1");
  gtk_scintilla_set_property(GTK_SCINTILLA (editor->scintilla), "fold.css", "1");
  gtk_scintilla_set_property(GTK_SCINTILLA (editor->scintilla), "fold", "1");
}
