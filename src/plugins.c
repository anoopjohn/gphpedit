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
/*
* TODO::inline plugins docs
*
*/
#include <config.h>
#include <stdlib.h>
#include <gtk/gtk.h>
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
  GPHPEDIT_PLUGIN_TYPE_DEBUG
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
static void               plugin_finalize         (GObject                *object);
static void               plugin_init             (gpointer                object,
							       gpointer                klass);
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
  #ifdef DEBUG
  gint stdout_len;
  #endif
  if (g_spawn_command_line_sync(command_line,&stdout,NULL, &exit_status,&error)) {
    #ifdef DEBUG
    stdout_len = strlen(stdout);
    g_print("COMMAND: %s\nOUTPUT: %s (%d)\n", command_line->str, stdout, stdout_len);
    #endif
    ret=g_strdup(stdout);
    g_free(stdout);
  } else {
    g_print("Command %s gave error %s\n", command_line, error->message);
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
  #ifdef DEBUG
  g_print("plugin name:%s\n",name);
  #endif

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
  #ifdef DEBUG
  g_print("Plugin description:%s\n",desc);
  #endif
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
  else if (g_str_has_prefix(result, "DEBUG")){
    type = GPHPEDIT_PLUGIN_TYPE_DEBUG;
  }
  #ifdef DEBUG
  g_print("Returning Discovered type of %d\n------------------------------------\n", type);
  #endif
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
  ftype = command_spawn(command_line->str);
  #ifdef DEBUG
  g_print("Plugin syntax File type:%s\n",ftype);
  #endif
  g_string_free(command_line, TRUE);
  
  if (g_strcmp0(ftype,"PHP")==0 || g_strcmp0(ftype,"HTML")==0 || g_strcmp0(ftype,"XML")==0) file_type=TAB_PHP;
  else if (g_strcmp0(ftype,"CSS")==0) file_type=TAB_CSS;
  else if (g_strcmp0(ftype,"C")==0 || g_strcmp0(ftype,"C++")==0) file_type=TAB_CXX;
  else if (g_strcmp0(ftype,"CSS")==0) file_type=TAB_CSS;
  else if (g_strcmp0(ftype,"COBOL")==0) file_type=TAB_COBOL;
  else if (g_strcmp0(ftype,"SQL")==0) file_type=TAB_SQL;
  else if (g_strcmp0(ftype,"PERL")==0) file_type=TAB_PERL;
  else if (g_strcmp0(ftype,"PYTHON")==0) file_type=TAB_PYTHON;
  else file_type=TAB_FILE;
  if (file_type==TAB_FILE) g_print("Unknown file type for Syntax plugin:%s\n",ftype);
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

gint get_plugin_syntax_type(Plugin *plugin){
  g_return_val_if_fail (OBJECT_IS_PLUGIN (plugin), 0); /**/
  PluginDetails *plugdet;
	plugdet = PLUGIN_GET_PRIVATE(plugin);
  if (plugdet->type!=GPHPEDIT_PLUGIN_TYPE_SYNTAX) return 0;
  return plugdet->file_type;
}

/* 
* syntax_window:
* this función accept debug info, show it in syntax pane and apply style to text.
* lines has form line number space message dot like next example:
* 59 invalid operator.\n
* lines end with \n 
* if data hasn't got that format it'll be shown be error will not be styled.
*/
/*
* TODO: double click in tree row should goto the corresponding line.
*/
static void syntax_window(GtkScintilla *scintilla, gchar *data){
  if (!scintilla) return;
  if (!data) return;
  gchar *copy;
  gchar *token;
  gchar *line_number;
  gchar *first_error = NULL;
  gint line_start;
  gint line_end;
  gint indent;

  /* clear document before start any styling action */
  gtk_scintilla_indicator_clear_range(scintilla, 0, gtk_scintilla_get_text_length(scintilla));

  gtk_widget_show(GTK_WIDGET(main_window.win));
  GtkTreeIter iter;
  GtkListStore *lint_store = gtk_list_store_new (1, G_TYPE_STRING);
  /*clear tree */
  gtk_list_store_clear(lint_store);
  copy = data;
  /* este codigo esta repetido aca y en el sintax check tal vez unificar en un solo lugar */
  gtk_scintilla_set_indicator_current(scintilla, 20);
  gtk_scintilla_indic_set_style(scintilla, 20, INDIC_SQUIGGLE);
  gtk_scintilla_indic_set_fore(scintilla, 20, 0x0000ff);

  gtk_scintilla_annotation_clear_all(scintilla);
  gtk_scintilla_annotation_set_visible(scintilla, 2);
  /* lines has form line number space message dot like 
  * 59 invalid operator.\n
  * lines end with \n
  */

  while ((token = strtok(copy, "\n"))) {
    gtk_list_store_append (lint_store, &iter);
    gtk_list_store_set (lint_store, &iter, 0, token, -1);
    gchar *anotationtext=g_strdup(token);
    line_number = strchr(token, ' ');
    line_number=strncpy(line_number,token,(int)(line_number-token));
    if (atoi(line_number)>0) {
      if (!first_error) {
      first_error = line_number;
      }
      guint current_line_number=atoi(line_number)-1;
      indent = gtk_scintilla_get_line_indentation(scintilla, current_line_number);
  
      line_start = gtk_scintilla_position_from_line(scintilla, current_line_number);
      line_start += (indent/preferences.indentation_size);
  
      line_end = gtk_scintilla_get_line_end_position(scintilla, current_line_number);
      gtk_scintilla_indicator_fill_range(scintilla, line_start, line_end-line_start);
      token=anotationtext + (int)(line_number-token+1);
      /* if first char is an E then set error style, else if first char is W set warning style */
      if (strncmp(token,"E",1)==0)
        gtk_scintilla_annotation_set_style(scintilla, current_line_number, STYLE_ANNOTATION_ERROR);
      else if (strncmp(token,"W",1)==0)
        gtk_scintilla_annotation_set_style(scintilla, current_line_number, STYLE_ANNOTATION_WARNING);
      token+=1;
      gtk_scintilla_annotation_set_text(scintilla, current_line_number, token);
    }
    g_free(anotationtext);
    copy = NULL;
  }
 gtk_syntax_check_window_set_model(main_window.win, lint_store);
}


void plugin_run(Plugin *plugin, Editor *editor)
{
  /* initial checks*/
  if (!OBJECT_IS_PLUGIN (plugin)) return;
  if (!editor) return;
  PluginDetails *plugdet;
	plugdet = PLUGIN_GET_PRIVATE(plugin);

  gchar *stdout = NULL;
  GString *command_line = NULL;
  gint wordStart;
  gint wordEnd;
  gchar *current_selection;
  gint ac_length;
  gchar *data;
  
  command_line = g_string_new(plugdet->filename);
  command_line = g_string_prepend(command_line, "'");
  command_line = g_string_append(command_line, "' \"");

  if (plugdet->type == GPHPEDIT_PLUGIN_TYPE_SELECTION) {
    wordStart = gtk_scintilla_get_selection_start(GTK_SCINTILLA(editor->scintilla));
    wordEnd = gtk_scintilla_get_selection_end(GTK_SCINTILLA(editor->scintilla));
    current_selection = gtk_scintilla_get_text_range (GTK_SCINTILLA(editor->scintilla), wordStart, wordEnd, &ac_length);
    gchar *escape= g_strescape(current_selection,"");
    command_line = g_string_append(command_line, escape);
    g_free(current_selection);
    g_free(escape);
  }
  else if (plugdet->type == GPHPEDIT_PLUGIN_TYPE_FILENAME || plugdet->type == GPHPEDIT_PLUGIN_TYPE_SYNTAX) {
    gchar *temp_path=filename_get_path(editor->filename->str); /* remove escaped chars*/
    command_line = g_string_append(command_line, temp_path);
    g_free(temp_path);
  }
  command_line = g_string_append(command_line, "\"");
  /* execute command */
  stdout = command_spawn(command_line->str);

  data = strstr(stdout, "\n");
  data++;
  if (g_str_has_prefix(stdout, "INSERT")){
      if (data) {
        gtk_scintilla_insert_text(GTK_SCINTILLA(editor->scintilla), 
        gtk_scintilla_get_current_pos(GTK_SCINTILLA(editor->scintilla)), data);
      }
    }
    else if (g_str_has_prefix(stdout, "REPLACE")){
      if (data) {
        gtk_scintilla_replace_sel(GTK_SCINTILLA(editor->scintilla), data);
      }
    }
    else if (g_str_has_prefix(stdout, "MESSAGE")){
        if (data){
        info_dialog(plugdet->name, data);
        }
    }
    else if (g_str_has_prefix(stdout, "SYNTAX")){
        /*TODO: save file before execute plugin?*/
        syntax_window(GTK_SCINTILLA(editor->scintilla), data);
    }
    else if (g_str_has_prefix(stdout, "OPEN")){
      if (DEBUG_MODE) { g_print("DEBUG: main_window.c:plugin_exec: Opening file :date: %s\n", data); }
      switch_to_file_or_open(data, 0);
    }
    else if (g_str_has_prefix(stdout, "DEBUG")){
      debug_dump_editors();
      DEBUG_MODE = TRUE;
    } else {
      g_print("Unexpected command");
    }
  g_free(stdout);
  g_string_free (command_line,TRUE);
}
