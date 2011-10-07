/* This file is part of gPHPEdit, a GNOME2 PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009-2010 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009-2010 José Rostagno (for vijona.com.ar) 

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
/*
PLUGIN SYSTEM:

A gPHPEdit plugin is a script that gPHPEdit will list in the menu and allow you to integrate somewhat with the editor. The files must be executable by the current user. There are a limited number of things a plugin will receive from gPHPEdit and a limited number of things it can return.

The plugins should be stored either in /usr/share/gphpedit/plugins/ if you want them to be available for all users on your machine or in ~/.gphpedit/plugins/ if you only want them to be available for the current user.

The number of plugins available is limited to 30. User specific plugins take preference over global plugins if the number is over 30.
The menu will asing automatically a shortcut to the first 10 plugins. [CONTROL + number]
Syntax plugins aren't show in plugins menu, because they are incorporate to the syntax check system. When you run the syntax check, they will be automatically run.
How it works
The general mechanism for interacting with a plugin is as follows:

* gPHPEdit runs your script/plugin with a command line parameter of -type
* gPHPEdit will ask for plugin information with the following command line parameters
** -name : return plugin name.
** -desc : return plugin description
** -icon : return plugin icon name
** -copyright : return plugin copyright info.
** -website : return plugin website.
** -version : return plugin version.
** -authors : return plugin authors. must be a null terminated string with names using ',' as separator.
* Your plugin must print one of the following types to STDOUT:
  o SELECTION - your plugin requires the current selection of text or nothing if none is selected
  o NO-INPUT - your plugin doesn't require anything
  o FNAME - your plugin requires the current full filename (local files only)
  o SYNTAX - like FNAME but will act as a syntax checker. (if no default check are present can be run by press F9)
    * Syntax plugins must return the file tipe when run with a command line parameter of -ftype
      ** languages supported:
        *** PHP
        *** HTML
        *** XML
        *** C
        *** C++
        *** C/C++
        *** COBOL
        *** SQL
        *** PERL
        *** PYTHON
* When the menu item is clicked, gPHPEdit runs your plugin with either the selection or the filename in single quotes.
* Your plugin must then print to STDOUT one of the following actions:
  o INSERT - your plugin requires that the gPHPEdit inserts the text that is about to be returned at the cursor position (i.e. after any current selection)
  o REPLACE - your plugin requires that the gPHPEdit replaces any current selection with the text that is about to be returned
  o OPEN - your plugin requires that gPHPEdit opens the file that is about to be specified (NB: this can be the same as the file specified inwards with FNAME, this will reload the current file)
  o MESSAGE - your plugin requires that the gPHPEdit show an info dialog
  o SYNTAX - your plugin requires that the gPHPEdit process syntax check output, show it in syntax pane and apply style to text
    ** output must have the following struct line number space message dot like next example:
    ** 59 invalid operator.\n
    ** lines end with \n 
* The next thing your plugin must print is a newline character
* Finally your editor should print either the content or the filename, then close
*/

#include <config.h>
#include <gtk/gtk.h>
#include "debug.h"
#include "plugins.h"
#include "main_window.h"
#include "gvfs_utils.h"
#include "syntax_check_window.h"

/* plugin type enum*/
enum {
  GPHPEDIT_PLUGIN_TYPE_UNKNOWN=0, 
  GPHPEDIT_PLUGIN_TYPE_NOINPUT,
  GPHPEDIT_PLUGIN_TYPE_SELECTION,
  GPHPEDIT_PLUGIN_TYPE_FILENAME,
  GPHPEDIT_PLUGIN_TYPE_SYNTAX,
};

/*
* plugin private struct
*/
struct PluginDetails
{
  guint type;
  gint file_type; /* only needed for SYNTAX PLUGINS*/
  gchar *filename;
  gchar *name;
  gchar *description;
	gchar *icon_name;
	gchar **authors;
	gchar *copyright;
	gchar *website;
	gchar *version;

  gint active :1; /* plugin status */
};

#define PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    PLUGIN_TYPE,\
					    PluginDetails))

static void plugin_finalize (GObject *object);
static void plugin_class_init (PluginClass *klass);

/* http://library.gnome.org/devel/gobject/unstable/gobject-Type-Information.html#G-DEFINE-TYPE:CAPS */
G_DEFINE_TYPE(Plugin, plugin, G_TYPE_OBJECT);  

static void
plugin_class_init (PluginClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = plugin_finalize;
	g_type_class_add_private (klass, sizeof (PluginDetails));
}

static void
plugin_init (Plugin *object)
{
	PluginDetails *plug;
	plug = PLUGIN_GET_PRIVATE(object);
}

