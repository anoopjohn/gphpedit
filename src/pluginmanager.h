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
#ifndef PLUGIN_MANAGER_MANAGER_H
#define PLUGIN_MANAGER_MANAGER_H

#include <gtk/gtk.h>
#include "tab.h"
#include "plugins.h"

#define PLUGIN_MANAGER_TYPE plugin_manager_get_type()
#define PLUGIN_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUGIN_MANAGER_TYPE, PluginManager))
#define PLUGIN_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), PLUGIN_MANAGER_TYPE, PluginManagerClass))
#define OBJECT_IS_PLUGIN_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLUGIN_MANAGER_TYPE))
#define OBJECT_IS_PLUGIN_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUGIN_MANAGER_TYPE))
#define PLUGIN_MANAGER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), PLUGIN_MANAGER_TYPE, PluginManagerClass))

typedef struct PluginManagerDetails PluginManagerDetails;

typedef struct
{
	GObject object;
	PluginManagerDetails *details;
} PluginManager;

typedef struct
{
	GObjectClass parent_class;

} PluginManagerClass;

/* Basic GObject requirements. */
GType plugin_manager_get_type (void) G_GNUC_CONST;
PluginManager *plugin_manager_new (void);
Plugin *get_plugin_by_name(PluginManager *plugmg, gchar *name);
guint get_plugin_manager_items_count(PluginManager *plugmg);
GList *get_plugin_manager_items(PluginManager *plugmg);
Plugin *get_plugin_by_num(PluginManager *plugmg, gint num);
gboolean run_syntax_plugin_by_ftype(PluginManager *plugmg, Documentable *document, MainWindow *main_window);
#endif /* PLUGIN_MANAGER_MANAGER_H */

