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

#ifndef TAB_H
#define TAB_H

#include "main.h"
#include "document.h"

/* annotations styles */
#define STYLE_ANNOTATION_ERROR 200
#define STYLE_ANNOTATION_WARNING 201

void register_file_opened(gchar *filename);
void str_replace(char *Str, char ToRp, char WithC);
void info_dialog (gchar *title, gchar *message);
gint yes_no_dialog (gchar *title, gchar *message);
//gchar *trunc_on_char(gchar * string, gchar which_char);
#endif