static void
plugin_finalize (GObject *object)
{
  Plugin *plug = PLUGIN(object);
  PluginDetails *plugdet;
	plugdet = PLUGIN_GET_PRIVATE(plug);
  /* free object resources*/
	if (plugdet->filename) g_free(plugdet->filename);
	if (plugdet->name) g_free(plugdet->name);
	if (plugdet->description) g_free(plugdet->description);
	if (plugdet->copyright) g_free(plugdet->copyright);
	if (plugdet->website) g_free(plugdet->website);
	if (plugdet->version) g_free(plugdet->version);
	if (plugdet->icon_name) g_free(plugdet->icon_name);

	G_OBJECT_CLASS (plugin_parent_class)->finalize (object);
}

#if 0
/*
 * internal function
 * execute a command in a command line
*/
static inline gchar *command_spawn(const gchar* command_line)
{
  gchar *stdout = NULL;
  GError *error = NULL;
  gint exit_status;
  gchar *ret=NULL;
  if (g_spawn_command_line_sync(command_line, &stdout, NULL, &exit_status,&error)) {
    #ifdef DEBUG
    guint stdout_len = strlen(stdout);
    gphpedit_debug_message(DEBUG_PLUGINS, "COMMAND: %s\nOUTPUT: %s (%d)\n", command_line, stdout, stdout_len);
    #endif
    ret=g_strdup(stdout);
    g_free(stdout);
  } else {
    gphpedit_debug_message(DEBUG_PLUGINS, "Command %s gave error %s\n", command_line, error->message);
    g_error_free (error);
  }  
  
  return ret;
}
#endif

gboolean is_internal_command(gchar *command){
  if (!command) return TRUE;
  if (g_str_has_prefix(command, "INSERT")){
      return TRUE;
    }
    else if (g_str_has_prefix(command, "REPLACE")){
      return TRUE;
    }
    else if (g_str_has_prefix(command, "MESSAGE")){
      return TRUE;
    }
    else if (g_str_has_prefix(command, "SYNTAX")){
      return TRUE;
    }
    else if (g_str_has_prefix(command, "OPEN")){
      return TRUE;
    }
  return FALSE;
}


/*
* plugin_discover_name
* return the plugin name. must be freed with g_free
*/
gchar *plugin_discover_name(const gchar *filename)
{
  GString *command_line;
  gchar *name=NULL;
  
  command_line = g_string_new(filename);
  command_line = g_string_prepend(command_line, "'");
  command_line = g_string_append(command_line, "' -name");
  name = command_spawn(command_line->str);
  gphpedit_debug_message(DEBUG_PLUGINS,"plugin name:%s\n",name);

  g_string_free(command_line, TRUE);
  
  return name;
}

gchar *plugin_discover_desc(gchar *filename)
{
  GString *command_line;
  gchar *desc=NULL;
  
  command_line = g_string_new(filename);
  command_line = g_string_prepend(command_line, "'");
  command_line = g_string_append(command_line, "' -desc");
  desc = command_spawn(command_line->str);

  gphpedit_debug_message(DEBUG_PLUGINS, "Plugin description:%s\n",desc);

  g_string_free(command_line, TRUE);
  
  /* plugin doesn't support this command */
  if (is_internal_command(desc)){
     g_free(desc);
     return NULL;
  }

  return desc;
}

/****/

int plugin_discover_type(gchar *filename)
{
  GString *command_line;
  gint type = GPHPEDIT_PLUGIN_TYPE_UNKNOWN;
  gchar *result;  
  command_line = g_string_new(filename);
  command_line = g_string_prepend(command_line, "'");
  command_line = g_string_append(command_line, "' -type");
  result= command_spawn(command_line->str);
  if(result){
    if (g_str_has_prefix(result, "SELECTION")){
      type = GPHPEDIT_PLUGIN_TYPE_SELECTION;
  }
  else if (g_str_has_prefix(result, "SYNTAX")){
    type = GPHPEDIT_PLUGIN_TYPE_SYNTAX;
  }
  else if (g_str_has_prefix(result, "NO-INPUT")){
    type = GPHPEDIT_PLUGIN_TYPE_NOINPUT;
  }
  else if (g_str_has_prefix(result, "FNAME")){
    type = GPHPEDIT_PLUGIN_TYPE_FILENAME;
  }
  gphpedit_debug_message(DEBUG_PLUGINS,"Returning Discovered type of %d\n", type);

  g_free(result);
  }
  g_string_free(command_line, TRUE);
  
  return type;
}

