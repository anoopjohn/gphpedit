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
#include "main_window.h"

void scintilla_python_set_lexer(GtkScintilla *scintilla)
{
  gtk_scintilla_clear_document_style (scintilla);
  // Don't know why - but it doesn't highlight if the next line is SCLEX_PYTHON
  gtk_scintilla_set_lexer(scintilla, SCLEX_PERL);
  gtk_scintilla_set_style_bits(scintilla, 5);

  gchar *font =NULL;
  gint size, fore, back;
  gboolean italic, bold;

  get_preferences_manager_style_settings(main_window.prefmg, "php_default", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_font (scintilla, SCE_P_DEFAULT, font);
  gtk_scintilla_style_set_fore (scintilla, SCE_P_DEFAULT, fore);
  gtk_scintilla_style_set_back (scintilla, SCE_P_DEFAULT, back);
  gtk_scintilla_style_set_size (scintilla, SCE_P_DEFAULT, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_P_DEFAULT, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_P_DEFAULT, bold);

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

  // Python keywords
  gtk_scintilla_set_keywords(scintilla, 0, "and assert break class continue def del elif else except exec finally for from global if import in is lambda None not or pass print raise return try while yield");
  
  get_preferences_manager_style_settings(main_window.prefmg, "php_hstring", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_P_TRIPLEDOUBLE, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_P_TRIPLEDOUBLE, font);
  gtk_scintilla_style_set_back (scintilla, SCE_P_TRIPLEDOUBLE, back);
  gtk_scintilla_style_set_size (scintilla, SCE_P_TRIPLEDOUBLE, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_P_TRIPLEDOUBLE, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_P_TRIPLEDOUBLE, bold);
  gtk_scintilla_style_set_fore (scintilla, SCE_P_TRIPLE, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_P_TRIPLE, font);
  gtk_scintilla_style_set_back (scintilla, SCE_P_TRIPLE, back);
  gtk_scintilla_style_set_size (scintilla, SCE_P_TRIPLE, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_P_TRIPLE, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_P_TRIPLE, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "php_simplestring", &font , &size, &fore, &back, &italic, &bold);
    
  gtk_scintilla_style_set_fore (scintilla, SCE_P_STRING, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_P_STRING, font);
  gtk_scintilla_style_set_back (scintilla, SCE_P_STRING, back);
  gtk_scintilla_style_set_size (scintilla, SCE_P_STRING, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_P_STRING, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_P_STRING, bold);
  gtk_scintilla_style_set_fore (scintilla, SCE_P_CHARACTER, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_P_CHARACTER, font);
  gtk_scintilla_style_set_back (scintilla, SCE_P_CHARACTER, back);
  gtk_scintilla_style_set_size (scintilla, SCE_P_CHARACTER, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_P_CHARACTER, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_P_CHARACTER, bold);
  
  get_preferences_manager_style_settings(main_window.prefmg, "php_word", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_font (scintilla, SCE_P_WORD, font);
  gtk_scintilla_style_set_fore (scintilla, SCE_P_WORD, fore);
  gtk_scintilla_style_set_back (scintilla, SCE_P_WORD, back);
  gtk_scintilla_style_set_size (scintilla, SCE_P_WORD, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_P_WORD, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_P_WORD, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "php_number", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_P_NUMBER, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_P_NUMBER, font);
  gtk_scintilla_style_set_back (scintilla, SCE_P_NUMBER, back);
  gtk_scintilla_style_set_size (scintilla, SCE_P_NUMBER, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_P_NUMBER, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_P_NUMBER, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "php_default", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_P_IDENTIFIER, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_P_IDENTIFIER, font);
  gtk_scintilla_style_set_back (scintilla, SCE_P_IDENTIFIER, back);
  gtk_scintilla_style_set_size (scintilla, SCE_P_IDENTIFIER, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_P_IDENTIFIER, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_P_IDENTIFIER, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "php_comment_line", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_P_COMMENTLINE, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_P_COMMENTLINE, font);
  gtk_scintilla_style_set_back (scintilla, SCE_P_COMMENTLINE, back);
  gtk_scintilla_style_set_size (scintilla, SCE_P_COMMENTLINE, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_P_COMMENTLINE, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_P_COMMENTLINE, bold);

  gtk_scintilla_set_property(scintilla, "fold", "1");
  gtk_scintilla_colourise(scintilla, 0, -1);
}

void tab_python_set_lexer(Document *document)
{
  scintilla_python_set_lexer(document_get_scintilla(document));
}
