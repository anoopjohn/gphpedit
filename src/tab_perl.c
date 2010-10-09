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

#include "tab_perl.h"
#include "tab_util.h"
#include "preferences_manager.h"
#include "main_window.h"
#include "gtksourcestyleschememanager.h"


void scintilla_perl_set_lexer(GtkScintilla *scintilla)
{
  PreferencesManager *prefmg = preferences_manager_new ();
  gtk_scintilla_clear_document_style (scintilla);
  gtk_scintilla_set_lexer(scintilla, SCLEX_PERL);
  gtk_scintilla_set_style_bits(scintilla, 5);

  gtk_scintilla_set_keywords(scintilla, 0, "NULL __FILE__ __LINE__ __PACKAGE__ __DATA__ __END__ AUTOLOAD BEGIN CORE DESTROY END EQ GE GT INIT LE LT NE CHECK abs accept alarm and atan2 bind binmode bless caller chdir chmod chomp chop chown chr chroot close closedir cmp connect continue cos crypt dbmclose dbmopen defined delete die do dump each else elsif endgrent endhostent endnetent endprotoent endpwent endservent eof eq eval exec exists exit exp fcntl fileno flock for foreach fork format formline ge getc getgrent getgrgid getgrnam gethostbyaddr gethostbyname gethostent getlogin getnetbyaddr getnetbyname getnetent getpeername getpgrp getppid getpriority getprotobyname getprotobynumber getprotoent getpwent getpwnam getpwuid getservbyname getservbyport getservent getsockname getsockopt glob gmtime goto grep gt hex if index int ioctl join keys kill last lc lcfirst le length link listen local localtime lock log lstat lt m map mkdir msgctl msgget msgrcv msgsnd my ne next no not oct open opendir or ord our pack package pipe pop pos print printf prototype push q qq qr quotemeta qu qw qx rand read readdir readline readlink readpipe recv redo ref rename require reset return reverse rewinddir rindex rmdir s scalar seek seekdir select semctl semget semop send setgrent sethostent setnetent setpgrp setpriority setprotoent setpwent setservent setsockopt shift shmctl shmget shmread shmwrite shutdown sin sleep socket socketpair sort splice split sprintf sqrt srand stat study sub substr symlink syscall sysopen sysread sysseek system syswrite tell telldir tie tied time times tr truncate uc ucfirst umask undef unless unlink unpack unshift untie until use utime values vec wait waitpid wantarray warn while write x xor y");
  
  const gchar *font = get_preferences_manager_style_font(prefmg);
  guint size = get_preferences_manager_style_size(prefmg);

  GtkSourceStyleScheme	*scheme = gtk_source_style_scheme_manager_get_scheme (main_window.stylemg, get_preferences_manager_style_name(prefmg));
  /* PHP LEXER STYLE */
  set_scintilla_lexer_doc_comment_style(GTK_WIDGET(scintilla), scheme, SCE_PL_FORMAT_IDENT, font, size);
  set_scintilla_lexer_doc_comment_style(GTK_WIDGET(scintilla), scheme, SCE_PL_FORMAT, font, size);
  set_scintilla_lexer_default_style(GTK_WIDGET(scintilla), scheme, SCE_PL_DEFAULT, font, size);
  set_scintilla_lexer_keyword_style(GTK_WIDGET(scintilla), scheme, SCE_PL_WORD, font, size);
  set_scintilla_lexer_type_style(GTK_WIDGET(scintilla), scheme, SCE_PL_SUB_PROTOTYPE, font, size);
  set_scintilla_lexer_keyword_style(GTK_WIDGET(scintilla), scheme, SCE_PL_POD_VERB, font, size);
  set_scintilla_lexer_comment_style(GTK_WIDGET(scintilla), scheme, SCE_PL_POD, font, size);
  set_scintilla_lexer_type_style(GTK_WIDGET(scintilla), scheme, SCE_PL_IDENTIFIER, font, size);
  set_scintilla_lexer_variable_style(GTK_WIDGET(scintilla), scheme, SCE_PL_ARRAY, font, size);
  set_scintilla_lexer_variable_style(GTK_WIDGET(scintilla), scheme, SCE_PL_SYMBOLTABLE, font, size);
  set_scintilla_lexer_variable_style(GTK_WIDGET(scintilla), scheme, SCE_PL_DATASECTION, font, size);
  set_scintilla_lexer_variable_style(GTK_WIDGET(scintilla), scheme, SCE_PL_VARIABLE_INDEXER, font, size);
  set_scintilla_lexer_xml_atribute_style(GTK_WIDGET(scintilla), scheme, SCE_PL_STRING_Q, font, size);
  set_scintilla_lexer_xml_atribute_style(GTK_WIDGET(scintilla), scheme, SCE_PL_STRING_QQ, font, size);
  set_scintilla_lexer_xml_atribute_style(GTK_WIDGET(scintilla), scheme, SCE_PL_STRING_QX, font, size);
  set_scintilla_lexer_xml_atribute_style(GTK_WIDGET(scintilla), scheme, SCE_PL_STRING_QR, font, size);
  set_scintilla_lexer_xml_atribute_style(GTK_WIDGET(scintilla), scheme, SCE_PL_STRING_QW, font, size);
  set_scintilla_lexer_string_style(GTK_WIDGET(scintilla), scheme, SCE_PL_LONGQUOTE, font, size);
  set_scintilla_lexer_string_style(GTK_WIDGET(scintilla), scheme, SCE_PL_STRING, font, size);
  set_scintilla_lexer_simple_string_style(GTK_WIDGET(scintilla), scheme, SCE_PL_BACKTICKS, font, size);
  set_scintilla_lexer_simple_string_style(GTK_WIDGET(scintilla), scheme, SCE_PL_CHARACTER, font, size);
  set_scintilla_lexer_preprocessor_style(GTK_WIDGET(scintilla), scheme, SCE_PL_PREPROCESSOR, font, size);
  set_scintilla_lexer_operator_style(GTK_WIDGET(scintilla), scheme, SCE_PL_OPERATOR, font, size);
  set_scintilla_lexer_number_style(GTK_WIDGET(scintilla), scheme, SCE_PL_SCALAR, font, size);
  set_scintilla_lexer_number_style(GTK_WIDGET(scintilla), scheme, SCE_PL_NUMBER, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(scintilla), scheme, SCE_PL_COMMENTLINE, font, size);
  set_scintilla_lexer_xml_element_style(GTK_WIDGET(scintilla), scheme, SCE_PL_HASH, font, size);
  set_scintilla_lexer_xml_element_style (GTK_WIDGET(scintilla), scheme, SCE_PL_REGSUBST, font, size);
  set_scintilla_lexer_string_style(GTK_WIDGET(scintilla), scheme, SCE_PL_REGEX, font, size);
  set_scintilla_lexer_special_constant_style (GTK_WIDGET(scintilla), scheme, SCE_PL_PUNCTUATION, font, size);
  set_scintilla_lexer_error_style (GTK_WIDGET(scintilla), scheme, SCE_PL_ERROR, font, size);
  set_scintilla_lexer_xml_instruction_style(GTK_WIDGET(scintilla), scheme, SCE_PL_HERE_DELIM, font, size);
  set_scintilla_lexer_xml_instruction_style(GTK_WIDGET(scintilla), scheme, SCE_PL_HERE_Q, font, size);
  set_scintilla_lexer_xml_instruction_style(GTK_WIDGET(scintilla), scheme, SCE_PL_HERE_QQ, font, size);
  set_scintilla_lexer_xml_instruction_style(GTK_WIDGET(scintilla), scheme, SCE_PL_HERE_QX, font, size);

  gtk_scintilla_set_property(scintilla, "fold", "1");
  gtk_scintilla_set_property(scintilla, "fold.comment", "1");
  gtk_scintilla_set_property(scintilla, "fold.compact", "1");
  gtk_scintilla_set_property(scintilla, "fold.perl.pod", "1");
  gtk_scintilla_set_property(scintilla, "fold.perl.package", "1");

  gtk_scintilla_colourise(scintilla, 0, -1);

  g_object_unref(prefmg);
}

void tab_perl_set_lexer(Document *document)
{
  scintilla_perl_set_lexer(document_get_scintilla(document));
}

gboolean is_perl_file(const gchar *filename)
{
  if (g_str_has_suffix(filename,".pl") || g_str_has_suffix(filename,".pm") || g_str_has_suffix(filename,".al") || g_str_has_suffix(filename,".perl"))
      return TRUE;
  return FALSE;
}

