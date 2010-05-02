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
   
#include "plugin.h"
#include "main_window.h"
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>
//#define DEBUG
/*
* transform a number into it's corresponding keysym
*/
gint parse_shortcut(gint accel_number){ 
     switch (accel_number) {
 	case 0: return GDK_0;
		break;
	case 1: return GDK_1;
		break;
	case 2: return GDK_2;
		break;
	case 3: return GDK_3;
		break;
	case 4: return GDK_4;
		break;
	case 5: return GDK_5;
		break;
	case 6: return GDK_6;
		break;
	case 7: return GDK_7;
		break;
	case 8: return GDK_8;
		break;
	case 9: return GDK_9;
		break;
}
 return GDK_0;
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
static void syntax_window(gchar *plugin_name,gchar *data){
g_print("dTA:%s",data);
gchar *copy;
gchar *token;
gchar *line_number;
gchar *first_error = NULL;
gint line_start;
gint line_end;
gint indent;

/* clear document before start any styling action */
gtk_scintilla_indicator_clear_range(GTK_SCINTILLA(main_window.current_editor->scintilla), 0, gtk_scintilla_get_text_length(GTK_SCINTILLA(main_window.current_editor->scintilla)));

gtk_widget_show(main_window.scrolledwindow1);
gtk_widget_show(main_window.lint_view);
GtkTreeIter iter;
main_window.lint_store = gtk_list_store_new (1, G_TYPE_STRING);
/*clear tree */
gtk_list_store_clear(main_window.lint_store);
copy = data;
gtk_scintilla_set_indicator_current(GTK_SCINTILLA(main_window.current_editor->scintilla), 20);
gtk_scintilla_indic_set_style(GTK_SCINTILLA(main_window.current_editor->scintilla), 20, INDIC_SQUIGGLE);
gtk_scintilla_indic_set_fore(GTK_SCINTILLA(main_window.current_editor->scintilla), 20, 0x0000ff);
/* lines has form line number space message dot like 
* 59 invalid operator.\n
* lines end with \n
*/
while ((token = strtok(copy, "\n"))) {
gtk_list_store_append (main_window.lint_store, &iter);
gtk_list_store_set (main_window.lint_store, &iter, 0, token, -1);	
	line_number = strchr(token, ' ');
	line_number=strncpy(line_number,token,(int)(line_number-token));
	if (atoi(line_number)>0) {
	if (!first_error) {
		first_error = line_number;
	}
	indent = gtk_scintilla_get_line_indentation(GTK_SCINTILLA(main_window.current_editor->scintilla), atoi(line_number)-1);
	
	line_start = gtk_scintilla_position_from_line(GTK_SCINTILLA(main_window.current_editor->scintilla), atoi(line_number)-1);
	line_start += (indent/preferences.indentation_size);
	
	line_end = gtk_scintilla_get_line_end_position(GTK_SCINTILLA(main_window.current_editor->scintilla), atoi(line_number)-1);
	gtk_scintilla_indicator_fill_range(GTK_SCINTILLA(main_window.current_editor->scintilla), line_start, line_end-line_start);
	}
copy = NULL;
}
gtk_tree_view_set_model(GTK_TREE_VIEW(main_window.lint_view), GTK_TREE_MODEL(main_window.lint_store));
}

GList *Plugins = NULL;

static inline gchar *plugin_spawn(const gchar* command_line)
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
		g_print("------------------------------------\nDISCOVERY\nCOMMAND: %s\nOUTPUT: %s (%d)\n", command_line->str, stdout, stdout_len);
		#endif
		ret=g_strdup(stdout);
		g_free(stdout);
	} else {
		g_print("Spawning %s gave error %s\n", command_line, error->message);
		g_error_free (error);
	}	
	
	return ret;
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
	result= plugin_spawn(command_line->str);
	if(result){
		if (g_str_has_prefix(result, "SELECTION")){
			type = GPHPEDIT_PLUGIN_TYPE_SELECTION;
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

/****/

gchar *plugin_discover_name(gchar *filename,const gchar *file_name)
{
	GString *command_line;
	gchar *name=NULL;
	
	command_line = g_string_new(filename);
	command_line = g_string_prepend(command_line, "'");
	command_line = g_string_append(command_line, "' -name");
        name = plugin_spawn(command_line->str);
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
	desc = plugin_spawn(command_line->str);
        #ifdef DEBUG
	g_print("Plugin description:%s\n",desc);
	#endif
	g_string_free(command_line, TRUE);
	
	return desc;
}

gint sort_plugin_func(gconstpointer a, gconstpointer b)
{
	Plugin *plugina = (Plugin *)a;
	Plugin *pluginb = (Plugin *)b;

	if (strcmp(plugina->name, pluginb->name) < 0) {
		return -1;
	}
	return 1;
}
static void new_plugin(gchar *filename,const gchar *plugin_name){
gchar *pluname= plugin_discover_name(filename,plugin_name);
if (pluname){
    Plugin *plugin;
    plugin = g_slice_new(Plugin);
    // TODO: Could do with replacing ' in name with \' for spawn
    plugin->filename = g_strdup(filename);
    plugin->name = pluname;
    #ifdef DEBUG
    g_print ("PLUGIN FILENAME: %s\n", plugin->filename);
    #endif
    plugin->type = plugin_discover_type(plugin->filename);
    plugin->description = plugin_discover_desc(plugin->filename);
    Plugins = g_list_prepend(Plugins, plugin);
}
}
void plugin_discover_available(void)
{
	GDir *dir;
	const gchar *plugin_name;
	GString *user_plugin_dir;
	GString *filename;
	
	user_plugin_dir = g_string_new( g_get_home_dir());
	user_plugin_dir = g_string_append(user_plugin_dir, "/.gphpedit/plugins/");
        #ifdef DEBUG
	g_print("User plugin dir: %s\n", user_plugin_dir->str);
        #endif
	if (g_file_test(user_plugin_dir->str, G_FILE_TEST_IS_DIR)) {
		dir = g_dir_open(user_plugin_dir->str, 0,NULL);
		if (dir) {
			for (plugin_name = g_dir_read_name(dir); plugin_name != NULL; plugin_name = g_dir_read_name(dir)) {
				filename = g_string_new(plugin_name);
				filename = g_string_prepend(filename, user_plugin_dir->str);
                                new_plugin(filename->str,plugin_name);
				g_string_free (filename,TRUE);
			}
			g_dir_close(dir);			
		}
	}
	g_string_free(user_plugin_dir, TRUE);

	if (g_file_test("/usr/share/gphpedit/plugins/", G_FILE_TEST_IS_DIR)) {
		dir = g_dir_open("/usr/share/gphpedit/plugins/", 0,NULL);
		if (dir) {
			for (plugin_name = g_dir_read_name(dir); plugin_name != NULL; plugin_name = g_dir_read_name(dir)) {
				filename = g_string_new(plugin_name);
				filename = g_string_prepend(filename, "/usr/share/gphpedit/plugins/");
                                new_plugin(filename->str,plugin_name);
				g_string_free (filename,TRUE);
			}
			g_dir_close(dir);			
		}
	}

	Plugins = g_list_sort(Plugins, sort_plugin_func);
#ifdef DEBUG
    g_print ("FOUND ALL PLUGINS\n");
#endif
}

void plugin_create_menu_items()
{
	GList *iterator;
	Plugin *plugin;
	guint num_plugin;
	guint hide_plugin;

	GtkBin *bin = NULL;
	
	num_plugin = 0;
	for (iterator = Plugins; iterator != NULL && num_plugin<NUM_PLUGINS_MAX; iterator = g_list_next(iterator)) {
		plugin = (Plugin *)(iterator->data);
                #ifdef DEBUG
		g_print ("Plugin %d:%s\n", num_plugin, plugin->filename);
		g_print("Getting child widget\n");
                #endif
                bin = GTK_BIN(main_window.menu->plugins[num_plugin]);
		//g_print("Bin is %p\n", bin);
		if (bin) {
			GtkLabel *label;
			label = GTK_LABEL(gtk_bin_get_child(bin));
		
			gtk_label_set_text(label, plugin->name);
                        gtk_widget_show(main_window.menu->plugins[num_plugin]);
			install_menu_hint(GTK_WIDGET(bin), plugin->description);
		}
		
		num_plugin++;
	}
        #ifdef DEBUG
	g_print("Blanking all non-found plugin entries\n");
        #endif
        for (hide_plugin=num_plugin; hide_plugin <NUM_PLUGINS_MAX; hide_plugin++) {
		/* don't hide destroy it */
            gtk_widget_destroy (main_window.menu->plugins[hide_plugin]);
	}
}


void plugin_setup_menu(void)
{
    
	plugin_discover_available();
	if (Plugins) {
		plugin_create_menu_items();
	}
	else {
              gtk_widget_hide(main_window.menu->plugin);
       }
}
void plugin_exec(gint plugin_num)
{
	Plugin *plugin;
	gchar *stdout = NULL;
	GError *error = NULL;
	gint exit_status;
	GString *command_line = NULL;
	gint wordStart;
	gint wordEnd;
	gchar *current_selection;
	gint ac_length;
	gchar *data;
	
	if (main_window.current_editor == NULL) {
		return;
	}
	
	plugin = (Plugin *)g_list_nth_data(Plugins, plugin_num);
	if (!plugin) {
                #ifdef DEBUG
		g_print(_("Plugin is null!\n"));
                #endif
		/* don't crash if plugin if null */
		return;
	}
        #ifdef DEBUG
	g_print("Plugin No: %d:%d (%s):%s\n", plugin_num, plugin->type, plugin->name, plugin->filename);
        #endif
	command_line = g_string_new(plugin->filename);
	command_line = g_string_prepend(command_line, "'");
	command_line = g_string_append(command_line, "' \"");
	if (plugin->type == GPHPEDIT_PLUGIN_TYPE_SELECTION) {
                wordStart = gtk_scintilla_get_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla));
		wordEnd = gtk_scintilla_get_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla));
		current_selection = gtk_scintilla_get_text_range (GTK_SCINTILLA(main_window.current_editor->scintilla), wordStart, wordEnd, &ac_length);
		gchar *escape=g_strescape(current_selection,"");
		command_line = g_string_append(command_line, escape);
		g_free(current_selection);
		g_free(escape);
	}
	else if (plugin->type == GPHPEDIT_PLUGIN_TYPE_FILENAME) {
		command_line = g_string_append(command_line, editor_convert_to_local(main_window.current_editor));		
	}
	command_line = g_string_append(command_line, "\"");
        #ifdef DEBUG
	g_print("SPAWNING: %s\n", command_line->str);
	#endif
	if (g_spawn_command_line_sync(command_line->str,&stdout,NULL, &exit_status,&error)) {
		data = strstr(stdout, "\n");
		data++;
	#ifdef DEBUG
		g_print("COMMAND: %s\nSTDOUT:%s\nOUTPUT: %s\n", command_line->str, stdout, data);
	#endif
		if (g_str_has_prefix(stdout, "INSERT")){
			if (data) {
				gtk_scintilla_insert_text(GTK_SCINTILLA(main_window.current_editor->scintilla), 
					gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla)), data);
			}
		}
		else if (g_str_has_prefix(stdout, "REPLACE")){
			if (data) {
				gtk_scintilla_replace_sel(GTK_SCINTILLA(main_window.current_editor->scintilla), data);
			}
		}
		else if (g_str_has_prefix(stdout, "MESSAGE")){
				if (data){
				info_dialog(plugin->name, data);
				}
		}
		else if (g_str_has_prefix(stdout, "SYNTAX")){
				/*TODO: save file before execute plugin?*/
				if (data){
				syntax_window(plugin->name, data);
				}
		}
		else if (g_str_has_prefix(stdout, "OPEN")){
			if (DEBUG_MODE) { g_print("DEBUG: main_window.c:plugin_exec: Opening file :date: %s\n", data); }
			switch_to_file_or_open(data, 0);
		}
		else if (g_str_has_prefix(stdout, "DEBUG")){
			debug_dump_editors();
			DEBUG_MODE = TRUE;
		}
		g_free(stdout);
	}
	else {
		g_print(_("Spawning %s gave error %s\n"), plugin->filename, error->message);
		g_error_free (error);
	}
	g_string_free ((GString *) command_line,TRUE);
}

static void clean_list_item (gpointer data, gpointer user_data){
     Plugin *plugin=(Plugin *)data;
     g_free(plugin->filename);
     g_free(plugin->name);
     g_free(plugin->description);
     g_slice_free(Plugin, plugin);
}

void cleanup_plugins(void){
	if (Plugins) {
	g_list_foreach(Plugins, (GFunc)clean_list_item, NULL);
	g_list_free(Plugins);
	}
}