/* internal function*/
static gint plugin_syntax_discover_type(gchar *filename)
{
  GString *command_line;
  gchar *ftype=NULL;
  gint file_type;
  command_line = g_string_new(filename);
  command_line = g_string_prepend(command_line, "'");
  command_line = g_string_append(command_line, "' -ftype");
  gchar *result= command_spawn(command_line->str);
  ftype = g_ascii_strup (result, -1);
  g_free(result);
  gphpedit_debug_message(DEBUG_PLUGINS, "Plugin syntax File type:%s\n",ftype);

  g_string_free(command_line, TRUE);
  
  if (g_strcmp0(ftype,"PHP")==0 || g_strcmp0(ftype,"HTML")==0 || g_strcmp0(ftype,"XML")==0) file_type=TAB_PHP;
  else if (g_strcmp0(ftype,"CSS")==0) file_type=TAB_CSS;
  else if (g_strcmp0(ftype,"C")==0 || g_strcmp0(ftype,"C++")==0 || g_strcmp0(ftype,"C/C++")==0) file_type=TAB_CXX;
  else if (g_strcmp0(ftype,"CSS")==0) file_type=TAB_CSS;
  else if (g_strcmp0(ftype,"COBOL")==0) file_type=TAB_COBOL;
  else if (g_strcmp0(ftype,"SQL")==0) file_type=TAB_SQL;
  else if (g_strcmp0(ftype,"PERL")==0) file_type=TAB_PERL;
  else if (g_strcmp0(ftype,"PYTHON")==0) file_type=TAB_PYTHON;
  else file_type=TAB_FILE;
  if (file_type==TAB_FILE) g_print("Unknown file type for Syntax plugin:%s\n",ftype);
  g_free(ftype);
  return file_type;
}

/*
* plugin_discover_copyright
* return the plugin copyright. must be freed with g_free
*/
gchar *plugin_discover_copyright(const gchar *filename)
{
  GString *command_line;
  gchar *name=NULL;
  
  command_line = g_string_new(filename);
  command_line = g_string_prepend(command_line, "'");
  command_line = g_string_append(command_line, "' -copyright");
  name = command_spawn(command_line->str);
  gphpedit_debug_message(DEBUG_PLUGINS,"plugin copyright:%s\n",name);

  g_string_free(command_line, TRUE);
  /* plugin doesn't support this command */
  if (is_internal_command(name)){
     g_free(name);
     return NULL;
  }
  return name;
}

/*
* plugin_discover_website
* return the plugin website. must be freed with g_free
*/
gchar *plugin_discover_website(const gchar *filename)
{
  GString *command_line;
  gchar *name=NULL;
  
  command_line = g_string_new(filename);
  command_line = g_string_prepend(command_line, "'");
  command_line = g_string_append(command_line, "' -website");
  name = command_spawn(command_line->str);
  gphpedit_debug_message(DEBUG_PLUGINS,"plugin website:%s\n",name);

  g_string_free(command_line, TRUE);
  /* plugin doesn't support this command */
  if (is_internal_command(name)){
     g_free(name);
     return NULL;
  }
  return name;
}

/*
* plugin_discover_version
* return the plugin version. must be freed with g_free
*/
gchar *plugin_discover_version(const gchar *filename)
{
  GString *command_line;
  gchar *name=NULL;
  
  command_line = g_string_new(filename);
  command_line = g_string_prepend(command_line, "'");
  command_line = g_string_append(command_line, "' -version");
  name = command_spawn(command_line->str);
  gphpedit_debug_message(DEBUG_PLUGINS,"plugin version: %s\n",name);

  g_string_free(command_line, TRUE);
  /* plugin doesn't support this command */
  if (is_internal_command(name)){
     g_free(name);
     return NULL;
  }
  return name;
}

/*
* plugin_discover_icon
* return the plugin icon name. must be freed with g_free
*/
gchar *plugin_discover_icon(const gchar *filename)
{
  GString *command_line;
  gchar *name=NULL;
  
  command_line = g_string_new(filename);
  command_line = g_string_prepend(command_line, "'");
  command_line = g_string_append(command_line, "' -icon");
  name = command_spawn(command_line->str);
  gphpedit_debug_message(DEBUG_PLUGINS,"plugin icon name: %s\n",name);

  g_string_free(command_line, TRUE);
  /* plugin doesn't support this command */
  if (is_internal_command(name)){
     g_free(name);
     return NULL;
  }
  return name;
}

