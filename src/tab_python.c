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

#include "tab_python.h"
#include "tab_util.h"
#include "preferences.h"

void scintilla_python_set_lexer(GtkScintilla *scintilla, Preferences prefs)
{
  gtk_scintilla_clear_document_style (scintilla);
  // Don't know why - but it doesn't highlight if the next line is SCLEX_PYTHON
  gtk_scintilla_set_lexer(scintilla, SCLEX_PERL);
  gtk_scintilla_set_style_bits(scintilla, 5);

  /* Example style setting up code! */
  gtk_scintilla_autoc_set_choose_single (scintilla, TRUE);
  gtk_scintilla_set_use_tabs (scintilla, 1);
  gtk_scintilla_set_tab_indents (scintilla, 1);
  gtk_scintilla_set_backspace_unindents (scintilla, 1);
  gtk_scintilla_set_tab_width (scintilla, prefs.indentation_size);
  gtk_scintilla_set_indent (scintilla, prefs.tab_size);

  gtk_scintilla_style_set_font (scintilla, STYLE_DEFAULT, prefs.default_font);
  gtk_scintilla_style_set_size (scintilla, STYLE_DEFAULT, prefs.default_size);
  gtk_scintilla_style_set_italic (scintilla, STYLE_DEFAULT, prefs.default_italic);
  gtk_scintilla_style_set_bold (scintilla, STYLE_DEFAULT, prefs.default_bold);
  gtk_scintilla_style_set_fore (scintilla, STYLE_DEFAULT, prefs.default_fore);
  gtk_scintilla_style_set_back (scintilla, STYLE_DEFAULT, prefs.default_back);

  gtk_scintilla_style_set_font (scintilla, SCE_P_DEFAULT, prefs.php_default_font);
  gtk_scintilla_style_set_fore (scintilla, SCE_P_DEFAULT, prefs.php_default_fore);
  gtk_scintilla_style_set_back (scintilla, SCE_P_DEFAULT, prefs.php_default_back);
  gtk_scintilla_style_set_size (scintilla, SCE_P_DEFAULT, prefs.php_default_size);
  gtk_scintilla_style_set_italic (scintilla, SCE_P_DEFAULT, prefs.php_default_italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_P_DEFAULT, prefs.php_default_bold);

  //gtk_scintilla_set_sel_back (GTK_SCINTILLA(editor->scintilla), TRUE, prefs.set_sel_back);
  //gtk_scintilla_marker_set_back (GTK_SCINTILLA(editor->scintilla), TRUE, prefs.marker_back);

  gtk_scintilla_set_caret_fore (scintilla, 0);
  gtk_scintilla_set_caret_width (scintilla, 2);
  gtk_scintilla_set_caret_period (scintilla, 250);

  gtk_scintilla_style_set_font (scintilla, STYLE_BRACELIGHT, prefs.php_default_font);
  gtk_scintilla_style_set_fore (scintilla, STYLE_BRACELIGHT, 16711680);// Matching bracket
  gtk_scintilla_style_set_back (scintilla, STYLE_BRACELIGHT, prefs.php_default_back);
  gtk_scintilla_style_set_size (scintilla, STYLE_BRACELIGHT, prefs.php_default_size);
  gtk_scintilla_style_set_italic (scintilla, STYLE_BRACELIGHT, prefs.php_default_italic);
  gtk_scintilla_style_set_bold (scintilla, STYLE_BRACELIGHT, TRUE);
  gtk_scintilla_style_set_font (scintilla, STYLE_BRACEBAD, prefs.php_default_font);
  gtk_scintilla_style_set_fore (scintilla, STYLE_BRACEBAD, 255);
  gtk_scintilla_style_set_back (scintilla, STYLE_BRACEBAD, prefs.php_default_back);
  gtk_scintilla_style_set_size (scintilla, STYLE_BRACEBAD, prefs.php_default_size);
  gtk_scintilla_style_set_italic (scintilla, STYLE_BRACEBAD, prefs.php_default_italic);
  gtk_scintilla_style_set_bold (scintilla, STYLE_BRACEBAD, TRUE);
  
  gtk_scintilla_set_indentation_guides (scintilla, prefs.show_indentation_guides);
  gtk_scintilla_set_edge_mode (scintilla, prefs.edge_mode);
  gtk_scintilla_set_edge_column (scintilla, prefs.edge_column);
  gtk_scintilla_set_edge_colour (scintilla, prefs.edge_colour);
  // Python keywords
  gtk_scintilla_set_keywords(scintilla, 0, "and assert break class continue def del elif else except exec finally for from global if import in is lambda None not or pass print raise return try while yield");
  
  gtk_scintilla_style_set_fore (scintilla, SCE_P_TRIPLEDOUBLE, prefs.php_hstring_fore);
  gtk_scintilla_style_set_font (scintilla, SCE_P_TRIPLEDOUBLE, prefs.php_hstring_font);
  gtk_scintilla_style_set_back (scintilla, SCE_P_TRIPLEDOUBLE, prefs.php_hstring_back);
  gtk_scintilla_style_set_size (scintilla, SCE_P_TRIPLEDOUBLE, prefs.php_hstring_size);
  gtk_scintilla_style_set_italic (scintilla, SCE_P_TRIPLEDOUBLE, prefs.php_hstring_italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_P_TRIPLEDOUBLE, prefs.php_hstring_bold);
  gtk_scintilla_style_set_fore (scintilla, SCE_P_TRIPLE, prefs.php_hstring_fore);
  gtk_scintilla_style_set_font (scintilla, SCE_P_TRIPLE, prefs.php_hstring_font);
  gtk_scintilla_style_set_back (scintilla, SCE_P_TRIPLE, prefs.php_hstring_back);
  gtk_scintilla_style_set_size (scintilla, SCE_P_TRIPLE, prefs.php_hstring_size);
  gtk_scintilla_style_set_italic (scintilla, SCE_P_TRIPLE, prefs.php_hstring_italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_P_TRIPLE, prefs.php_hstring_bold);
    
  gtk_scintilla_style_set_fore (scintilla, SCE_P_STRING, prefs.php_simplestring_fore);
  gtk_scintilla_style_set_font (scintilla, SCE_P_STRING, prefs.php_simplestring_font);
  gtk_scintilla_style_set_back (scintilla, SCE_P_STRING, prefs.php_simplestring_back);
  gtk_scintilla_style_set_size (scintilla, SCE_P_STRING, prefs.php_simplestring_size);
  gtk_scintilla_style_set_italic (scintilla, SCE_P_STRING, prefs.php_simplestring_italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_P_STRING, prefs.php_simplestring_bold);
  gtk_scintilla_style_set_fore (scintilla, SCE_P_CHARACTER, prefs.php_simplestring_fore);
  gtk_scintilla_style_set_font (scintilla, SCE_P_CHARACTER, prefs.php_simplestring_font);
  gtk_scintilla_style_set_back (scintilla, SCE_P_CHARACTER, prefs.php_simplestring_back);
  gtk_scintilla_style_set_size (scintilla, SCE_P_CHARACTER, prefs.php_simplestring_size);
  gtk_scintilla_style_set_italic (scintilla, SCE_P_CHARACTER, prefs.php_simplestring_italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_P_CHARACTER, prefs.php_simplestring_bold);
  
  gtk_scintilla_style_set_font (scintilla, SCE_P_WORD, prefs.php_word_font);
  gtk_scintilla_style_set_fore (scintilla, SCE_P_WORD, prefs.php_word_fore);
  gtk_scintilla_style_set_back (scintilla, SCE_P_WORD, prefs.php_word_back);
  gtk_scintilla_style_set_size (scintilla, SCE_P_WORD, prefs.php_word_size);
  gtk_scintilla_style_set_italic (scintilla, SCE_P_WORD, prefs.php_word_italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_P_WORD, prefs.php_word_bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_P_NUMBER, prefs.php_number_fore);
  gtk_scintilla_style_set_font (scintilla, SCE_P_NUMBER, prefs.php_number_font);
  gtk_scintilla_style_set_back (scintilla, SCE_P_NUMBER, prefs.php_number_back);
  gtk_scintilla_style_set_size (scintilla, SCE_P_NUMBER, prefs.php_number_size);
  gtk_scintilla_style_set_italic (scintilla, SCE_P_NUMBER, prefs.php_number_italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_P_NUMBER, prefs.php_number_bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_P_IDENTIFIER, prefs.php_default_fore);
  gtk_scintilla_style_set_font (scintilla, SCE_P_IDENTIFIER, prefs.php_default_font);
  gtk_scintilla_style_set_back (scintilla, SCE_P_IDENTIFIER, prefs.php_default_back);
  gtk_scintilla_style_set_size (scintilla, SCE_P_IDENTIFIER, prefs.php_default_size);
  gtk_scintilla_style_set_italic (scintilla, SCE_P_IDENTIFIER, prefs.php_default_italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_P_IDENTIFIER, prefs.php_default_bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_P_COMMENTLINE, prefs.php_comment_line_fore);
  gtk_scintilla_style_set_font (scintilla, SCE_P_COMMENTLINE, prefs.php_comment_line_font);
  gtk_scintilla_style_set_back (scintilla, SCE_P_COMMENTLINE, prefs.php_comment_line_back);
  gtk_scintilla_style_set_size (scintilla, SCE_P_COMMENTLINE, prefs.php_comment_line_size);
  gtk_scintilla_style_set_italic (scintilla, SCE_P_COMMENTLINE, prefs.php_comment_line_italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_P_COMMENTLINE, prefs.php_comment_line_bold);

  gtk_scintilla_set_property(scintilla, "fold", "1");
  gtk_scintilla_colourise(scintilla, 0, -1);
}

void tab_python_set_lexer(Editor *editor)
{
  scintilla_python_set_lexer(GTK_SCINTILLA(editor->scintilla), preferences);
}
