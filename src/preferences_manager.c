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
 * PREFERENCES_MANAGER SYSTEM:
 */
/*
* manage all aplication preferences in a clean way, so aplication don't need to know how it's settings are stored.
* 
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "preferences_manager.h"
//#include "main_window.h"
#include <gconf/gconf-client.h>

#define MAXHISTORY 16

typedef struct
{
  gchar *name; /* useful value */
	gint color_back;
	gchar *font;
	gint color_fore;
	gint font_size;
	gboolean font_bold;
	gboolean font_italic;
} Scintilla_Style;

/*
* preferences_manager private struct
*/
struct Preferences_ManagerDetails
{
  /* important value */
	gboolean save_session;

	/* Window session settings */
	gint width;
	gint height;
	gint left;
	gint top;
	gboolean maximized;

  /* session settings*/
  gchar *last_opened_folder;
  guint parseonlycurrentfile:1;
  guint classbrowser_hidden:1;
  gint classbrowser_size;
  gchar *filebrowser_last_folder;
	gboolean showfilebrowser;

	// Default settings
	gint set_sel_back;
	gint marker_back;
	gint indentation_size;
	gint tab_size;
	gint show_indentation_guides:1;
	gint show_folding:1;
	gint edge_mode;
	gint edge_column;
	gint edge_colour;
	gchar *php_binary_location;
	gchar *shared_source_location;
	gchar *php_file_extensions;

	guint auto_complete_braces:1;
	guint higthlightcaretline:1;
	gint higthlightcaretline_color;
	//gint auto_indent_after_brace;
	gint auto_complete_delay;
	gint calltip_delay;
	guint line_wrapping:1;
	gboolean use_tabs_instead_spaces;
	gboolean single_instance_only;
	gint font_quality;	
	GSList *search_history; /* for incremental search history */
  /* style_table:
  * has a list with styles of the diferent lexers.
  */
  GHashTable *styles_table;
};

#define PREFERENCES_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    PREFERENCES_MANAGER_TYPE,\
					    Preferences_ManagerDetails))

static gpointer parent_class;
static void               preferences_manager_finalize         (GObject                *object);
static void               preferences_manager_init             (gpointer                object,
							       gpointer                klass);
static void  preferences_manager_class_init (Preferences_ManagerClass *klass);
static void preferences_manager_dispose (GObject *gobject);
void load_default_settings(Preferences_ManagerDetails *prefdet);
void load_styles(Preferences_ManagerDetails *prefdet);
void load_session_settings(Preferences_ManagerDetails *prefdet);
void load_window_settings(Preferences_ManagerDetails *prefdet);
static void set_string (const gchar *key, const gchar *string);
static void set_int (const gchar *key, gint number);
static void set_bool (const gchar *key, gboolean value);
static void set_string_list (const gchar *key, GSList *value);

static GSList *get_string_list(const gchar *key);
static gchar *get_string(const gchar *key,gchar *default_string);
static gint get_size(const gchar *key,gint default_size);
static gint get_color(const gchar *key,const gchar *subdir,gint default_color);
static gboolean get_bool(const gchar *key,gboolean default_value);
/*
 * preferences_manager_get_type
 * register Preferences_Manager type and returns a new GType
*/
GType
preferences_manager_get_type (void)
{
    static GType our_type = 0;
    
    if (!our_type) {
        static const GTypeInfo our_info =
        {
            sizeof (Preferences_ManagerClass),
            NULL,               /* base_init */
            NULL,               /* base_finalize */
            (GClassInitFunc) preferences_manager_class_init,
            NULL,               /* class_finalize */
            NULL,               /* class_data */
            sizeof (Preferences_Manager),
            0,                  /* n_preallocs */
            (GInstanceInitFunc) preferences_manager_init,
        };

        our_type = g_type_register_static (G_TYPE_OBJECT, "Preferences_Manager",
                                           &our_info, 0);
  }
    
    return our_type;
}
static void
preferences_manager_class_init (Preferences_ManagerClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
  parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = preferences_manager_finalize;
  object_class->dispose = preferences_manager_dispose;
	g_type_class_add_private (klass, sizeof (Preferences_ManagerDetails));
}
/*
* clean_style (internal)
* free resources
*/
void clean_style (gpointer data){
  Scintilla_Style *style= (Scintilla_Style *) data;
  if (!style) return ;
  if (style->name) g_free(style->name);
  if (style->font) g_free(style->font);
  g_slice_free(Scintilla_Style, style);
}

void clean_default_settings(Preferences_ManagerDetails *prefdet){
  /* free object resources*/
  if (prefdet->last_opened_folder) g_free(prefdet->last_opened_folder);
  if (prefdet->filebrowser_last_folder) g_free(prefdet->filebrowser_last_folder);
  if (prefdet->php_binary_location) g_free(prefdet->php_binary_location);
  if (prefdet->shared_source_location) g_free(prefdet->shared_source_location);
  if (prefdet->php_file_extensions) g_free(prefdet->php_file_extensions);
}
static void
preferences_manager_init (gpointer object, gpointer klass)
{
	Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(object);
  /* init styles table*/
  prefdet->styles_table= g_hash_table_new_full (g_str_hash, g_str_equal,NULL, clean_style);
}

