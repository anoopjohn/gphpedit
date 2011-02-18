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

#include "tab_cxx.h"
#include "tab_util.h"
#include "preferences_manager.h"
#include "main_window.h"
#include "gtksourcestyleschememanager.h"

void scintilla_cxx_set_lexer(GtkScintilla *scintilla)
{
  PreferencesManager *prefmg = preferences_manager_new ();

  gtk_scintilla_clear_document_style (scintilla);
  gtk_scintilla_set_lexer(scintilla, SCLEX_CPP);
  gtk_scintilla_set_style_bits(scintilla, 5);

  gtk_scintilla_set_keywords(scintilla, 0, "asm auto bool break case catch char class const const_cast continue default delete do double dynamic_cast else enum explicit export extern false float for friend goto if inline int long mutable namespace new operator private protected public register reinterpret_cast return short signed sizeof static static_cast struct switch template this throw true try typedef typeid typename union unsigned using virtual void volatile wchar_t while addindex addtogroup anchor arg attention author b brief bug c class code date def defgroup deprecated dontinclude e em endcode endhtmlonly endif endlatexonly endlink endverbatim enum example exception file hideinitializer htmlinclude htmlonly if image include ingroup internal invariant interface latexonly li line link mainpage name namespace nosubgrouping note overload p page par param post pre ref relates remarks return retval sa section see showinitializer since skip skipline struct subsection test throw todo typedef union until var verbatim verbinclude version warning weakgroup printf scanf");
  
  gtk_scintilla_set_keywords(scintilla, 1, "strstr strlen strcmp clrscr gotoXY FILE stat memcpy memmove memccpy memset strncpy strcpy strdup strndup fclose fopen freopen fdopen remove rename rewind tmpfile clearerr feof ferror fflush fflush fgetpos fgetc fgets fputc fputs ftell fseek fsetpos fread fwrite getc getchar gets fprintf sprintf vprintf perror putc putchar fputchar fscanf sscanf setbuf setvbuf tmpnam ungetc puts atof atoi atol strtod strtol strtoul rand srand malloc calloc realloc free abort atexit exit getenv system bsearch qsort abs div ldiv");

  const gchar *font;
  guint size;
  g_object_get(prefmg, "style_font_name", &font,"font_size", &size, NULL);

  const gchar *style_name;
  g_object_get(prefmg, "style_name", &style_name, NULL);

  GtkSourceStyleScheme	*scheme = gtk_source_style_scheme_manager_get_scheme (main_window.stylemg, style_name);
  /* PYTHON LEXER STYLE */
  set_scintilla_lexer_default_style(GTK_WIDGET(scintilla), scheme, SCE_C_DEFAULT, font, size);
  set_scintilla_lexer_keyword_style(GTK_WIDGET(scintilla), scheme, SCE_C_WORD, font, size);
  set_scintilla_lexer_keyword_style(GTK_WIDGET(scintilla), scheme, SCE_C_WORD2, font, size);
  set_scintilla_lexer_type_style(GTK_WIDGET(scintilla), scheme, SCE_C_IDENTIFIER, font, size);
  set_scintilla_lexer_string_style(GTK_WIDGET(scintilla), scheme, SCE_C_STRING, font, size);
  set_scintilla_lexer_simple_string_style(GTK_WIDGET(scintilla), scheme, SCE_C_CHARACTER, font, size);
  set_scintilla_lexer_operator_style(GTK_WIDGET(scintilla), scheme, SCE_C_OPERATOR, font, size);
  set_scintilla_lexer_number_style(GTK_WIDGET(scintilla), scheme, SCE_C_NUMBER, font, size);
  set_scintilla_lexer_doc_comment_style(GTK_WIDGET(scintilla), scheme, SCE_C_COMMENTDOC, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(scintilla), scheme, SCE_C_COMMENT, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(scintilla), scheme, SCE_C_COMMENTLINE, font, size);
  set_scintilla_lexer_preprocessor_style(GTK_WIDGET(scintilla), scheme, SCE_C_PREPROCESSOR, font, size);
  set_scintilla_lexer_xml_element_style(GTK_WIDGET(scintilla), scheme, SCE_C_UUID, font, size);
  set_scintilla_lexer_special_constant_style (GTK_WIDGET(scintilla), scheme, SCE_C_REGEX, font, size);
  set_scintilla_lexer_xml_atribute_style(GTK_WIDGET(scintilla), scheme, SCE_C_VERBATIM, font, size);
  set_scintilla_lexer_xml_instruction_style(GTK_WIDGET(scintilla), scheme, SCE_C_GLOBALCLASS, font, size);

  gtk_scintilla_set_property(scintilla, "lexer.cpp.allow.dollars", "1");
  gtk_scintilla_set_property(scintilla, "fold.comment", "1");
  gtk_scintilla_set_property(scintilla, "fold.preprocessor", "1");
  gtk_scintilla_set_property(scintilla, "fold.compact", "1");
  gtk_scintilla_set_property(scintilla, "fold.at.else", "1");
  gtk_scintilla_set_property(scintilla, "fold", "1");
  gtk_scintilla_colourise(scintilla, 0, -1);

  g_object_unref(prefmg);
}

void tab_cxx_set_lexer(GtkScintilla *scintilla)
{
  scintilla_cxx_set_lexer(scintilla);
}

gboolean is_cxx_file(const gchar *filename)
{
  if (g_str_has_suffix(filename,".cxx") || g_str_has_suffix(filename,".c") || g_str_has_suffix(filename,".h")
  || g_str_has_suffix(filename,".cpp") || g_str_has_suffix(filename,".cc") || g_str_has_suffix(filename,".c++"))
      return TRUE;
  return FALSE;
}

gboolean is_cxx_char_autoindent(char ch)
{
  return (ch=='{');
}

gboolean is_cxx_char_autounindent(char ch)
{
  return (ch=='}');
}

