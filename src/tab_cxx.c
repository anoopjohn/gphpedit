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
#include "tab_util.h"
#include "main_window.h"

void scintilla_cxx_set_lexer(GtkScintilla *scintilla)
{
  gtk_scintilla_clear_document_style (scintilla);
  gtk_scintilla_set_lexer(scintilla, SCLEX_CPP);
  gtk_scintilla_set_style_bits(scintilla, 5);

  gchar *font =NULL;
  gint size, fore, back;
  gboolean italic, bold;

  get_preferences_manager_style_settings(main_window.prefmg, "c_default", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_font (scintilla, SCE_C_DEFAULT, font);
  gtk_scintilla_style_set_fore (scintilla, SCE_C_DEFAULT, fore);
  gtk_scintilla_style_set_back (scintilla, SCE_C_DEFAULT, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_DEFAULT, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_DEFAULT, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_DEFAULT, bold);

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

  gtk_scintilla_set_keywords(scintilla, 0, "asm auto bool break case catch char class const const_cast continue default delete do double dynamic_cast else enum explicit export extern false float for friend goto if inline int long mutable namespace new operator private protected public register reinterpret_cast return short signed sizeof static static_cast struct switch template this throw true try typedef typeid typename union unsigned using virtual void volatile wchar_t while addindex addtogroup anchor arg attention author b brief bug c class code date def defgroup deprecated dontinclude e em endcode endhtmlonly endif endlatexonly endlink endverbatim enum example exception file hideinitializer htmlinclude htmlonly if image include ingroup internal invariant interface latexonly li line link mainpage name namespace nosubgrouping note overload p page par param post pre ref relates remarks return retval sa section see showinitializer since skip skipline struct subsection test throw todo typedef union until var verbatim verbinclude version warning weakgroup printf scanf");
  
  gtk_scintilla_set_keywords(scintilla, 1, "strstr strlen strcmp clrscr gotoXY FILE stat memcpy memmove memccpy memset strncpy strcpy strdup strndup fclose fopen freopen fdopen remove rename rewind tmpfile clearerr feof ferror fflush fflush fgetpos fgetc fgets fputc fputs ftell fseek fsetpos fread fwrite getc getchar gets fprintf sprintf vprintf perror putc putchar fputchar fscanf sscanf setbuf setvbuf tmpnam ungetc puts atof atoi atol strtod strtol strtoul rand srand malloc calloc realloc free abort atexit exit getenv system bsearch qsort abs div ldiv");

  get_preferences_manager_style_settings(main_window.prefmg, "c_string", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_STRING, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_STRING, font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_STRING, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_STRING, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_STRING, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_STRING, bold);
  
  get_preferences_manager_style_settings(main_window.prefmg, "c_character", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_CHARACTER, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_CHARACTER, font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_CHARACTER, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_CHARACTER, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_CHARACTER, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_CHARACTER, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "c_word", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_font (scintilla, SCE_C_WORD, font);
  gtk_scintilla_style_set_fore (scintilla, SCE_C_WORD, fore);
  gtk_scintilla_style_set_back (scintilla, SCE_C_WORD, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_WORD, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_WORD, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_WORD, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "css_pseudoclass", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_font (scintilla, SCE_C_WORD2, font);
  gtk_scintilla_style_set_fore (scintilla, SCE_C_WORD2, fore);
  gtk_scintilla_style_set_back (scintilla, SCE_C_WORD2, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_WORD2, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_WORD2, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_WORD2, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "c_number", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_NUMBER, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_NUMBER, font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_NUMBER, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_NUMBER, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_NUMBER, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_NUMBER, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "c_identifier", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_IDENTIFIER, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_IDENTIFIER, font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_IDENTIFIER, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_IDENTIFIER, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_IDENTIFIER, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_IDENTIFIER, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "c_comment", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_COMMENT, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_COMMENT, font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_COMMENT, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_COMMENT, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_COMMENT, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_COMMENT, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "c_comment", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_COMMENTDOC, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_COMMENTDOC, font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_COMMENTDOC, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_COMMENTDOC, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_COMMENTDOC, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_COMMENTDOC, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "c_commentline", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_COMMENTLINE, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_COMMENTLINE, font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_COMMENTLINE, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_COMMENTLINE, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_COMMENTLINE, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_COMMENTLINE, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "c_preprocesor", &font , &size, &fore, &back, &italic, NULL);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_PREPROCESSOR, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_PREPROCESSOR, font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_PREPROCESSOR, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_PREPROCESSOR, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_PREPROCESSOR, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_PREPROCESSOR, TRUE);

  get_preferences_manager_style_settings(main_window.prefmg, "c_operator", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_OPERATOR, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_OPERATOR, font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_OPERATOR, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_OPERATOR, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_OPERATOR, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_OPERATOR, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "c_uuid", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_UUID, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_UUID, font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_UUID, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_UUID, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_UUID, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_UUID, bold);
 
  get_preferences_manager_style_settings(main_window.prefmg, "c_regex", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_REGEX, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_REGEX, font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_REGEX, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_REGEX, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_REGEX, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_REGEX, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "c_verbatim", &font , &size, &fore, &back, &italic, &bold);
  
  gtk_scintilla_style_set_fore (scintilla, SCE_C_VERBATIM, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_VERBATIM, font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_VERBATIM, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_VERBATIM, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_VERBATIM, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_VERBATIM, bold);
  
  get_preferences_manager_style_settings(main_window.prefmg, "c_globalclass", &font , &size, &fore, &back, &italic, &bold);
  
  gtk_scintilla_style_set_fore (scintilla, SCE_C_GLOBALCLASS, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_GLOBALCLASS, font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_GLOBALCLASS, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_GLOBALCLASS, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_GLOBALCLASS, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_GLOBALCLASS, bold);

  gtk_scintilla_indic_set_style(scintilla, 0, INDIC_SQUIGGLE);
  gtk_scintilla_indic_set_fore(scintilla, 0, scintilla_color(255,0,0));

  gtk_scintilla_set_property(scintilla, "lexer.cpp.allow.dollars", "1");
  gtk_scintilla_set_property(scintilla, "fold.comment", "1");
  gtk_scintilla_set_property(scintilla, "fold.preprocessor", "1");
  gtk_scintilla_set_property(scintilla, "fold.compact", "1");
  gtk_scintilla_set_property(scintilla, "fold.at.else", "1");
  gtk_scintilla_set_property(scintilla, "fold", "1");
  gtk_scintilla_colourise(scintilla, 0, -1);
}

void tab_cxx_set_lexer(Editor *editor)
{
  scintilla_cxx_set_lexer(GTK_SCINTILLA(editor->scintilla));
}