/*
* plugin_discover_authors
* return the plugin authors. must be freed with g_free
*/
gchar **plugin_discover_authors(const gchar *filename)
{
  GString *command_line;
  gchar *name=NULL;
  
  command_line = g_string_new(filename);
  command_line = g_string_prepend(command_line, "'");
  command_line = g_string_append(command_line, "' -authors");
  name = command_spawn(command_line->str);
  gphpedit_debug_message(DEBUG_PLUGINS,"plugin authors: %s\n",name);

  g_string_free(command_line, TRUE);
  /* plugin doesn't support this command */
  if (is_internal_command(name)){
     g_free(name);
     return (gchar **) NULL;
  }
  gchar **authors = g_strsplit (name,",",-1);
  g_free(name);
  return authors;
}
             
Plugin *plugin_new (gchar *filename)
{
	Plugin *plug;
  plug = g_object_new (PLUGIN_TYPE, NULL);
  PluginDetails *plugdet;
	plugdet = PLUGIN_GET_PRIVATE(plug);
  /* search plugin properties */
  plugdet->filename=g_strdup(filename);
  plugdet->name= plugin_discover_name(filename);
  plugdet->description= plugin_discover_desc(filename);
  plugdet->type= plugin_discover_type(filename);
  if (plugdet->type==GPHPEDIT_PLUGIN_TYPE_SYNTAX) plugdet->file_type = plugin_syntax_discover_type(filename);
	plugdet->authors = plugin_discover_authors(filename);
	plugdet->copyright = plugin_discover_copyright(filename);
	plugdet->website = plugin_discover_website(filename);
	plugdet->version = plugin_discover_version(filename);
	plugdet->icon_name = plugin_discover_icon(filename);

  /* get active status, default value TRUE */
  /* Note:: multiple plugins with the same name share the same status */
  if (plugdet->name) {
    plugdet->active = get_plugin_is_active(main_window.prefmg, plugdet->name);
  } else {
    plugdet->active = FALSE;
  }
  gphpedit_debug_message(DEBUG_PLUGINS,"Name: %s(%d)\n", plugdet->name, plugdet->type);
	return plug; /* return new object */
}

const gchar *get_plugin_name(Plugin *plugin){
  gphpedit_debug(DEBUG_PLUGINS);
  g_return_val_if_fail (OBJECT_IS_PLUGIN (plugin), NULL);
  PluginDetails *plugdet;
	plugdet = PLUGIN_GET_PRIVATE(plugin);
  return plugdet->name;
}

const gchar *get_plugin_description(Plugin *plugin){
  gphpedit_debug(DEBUG_PLUGINS);
  g_return_val_if_fail (OBJECT_IS_PLUGIN (plugin), NULL);
  PluginDetails *plugdet;
	plugdet = PLUGIN_GET_PRIVATE(plugin);
  return plugdet->description;
}

const gchar *get_plugin_icon_name(Plugin *plugin){
  gphpedit_debug(DEBUG_PLUGINS);
  g_return_val_if_fail (OBJECT_IS_PLUGIN (plugin), NULL);
  PluginDetails *plugdet;
	plugdet = PLUGIN_GET_PRIVATE(plugin);
  return plugdet->icon_name;
}

gboolean get_plugin_active (Plugin *plugin)
{
  gphpedit_debug(DEBUG_PLUGINS);
	g_return_val_if_fail (plugin != NULL, FALSE);

  PluginDetails *plugdet = PLUGIN_GET_PRIVATE(plugin);

	return plugdet->active;
}

void set_plugin_active (Plugin *plugin, gboolean status)
{
  gphpedit_debug(DEBUG_PLUGINS);
	g_return_if_fail (plugin != NULL);

  PluginDetails *plugdet = PLUGIN_GET_PRIVATE(plugin);

  set_plugin_is_active(main_window.prefmg, plugdet->name, status);

	plugdet->active = status;
}
const gchar **
get_plugin_authors (Plugin *plugin)
{
  gphpedit_debug(DEBUG_PLUGINS);
	g_return_val_if_fail (plugin != NULL, (const gchar **)NULL);

  PluginDetails *plugdet = PLUGIN_GET_PRIVATE(plugin);

	return (const gchar **) plugdet->authors;
}

const gchar *
get_plugin_website (Plugin *plugin)
{
  gphpedit_debug(DEBUG_PLUGINS);
	g_return_val_if_fail (plugin != NULL, NULL);

  PluginDetails *plugdet = PLUGIN_GET_PRIVATE(plugin);

	return plugdet->website;
}

const gchar *
get_plugin_copyright (Plugin *plugin)
{
  gphpedit_debug(DEBUG_PLUGINS);
  g_return_val_if_fail (plugin != NULL, NULL);

  PluginDetails *plugdet = PLUGIN_GET_PRIVATE(plugin);

  return plugdet->copyright;
}

