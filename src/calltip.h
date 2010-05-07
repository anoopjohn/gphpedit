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

#ifndef CALLTIP_H
#define CALLTIP_H

#include "main.h"
void show_call_tip(GtkWidget *scintilla,gint type, gint pos);
void autocomplete_word(GtkWidget *scintilla, gint wordStart, gint wordEnd);
void function_list_prepare(void);
void css_function_list_prepare(void);
GString *complete_function_list(gchar *original_list);
GString *complete_css_function_list(gchar *original_list);
void css_autocomplete_word(GtkWidget *scintilla, gint wordStart, gint wordEnd);
void cobol_autocomplete_word(GtkWidget *scintilla, gint wordStart, gint wordEnd);
void sql_autocomplete_word(GtkWidget *scintilla, gint wordStart, gint wordEnd);
void cleanup_calltip(void);
#endif
