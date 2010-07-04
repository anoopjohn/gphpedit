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
#ifndef PLUGINS_H
#define PLUGINS_H

#include <gtk/gtk.h>
#include <gio/gio.h>
#include "tab.h"

#define PLUGIN_TYPE plugin_get_type()
#define PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), PLUGIN_TYPE, Plugin))
#define PLUGIN_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), PLUGIN_TYPE, PluginClass))
#define OBJECT_IS_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLUGIN_TYPE))
#define OBJECT_IS_PLUGIN_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), PLUGIN_TYPE))
#define PLUGIN_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), PLUGIN_TYPE, PluginClass))

typedef struct PluginDetails PluginDetails;

typedef struct
{
	GObject object;
	PluginDetails *details;
} Plugin;

typedef struct
{
	GObjectClass parent_class;

} PluginClass;

/* Basic GObject requirements. */
GType plugin_get_type (void);
Plugin *plugin_new (gchar *filename);
const gchar *get_plugin_name(Plugin *plugin);
const gchar *get_plugin_description(Plugin *plugin);
void plugin_run(Plugin *plugin, Editor *editor);

#endif /* PLUGINS_H */

