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

#ifndef PLUGIN_H
#define PLUGIN_H

#include "main.h"

#define GPHPEDIT_PLUGIN_TYPE_UNKNOWN 0
#define GPHPEDIT_PLUGIN_TYPE_NOINPUT 1
#define GPHPEDIT_PLUGIN_TYPE_SELECTION 2
#define GPHPEDIT_PLUGIN_TYPE_FILENAME 3
#define GPHPEDIT_PLUGIN_TYPE_DEBUG 4

typedef struct
{
	guint type;
	GtkWidget *menu_item;
	GString *filename;
	gchar *name;
	gchar *description;
} Plugin;

gint parse_shortcut(gint accel_number);
void plugin_setup_menu(void);
void plugin_exec(gint plugin_num);
#endif
