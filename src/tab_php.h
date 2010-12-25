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

#ifndef TAB_PHP_H
#define TAB_PHP_H

#include "document.h"

void tab_php_set_lexer(GtkScintilla *scintilla);
void scintilla_php_set_lexer(GtkScintilla *scintilla);
gboolean is_php_file_from_filename(const gchar *filename);
gboolean is_php_file_from_content(const gchar *content);
gboolean is_php_char_autoindent(char ch);
gboolean is_php_char_autounindent(char ch);
gboolean check_php_variable_before(const gchar *line_text);
#endif