const gchar *
get_plugin_version (Plugin *plugin)
{
  gphpedit_debug(DEBUG_PLUGINS);
  g_return_val_if_fail (plugin != NULL, NULL);

  PluginDetails *plugdet = PLUGIN_GET_PRIVATE(plugin);

  return plugdet->version;
}

/*
* get_plugin_syntax_type
* return the file type of the syntax plugin
* if plugin isn't a syntax plugin return -1
*/
gint get_plugin_syntax_type(Plugin *plugin){
  gphpedit_debug(DEBUG_PLUGINS);
  g_return_val_if_fail (OBJECT_IS_PLUGIN (plugin), -1); /**/
  PluginDetails *plugdet;
	plugdet = PLUGIN_GET_PRIVATE(plugin);
  if (plugdet->type!=GPHPEDIT_PLUGIN_TYPE_SYNTAX) return -1;
  return plugdet->file_type;
}

/*
* save_as_temp_file (internal)
* save the content of an editor and return the filename of the temp file or NULL on error.
*/
static GString *save_as_temp_file(Documentable *document)
{
  gchar *write_buffer = documentable_get_text(document);
  GString *filename = text_save_as_temp_file(write_buffer);
  g_free(write_buffer);
  return filename;
}


void plugin_run(Plugin *plugin, Documentable *document, MainWindow *main_window)
{
  gphpedit_debug(DEBUG_PLUGINS);
  /* initial checks*/
  if (!OBJECT_IS_PLUGIN (plugin)) return;
  if (!document) return;
  PluginDetails *plugdet;
	plugdet = PLUGIN_GET_PRIVATE(plugin);
  
  if(!plugdet->active){
    gphpedit_debug_message(DEBUG_PLUGINS, "Plugin %s is not active\n", plugdet->name);
    return ;
  }

  gchar *stdout = NULL;
  GString *command_line = NULL;
  gchar *current_selection;
  gchar *data;
  gboolean using_temp = FALSE;
  GString *temp_name = NULL;
  command_line = g_string_new(plugdet->filename);
  command_line = g_string_prepend(command_line, "'");
  command_line = g_string_append(command_line, "' \"");

  if (plugdet->type == GPHPEDIT_PLUGIN_TYPE_SELECTION) {
    gboolean is_empty;
    g_object_get(document, "is_empty", &is_empty, NULL);
    if (!is_empty){
    current_selection = documentable_get_current_selected_text(document);
    gchar *escape= g_strescape(current_selection,"");
    command_line = g_string_append(command_line, escape);
    g_free(current_selection);
    g_free(escape);
    }
  }
  else if (plugdet->type == GPHPEDIT_PLUGIN_TYPE_FILENAME || plugdet->type == GPHPEDIT_PLUGIN_TYPE_SYNTAX) {
    gboolean saved;
    g_object_get(document, "saved", &saved, NULL);
    if (saved){
    gchar *filename = documentable_get_filename(document);
    gchar *temp_path=filename_get_path(filename); /* remove escaped chars*/
    g_free(filename);
    command_line = g_string_append(command_line, temp_path);
    g_free(temp_path);
    } else {
      temp_name = save_as_temp_file(document);
      command_line = g_string_append(command_line, temp_name->str);
      using_temp = TRUE;
    }
  }
  command_line = g_string_append(command_line, "\"");
  /* execute command */
  stdout = command_spawn(command_line->str);

  data = strstr(stdout, "\n");
  data++;
  if (g_str_has_prefix(stdout, "INSERT")){
      documentable_insert_text(document, data);
    }
    else if (g_str_has_prefix(stdout, "REPLACE")){
      documentable_replace_current_selection(document, data);
    }
    else if (g_str_has_prefix(stdout, "MESSAGE")){
        if (data){
        info_dialog(plugdet->name, data);
        }
    }
    else if (g_str_has_prefix(stdout, "SYNTAX")){
        syntax_window(GTK_SYNTAX_CHECK_WINDOW(main_window->pwin), document, data);
    }
    else if (g_str_has_prefix(stdout, "OPEN")){
      gphpedit_debug_message(DEBUG_PLUGINS,"Opening file :date: %s\n", data);
      document_manager_switch_to_file_or_open(main_window->docmg, data, 0);
    } else {
      g_print("Unexpected command\n");
    }
    if (using_temp) {
      release_temp_file (temp_name->str);
      g_string_free(temp_name, TRUE);
    }

  g_free(stdout);
  g_string_free (command_line,TRUE);
}
