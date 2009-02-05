/* This file is part of gPHPEdit, a GNOME2 PHP Editor.

   Copyright (C) 2003-2005 Andy Jeffries
      andy@gphpedit.org
	  
   For more information or to find the latest release, visit our 
   website at http://www.gphpedit.org/

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.*/
   

#include "tab_php.h"
#include "tab_util.h"
#include "preferences.h"

void scintilla_perl_set_lexer(GtkScintilla *scintilla, Preferences prefs)
{
	gtk_scintilla_clear_document_style (scintilla);
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

    gtk_scintilla_style_set_font (scintilla, SCE_PL_DEFAULT, prefs.php_default_font);
    gtk_scintilla_style_set_fore (scintilla, SCE_PL_DEFAULT, prefs.php_default_fore);
    gtk_scintilla_style_set_back (scintilla, SCE_PL_DEFAULT, prefs.php_default_back);
    gtk_scintilla_style_set_size (scintilla, SCE_PL_DEFAULT, prefs.php_default_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_PL_DEFAULT, prefs.php_default_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_PL_DEFAULT, prefs.php_default_bold);

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

	gtk_scintilla_set_keywords(scintilla, 0, "NULL __FILE__ __LINE__ __PACKAGE__ __DATA__ __END__ AUTOLOAD BEGIN CORE DESTROY END EQ GE GT INIT LE LT NE CHECK abs accept alarm and atan2 bind binmode bless caller chdir chmod chomp chop chown chr chroot close closedir cmp connect continue cos crypt dbmclose dbmopen defined delete die do dump each else elsif endgrent endhostent endnetent endprotoent endpwent endservent eof eq eval exec exists exit exp fcntl fileno flock for foreach fork format formline ge getc getgrent getgrgid getgrnam gethostbyaddr gethostbyname gethostent getlogin getnetbyaddr getnetbyname getnetent getpeername getpgrp getppid getpriority getprotobyname getprotobynumber getprotoent getpwent getpwnam getpwuid getservbyname getservbyport getservent getsockname getsockopt glob gmtime goto grep gt hex if index int ioctl join keys kill last lc lcfirst le length link listen local localtime lock log lstat lt m map mkdir msgctl msgget msgrcv msgsnd my ne next no not oct open opendir or ord our pack package pipe pop pos print printf prototype push q qq qr quotemeta qu qw qx rand read readdir readline readlink readpipe recv redo ref rename require reset return reverse rewinddir rindex rmdir s scalar seek seekdir select semctl semget semop send setgrent sethostent setnetent setpgrp setpriority setprotoent setpwent setservent setsockopt shift shmctl shmget shmread shmwrite shutdown sin sleep socket socketpair sort splice split sprintf sqrt srand stat study sub substr symlink syscall sysopen sysread sysseek system syswrite tell telldir tie tied time times tr truncate uc ucfirst umask undef unless unlink unpack unshift untie until use utime values vec wait waitpid wantarray warn while write x xor y");
	
    gtk_scintilla_style_set_fore (scintilla, SCE_PL_STRING_Q, prefs.php_hstring_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_PL_STRING_Q, prefs.php_hstring_font);
    gtk_scintilla_style_set_back (scintilla, SCE_PL_STRING_Q, prefs.php_hstring_back);
    gtk_scintilla_style_set_size (scintilla, SCE_PL_STRING_Q, prefs.php_hstring_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_PL_STRING_Q, prefs.php_hstring_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_PL_STRING_Q, prefs.php_hstring_bold);
    gtk_scintilla_style_set_fore (scintilla, SCE_PL_STRING_QQ, prefs.php_hstring_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_PL_STRING_QQ, prefs.php_hstring_font);
    gtk_scintilla_style_set_back (scintilla, SCE_PL_STRING_QQ, prefs.php_hstring_back);
    gtk_scintilla_style_set_size (scintilla, SCE_PL_STRING_QQ, prefs.php_hstring_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_PL_STRING_QQ, prefs.php_hstring_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_PL_STRING_QQ, prefs.php_hstring_bold);
    gtk_scintilla_style_set_fore (scintilla, SCE_PL_STRING_QX, prefs.php_hstring_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_PL_STRING_QX, prefs.php_hstring_font);
    gtk_scintilla_style_set_back (scintilla, SCE_PL_STRING_QX, prefs.php_hstring_back);
    gtk_scintilla_style_set_size (scintilla, SCE_PL_STRING_QX, prefs.php_hstring_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_PL_STRING_QX, prefs.php_hstring_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_PL_STRING_QX, prefs.php_hstring_bold);
    gtk_scintilla_style_set_fore (scintilla, SCE_PL_STRING_QR, prefs.php_hstring_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_PL_STRING_QR, prefs.php_hstring_font);
    gtk_scintilla_style_set_back (scintilla, SCE_PL_STRING_QR, prefs.php_hstring_back);
    gtk_scintilla_style_set_size (scintilla, SCE_PL_STRING_QR, prefs.php_hstring_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_PL_STRING_QR, prefs.php_hstring_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_PL_STRING_QR, prefs.php_hstring_bold);
    gtk_scintilla_style_set_fore (scintilla, SCE_PL_STRING_QW, prefs.php_hstring_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_PL_STRING_QW, prefs.php_hstring_font);
    gtk_scintilla_style_set_back (scintilla, SCE_PL_STRING_QW, prefs.php_hstring_back);
    gtk_scintilla_style_set_size (scintilla, SCE_PL_STRING_QW, prefs.php_hstring_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_PL_STRING_QW, prefs.php_hstring_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_PL_STRING_QW, prefs.php_hstring_bold);
    gtk_scintilla_style_set_fore (scintilla, SCE_PL_STRING, prefs.php_hstring_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_PL_STRING, prefs.php_hstring_font);
    gtk_scintilla_style_set_back (scintilla, SCE_PL_STRING, prefs.php_hstring_back);
    gtk_scintilla_style_set_size (scintilla, SCE_PL_STRING, prefs.php_hstring_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_PL_STRING, prefs.php_hstring_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_PL_STRING, prefs.php_hstring_bold);
	
    gtk_scintilla_style_set_fore (scintilla, SCE_PL_BACKTICKS, prefs.php_simplestring_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_PL_BACKTICKS, prefs.php_simplestring_font);
    gtk_scintilla_style_set_back (scintilla, SCE_PL_BACKTICKS, prefs.php_simplestring_back);
    gtk_scintilla_style_set_size (scintilla, SCE_PL_BACKTICKS, prefs.php_simplestring_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_PL_BACKTICKS, prefs.php_simplestring_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_PL_BACKTICKS, prefs.php_simplestring_bold);

	gtk_scintilla_style_set_font (scintilla, SCE_PL_WORD, prefs.php_word_font);
	gtk_scintilla_style_set_fore (scintilla, SCE_PL_WORD, prefs.php_word_fore);
    gtk_scintilla_style_set_back (scintilla, SCE_PL_WORD, prefs.php_word_back);
    gtk_scintilla_style_set_size (scintilla, SCE_PL_WORD, prefs.php_word_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_PL_WORD, prefs.php_word_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_PL_WORD, prefs.php_word_bold);

    gtk_scintilla_style_set_fore (scintilla, SCE_PL_NUMBER, prefs.php_number_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_PL_NUMBER, prefs.php_number_font);
    gtk_scintilla_style_set_back (scintilla, SCE_PL_NUMBER, prefs.php_number_back);
    gtk_scintilla_style_set_size (scintilla, SCE_PL_NUMBER, prefs.php_number_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_PL_NUMBER, prefs.php_number_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_PL_NUMBER, prefs.php_number_bold);

    gtk_scintilla_style_set_fore (scintilla, SCE_PL_IDENTIFIER, prefs.php_variable_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_PL_IDENTIFIER, prefs.php_variable_font);
    gtk_scintilla_style_set_back (scintilla, SCE_PL_IDENTIFIER, prefs.php_variable_back);
    gtk_scintilla_style_set_size (scintilla, SCE_PL_IDENTIFIER, prefs.php_variable_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_PL_IDENTIFIER, prefs.php_variable_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_PL_IDENTIFIER, prefs.php_variable_bold);

    gtk_scintilla_style_set_fore (scintilla, SCE_PL_COMMENTLINE, prefs.php_comment_line_fore);
    gtk_scintilla_style_set_font (scintilla, SCE_PL_COMMENTLINE, prefs.php_comment_line_font);
    gtk_scintilla_style_set_back (scintilla, SCE_PL_COMMENTLINE, prefs.php_comment_line_back);
    gtk_scintilla_style_set_size (scintilla, SCE_PL_COMMENTLINE, prefs.php_comment_line_size);
    gtk_scintilla_style_set_italic (scintilla, SCE_PL_COMMENTLINE, prefs.php_comment_line_italic);
    gtk_scintilla_style_set_bold (scintilla, SCE_PL_COMMENTLINE, prefs.php_comment_line_bold);

	gtk_scintilla_set_property(scintilla, "fold", "1");
	gtk_scintilla_colourise(scintilla, 0, -1);
}

void tab_perl_set_lexer(Editor *editor)
{
	scintilla_perl_set_lexer(GTK_SCINTILLA(editor->scintilla), preferences);
}
