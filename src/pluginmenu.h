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
#ifndef __GTK_PLUGIN_MANAGER_MENU_H__
#define __GTK_PLUGIN_MANAGER_MENU_H__

#include <gtk/gtk.h>
#include "pluginmanager.h"

G_BEGIN_DECLS

#define GTK_TYPE_PLUGIN_MANAGER_MENU		(gtk_plugin_manager_menu_get_type ())
#define GTK_PLUGIN_MANAGER_MENU(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_PLUGIN_MANAGER_MENU, GtkPlugin_Manager_Menu))
#define GTK_IS_PLUGIN_MANAGER_MENU(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_PLUGIN_MANAGER_MENU))
#define GTK_PLUGIN_MANAGER_MENU_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_PLUGIN_MANAGER_MENU, GtkPlugin_Manager_MenuClass))
#define GTK_IS_PLUGIN_MANAGER_MENU_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_PLUGIN_MANAGER_MENU))
#define GTK_PLUGIN_MANAGER_MENU_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_PLUGIN_MANAGER_MENU, GtkPlugin_Manager_MenuClass))

typedef struct _GtkPlugin_Manager_Menu		GtkPlugin_Manager_Menu;
typedef struct _GtkPlugin_Manager_MenuClass	GtkPlugin_Manager_MenuClass;
typedef struct _GtkPlugin_Manager_MenuPrivate	GtkPlugin_Manager_MenuPrivate;

struct _GtkPlugin_Manager_Menu
{
  /*< private >*/
  GtkMenu parent_instance;

  GtkPlugin_Manager_MenuPrivate *GSEAL (priv);
};

struct _GtkPlugin_Manager_MenuClass
{
  GtkMenuClass parent_class;

};

GType      gtk_plugin_manager_menu_get_type         (void) G_GNUC_CONST;
GtkWidget *gtk_plugin_manager_menu_new              (void);
Plugin_Manager *get_plugin_manager(GtkPlugin_Manager_Menu *menu);
void plugin_exec_with_num(GtkWidget *widget, gint num);
G_END_DECLS

#endif /* ! __GTK_PLUGIN_MANAGER_MENU_H__ */