/*
* disposes the Gobject
*/
static void preferences_manager_dispose (GObject *object)
{
  Preferences_Manager *pref = PREFERENCES_MANAGER(object);
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(pref);
  /* free object resources*/
  clean_default_settings(prefdet);
  /* Chain up to the parent class */
  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
preferences_manager_finalize (GObject *object)
{
  Preferences_Manager *pref = PREFERENCES_MANAGER(object);
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(pref);

  g_hash_table_destroy (prefdet->styles_table); /* clean data */

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

Preferences_Manager *preferences_manager_new (void)
{
	Preferences_Manager *pref;
  pref = g_object_new (PREFERENCES_MANAGER_TYPE, NULL);
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(pref);
  load_default_settings(prefdet);
  load_window_settings(prefdet); /* load main window settings*/
  load_session_settings(prefdet);
  load_styles(prefdet); /* load lexer styles */
	return pref; /* return new object */
}

#define DEFAULT_DELAY 500
#define DEFAULT_PHP_EXTENSIONS "php,inc,phtml,php3,xml,htm,html"

void load_default_settings(Preferences_ManagerDetails *prefdet)
{
  prefdet->set_sel_back = get_color("/gPHPEdit/default_style/selection","default_style",11250603);
  prefdet->marker_back = get_color("/gPHPEdit/default_style/bookmark","default_style",15908608);
  prefdet->php_binary_location= get_string("/gPHPEdit/locations/phpbinary","php");
  prefdet->shared_source_location = get_string("/gPHPEdit/locations/shared_source","");

  prefdet->indentation_size = get_size("/gPHPEdit/defaults/indentationsize",4); 
	prefdet->tab_size = get_size("/gPHPEdit/defaults/tabsize",4); 
	prefdet->auto_complete_delay = get_size("/gPHPEdit/defaults/auto_complete_delay", DEFAULT_DELAY);
	prefdet->calltip_delay = get_size("/gPHPEdit/defaults/calltip_delay", DEFAULT_DELAY);
	prefdet->show_indentation_guides = get_bool ("/gPHPEdit/defaults/showindentationguides",FALSE);
	prefdet->show_folding = TRUE;//gconf_client_get_bool (config,"/gPHPEdit/defaults/showfolding",NULL);
	prefdet->edge_mode = get_bool ("/gPHPEdit/defaults/edgemode",FALSE);
	prefdet->edge_column = get_size("/gPHPEdit/defaults/edgecolumn", 80);
	prefdet->edge_colour = get_color("/gPHPEdit/defaults/edgecolour","defaults",8355712);
	prefdet->line_wrapping = get_color("/gPHPEdit/defaults/linewrapping","defaults", TRUE);
	/* font quality */
	prefdet->font_quality = get_size("/gPHPEdit/defaults/fontquality", 0);
	prefdet->auto_complete_braces= get_bool("/gPHPEdit/defaults/autocompletebraces", FALSE);
	prefdet->higthlightcaretline= get_bool("/gPHPEdit/defaults/higthlightcaretline", FALSE);
	prefdet->higthlightcaretline_color= get_color("/gPHPEdit/defaults/higthlightcaretline_color","higthlightcaretline_color",13684944);
	prefdet->save_session = get_color("/gPHPEdit/defaults/save_session","defaults", 1);
	prefdet->use_tabs_instead_spaces = get_color("/gPHPEdit/defaults/use_tabs_instead_spaces","defaults", TRUE);
  prefdet->single_instance_only = get_color("/gPHPEdit/defaults/single_instance_only","defaults", TRUE);
  prefdet->php_file_extensions = get_string("/gPHPEdit/defaults/php_file_extensions",DEFAULT_PHP_EXTENSIONS);
	prefdet->search_history= get_string_list("/gPHPEdit/search_history");
	prefdet->showfilebrowser = get_color("/gPHPEdit/defaults/showfolderbrowser", "defaults",TRUE);
}

void load_window_settings(Preferences_ManagerDetails *prefdet)
{
  prefdet->left = get_color("/gPHPEdit/main_window/x","main_window",20);
  prefdet->top = get_color("/gPHPEdit/main_window/y","main_window",20);
  prefdet->width = get_color("/gPHPEdit/main_window/width","main_window",400);
  prefdet->height = get_color("/gPHPEdit/main_window/height","main_window",400);
	prefdet->maximized = get_size("/gPHPEdit/main_window/maximized",0);
}

void load_session_settings(Preferences_ManagerDetails *prefdet){

  prefdet->last_opened_folder = get_string("/gPHPEdit/general/last_opened_folder","");
  prefdet->parseonlycurrentfile = get_size("/gPHPEdit/classbrowser/onlycurrentfile", FALSE);
  prefdet->classbrowser_hidden = get_size("/gPHPEdit/main_window/classbrowser_hidden", FALSE);
  prefdet->classbrowser_size = get_color("/gPHPEdit/main_window/classbrowser_size", "main_window", 100);
  prefdet->filebrowser_last_folder=get_string("/gPHPEdit/main_window/folderbrowser/folder","");
}

        /* accesor functions */
  /*
  * each propertly has accesor functions to get and set his corresponding value
  * set functions don't store new value in gconf directly, instead store new value internally. 
  * you must call "preferences_manager_save_data" in order to update values in gconf.
  */

GSList *get_preferences_manager_search_history(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->search_history;
}
/*
 *get_preferences_manager_classbrowser_status
 *return 0 if classbrowser is hidden
 *return 1 if classbrowser is show
*/
gint get_preferences_manager_classbrowser_status(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->classbrowser_hidden;
}

void set_preferences_manager_parse_classbrowser_status(Preferences_Manager *preferences_manager, gint new_status){
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->classbrowser_hidden=new_status;
  set_int("/gPHPEdit/main_window/classbrowser_hidden",new_status);
}

/*
 *classbrowser_get_size
 *return current classbrowser size
 * default size 100
*/
gint get_preferences_manager_classbrowser_get_size(Preferences_Manager *preferences_manager){
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->classbrowser_size;
}

void set_preferences_manager_classbrowser_size(Preferences_Manager *preferences_manager, gint new_size){
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->classbrowser_size=new_size;
  set_int("/gPHPEdit/main_window/classbrowser_size",new_size);
}

/*
 *get_preferences_manager_parse_only_current_file
*/
gint get_preferences_manager_parse_only_current_file(Preferences_Manager *preferences_manager){
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->parseonlycurrentfile;
}

void set_preferences_manager_parse_only_current_file(Preferences_Manager *preferences_manager, gint new_status){
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->parseonlycurrentfile=new_status;
  set_int("/gPHPEdit/classbrowser/onlycurrentfile",new_status);
}

/*
 *get_preferences_manager_last_opened_folder
 * return an internal string must not be freed.
*/
const gchar *get_preferences_manager_last_opened_folder(Preferences_Manager *preferences_manager){
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->last_opened_folder;
}
/*
*store a new value for last opened folder
*/
void set_preferences_manager_last_opened_folder(Preferences_Manager *preferences_manager, const gchar *new_last_folder){
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  if (!new_last_folder) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  if (prefdet->last_opened_folder) g_free(prefdet->last_opened_folder);
  prefdet->last_opened_folder= g_strdup(new_last_folder);
  set_string ("/gPHPEdit/general/last_opened_folder", new_last_folder);
}
/*
 *get_preferences_manager_filebrowser_last_folder
 * return an internal string must not be freed.
*/
const gchar *get_preferences_manager_filebrowser_last_folder(Preferences_Manager *preferences_manager){
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->filebrowser_last_folder;
}
/*
*store a new value for filebrowser last folder
*/
void set_preferences_manager_filebrowser_last_folder(Preferences_Manager *preferences_manager, const gchar *new_last_folder){
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  if (prefdet->filebrowser_last_folder && strlen(prefdet->filebrowser_last_folder)!=0) g_free(prefdet->filebrowser_last_folder);
  prefdet->filebrowser_last_folder= g_strdup(new_last_folder);
  set_string ("/gPHPEdit/main_window/folderbrowser/folder", new_last_folder);
}

gboolean get_preferences_manager_show_filebrowser(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->showfilebrowser;
}

void set_preferences_manager_show_filebrowser(Preferences_Manager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->showfilebrowser = newstate; 

}

gint get_preferences_manager_window_height(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->height;
}

void set_preferences_manager_window_height(Preferences_Manager *preferences_manager, gint newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->height = newstate; 

}

gint get_preferences_manager_window_width(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->width;
}

void set_preferences_manager_window_width(Preferences_Manager *preferences_manager, gint newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->width = newstate; 

}

gint get_preferences_manager_window_left(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->left;
}

void set_preferences_manager_window_left(Preferences_Manager *preferences_manager, gint newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->left = newstate; 

}

gint get_preferences_manager_window_top(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->top;
}

void set_preferences_manager_window_top(Preferences_Manager *preferences_manager, gint newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->top = newstate; 

}

gboolean get_preferences_manager_window_maximized(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->maximized;
}

void set_preferences_manager_window_maximized(Preferences_Manager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->maximized = newstate; 

}
gint get_preferences_manager_indentation_size(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->indentation_size;
}

void set_preferences_manager_indentation_size(Preferences_Manager *preferences_manager, gint newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->indentation_size = newstate; 

}

const gchar *get_preferences_manager_php_binary_location(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->php_binary_location;
}

void set_preferences_manager_php_binary_location(Preferences_Manager *preferences_manager, gchar *newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->php_binary_location = newstate; 

}

gboolean get_preferences_manager_saved_session(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->save_session;
}

void set_preferences_manager_saved_session(Preferences_Manager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->save_session = newstate; 

}

gint get_preferences_manager_font_quality(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->font_quality;
}

void set_preferences_manager_font_quality(Preferences_Manager *preferences_manager, gint newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->font_quality = newstate; 

}

gboolean get_preferences_manager_line_wrapping(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->line_wrapping;
}

void set_preferences_manager_line_wrapping(Preferences_Manager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->line_wrapping = newstate; 

}

gint get_preferences_manager_calltip_delay(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->calltip_delay;
}

void set_preferences_manager_calltip_delay(Preferences_Manager *preferences_manager, gint newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->calltip_delay = newstate; 

}

gint get_preferences_manager_auto_complete_delay(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->auto_complete_delay;
}

void set_preferences_manager_auto_complete_delay(Preferences_Manager *preferences_manager, gint newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->auto_complete_delay = newstate; 

}

gboolean get_preferences_manager_use_tabs_instead_spaces(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->use_tabs_instead_spaces;
}

void set_preferences_manager_use_tabs_instead_spaces(Preferences_Manager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->use_tabs_instead_spaces = newstate; 

}


gboolean get_preferences_manager_auto_complete_braces(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->auto_complete_braces;
}

void set_preferences_manager_auto_complete_braces(Preferences_Manager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->auto_complete_braces = newstate; 

}

gboolean get_preferences_manager_show_folding(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->show_folding;
}

gboolean get_preferences_manager_single_instance_only(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->single_instance_only;
}

void set_preferences_manager_single_instance_only(Preferences_Manager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->single_instance_only = newstate; 

}


gboolean get_preferences_manager_higthlight_caret_line(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->higthlightcaretline;
}

void set_preferences_manager_higthlight_caret_line(Preferences_Manager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->higthlightcaretline = newstate; 

}

gboolean get_preferences_manager_higthlight_caret_line_color(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->higthlightcaretline_color;
}

void set_preferences_manager_higthlight_caret_line_color(Preferences_Manager *preferences_manager, gint newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->higthlightcaretline_color = newstate; 

}

gboolean get_preferences_manager_show_indentation_guides(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->show_indentation_guides;
}

void set_preferences_manager_show_indentation_guides(Preferences_Manager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->show_indentation_guides = newstate; 

}

gboolean get_preferences_manager_edge_mode(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->edge_mode;
}

void set_preferences_manager_edge_mode(Preferences_Manager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->edge_mode = newstate; 

}

gint get_preferences_manager_edge_column(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->edge_column;
}

void set_preferences_manager_edge_column(Preferences_Manager *preferences_manager, gint newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->edge_column = newstate; 

}

gint get_preferences_manager_edge_colour(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->edge_colour;
}

void set_preferences_manager_edge_colour(Preferences_Manager *preferences_manager, gint newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->edge_colour = newstate; 

}

gint get_preferences_manager_set_sel_back(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->set_sel_back;
}

void set_preferences_manager_set_sel_back(Preferences_Manager *preferences_manager, gint newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->set_sel_back = newstate; 

}

gint get_preferences_manager_tab_size(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->tab_size;
}

void set_preferences_manager_tab_size(Preferences_Manager *preferences_manager, gint newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->tab_size = newstate; 

}

gchar *get_preferences_manager_php_file_extensions(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->php_file_extensions;
}

void set_preferences_manager_php_file_extensions(Preferences_Manager *preferences_manager, gchar *newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  if (!newstate) return;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->php_file_extensions = newstate; 

}

gchar *get_preferences_manager_shared_source_location(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->shared_source_location;
}
void set_preferences_manager_shared_source_location(Preferences_Manager *preferences_manager, gchar *newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->shared_source_location = newstate; 
}

GSList *get_preferences_manager_php_search_history(Preferences_Manager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->search_history;
}

void set_preferences_manager_new_search_history_item(Preferences_Manager *preferences_manager, gint pos, const gchar *new_text)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->search_history = g_slist_prepend (prefdet->search_history, g_strdup(new_text));

    if (pos == MAXHISTORY){
    /* delete last item */
    GSList *temp= g_slist_nth (prefdet->search_history, MAXHISTORY);
    prefdet->search_history = g_slist_remove (prefdet->search_history, temp->data);
    }
}

/*
 * get_preferences_manager_style_settings
 * get an style data. If stylename isn't found in the hash table set parameters to NULL.
 * @stylename: name of the style to find
 * @font: name of the style font.
 * @size: pointer to the style size. 
 * @fore: pointer to the style fore color
 * @back: pointer to the style bold color
 * @italic: pointer to the boolean value
 * @bold: pointer to the boolean value
 * this function retrieves style settings. Set parameter to NULL to avoid it.
*/
void get_preferences_manager_style_settings(Preferences_Manager *preferences_manager, gchar *stylename, gchar **font , gint *size, gint *fore, gint *back, gboolean *italic, gboolean *bold)
{
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  Scintilla_Style *style = g_hash_table_lookup (prefdet->styles_table,stylename);
  if (!style) /*style not found set info to NULL to avoid crash */ {
    if (font) *font = NULL;
    /*FIXME:: is this value correct?? */
    if (size) size = NULL;
    if (fore) fore = NULL;
    if (back) back = NULL;
    if (italic) italic = NULL;
    if (bold) bold = NULL;
//    g_print("not found:%s", stylename);
    return;
  }
  if (font) *font= style->font;
  if (size) *size= style->font_size;
  if (fore) *fore= style->color_fore;
  if (back) *back= style->color_back;
  if (italic) *italic = style->font_italic;
  if (bold) *bold = style->font_bold;
}

/*
 * set_preferences_manager_style_settings
 * update style data. If stylename isn't found in the hash table do nothing.
 * @stylename: name of the style to modify
 * @font: name of the new font or NULL to keep actual font.
 * @size: pointer to the new size or NULL to keep actual size. 
 * @fore: pointer to the new fore color or NULL to keep actual fore color
 * @back: pointer to the new bold color or NULL to keep actual bold color
 * @italic: pointer to the boolean value or NULL to keep actual value. TRUE to set italic mode.
 * @bold: pointer to the boolean value or NULL to keep actual value. TRUE to set bold mode.
 * this function store new values internally, you need to call "preferences_manager_save_data" in order to update gconf values.
*/
void set_preferences_manager_style_settings(Preferences_Manager *preferences_manager, gchar *stylename, gchar *font , gint *size, gint *fore, gint *back, gboolean *italic, gboolean *bold)
{
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  Scintilla_Style *style = g_hash_table_lookup (prefdet->styles_table,stylename);
  if (!style) return;
  if (font) style->font = g_strdup(font);
  if (size) style->font_size = *size;
  if (fore) style->color_fore = *fore;
  if (back) style->color_back = *back;
  if (italic) style->font_italic = *italic;
  if (bold) style->font_bold = *bold;
}

/*
* save_style_settings
* save actual style data of each style in the hash table
*/
void save_style_settings (gpointer key, gpointer value, gpointer user_data)
{
  gchar *keyfont, *keysize, *keyfore, *keyback, *keyitalic, *keybold;
  Scintilla_Style *style = (Scintilla_Style *) value;
  if (!style) return;
  keyfont=g_strdup_printf("/gPHPEdit/%s/font",style->name);
  keysize=g_strdup_printf("/gPHPEdit/%s/size",style->name);
  keyfore=g_strdup_printf("/gPHPEdit/%s/fore",style->name);
  keyback=g_strdup_printf("/gPHPEdit/%s/back",style->name);
  keyitalic=g_strdup_printf("/gPHPEdit/%s/italic",style->name);
  keybold=g_strdup_printf("/gPHPEdit/%s/bold",style->name);
  
  set_string(keyfont, style->font);
  set_int (keysize, style->font_size);
//  g_print("%s %d %d",style->name, style->color_fore, style->color_back);
  set_int (keyfore, style->color_fore);
  set_int (keyback, style->color_back);
  set_bool (keyitalic, style->font_italic);
  set_bool (keybold, style->font_bold);

  g_free(keysize);
  g_free(keyfont);
  g_free(keyfore);
  g_free(keyback);
  g_free(keyitalic);
  g_free(keybold);
}

/*
* preferences_manager_save_data
* update session preferences data in gconf with new internal data
* this version only save a few preferences that can change often in the program.
* other preferences are only save when you call "preferences_manager_save_data_full"
* so we speed up process by not save unchanged data.
*/
void preferences_manager_save_data(Preferences_Manager *preferences_manager){
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  /*store window  settings*/
  if (!prefdet->maximized) {
  set_int ("/gPHPEdit/main_window/x", prefdet->left);
  set_int ("/gPHPEdit/main_window/y",prefdet->top);
  set_int ("/gPHPEdit/main_window/width", prefdet->width);
  set_int ("/gPHPEdit/main_window/height", prefdet->height);
  }
  set_int ("/gPHPEdit/main_window/maximized", prefdet->maximized);
  /**/
  set_bool("/gPHPEdit/defaults/showfolderbrowser", prefdet->showfilebrowser);
  set_string_list ("/gPHPEdit/search_history", prefdet->search_history);
}

/*
* preferences_manager_save_data_full
* update all preferences data in gconf with new internal data
*/
void preferences_manager_save_data_full(Preferences_Manager *preferences_manager){
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  /*store window  settings*/
  if (!prefdet->maximized) {
  set_int ("/gPHPEdit/main_window/x", prefdet->left);
  set_int ("/gPHPEdit/main_window/y",prefdet->top);
  set_int ("/gPHPEdit/main_window/width", prefdet->width);
  set_int ("/gPHPEdit/main_window/height", prefdet->height);
  }
  set_int ("/gPHPEdit/main_window/maximized", prefdet->maximized);
  /**/
  /* store style settings */
  g_hash_table_foreach (prefdet->styles_table, save_style_settings, NULL);
  /**/
  set_bool("/gPHPEdit/defaults/showfolderbrowser", prefdet->showfilebrowser);
  set_int ("/gPHPEdit/default_style/selection", prefdet->set_sel_back);
  set_int ("/gPHPEdit/default_style/bookmark", prefdet->marker_back);
  set_string ("/gPHPEdit/locations/phpbinary", prefdet->php_binary_location);
  set_string ("/gPHPEdit/locations/shared_source", prefdet->shared_source_location);
  set_int ("/gPHPEdit/defaults/indentationsize", prefdet->indentation_size); 
	set_int ("/gPHPEdit/defaults/tabsize", prefdet->tab_size); 
	set_int ("/gPHPEdit/defaults/auto_complete_delay", prefdet->auto_complete_delay);
	set_int ("/gPHPEdit/defaults/calltip_delay", prefdet->calltip_delay);
	set_bool ("/gPHPEdit/defaults/showindentationguides", prefdet->show_indentation_guides);
	prefdet->show_folding = TRUE;
	set_bool ("/gPHPEdit/defaults/edgemode", prefdet->edge_mode);
	set_int ("/gPHPEdit/defaults/edgecolumn", prefdet->edge_column);
	set_int ("/gPHPEdit/defaults/edgecolour", prefdet->edge_colour);
	set_bool ("/gPHPEdit/defaults/linewrapping", prefdet->line_wrapping);
	/* font quality */
	set_int ("/gPHPEdit/defaults/fontquality", prefdet->font_quality);
	set_bool("/gPHPEdit/defaults/autocompletebraces", prefdet->auto_complete_braces);
	set_bool ("/gPHPEdit/defaults/higthlightcaretline", prefdet->higthlightcaretline);
	set_int ("/gPHPEdit/defaults/higthlightcaretline_color", prefdet->higthlightcaretline_color);
	set_bool ("/gPHPEdit/defaults/save_session", prefdet->save_session);
	set_bool ("/gPHPEdit/defaults/use_tabs_instead_spaces", prefdet->use_tabs_instead_spaces);
  set_bool ("/gPHPEdit/defaults/single_instance_only", prefdet->single_instance_only);
  set_string ("/gPHPEdit/defaults/php_file_extensions", prefdet->php_file_extensions);
  set_string_list ("/gPHPEdit/search_history", prefdet->search_history);
}

/*
* preferences_manager_restore_data
* reload preferences data from gconf and discard internal data.
*/
void preferences_manager_restore_data(Preferences_Manager *preferences_manager){
  Preferences_ManagerDetails *prefdet;
	prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  clean_default_settings(prefdet);
  load_default_settings(prefdet);
  g_hash_table_destroy (prefdet->styles_table); /* clean styles data */
  prefdet->styles_table= g_hash_table_new_full (g_str_hash, g_str_equal,NULL, clean_style);
  load_styles(prefdet); /* load lexer styles */
}
/* internal functions to store values in gconf */

/*
* set_string (internal)
* store a string value in gconf
*/
static void set_string (const gchar *key, const gchar *string)
{
  GConfClient *config;
  config = gconf_client_get_default ();
  gconf_client_set_string (config,key,string,NULL);
  g_object_unref (G_OBJECT (config));
}
/*
* set_int (internal)
* store an integer value in gconf
*/
static void set_int (const gchar *key, gint number)
{
  GConfClient *config;
  config = gconf_client_get_default ();
  gconf_client_set_int (config,key,number,NULL);
  gconf_client_suggest_sync (config,NULL);
  gconf_client_clear_cache(config);
  g_object_unref (G_OBJECT (config));
}
/*
* set_bool (internal)
* store a boolean value in gconf
*/
static void set_bool (const gchar *key, gboolean value)
{
  GConfClient *config;
  config = gconf_client_get_default ();
	gconf_client_set_bool (config,key, value, NULL);
  g_object_unref (G_OBJECT (config));
}

/*
* set_string_list (internal)
* store a list of strings in gconf
*/
static void set_string_list (const gchar *key, GSList *value)
{
  GConfClient *config;
  config = gconf_client_get_default ();
	gconf_client_set_list (config, key, GCONF_VALUE_STRING, value, NULL);
  g_object_unref (G_OBJECT (config));
}

  /** internal functions to get value from gconf **/

/*
* get_string (internal)
* load an string value from gconf
* if value isn't found return default_string
*/
static gchar *get_string(const gchar *key,gchar *default_string){
  GError *error=NULL;
  GConfClient *config = gconf_client_get_default ();
  gchar *temp= gconf_client_get_string(config,key,NULL);
  g_object_unref (G_OBJECT (config));
  if (!temp || error){
    if (error) g_error_free (error);
    return default_string;
  }
	return temp;
}
/*
* get_size (internal)
* load a size value from gconf
* if value isn't found return default_size
*/
static gint get_size(const gchar *key,gint default_size){
  GConfClient *config = gconf_client_get_default ();
  GError *error=NULL;
  gint temp= gconf_client_get_int (config,key,&error);
  g_object_unref (G_OBJECT (config));
  if (temp==0){
    if (error){
     g_print("%s",error->message);
     g_error_free (error);
    }
    return default_size; 
  }
  return temp;
}

/*
* get_bool (internal)
* load a boolean value from gconf
* if value isn't found return default_value
*/
static gboolean get_bool(const gchar *key,gboolean default_value){
  GConfClient *config = gconf_client_get_default ();
  GError *error=NULL;
  gboolean temp= gconf_client_get_bool (config,key,&error);
  g_object_unref (G_OBJECT (config));
  if (error){
     g_error_free (error);
    return default_value; 
  }
  return temp;
}

/*
* get_color (internal)
* load a color value from gconf
* if value isn't found return default_color
*/
static gint get_color(const gchar *key,const gchar *subdir,gint default_color){
  gchar *uri= g_strdup_printf("%s/%s/%s",g_get_home_dir(),".gconf/gPHPEdit",subdir);
  if (!g_file_test (uri,G_FILE_TEST_EXISTS)){
    #ifdef DEBUG
    g_print("key %s don't exist. load default value\n",key);
    #endif
    //load default value
    g_free(uri);
    return default_color;
  } else {
    g_free(uri);
    //load key value
    GConfClient *config = gconf_client_get_default ();
    GError *error=NULL;
    gint temp;
    temp = gconf_client_get_int (config,key,&error);
    g_object_unref (G_OBJECT (config));
    if (error){
      //return default value
      g_error_free (error);
      return default_color;
    } else {
      return temp;
    }
  }
}

/*
* get_string_list (internal)
* load a list of strings values from gconf
*/
static GSList *get_string_list(const gchar *key){
    GConfClient *config = gconf_client_get_default ();
    GSList *temp =  gconf_client_get_list (config,key, GCONF_VALUE_STRING, NULL);
    g_object_unref (G_OBJECT (config));
    return temp;
}

  /**internal styles functions **/

/*
* load_style_font_string (internal)
* @style_name: the name of the style
* return a new allocated string. must be freed with g_free when no longer needed
* if font isn't found return DEFAULT font
* DEFAULT_FONT is defined by configure.ac, change value there. default value is "!sans"
*/
gchar *load_style_font_string(const gchar *style_name){
  gchar *key = g_strdup_printf("/gPHPEdit/%s/font",style_name);
  gchar *result = get_string(key, DEFAULT_FONT);
  g_free(key);
  return result;
}
/*
* load_style_color_back (internal)
* @style_name: the name of the style
* return a back color.
* if value isn't found return default back
*/
gint load_style_color_back(const gchar *style_name, gint default_back){
  gchar *key = g_strdup_printf("/gPHPEdit/%s/back",style_name);
  gint result = get_color(key, style_name, default_back);
  g_free(key);
  return result;
}
/*
* load_style_color_fore (internal)
* @style_name: the name of the style
* return a fore color.
* if value isn't found return default fore
*/
gint load_style_color_fore(const gchar *style_name, gint default_fore){
  gchar *key = g_strdup_printf("/gPHPEdit/%s/fore",style_name);
  gint result = get_color(key, style_name, default_fore);
  g_free(key);
  return result;
}
/*
* load_style_font_size (internal)
* @style_name: the name of the style
* return the font size for the requested style.
* if size isn't found return DEFAULT_FONT_SIZE
* DEFAULT_FONT_SIZE is defined by configure.ac, change value there. default value is "12"
*/
gint load_style_size (const gchar *style_name){
  gchar *key = g_strdup_printf("/gPHPEdit/%s/size",style_name);
  gint result = get_size(key, DEFAULT_FONT_SIZE);
  g_free(key);
  return result;
}
/*
* load_style_bold (internal)
* @style_name: the name of the style
* return TRUE if bold is activated.
* default value always FALSE.
*/
gboolean load_style_bold (const gchar *style_name){
  GConfClient *config = gconf_client_get_default ();
  gchar *key = g_strdup_printf("/gPHPEdit/%s/bold",style_name);
 	gboolean result = gconf_client_get_bool(config, key, NULL);
  g_free(key);
  g_object_unref (G_OBJECT (config));
  return result;
}

/*
* load_style_italic (internal)
* @style_name: the name of the style
* return TRUE if italic is activated.
* default value always FALSE.
*/
gboolean load_style_italic (const gchar *style_name){
  GConfClient *config = gconf_client_get_default ();
  gchar *key = g_strdup_printf("/gPHPEdit/%s/italic",style_name);
 	gboolean result = gconf_client_get_bool(config, key, NULL);
  g_free(key);
  g_object_unref (G_OBJECT (config));
  return result;
}
/*
* load_style (internal)
* @style_name: the name of the style
* @default_back: default back value. used if no back value is found.
* @default_fore: default back value. used if no fore value is found.
* loads the requested style settings and insert it in the hash table.
*/
void load_style(Preferences_ManagerDetails *prefdet, const gchar *style_name, gint default_back, gint default_fore){

  Scintilla_Style *style= g_slice_new(Scintilla_Style);  /* create new style */
  style->name = g_strdup(style_name);
  style->font = load_style_font_string(style_name);
	style->color_fore = load_style_color_fore(style_name, default_fore);
	style->color_back = load_style_color_back(style_name, default_back);
	style->font_size = load_style_size(style_name);
	style->font_bold = load_style_bold(style_name);
	style->font_italic = load_style_italic(style_name);
  
  g_hash_table_insert (prefdet->styles_table, style->name, style); /* add style to hash table */
  
//  g_print("%s %s %d %d %d", style->name, style->font, style->font_size, style->color_back, style->color_fore);
}

/* Default values */
#define DEFAULT_BACK_COLOR 16777215
/*
* load_styles (internal)
* loads lexer styles
*/
void load_styles(Preferences_ManagerDetails *prefdet)
{ 
  load_style(prefdet ,"default_style", DEFAULT_BACK_COLOR, 0);
  load_style(prefdet ,"line_numbers", 11053224, 0);
  load_style(prefdet ,"html_tag", DEFAULT_BACK_COLOR, 7553164);
  load_style(prefdet ,"html_tag_unknown", DEFAULT_BACK_COLOR, 7553164);
  load_style(prefdet ,"html_attribute", DEFAULT_BACK_COLOR, 9204544);
  load_style(prefdet ,"html_attribute_unknown", DEFAULT_BACK_COLOR, 7472544);
  load_style(prefdet ,"html_number", DEFAULT_BACK_COLOR, 9204544);
  load_style(prefdet ,"html_single_string", DEFAULT_BACK_COLOR, 32768);
  load_style(prefdet ,"html_double_string", DEFAULT_BACK_COLOR, 32768);
  load_style(prefdet ,"html_comment", DEFAULT_BACK_COLOR, 842125504);
  load_style(prefdet ,"html_entity", DEFAULT_BACK_COLOR, 8421504);
  load_style(prefdet ,"html_script", DEFAULT_BACK_COLOR, 7553165);
  load_style(prefdet ,"html_question", DEFAULT_BACK_COLOR, 7553165);
  load_style(prefdet ,"html_value", DEFAULT_BACK_COLOR, 21632);
  load_style(prefdet ,"javascript_comment", DEFAULT_BACK_COLOR, 8421504);
  load_style(prefdet ,"javascript_comment_line", DEFAULT_BACK_COLOR, 8421504);
  load_style(prefdet ,"javascript_comment_doc", DEFAULT_BACK_COLOR, 8355712);
  load_style(prefdet ,"javascript_word", DEFAULT_BACK_COLOR, 9204544);
  load_style(prefdet ,"javascript_keyword", DEFAULT_BACK_COLOR, 8388608);
  load_style(prefdet ,"javascript_doublestring", DEFAULT_BACK_COLOR, 8388608);
  load_style(prefdet ,"javascript_singlestring", DEFAULT_BACK_COLOR, 8388608);
  load_style(prefdet ,"javascript_symbols", DEFAULT_BACK_COLOR, 8355712);
  load_style(prefdet ,"php_default", DEFAULT_BACK_COLOR, 1052688);
  load_style(prefdet ,"php_hstring", DEFAULT_BACK_COLOR, 8388736);
  load_style(prefdet ,"php_simplestring", DEFAULT_BACK_COLOR, 8388736);
  load_style(prefdet ,"php_word", DEFAULT_BACK_COLOR, 3946645);
  load_style(prefdet ,"php_number", DEFAULT_BACK_COLOR, 9204544);
  load_style(prefdet ,"php_variable", DEFAULT_BACK_COLOR, 16746496);
  load_style(prefdet ,"php_comment", DEFAULT_BACK_COLOR, 8421594);
  load_style(prefdet ,"php_comment_line", DEFAULT_BACK_COLOR, 8421504);
  load_style(prefdet ,"css_tag", DEFAULT_BACK_COLOR, 8388608);
  load_style(prefdet ,"css_class", DEFAULT_BACK_COLOR, 8388608);
  load_style(prefdet ,"css_pseudoclass", DEFAULT_BACK_COLOR, 8388608);
  load_style(prefdet ,"css_unknown_pseudoclass", DEFAULT_BACK_COLOR, 16711680);
  load_style(prefdet ,"css_operator", DEFAULT_BACK_COLOR, 128);
  load_style(prefdet ,"css_identifier", DEFAULT_BACK_COLOR, 256);
  load_style(prefdet ,"css_unknown_identifier", DEFAULT_BACK_COLOR, 16711680);
  load_style(prefdet ,"css_value", DEFAULT_BACK_COLOR, 8388736);
  load_style(prefdet ,"css_comment", DEFAULT_BACK_COLOR, 84215504);
  load_style(prefdet ,"css_id", DEFAULT_BACK_COLOR, 8388608);
  load_style(prefdet ,"css_important", DEFAULT_BACK_COLOR, 255);
  load_style(prefdet ,"css_directive", DEFAULT_BACK_COLOR, 32768);
  load_style(prefdet ,"sql_word", DEFAULT_BACK_COLOR, 8388608);
  load_style(prefdet ,"sql_string", DEFAULT_BACK_COLOR, 8388736);
  load_style(prefdet ,"sql_operator", DEFAULT_BACK_COLOR, 128);
  load_style(prefdet ,"sql_comment", DEFAULT_BACK_COLOR, 8421504);
  load_style(prefdet ,"sql_number", DEFAULT_BACK_COLOR, 9204544);
  load_style(prefdet ,"sql_identifier", DEFAULT_BACK_COLOR, 16746496);
  load_style(prefdet ,"c_default", DEFAULT_BACK_COLOR, 1052688);
  load_style(prefdet ,"c_string", DEFAULT_BACK_COLOR, 8388736);
  load_style(prefdet ,"c_character", DEFAULT_BACK_COLOR, 8388736);
  load_style(prefdet ,"c_word", DEFAULT_BACK_COLOR, 16746496);
  load_style(prefdet ,"c_commentline", DEFAULT_BACK_COLOR, 8421504);
  load_style(prefdet ,"c_number", DEFAULT_BACK_COLOR, 9204544);
	load_style(prefdet ,"c_identifier", DEFAULT_BACK_COLOR, 1052688);
  load_style(prefdet ,"c_comment", DEFAULT_BACK_COLOR, 8421594);
  load_style(prefdet ,"c_preprocesor", DEFAULT_BACK_COLOR, 7553165);
  load_style(prefdet ,"c_operator", DEFAULT_BACK_COLOR, 128);
  load_style(prefdet ,"c_regex", DEFAULT_BACK_COLOR, 8388608);
  load_style(prefdet ,"c_uuid", DEFAULT_BACK_COLOR, 8388608);
  load_style(prefdet ,"c_verbatim", DEFAULT_BACK_COLOR, 255);
  load_style(prefdet ,"c_globalclass", DEFAULT_BACK_COLOR, 8388608);
}
