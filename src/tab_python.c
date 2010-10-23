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

#include "tab_python.h"
#include "tab_util.h"
#include "preferences_manager.h"
#include "main_window.h"
#include "gtksourcestyleschememanager.h"

void scintilla_python_set_lexer(GtkScintilla *scintilla)
{
  PreferencesManager *prefmg = preferences_manager_new ();
  gtk_scintilla_clear_document_style (scintilla);
  gtk_scintilla_set_lexer(scintilla, SCLEX_PYTHON);
  gtk_scintilla_set_style_bits(scintilla, 8);

  // Python keywords
  gtk_scintilla_set_keywords(scintilla, 0, "abs all any apply and assert break class continue def del dict dir elif else except exec finally for from global if import  intern in is lambda len None not or pass print raise return try while yield from as with open __import__ self");

  gtk_scintilla_set_keywords(scintilla, 1, "Ellipsis NotImplemented ArithmeticError AssertionError AttributeError EnvironmentError EOFError Exception FloatingPointError ImportError IndentationError IndexError IOError KeyboardInterrupt KeyError LookupError MemoryError NameError NotImplementedError OSError OverflowError ReferenceError RuntimeError StandardError StopIteration SyntaxError SystemError SystemExit TabError TypeError UnboundLocalError UnicodeDecodeError UnicodeEncodeError UnicodeError UnicodeTranslateError ValueError WindowsError ZeroDivisionError Warning UserWarning DeprecationWarning PendingDeprecationWarning SyntaxWarning OverflowWarning RuntimeWarning FutureWarning basestring bool buffer callable chr classmethod cmp coerce compile complex delattr divmod enumerate eval execfile file filter float frozenset getattr globals hasattr hash hex id input int  isinstance issubclass iter  list locals long map max min object oct ord pow property range raw_input reduce reload repr reversed round setattr set slice sorted staticmethod str sum super tuple type unichr unicode vars xrange zip");

  const gchar *style_name;
  g_object_get(prefmg, "style_name", &style_name, NULL);

  GtkSourceStyleScheme	*scheme = gtk_source_style_scheme_manager_get_scheme (main_window.stylemg, style_name);
  /* PYTHON LEXER STYLE */
  const gchar *font;
  guint size;
  g_object_get(prefmg, "style_font_name", &font,"font_size", &size, NULL);

  set_scintilla_lexer_default_style(GTK_WIDGET(scintilla), scheme, SCE_P_DEFAULT, font, size);

  set_scintilla_lexer_keyword_style(GTK_WIDGET(scintilla), scheme, SCE_P_WORD, font, size);
  set_scintilla_lexer_type_style(GTK_WIDGET(scintilla), scheme, SCE_P_WORD2, font, size);
  set_scintilla_lexer_variable_style(GTK_WIDGET(scintilla), scheme, SCE_P_IDENTIFIER, font, size);
  set_scintilla_lexer_string_style(GTK_WIDGET(scintilla), scheme, SCE_P_STRING, font, size);
  set_scintilla_lexer_simple_string_style(GTK_WIDGET(scintilla), scheme, SCE_P_TRIPLE, font, size);
  set_scintilla_lexer_string_style(GTK_WIDGET(scintilla), scheme, SCE_P_TRIPLEDOUBLE, font, size);
  set_scintilla_lexer_simple_string_style(GTK_WIDGET(scintilla), scheme, SCE_P_CHARACTER, font, size);
  set_scintilla_lexer_operator_style(GTK_WIDGET(scintilla), scheme, SCE_P_OPERATOR, font, size);
  set_scintilla_lexer_number_style(GTK_WIDGET(scintilla), scheme, SCE_P_NUMBER, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(scintilla), scheme, SCE_P_COMMENTBLOCK, font, size);
  set_scintilla_lexer_special_constant_style (GTK_WIDGET(scintilla), scheme, SCE_P_STRINGEOL, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(scintilla), scheme, SCE_P_COMMENTLINE, font, size);
  set_scintilla_lexer_xml_entity_style(GTK_WIDGET(scintilla), scheme, SCE_P_DECORATOR, font, size);
  set_scintilla_lexer_preprocessor_style(GTK_WIDGET(scintilla), scheme, SCE_P_DEFNAME, font, size);
  set_scintilla_lexer_xml_instruction_style(GTK_WIDGET(scintilla), scheme, SCE_P_CLASSNAME, font, size);

  gtk_scintilla_set_property(scintilla, "fold", "1");
  gtk_scintilla_set_property(scintilla, "lexer.python.strings.over.newline", "1");
  gtk_scintilla_colourise(scintilla, 0, -1);

  gtk_scintilla_set_word_chars(scintilla, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-");

  g_object_unref(prefmg);
}

void tab_python_set_lexer(Document *document)
{
  scintilla_python_set_lexer(document_get_scintilla(document));
}

gboolean is_python_file(const gchar *filename)
{
  if (g_str_has_suffix(filename,".py"))
      return TRUE;
  return FALSE;
}
