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

  gtk_scintilla_set_keywords(GTK_SCINTILLA(editor->scintilla), 0, "font-family font-style font-variant font-weight font-size font color background-color background-image background-repeat background-attachment background-position background word-spacing letter-spacing text-decoration vertical-align text-transform text-align text-indent line-height margin-top margin-right margin-bottom margin-left margin padding-top padding-right padding-bottom padding-left padding border-top-width border-right-width border-bottom-width border-left-width border-width border-top border-right border-bottom border-left border border-color border-style width height float clear display white-space list-style-type list-style-image list-style-position list-style position left right top bottom");

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
