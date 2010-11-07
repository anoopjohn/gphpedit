/* This file is part of gPHPEdit, a GNOME2 PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009, 2010 José Rostagno (for vijona.com.ar) 

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

#include <config.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "debug.h"
#include "pluginmanager.h"
#include "gvfs_utils.h"

/*
* plugin_manager private struct
*/
struct PluginManagerDetails
{
  GHashTable *plugins_table;
};

/*max plugins */
#define NUM_PLUGINS_MAX 30

#define PLUGIN_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    PLUGIN_MANAGER_TYPE,\
					    PluginManagerDetails))

static void plugin_manager_finalize (GObject  *object);
static void  plugin_manager_class_init (PluginManagerClass *klass);
static void plugin_discover_available(PluginManager *plugmg);

/* http://library.gnome.org/devel/gobject/unstable/gobject-Type-Information.html#G-DEFINE-TYPE:CAPS */
G_DEFINE_TYPE(PluginManager, plugin_manager, G_TYPE_OBJECT);

/*
* overide default contructor to make a singleton.
* see http://blogs.gnome.org/xclaesse/2010/02/11/how-to-make-a-gobject-singleton/
*/
static GObject* 
plugin_manager_constructor (GType type,
                 guint n_construct_params,
                 GObjectConstructParam *construct_params)
{
  static GObject *self = NULL;

  if (self == NULL)
    {
      self = G_OBJECT_CLASS (plugin_manager_parent_class)->constructor (
          type, n_construct_params, construct_params);
      g_object_add_weak_pointer (self, (gpointer) &self);
      return self;
    }

  return g_object_ref (self);
}

static void
plugin_manager_class_init (PluginManagerClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = plugin_manager_finalize;
  object_class->constructor = plugin_manager_constructor;
	g_type_class_add_private (klass, sizeof (PluginManagerDetails));
}

static void
plugin_manager_init (PluginManager  *object)
{
	PluginManagerDetails *plugmgdet;
	plugmgdet = PLUGIN_MANAGER_GET_PRIVATE(object);
  /* init plugins table*/
  plugmgdet->plugins_table = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_object_unref);
  /* check for plugins directory, if doesn't exist create it */
  GError *error=NULL;
  gchar *uri = g_build_filename (g_get_user_config_dir (), "gphpedit", "plugins", NULL);
  GFile *plugin = g_file_new_for_commandline_arg(uri);
  if(!g_file_query_exists(plugin,NULL)){
    if (!g_file_make_directory_with_parents (plugin, NULL, &error)){
      if (error->code !=G_IO_ERROR_EXISTS){
        g_print(_("Unable to create ~/.gphpedit/ (%d) %s"), error->code,error->message);
      }
      g_error_free(error);
    }
    g_object_unref(plugin);
  }
  g_free(uri);

  plugin_discover_available(object); /* fill plugin table */
}

static void
plugin_manager_finalize (GObject *object)
{
  PluginManager *plugmg = PLUGIN_MANAGER(object);
  PluginManagerDetails *plugmgdet;
	plugmgdet = PLUGIN_MANAGER_GET_PRIVATE(plugmg);
  /* free object resources*/
  g_hash_table_destroy(plugmgdet->plugins_table);
	G_OBJECT_CLASS (plugin_manager_parent_class)->finalize (object);
}


PluginManager *plugin_manager_new (void)
{
	PluginManager *plugmg;
  plugmg = g_object_new (PLUGIN_MANAGER_TYPE, NULL);
  
	return plugmg; /* return new object */
}

