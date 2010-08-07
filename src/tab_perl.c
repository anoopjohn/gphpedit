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

#include "tab_perl.h"
#include "tab_util.h"
#include "main_window.h"

void scintilla_perl_set_lexer(GtkScintilla *scintilla)
{
  gtk_scintilla_clear_document_style (scintilla);
  gtk_scintilla_set_lexer(scintilla, SCLEX_PERL);
  gtk_scintilla_set_style_bits(scintilla, 5);


  gchar *font =NULL;
  gint size, fore, back;
  gboolean italic, bold;

  get_preferences_manager_style_settings(main_window.prefmg, "php_default", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_font (scintilla, SCE_PL_DEFAULT, font);
  gtk_scintilla_style_set_fore (scintilla, SCE_PL_DEFAULT, fore);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_DEFAULT, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_DEFAULT, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_DEFAULT, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_DEFAULT, bold);

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

  gtk_scintilla_set_keywords(scintilla, 0, "NULL __FILE__ __LINE__ __PACKAGE__ __DATA__ __END__ AUTOLOAD BEGIN CORE DESTROY END EQ GE GT INIT LE LT NE CHECK abs accept alarm and atan2 bind binmode bless caller chdir chmod chomp chop chown chr chroot close closedir cmp connect continue cos crypt dbmclose dbmopen defined delete die do dump each else elsif endgrent endhostent endnetent endprotoent endpwent endservent eof eq eval exec exists exit exp fcntl fileno flock for foreach fork format formline ge getc getgrent getgrgid getgrnam gethostbyaddr gethostbyname gethostent getlogin getnetbyaddr getnetbyname getnetent getpeername getpgrp getppid getpriority getprotobyname getprotobynumber getprotoent getpwent getpwnam getpwuid getservbyname getservbyport getservent getsockname getsockopt glob gmtime goto grep gt hex if index int ioctl join keys kill last lc lcfirst le length link listen local localtime lock log lstat lt m map mkdir msgctl msgget msgrcv msgsnd my ne next no not oct open opendir or ord our pack package pipe pop pos print printf prototype push q qq qr quotemeta qu qw qx rand read readdir readline readlink readpipe recv redo ref rename require reset return reverse rewinddir rindex rmdir s scalar seek seekdir select semctl semget semop send setgrent sethostent setnetent setpgrp setpriority setprotoent setpwent setservent setsockopt shift shmctl shmget shmread shmwrite shutdown sin sleep socket socketpair sort splice split sprintf sqrt srand stat study sub substr symlink syscall sysopen sysread sysseek system syswrite tell telldir tie tied time times tr truncate uc ucfirst umask undef unless unlink unpack unshift untie until use utime values vec wait waitpid wantarray warn while write x xor y");
  
  get_preferences_manager_style_settings(main_window.prefmg, "php_hstring", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_PL_STRING_Q, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_PL_STRING_Q, font);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_STRING_Q, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_STRING_Q, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_STRING_Q, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_STRING_Q, bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_PL_STRING_QQ, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_PL_STRING_QQ, font);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_STRING_QQ, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_STRING_QQ, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_STRING_QQ, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_STRING_QQ, bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_PL_STRING_QX, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_PL_STRING_QX, font);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_STRING_QX, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_STRING_QX, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_STRING_QX, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_STRING_QX, bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_PL_STRING_QR, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_PL_STRING_QR, font);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_STRING_QR, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_STRING_QR, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_STRING_QR, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_STRING_QR, bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_PL_STRING_QW, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_PL_STRING_QW, font);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_STRING_QW, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_STRING_QW, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_STRING_QW, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_STRING_QW, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "c_string", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_PL_STRING, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_PL_STRING, font);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_STRING, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_STRING, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_STRING, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_STRING, bold);
  
  get_preferences_manager_style_settings(main_window.prefmg, "php_simplestring", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_PL_BACKTICKS, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_PL_BACKTICKS, font);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_BACKTICKS, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_BACKTICKS, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_BACKTICKS, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_BACKTICKS, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "php_word", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_font (scintilla, SCE_PL_WORD, font);
  gtk_scintilla_style_set_fore (scintilla, SCE_PL_WORD, fore);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_WORD, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_WORD, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_WORD, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_WORD, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "php_number", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_PL_NUMBER, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_PL_NUMBER, font);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_NUMBER, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_NUMBER, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_NUMBER, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_NUMBER, bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_PL_SCALAR, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_PL_SCALAR, font);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_SCALAR, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_SCALAR, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_SCALAR, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_SCALAR, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "php_variable", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_PL_IDENTIFIER, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_PL_IDENTIFIER, font);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_IDENTIFIER, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_IDENTIFIER, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_IDENTIFIER, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_IDENTIFIER, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "c_character", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_PL_CHARACTER, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_PL_CHARACTER, font);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_CHARACTER, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_CHARACTER, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_CHARACTER, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_CHARACTER, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "c_preprocesor", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_PL_PREPROCESSOR, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_PL_PREPROCESSOR, font);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_PREPROCESSOR, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_PREPROCESSOR, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_PREPROCESSOR, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_PREPROCESSOR, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "php_comment_line", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_PL_COMMENTLINE, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_PL_COMMENTLINE, font);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_COMMENTLINE, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_COMMENTLINE, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_COMMENTLINE, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_COMMENTLINE, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "c_uuid", &font , &size, &fore, &back, &italic, &bold);
  gtk_scintilla_style_set_fore (scintilla, SCE_PL_HASH, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_PL_HASH, font);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_HASH, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_HASH, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_HASH, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_HASH, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "c_operator", &font , &size, &fore, &back, &italic, &bold);
  gtk_scintilla_style_set_fore (scintilla, SCE_PL_OPERATOR, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_PL_OPERATOR, font);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_OPERATOR, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_OPERATOR, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_OPERATOR, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_OPERATOR, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "c_regex", &font , &size, &fore, &back, &italic, &bold);
  gtk_scintilla_style_set_fore (scintilla, SCE_PL_REGEX, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_PL_REGEX, font);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_REGEX, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_REGEX, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_REGEX, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_REGEX, bold);

  gtk_scintilla_style_set_fore (scintilla,SCE_PL_REGSUBST, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_PL_REGSUBST, font);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_REGSUBST, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_REGSUBST, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_REGSUBST, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_REGSUBST, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "c_verbatim", &font , &size, &fore, &back, &italic, &bold);
  gtk_scintilla_style_set_fore (scintilla, SCE_PL_ARRAY, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_PL_ARRAY, font);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_ARRAY, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_ARRAY, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_ARRAY, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_ARRAY, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "css_pseudoclass", &font , &size, &fore, &back, &italic, &bold);
  gtk_scintilla_style_set_fore (scintilla, SCE_PL_SUB_PROTOTYPE, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_PL_SUB_PROTOTYPE, font);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_SUB_PROTOTYPE, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_SUB_PROTOTYPE, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_SUB_PROTOTYPE, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_SUB_PROTOTYPE, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "html_double_string", &font , &size, &fore, &back, &italic, &bold);
  gtk_scintilla_style_set_fore (scintilla, SCE_PL_LONGQUOTE, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_PL_LONGQUOTE, font);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_LONGQUOTE, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_LONGQUOTE, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_LONGQUOTE, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_LONGQUOTE, bold);

  get_preferences_manager_style_settings(main_window.prefmg, "css_important", &font , &size, &fore, &back, &italic, &bold);
  gtk_scintilla_style_set_fore (scintilla, SCE_PL_ERROR, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_PL_ERROR, font);
  gtk_scintilla_style_set_back (scintilla, SCE_PL_ERROR, back);
  gtk_scintilla_style_set_size (scintilla, SCE_PL_ERROR, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_PL_ERROR, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_PL_ERROR, bold);

  gtk_scintilla_set_property(scintilla, "fold", "1");
  gtk_scintilla_set_property(scintilla, "fold.comment", "1");
  gtk_scintilla_set_property(scintilla, "fold.compact", "1");
  gtk_scintilla_set_property(scintilla, "fold.perl.pod", "1");
  gtk_scintilla_set_property(scintilla, "fold.perl.package", "1");

  gtk_scintilla_colourise(scintilla, 0, -1);
}

void tab_perl_set_lexer(Document *document)
{
  scintilla_perl_set_lexer(document_get_scintilla(document));
}
