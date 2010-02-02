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
#include "preferences.h"

void scintilla_cxx_set_lexer(GtkScintilla *scintilla, Preferences prefs)
{
	gtk_scintilla_clear_document_style (scintilla);
	gtk_scintilla_set_lexer(scintilla, SCLEX_CPP);
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

    gtk_scintilla_style_set_font (scintilla, SCE_C_DEFAULT, prefs.php_default_font);
    gtk_scintilla_style_set_fore (scintilla, SCE_C_DEFAULT, prefs.php_default_fore);
    gtk_scintilla_style_set_back (scintilla, SCE_C_DEFAULT, prefs.php_default_back);
    gtk_scintilla_style_set_size (scintilla, SCE_C_DEFAULT, prefs.php_default_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_C_DEFAULT, prefs.php_default_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_C_DEFAULT, prefs.php_default_bold);

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

	gtk_scintilla_set_keywords(scintilla, 0, "asm auto bool break case catch char class const const_cast continue default delete do double dynamic_cast else enum explicit export extern false float for friend goto if inline int long mutable namespace new operator private protected public register reinterpret_cast return short signed sizeof static static_cast struct switch template this throw true try typedef typeid typename union unsigned using virtual void volatile wchar_t while addindex addtogroup anchor arg attention author b brief bug c class code date def defgroup deprecated dontinclude e em endcode endhtmlonly endif endlatexonly endlink endverbatim enum example exception file hideinitializer htmlinclude htmlonly if image include ingroup internal invariant interface latexonly li line link mainpage name namespace nosubgrouping note overload p page par param post pre ref relates remarks return retval sa section see showinitializer since skip skipline struct subsection test throw todo typedef union until var verbatim verbinclude version warning weakgroup printf scanf");
	
    gtk_scintilla_set_keywords(scintilla, 1, "strstr strlen strcmp clrscr gotoXY FILE stat memcpy memmove memccpy memset strncpy strcpy strdup strndup fclose fopen freopen fdopen remove rename rewind tmpfile clearerr feof ferror fflush fflush fgetpos fgetc fgets fputc fputs ftell fseek fsetpos fread fwrite getc getchar gets fprintf sprintf vprintf perror putc putchar fputchar fscanf sscanf setbuf setvbuf tmpnam ungetc puts atof atoi atol strtod strtol strtoul rand srand malloc calloc realloc free abort atexit exit getenv system bsearch qsort abs div ldiv");
    gtk_scintilla_style_set_fore (scintilla, SCE_C_STRING, prefs.php_hstring_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_C_STRING, prefs.php_hstring_font);
    gtk_scintilla_style_set_back (scintilla, SCE_C_STRING, prefs.php_hstring_back);
    gtk_scintilla_style_set_size (scintilla, SCE_C_STRING, prefs.php_hstring_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_C_STRING, prefs.php_hstring_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_C_STRING, prefs.php_hstring_bold);
	
    gtk_scintilla_style_set_fore (scintilla, SCE_C_CHARACTER, prefs.php_simplestring_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_C_CHARACTER, prefs.php_simplestring_font);
    gtk_scintilla_style_set_back (scintilla, SCE_C_CHARACTER, prefs.php_simplestring_back);
    gtk_scintilla_style_set_size (scintilla, SCE_C_CHARACTER, prefs.php_simplestring_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_C_CHARACTER, prefs.php_simplestring_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_C_CHARACTER, prefs.php_simplestring_bold);

	gtk_scintilla_style_set_font (scintilla, SCE_C_WORD, prefs.php_variable_font);
	gtk_scintilla_style_set_fore (scintilla, SCE_C_WORD, prefs.php_variable_fore);
    gtk_scintilla_style_set_back (scintilla, SCE_C_WORD, prefs.php_variable_back);
    gtk_scintilla_style_set_size (scintilla, SCE_C_WORD, prefs.php_variable_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_C_WORD, prefs.php_variable_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_C_WORD, prefs.php_variable_bold);

    gtk_scintilla_style_set_fore (scintilla, SCE_C_NUMBER, prefs.php_number_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_C_NUMBER, prefs.php_number_font);
    gtk_scintilla_style_set_back (scintilla, SCE_C_NUMBER, prefs.php_number_back);
    gtk_scintilla_style_set_size (scintilla, SCE_C_NUMBER, prefs.php_number_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_C_NUMBER, prefs.php_number_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_C_NUMBER, prefs.php_number_bold);

    gtk_scintilla_style_set_fore (scintilla, SCE_C_IDENTIFIER, prefs.php_default_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_C_IDENTIFIER, prefs.php_default_font);
    gtk_scintilla_style_set_back (scintilla, SCE_C_IDENTIFIER, prefs.php_default_back);
    gtk_scintilla_style_set_size (scintilla, SCE_C_IDENTIFIER, prefs.php_default_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_C_IDENTIFIER, prefs.php_default_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_C_IDENTIFIER, prefs.php_default_bold);

    gtk_scintilla_style_set_fore (scintilla, SCE_C_COMMENT, prefs.php_comment_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_C_COMMENT, prefs.php_comment_font);
    gtk_scintilla_style_set_back (scintilla, SCE_C_COMMENT, prefs.php_comment_back);
    gtk_scintilla_style_set_size (scintilla, SCE_C_COMMENT, prefs.php_comment_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_C_COMMENT, prefs.php_comment_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_C_COMMENT, prefs.php_comment_bold);

    gtk_scintilla_style_set_fore (scintilla, SCE_C_COMMENTLINE, prefs.php_comment_line_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_C_COMMENTLINE, prefs.php_comment_line_font);
    gtk_scintilla_style_set_back (scintilla, SCE_C_COMMENTLINE, prefs.php_comment_line_back);
    gtk_scintilla_style_set_size (scintilla, SCE_C_COMMENTLINE, prefs.php_comment_line_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_C_COMMENTLINE, prefs.php_comment_line_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_C_COMMENTLINE, prefs.php_comment_line_bold);

    gtk_scintilla_style_set_fore (scintilla, SCE_C_PREPROCESSOR, prefs.html_script_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_C_PREPROCESSOR, prefs.html_script_font);
    gtk_scintilla_style_set_back (scintilla, SCE_C_PREPROCESSOR, prefs.html_script_back);
    gtk_scintilla_style_set_size (scintilla, SCE_C_PREPROCESSOR, prefs.html_script_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_C_PREPROCESSOR, prefs.html_script_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_C_PREPROCESSOR, TRUE);

    gtk_scintilla_style_set_fore (scintilla, SCE_C_OPERATOR, prefs.css_operator_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_C_OPERATOR, prefs.css_operator_font);
    gtk_scintilla_style_set_back (scintilla, SCE_C_OPERATOR, prefs.css_operator_back);
    gtk_scintilla_style_set_size (scintilla, SCE_C_OPERATOR, prefs.css_operator_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_C_OPERATOR, prefs.css_operator_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_C_OPERATOR, prefs.css_operator_bold);
 
    gtk_scintilla_style_set_fore (scintilla, SCE_C_REGEX, prefs.css_pseudoclass_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_C_REGEX, prefs.css_pseudoclass_font);
    gtk_scintilla_style_set_back (scintilla, SCE_C_REGEX, prefs.css_pseudoclass_back);
    gtk_scintilla_style_set_size (scintilla, SCE_C_REGEX, prefs.css_pseudoclass_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_C_REGEX, prefs.css_pseudoclass_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_C_REGEX, prefs.css_pseudoclass_bold);

    gtk_scintilla_style_set_fore (scintilla, SCE_C_UUID, prefs.css_id_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_C_UUID, prefs.css_id_font);
    gtk_scintilla_style_set_back (scintilla, SCE_C_UUID, prefs.css_id_back);
    gtk_scintilla_style_set_size (scintilla, SCE_C_UUID, prefs.css_id_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_C_UUID, prefs.css_id_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_C_UUID, prefs.css_id_bold);
    
    gtk_scintilla_style_set_fore (scintilla, SCE_C_VERBATIM, prefs.css_important_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_C_VERBATIM, prefs.css_important_font);
    gtk_scintilla_style_set_back (scintilla, SCE_C_VERBATIM, prefs.css_important_back);
    gtk_scintilla_style_set_size (scintilla, SCE_C_VERBATIM, prefs.css_important_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_C_VERBATIM, prefs.css_important_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_C_VERBATIM, prefs.css_important_bold);
    
    gtk_scintilla_style_set_fore (scintilla, SCE_C_GLOBALCLASS, prefs.javascript_keyword_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_C_GLOBALCLASS, prefs.javascript_keyword_font);
    gtk_scintilla_style_set_back (scintilla, SCE_C_GLOBALCLASS, prefs.javascript_keyword_back);
    gtk_scintilla_style_set_size (scintilla, SCE_C_GLOBALCLASS, prefs.javascript_keyword_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_C_GLOBALCLASS, prefs.javascript_keyword_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_C_GLOBALCLASS, prefs.javascript_keyword_bold);

	gtk_scintilla_indic_set_style(scintilla, 0, INDIC_SQUIGGLE);
	gtk_scintilla_indic_set_fore(scintilla, 0, scintilla_color(255,0,0));

	gtk_scintilla_set_property(scintilla, "fold.html", "1");
	gtk_scintilla_set_property(scintilla, "fold", "1");
	gtk_scintilla_colourise(scintilla, 0, -1);
}

void tab_cxx_set_lexer(Editor *editor)
{
	scintilla_cxx_set_lexer(GTK_SCINTILLA(editor->scintilla), preferences);
}