static void new_plugin(PluginManager *plugmg, gchar *filename){
    Plugin *plugin;
    plugin=plugin_new (filename);
    PluginManagerDetails *plugmgdet;
    plugmgdet = PLUGIN_MANAGER_GET_PRIVATE(plugmg);
/* insert new plugin in the plugin table */
    g_hash_table_insert (plugmgdet->plugins_table, (gchar *)get_plugin_name(plugin), plugin); 
}
/* internal function */
static void plugin_discover_available(PluginManager *plugmg)
{
  GDir *dir;
  const gchar *plugin_name;
  GString *filename;

  gchar *uri = g_build_filename (g_get_user_config_dir (), "gphpedit", "plugins", NULL);
  gphpedit_debug_message(DEBUG_PLUGINS, "User plugin directory: %s\n", uri);

  if (g_file_test(uri, G_FILE_TEST_IS_DIR)) {
    dir = g_dir_open(uri, 0, NULL);
    if (dir) {
      for (plugin_name = g_dir_read_name(dir); plugin_name != NULL; plugin_name = g_dir_read_name(dir)) {
        gchar *filename2 = g_strdup_printf("%s/%s", uri, plugin_name);
        new_plugin(plugmg, filename2);
        g_free(filename2);
      }
      g_dir_close(dir);      
    }
  }
  g_free(uri);

  gchar *plugin_dir = NULL;
  /* use autoconf macro to build plugins path */
  plugin_dir = g_build_path (G_DIR_SEPARATOR_S, API_DIR, "plugins/", NULL);

  if (g_file_test(plugin_dir, G_FILE_TEST_IS_DIR)) { 
    dir = g_dir_open(plugin_dir, 0,NULL);
    if (dir) {
      for (plugin_name = g_dir_read_name(dir); plugin_name != NULL; plugin_name = g_dir_read_name(dir)) {
        filename = g_string_new(plugin_name);
        filename = g_string_prepend(filename, plugin_dir);
        GFile *file = g_file_new_for_commandline_arg (filename->str);
        GError *error=NULL;
        GFileInfo *info =g_file_query_info (file, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL,&error);
        if (!info) continue;
        if (g_file_info_get_attribute_boolean (info, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE)){
          new_plugin(plugmg,filename->str);
        } else {
          g_warning("Don't have execute access for: %s\n", filename->str);
        }
        g_object_unref(info);
        g_string_free (filename,TRUE);
      }
      g_dir_close(dir);      
    }
  }
  g_free(plugin_dir);
  gphpedit_debug_message(DEBUG_PLUGINS,"%s","FOUND ALL PLUGINS\n");
}
/*
*
*/
Plugin *get_plugin_by_name(PluginManager *plugmg, gchar *name){
  g_return_val_if_fail (name, NULL);
  g_return_val_if_fail (OBJECT_IS_PLUGIN_MANAGER(plugmg), NULL);
  PluginManagerDetails *plugmgdet;
	plugmgdet = PLUGIN_MANAGER_GET_PRIVATE(plugmg);
  Plugin *plug= g_hash_table_lookup (plugmgdet->plugins_table,name);
  return plug;
}

Plugin *get_plugin_by_num(PluginManager *plugmg, gint num){
  g_return_val_if_fail (num<10, NULL);
  g_return_val_if_fail (OBJECT_IS_PLUGIN_MANAGER(plugmg), NULL);
  PluginManagerDetails *plugmgdet;
	plugmgdet = PLUGIN_MANAGER_GET_PRIVATE(plugmg);
  GList *pluglist = g_hash_table_get_values (plugmgdet->plugins_table);
  Plugin *plug= g_list_nth_data (pluglist,num);
  g_list_free (pluglist);
  return plug;
}

guint get_plugin_manager_items_count(PluginManager *plugmg){
  g_return_val_if_fail (OBJECT_IS_PLUGIN_MANAGER(plugmg), 0);
  PluginManagerDetails *plugmgdet;
	plugmgdet = PLUGIN_MANAGER_GET_PRIVATE(plugmg);
  return MIN(g_hash_table_size (plugmgdet->plugins_table),NUM_PLUGINS_MAX);
}

GList *get_plugin_manager_items(PluginManager *plugmg){
  g_return_val_if_fail (OBJECT_IS_PLUGIN_MANAGER(plugmg), NULL);
  PluginManagerDetails *plugmgdet;
	plugmgdet = PLUGIN_MANAGER_GET_PRIVATE(plugmg);
  return g_hash_table_get_values (plugmgdet->plugins_table);
}

gboolean get_syntax_plugin_by_ftype (gpointer key, gpointer value, gpointer user_data){
  Plugin *plug= PLUGIN(value);
  if (get_plugin_syntax_type(plug)==GPOINTER_TO_INT (user_data)) return TRUE;
  return FALSE;
}

gboolean run_syntax_plugin_by_ftype(PluginManager *plugmg, Document *document){
  g_return_val_if_fail (OBJECT_IS_PLUGIN_MANAGER(plugmg), FALSE);
  PluginManagerDetails *plugmgdet;
	plugmgdet = PLUGIN_MANAGER_GET_PRIVATE(plugmg);
  gint ftype;
  g_object_get(document, "type", &ftype, NULL);
  Plugin *plug=g_hash_table_find (plugmgdet->plugins_table, get_syntax_plugin_by_ftype, GINT_TO_POINTER(ftype));
  if (plug){
    gphpedit_debug_message(DEBUG_PLUGINS,"%s","Plugin FOUND!!\n");
    plugin_run(plug, document);
    return TRUE;
  } else {
    gphpedit_debug_message(DEBUG_PLUGINS,"%s","Plugin NOT FOUND!!\n");
    return FALSE;
  }
}
