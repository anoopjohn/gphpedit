/* This file is part of gPHPEdit, a GNOME2 PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009 José Rostagno (for vijona.com.ar) 

   For more information or to find the latest release, visit our 
   website at http://www.gphpedit.org/

   gPHPEdit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   gPHPEdit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with gPHPEdit. If not, see <http://www.gnu.org/licenses/>.

   The GNU General Public License is contained in the file COPYING.
*/
#ifndef SYNTAX_CHECK_MANAGER_H
#define SYNTAX_CHECK_MANAGER_H

#include <gtk/gtk.h>
#include "tab.h"
#include "syntax_check_window.h"

#define SYNTAX_CHECK_MANAGER_TYPE syntax_check_manager_get_type()
#define SYNTAX_CHECK_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), SYNTAX_CHECK_MANAGER_TYPE, Syntax_Check_Manager))
#define SYNTAX_CHECK_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), SYNTAX_CHECK_MANAGER_TYPE, Syntax_Check_ManagerClass))
#define OBJECT_IS_SYNTAX_CHECK_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SYNTAX_CHECK_MANAGER_TYPE))
#define OBJECT_IS_SYNTAX_CHECK_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), SYNTAX_CHECK_MANAGER_TYPE))
#define SYNTAX_CHECK_MANAGER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), SYNTAX_CHECK_MANAGER_TYPE, Syntax_Check_ManagerClass))

typedef struct Syntax_Check_Manager_Details Syntax_Check_Manager_Details;

typedef struct
{
	GObject object;
	Syntax_Check_Manager_Details *details;
} Syntax_Check_Manager;

typedef struct
{
	GObjectClass parent_class;

} Syntax_Check_ManagerClass;

/* Basic GObject requirements. */
GType syntax_check_manager_get_type (void) G_GNUC_CONST;
Syntax_Check_Manager *syntax_check_manager_new (void);
gchar *syntax_check_manager_run(Editor *editor, gint ftype);
#endif /* SYNTAX_CHECK_MANAGER_H */

