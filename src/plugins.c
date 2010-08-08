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
* gPHPEdit will ask for plugin name with a command line parameter of -name and a description with -desc
  ** name will be show in plugin menu and description in the statusbar
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
};

#define PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    PLUGIN_TYPE,\
					    PluginDetails))

static gpointer parent_class;
static void plugin_finalize (GObject *object);
static void plugin_init (gpointer object, gpointer klass);
static void  plugin_class_init (PluginClass *klass);


/*
 * plugin_get_type
 * register Plugin type and returns a new GType
*/
GType
plugin_get_type (void)
{
    static GType our_type = 0;
    
    if (!our_type) {
        static const GTypeInfo our_info =
        {
            sizeof (PluginClass),
            NULL,               /* base_init */
            NULL,               /* base_finalize */
            (GClassInitFunc) plugin_class_init,
            NULL,               /* class_finalize */
            NULL,               /* class_data */
            sizeof (Plugin),
            0,                  /* n_preallocs */
            (GInstanceInitFunc) plugin_init,
        };

        our_type = g_type_register_static (G_TYPE_OBJECT, "Plugin",
                                           &our_info, 0);
  }
    
    return our_type;
}
static void
plugin_class_init (PluginClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
  parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = plugin_finalize;
	g_type_class_add_private (klass, sizeof (PluginDetails));
}

static void
plugin_init (gpointer object, gpointer klass)
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

	G_OBJECT_CLASS (parent_class)->finalize (object);
}


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
  if (g_spawn_command_line_sync(command_line,&stdout,NULL, &exit_status,&error)) {
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
  gphpedit_debug_message(DEBUG_PLUGINS,"Name: %s(%d)\n", plugdet->name, plugdet->type);
	return plug; /* return new object */
}

const gchar *get_plugin_name(Plugin *plugin){
  g_return_val_if_fail (OBJECT_IS_PLUGIN (plugin), NULL);
  PluginDetails *plugdet;
	plugdet = PLUGIN_GET_PRIVATE(plugin);
  return plugdet->name;
}

const gchar *get_plugin_description(Plugin *plugin){
  g_return_val_if_fail (OBJECT_IS_PLUGIN (plugin), NULL);
  PluginDetails *plugdet;
	plugdet = PLUGIN_GET_PRIVATE(plugin);
  return plugdet->description;
}
/*
* get_plugin_syntax_type
* return the file type of the syntax plugin
* if plugin isn't a syntax plugin return -1
*/
gint get_plugin_syntax_type(Plugin *plugin){
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
static GString *save_as_temp_file(Document *document)
{
  gchar *write_buffer = document_get_text(document);
  GString *filename = text_save_as_temp_file(write_buffer);
  g_free(write_buffer);
  return filename;
}


void plugin_run(Plugin *plugin, Document *document)
{
  /* initial checks*/
  if (!OBJECT_IS_PLUGIN (plugin)) return;
  if (!document) return;
  PluginDetails *plugdet;
	plugdet = PLUGIN_GET_PRIVATE(plugin);

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
    if (!document_get_is_empty(document)){
    current_selection = document_get_current_selected_text(document);
    gchar *escape= g_strescape(current_selection,"");
    command_line = g_string_append(command_line, escape);
    g_free(current_selection);
    g_free(escape);
    }
  }
  else if (plugdet->type == GPHPEDIT_PLUGIN_TYPE_FILENAME || plugdet->type == GPHPEDIT_PLUGIN_TYPE_SYNTAX) {
    if (document_get_saved_status(document)){
    gchar *filename = document_get_filename(document);
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
      document_insert_text(document, data);
    }
    else if (g_str_has_prefix(stdout, "REPLACE")){
      document_replace_current_selection(document, data);
    }
    else if (g_str_has_prefix(stdout, "MESSAGE")){
        if (data){
        info_dialog(plugdet->name, data);
        }
    }
    else if (g_str_has_prefix(stdout, "SYNTAX")){
        syntax_window(main_window.win, document, data);
    }
    else if (g_str_has_prefix(stdout, "OPEN")){
      gphpedit_debug_message(DEBUG_PLUGINS,"Opening file :date: %s\n", data);
      switch_to_file_or_open(data, 0);
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
