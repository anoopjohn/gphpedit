/* This file is part of gPHPEdit, a GNOME2 PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
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
#include <gconf/gconf-client.h>
#include <glib/gi18n.h>

#include "debug.h"
#include "preferences_manager.h"
#include "gvfs_utils.h"
#include "document.h"

#define MAXHISTORY 16

/*
* preferences_manager private struct
*/
struct PreferencesManagerDetails
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
  gboolean parseonlycurrentfile;
  gboolean classbrowser_hidden;
  gint side_panel_size;
  gchar *filebrowser_last_folder;
  gboolean showfilebrowser;
  guint showstatusbar:1;
  guint showmaintoolbar:1;
  guint showfindtoolbar:1;
  // Default settings
  gint marker_back;
  gint indentation_size;
  gint tab_size;
  guint show_indentation_guides:1;
  guint show_folding:1;
  gint edge_mode;
  gint edge_column;
  gchar *php_binary_location;
  gchar *shared_source_location;
  gchar *php_file_extensions;

  guint auto_complete_braces:1;
  guint higthlightcaretline:1;
  //gint auto_indent_after_brace;
  gint auto_complete_delay;
  gint calltip_delay;
  guint line_wrapping:1;
  guint use_tabs_instead_spaces:1;
  guint single_instance_only:1;
  gint font_quality;  
  GSList *search_history; /* for incremental search history */

  gchar *style_name;
  gint font_size;
  gchar *font_name;
};

#define PREFERENCES_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
              PREFERENCES_MANAGER_TYPE,\
              PreferencesManagerDetails))

static void               preferences_manager_finalize         (GObject                *object);
static void  preferences_manager_class_init (PreferencesManagerClass *klass);
static void preferences_manager_dispose (GObject *gobject);
void load_default_settings(PreferencesManagerDetails *prefdet);
void load_styles(PreferencesManagerDetails *prefdet);
void load_session_settings(PreferencesManagerDetails *prefdet);
void load_window_settings(PreferencesManagerDetails *prefdet);
static void set_string (const gchar *key, const gchar *string);
static void set_int (const gchar *key, gint number);
static void set_bool (const gchar *key, gboolean value);
static void set_string_list (const gchar *key, GSList *value);

static GSList *get_string_list(const gchar *key);
static gchar *get_string(const gchar *key,gchar *default_string);
static gint get_size(const gchar *key,gint default_size);
static gint get_color(const gchar *key,const gchar *subdir,gint default_color);
static gboolean get_bool(const gchar *key,gboolean default_value);

/* http://library.gnome.org/devel/gobject/unstable/gobject-Type-Information.html#G-DEFINE-TYPE:CAPS */
G_DEFINE_TYPE(PreferencesManager, preferences_manager, G_TYPE_OBJECT);

/*
* overide default contructor to make a singleton.
* see http://blogs.gnome.org/xclaesse/2010/02/11/how-to-make-a-gobject-singleton/
*/
static GObject*
preferences_manager_constructor (GType type,
                 guint n_construct_params,
                 GObjectConstructParam *construct_params)
{
  static GObject *self = NULL;

  if (self == NULL)
    {
      self = G_OBJECT_CLASS (preferences_manager_parent_class)->constructor (
          type, n_construct_params, construct_params);
      g_object_add_weak_pointer (self, (gpointer) &self);
      return self;
    }

  return g_object_ref (self);
}

enum
{
  PROP_0,
  PROP_SAVE_SESSION,
  PROP_LAST_OPENED_FOLDER,
  PROP_CLASSBROWSER_HIDDEN,
  PROP_PARSE_ONLY_CURRENT_FILE,
  PROP_SIDE_PANEL_SIZE,
  PROP_FILEBROWSER_LAST_FOLDER
};

static void
preferences_manager_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
  PreferencesManagerDetails *prefdet = PREFERENCES_MANAGER_GET_PRIVATE(object);

	switch (prop_id)
	{
		case PROP_SAVE_SESSION:
			prefdet->save_session = g_value_get_boolean (value);
			break;
		case PROP_PARSE_ONLY_CURRENT_FILE:
			prefdet->parseonlycurrentfile = g_value_get_boolean (value);
			set_bool("/gPHPEdit/classbrowser/onlycurrentfile", prefdet->parseonlycurrentfile);
			break;
		case PROP_SIDE_PANEL_SIZE:
			prefdet->side_panel_size = g_value_get_int (value);
			set_int("/gPHPEdit/main_window/classbrowser_size", prefdet->side_panel_size);
			break;
		case PROP_CLASSBROWSER_HIDDEN:
			prefdet->classbrowser_hidden = g_value_get_boolean (value);
			set_bool("/gPHPEdit/main_window/classbrowser_hidden", prefdet->classbrowser_hidden);
			break;
		case PROP_LAST_OPENED_FOLDER:
			g_free(prefdet->last_opened_folder);
			prefdet->last_opened_folder = g_value_dup_string (value);
			set_string ("/gPHPEdit/general/last_opened_folder", prefdet->last_opened_folder);
			break;
		case PROP_FILEBROWSER_LAST_FOLDER:
			g_free(prefdet->filebrowser_last_folder);
			prefdet->filebrowser_last_folder = g_value_dup_string (value);
			set_string ("/gPHPEdit/main_window/folderbrowser/folder", prefdet->filebrowser_last_folder);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
preferences_manager_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
  PreferencesManagerDetails *prefdet = PREFERENCES_MANAGER_GET_PRIVATE(object);

	switch (prop_id)
	{
		case PROP_SAVE_SESSION:
			g_value_set_boolean (value, prefdet->save_session);
			break;
		case PROP_PARSE_ONLY_CURRENT_FILE:
			g_value_set_boolean (value, prefdet->parseonlycurrentfile);
			break;
		case PROP_SIDE_PANEL_SIZE:
			g_value_set_int (value, prefdet->side_panel_size);
			break;
		case PROP_CLASSBROWSER_HIDDEN:
			g_value_set_boolean (value, prefdet->classbrowser_hidden);
			break;
		case PROP_LAST_OPENED_FOLDER:
			g_value_set_string (value, prefdet->last_opened_folder);
			break;
		case PROP_FILEBROWSER_LAST_FOLDER:
			g_value_set_string (value, prefdet->filebrowser_last_folder);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
preferences_manager_class_init (PreferencesManagerClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = preferences_manager_finalize;
  object_class->dispose = preferences_manager_dispose;
  object_class->constructor = preferences_manager_constructor;
  object_class->set_property = preferences_manager_set_property;
  object_class->get_property = preferences_manager_get_property;

  /* save_session property */
  g_object_class_install_property (object_class,
                              PROP_SAVE_SESSION,
                              g_param_spec_boolean ("save_session", 
                              "Save_Session", "If gPHPEdit save session", 
                              FALSE, G_PARAM_READWRITE));

  /* when classbrowser will parse only the current file property */
  g_object_class_install_property (object_class,
                              PROP_PARSE_ONLY_CURRENT_FILE,
                              g_param_spec_boolean ("parse_only_current_file",
                              NULL, NULL,
                              FALSE, G_PARAM_READWRITE));

 //value is true if side_panel is hidden
 //value is false if side_panel is show
 //FIXME: the value of this property is the hidden status of the side panel
 // it's name is for compatibility reason.
  g_object_class_install_property (object_class,
                              PROP_CLASSBROWSER_HIDDEN,
                              g_param_spec_boolean ("classbrowser_hidden",
                              NULL, NULL,
                              TRUE, G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_SIDE_PANEL_SIZE,
                              g_param_spec_int ("side_panel_size",
                              NULL, NULL, 0, G_MAXINT, 
                              100, G_PARAM_READWRITE));

  /* last folder gPHPEdit open property */
  g_object_class_install_property (object_class,
                              PROP_LAST_OPENED_FOLDER,
                              g_param_spec_string ("last_opened_folder",
                              "Last_Opened_Folder", NULL,
                              "", G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_FILEBROWSER_LAST_FOLDER,
                              g_param_spec_string ("filebrowser_last_folder",
                              NULL, NULL,
                              "", G_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (PreferencesManagerDetails));
}

void clean_default_settings(PreferencesManagerDetails *prefdet){
  /* free object resources*/
  if (prefdet->last_opened_folder) g_free(prefdet->last_opened_folder);
  if (prefdet->filebrowser_last_folder) g_free(prefdet->filebrowser_last_folder);
  if (prefdet->php_binary_location) g_free(prefdet->php_binary_location);
  if (prefdet->shared_source_location) g_free(prefdet->shared_source_location);
  if (prefdet->php_file_extensions) g_free(prefdet->php_file_extensions);
}

static void force_config_folder(void)
{
  GError *error=NULL;
  GFile *config;
  gchar *uri=g_strdup_printf("%s/%s",g_get_home_dir(),".gphpedit");
  if (!filename_file_exist(uri)){
    config=get_gfile_from_filename (uri);
    if (!g_file_make_directory (config, NULL, &error)){
      if (error->code !=G_IO_ERROR_EXISTS){
        g_print(_("Unable to create ~/.gphpedit/ (%d) %s"), error->code, error->message);
        }
        g_error_free(error);
      }
    g_object_unref(config);
  }
  g_free(uri);
}

static void
preferences_manager_init (PreferencesManager *object)
{
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(object);
  /* create config folder if doesn't exist */
#if !GLIB_CHECK_VERSION (2, 24, 0)
  /* init gconf system */
  gconf_init(0, NULL, NULL);
#endif

  force_config_folder();
  load_default_settings(prefdet);
  load_window_settings(prefdet); /* load main window settings*/
  load_session_settings(prefdet);
  load_styles(prefdet); /* load lexer styles */
}

/*
* disposes the Gobject
*/
static void preferences_manager_dispose (GObject *object)
{
  gphpedit_debug(DEBUG_PREFS);
  PreferencesManager *pref = PREFERENCES_MANAGER(object);
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(pref);
  /* free object resources*/
  preferences_manager_save_data(pref);
  /* Chain up to the parent class */
  G_OBJECT_CLASS (preferences_manager_parent_class)->dispose (object);
}

static void
preferences_manager_finalize (GObject *object)
{
  PreferencesManager *pref = PREFERENCES_MANAGER(object);
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(pref);
  clean_default_settings(prefdet);

  g_free(prefdet->font_name);
  g_free(prefdet->style_name);

  G_OBJECT_CLASS (preferences_manager_parent_class)->finalize (object);
}

PreferencesManager *preferences_manager_new (void)
{
  PreferencesManager *pref;
  pref = g_object_new (PREFERENCES_MANAGER_TYPE, NULL);
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(pref);
  return pref; /* return new object */
}

/*
* preferences_manager_parse_font_quality
* reads font quality from gnome gconf key or from GtkSettings 
* and return corresponding scintilla font quality
*/
gint preferences_manager_parse_font_quality(void){
  gchar *antialiasing = get_string("/desktop/gnome/font_rendering/antialiasing","default");
  
  if (g_strcmp0(antialiasing,"none")==0){
      return SC_EFF_QUALITY_NON_ANTIALIASED;
  } else if (g_strcmp0(antialiasing,"grayscale")==0){
    return SC_EFF_QUALITY_ANTIALIASED;
  } else if (g_strcmp0(antialiasing,"rgba")==0){
    return SC_EFF_QUALITY_LCD_OPTIMIZED;
  }
  /* gconf key not found, try GtkSettings value */
  gint x;
  g_object_get(G_OBJECT(gtk_settings_get_default()), "gtk-xft-antialias", &x, NULL);
  if (x == 0) return SC_EFF_QUALITY_NON_ANTIALIASED;
  if (x == 1) return SC_EFF_QUALITY_ANTIALIASED;
  return SC_EFF_QUALITY_DEFAULT;
}

gint get_default_delay(void){
  gint delay;
  g_object_get(G_OBJECT(gtk_settings_get_default()), "gtk-tooltip-timeout", &delay, NULL);
  return delay;
}

#define DEFAULT_PHP_EXTENSIONS "php,inc,phtml,php3,xml,htm,html"

void load_default_settings(PreferencesManagerDetails *prefdet)
{
  gphpedit_debug(DEBUG_PREFS);
  prefdet->marker_back = get_color("/gPHPEdit/default_style/bookmark", "default_style", 15908608);
  prefdet->php_binary_location= get_string("/gPHPEdit/locations/phpbinary", "php");
  prefdet->shared_source_location = get_string("/gPHPEdit/locations/shared_source", "");

  prefdet->indentation_size = get_size("/gPHPEdit/defaults/indentationsize", 4); 
  prefdet->tab_size = get_size("/gPHPEdit/defaults/tabsize", 4); 
  prefdet->auto_complete_delay = get_size("/gPHPEdit/defaults/auto_complete_delay", get_default_delay());
  prefdet->calltip_delay = get_size("/gPHPEdit/defaults/calltip_delay", get_default_delay());

  prefdet->show_indentation_guides = get_bool("/gPHPEdit/defaults/showindentationguides", FALSE);
  prefdet->show_folding = TRUE;
  //gconf_client_get_bool (config,"/gPHPEdit/defaults/showfolding",NULL);
  prefdet->edge_mode = get_bool("/gPHPEdit/defaults/edgemode", FALSE);
  prefdet->edge_column = get_size("/gPHPEdit/defaults/edgecolumn", 80);
  prefdet->line_wrapping = get_bool("/gPHPEdit/defaults/linewrapping", TRUE);
  /* font quality */
  prefdet->font_quality = preferences_manager_parse_font_quality();
  prefdet->auto_complete_braces= get_bool("/gPHPEdit/defaults/autocompletebraces", FALSE);
  prefdet->higthlightcaretline= get_bool("/gPHPEdit/defaults/higthlightcaretline", FALSE);
  prefdet->save_session = get_bool("/gPHPEdit/defaults/save_session", TRUE);
  prefdet->use_tabs_instead_spaces = get_bool("/gPHPEdit/defaults/use_tabs_instead_spaces", FALSE);
  prefdet->single_instance_only = get_bool("/gPHPEdit/defaults/single_instance_only", TRUE);
  prefdet->php_file_extensions = get_string("/gPHPEdit/defaults/php_file_extensions", DEFAULT_PHP_EXTENSIONS);
  prefdet->search_history= get_string_list("/gPHPEdit/search_history");
  prefdet->showfilebrowser = get_bool("/gPHPEdit/defaults/showfolderbrowser", TRUE);
  prefdet->showstatusbar = get_bool("/gPHPEdit/defaults/showstatusbar", TRUE);
  prefdet->showmaintoolbar = get_bool("/gPHPEdit/defaults/showmaintoolbar", TRUE);
  prefdet->showfindtoolbar = get_bool("/gPHPEdit/defaults/showfindtoolbar", TRUE);
}

void load_window_settings(PreferencesManagerDetails *prefdet)
{
  gphpedit_debug(DEBUG_PREFS);
  prefdet->left = get_color("/gPHPEdit/main_window/x","main_window",20);
  prefdet->top = get_color("/gPHPEdit/main_window/y","main_window",20);
  prefdet->width = get_color("/gPHPEdit/main_window/width","main_window",400);
  prefdet->height = get_color("/gPHPEdit/main_window/height","main_window",400);
  prefdet->maximized = get_size("/gPHPEdit/main_window/maximized",0);
}

void load_session_settings(PreferencesManagerDetails *prefdet){
  gphpedit_debug(DEBUG_PREFS);
  prefdet->last_opened_folder = get_string("/gPHPEdit/general/last_opened_folder", (gchar *)g_get_home_dir());
  prefdet->parseonlycurrentfile = get_bool("/gPHPEdit/classbrowser/onlycurrentfile", FALSE);
  prefdet->classbrowser_hidden = get_bool("/gPHPEdit/main_window/classbrowser_hidden", FALSE);
  prefdet->side_panel_size = get_color("/gPHPEdit/main_window/classbrowser_size", "main_window", 100);
  
  prefdet->filebrowser_last_folder=get_string("/gPHPEdit/main_window/folderbrowser/folder", (gchar *)g_get_home_dir());
}

        /* accesor functions */
  /*
  * each propertly has accesor functions to get and set his corresponding value
  * set functions don't store new value in gconf directly, instead store new value internally. 
  * you must call "preferences_manager_save_data" in order to update values in gconf.
  */

GSList *get_preferences_manager_search_history(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->search_history;
}

gboolean get_preferences_manager_show_filebrowser(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->showfilebrowser;
}

void set_preferences_manager_show_filebrowser(PreferencesManager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->showfilebrowser = newstate; 

}

gboolean get_preferences_manager_show_statusbar(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->showstatusbar;
}

void set_preferences_manager_show_statusbar(PreferencesManager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->showstatusbar = newstate; 
}

gboolean get_preferences_manager_show_findtoolbar(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->showfindtoolbar;
}

void set_preferences_manager_show_findtoolbar(PreferencesManager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->showfindtoolbar = newstate; 
}

gboolean get_preferences_manager_show_maintoolbar(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->showmaintoolbar;
}

void set_preferences_manager_show_maintoolbar(PreferencesManager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->showmaintoolbar = newstate; 
}

void get_preferences_manager_window_size (PreferencesManager *preferences_manager, gint *width, gint *height)
{
  g_return_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager));
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  if (width) *width = prefdet->width;
  if (height) *height = prefdet->height;
}

void set_preferences_manager_window_size (PreferencesManager *preferences_manager, gint width, gint height)
{
  g_return_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager));
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->width = width;
  prefdet->height = height;
}

void get_preferences_manager_window_position (PreferencesManager *preferences_manager, gint *top, gint *left)
{
  g_return_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager));
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  if (top) *top = prefdet->top;
  if (left) *left = prefdet->left;
}

void set_preferences_manager_window_position (PreferencesManager *preferences_manager, gint top, gint left)
{
  g_return_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager));
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->top = top;
  prefdet->left = left;
}

gboolean get_preferences_manager_window_maximized(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->maximized;
}

void set_preferences_manager_window_maximized(PreferencesManager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->maximized = newstate; 
}

gint get_preferences_manager_indentation_size(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->indentation_size;
}

void set_preferences_manager_indentation_size(PreferencesManager *preferences_manager, gint newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->indentation_size = newstate; 

}

const gchar *get_preferences_manager_php_binary_location(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->php_binary_location;
}

void set_preferences_manager_php_binary_location(PreferencesManager *preferences_manager, gchar *newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->php_binary_location = newstate; 

}

gint get_preferences_manager_font_quality(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->font_quality;
}

gboolean get_preferences_manager_line_wrapping(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->line_wrapping;
}

void set_preferences_manager_line_wrapping(PreferencesManager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->line_wrapping = newstate; 

}

gint get_preferences_manager_calltip_delay(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->calltip_delay;
}

void set_preferences_manager_calltip_delay(PreferencesManager *preferences_manager, gint newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->calltip_delay = newstate; 

}

gint get_preferences_manager_auto_complete_delay(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->auto_complete_delay;
}

void set_preferences_manager_auto_complete_delay(PreferencesManager *preferences_manager, gint newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->auto_complete_delay = newstate; 

}

gboolean get_preferences_manager_use_tabs_instead_spaces(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->use_tabs_instead_spaces;
}

void set_preferences_manager_use_tabs_instead_spaces(PreferencesManager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->use_tabs_instead_spaces = newstate; 

}


gboolean get_preferences_manager_auto_complete_braces(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->auto_complete_braces;
}

void set_preferences_manager_auto_complete_braces(PreferencesManager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->auto_complete_braces = newstate; 

}

gboolean get_preferences_manager_show_folding(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->show_folding;
}

gboolean get_preferences_manager_single_instance_only(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->single_instance_only;
}

void set_preferences_manager_single_instance_only(PreferencesManager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->single_instance_only = newstate; 

}


gboolean get_preferences_manager_higthlight_caret_line(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->higthlightcaretline;
}

void set_preferences_manager_higthlight_caret_line(PreferencesManager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->higthlightcaretline = newstate; 

}

gboolean get_preferences_manager_show_indentation_guides(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->show_indentation_guides;
}

void set_preferences_manager_show_indentation_guides(PreferencesManager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->show_indentation_guides = newstate; 

}

gboolean get_preferences_manager_edge_mode(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->edge_mode;
}

void set_preferences_manager_edge_mode(PreferencesManager *preferences_manager, gboolean newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->edge_mode = newstate; 

}

gint get_preferences_manager_edge_column(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->edge_column;
}

void set_preferences_manager_edge_column(PreferencesManager *preferences_manager, gint newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->edge_column = newstate; 

}

gint get_preferences_manager_tab_size(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->tab_size;
}

void set_preferences_manager_tab_size(PreferencesManager *preferences_manager, gint newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->tab_size = newstate; 

}

gchar *get_preferences_manager_php_file_extensions(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->php_file_extensions;
}

void set_preferences_manager_php_file_extensions(PreferencesManager *preferences_manager, gchar *newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  if (!newstate) return;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->php_file_extensions = newstate; 

}

gchar *get_preferences_manager_shared_source_location(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->shared_source_location;
}
void set_preferences_manager_shared_source_location(PreferencesManager *preferences_manager, gchar *newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->shared_source_location = newstate; 
}

GSList *get_preferences_manager_php_search_history(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->search_history;
}

void set_preferences_manager_new_search_history_item(PreferencesManager *preferences_manager, gint pos, const gchar *new_text)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->search_history = g_slist_prepend (prefdet->search_history, g_strdup(new_text));

    if (pos == MAXHISTORY){
    /* delete last item */
    GSList *temp= g_slist_nth (prefdet->search_history, MAXHISTORY);
    prefdet->search_history = g_slist_remove (prefdet->search_history, temp->data);
    }
}

gchar *get_preferences_manager_style_name(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  if (!prefdet->style_name) return "classic";
  return prefdet->style_name;
}

void set_preferences_manager_style_name(PreferencesManager *preferences_manager, gchar *newstate)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  if (!newstate) return;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  prefdet->style_name = g_strdup(newstate);
}

guint get_preferences_manager_style_size(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->font_size;
}

gchar *get_preferences_manager_style_font(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->font_name;
}
/*
* preferences_manager_save_data
* update session preferences data in gconf with new internal data
* this version only save a few preferences that can change often in the program.
* other preferences are only save when you call "preferences_manager_save_data_full"
* so we speed up process by not save unchanged data.
*/
void preferences_manager_save_data(PreferencesManager *preferences_manager){
  gphpedit_debug(DEBUG_PREFS);
  PreferencesManagerDetails *prefdet;
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
  set_int("/gPHPEdit/defaults/showstatusbar", prefdet->showstatusbar);
  set_int("/gPHPEdit/defaults/showmaintoolbar", prefdet->showmaintoolbar);
  set_int("/gPHPEdit/defaults/showfindtoolbar", prefdet->showfindtoolbar);
  set_string_list ("/gPHPEdit/search_history", prefdet->search_history);
}

/*
* preferences_manager_save_data_full
* update all preferences data in gconf with new internal data
*/
void preferences_manager_save_data_full(PreferencesManager *preferences_manager){
  gphpedit_debug(DEBUG_PREFS);
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  preferences_manager_save_data(preferences_manager);  /* save session data */

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
  set_bool ("/gPHPEdit/defaults/linewrapping", prefdet->line_wrapping);
  /* font quality */
  set_int ("/gPHPEdit/defaults/fontquality", prefdet->font_quality);
  set_bool("/gPHPEdit/defaults/autocompletebraces", prefdet->auto_complete_braces);
  set_bool ("/gPHPEdit/defaults/higthlightcaretline", prefdet->higthlightcaretline);
  set_bool ("/gPHPEdit/defaults/save_session", prefdet->save_session);
  set_bool ("/gPHPEdit/defaults/use_tabs_instead_spaces", prefdet->use_tabs_instead_spaces);
  set_bool ("/gPHPEdit/defaults/single_instance_only", prefdet->single_instance_only);
  set_string ("/gPHPEdit/defaults/php_file_extensions", prefdet->php_file_extensions);
  /* style settings */
  gchar *font_setting = g_strdup_printf("%s %d",prefdet->font_name + 1, prefdet->font_size);
  set_string ("/gPHPEdit/defaults/font", font_setting);
  g_free(font_setting);
  set_string ("/gPHPEdit/defaults/style", prefdet->style_name);
}

/*
* preferences_manager_restore_data
* reload preferences data from gconf and discard internal data.
*/
void preferences_manager_restore_data(PreferencesManager *preferences_manager){
  gphpedit_debug(DEBUG_PREFS);
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  clean_default_settings(prefdet);
  prefdet->last_opened_folder = get_string("/gPHPEdit/general/last_opened_folder", (gchar *)g_get_home_dir());
  prefdet->filebrowser_last_folder=get_string("/gPHPEdit/main_window/folderbrowser/folder", (gchar *)g_get_home_dir());

  load_default_settings(prefdet);
  load_styles(prefdet); /* load lexer styles */
}

/* plugin preferences functions */

gboolean get_plugin_is_active(PreferencesManager *preferences_manager, const gchar *name)
{
  gphpedit_debug(DEBUG_PREFS);
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  
  gchar *key = g_strdup_printf("/gPHPEdit/plugins/%s/active", name);
  gchar *subdir = g_strdup_printf("plugins/%s", name);
  gboolean result = get_color(key, subdir, TRUE);
  g_free(subdir);
  g_free(key);
  return result;
}

void set_plugin_is_active(PreferencesManager *preferences_manager, const gchar *name, gboolean status)
{
  if(!preferences_manager || !name) return ;
  gphpedit_debug(DEBUG_PREFS);
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  
  gchar *key = g_strdup_printf("/gPHPEdit/plugins/%s/active", name);
  set_int (key, status);
  g_free(key);
}

/**/
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
    return g_strdup(default_string);
  }
  return temp;
}
/*
* get_size (internal)
* load a size value from gconf
* if value isn't found return default_size
*/
static gint get_size(const gchar *key, gint default_size){
  GConfClient *config = gconf_client_get_default ();
  GError *error=NULL;
  gint temp= gconf_client_get_int (config,key,&error);
  g_object_unref (G_OBJECT (config));
  if (temp==0){
    if (error){
     gphpedit_debug_message(DEBUG_PREFS, "%s", error->message);
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
    gphpedit_debug_message(DEBUG_PREFS, "key %s don't exist. load default value",key);
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
* get_default_font_settings
* read default font and default font size from GtkSettings
*/
gchar *get_default_font_settings(PreferencesManagerDetails *prefdet){
  gphpedit_debug(DEBUG_PREFS);
  gchar *string=NULL;
  g_object_get(G_OBJECT(gtk_settings_get_default()), "gtk-font-name", &string, NULL);
  return string;
}

void load_font_settings(PreferencesManagerDetails *prefdet)
{
  gphpedit_debug(DEBUG_PREFS);
  gchar *font_desc= get_string("/gPHPEdit/defaults/font" , get_default_font_settings(prefdet));
  PangoFontDescription *desc = pango_font_description_from_string (font_desc);
  prefdet->font_size = PANGO_PIXELS(pango_font_description_get_size (desc));
  /* add ! needed by scintilla for pango render */
  prefdet->font_name = g_strdup_printf("!%s",pango_font_description_get_family (desc));
  pango_font_description_free (desc);
}

/**
* set_font_settings
* set a new font name and font size.
* font_desc must be a pango font description string like "Helvetica 12"
* this function don't save new values to gconf. 
* you must call "preferences_manager_save_data_full" to update gconf values.
*/
void set_font_settings (PreferencesManager *preferences_manager, gchar *font_desc)
{
  gphpedit_debug(DEBUG_PREFS);
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  if (!font_desc) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);

  PangoFontDescription *desc = pango_font_description_from_string (font_desc);
  prefdet->font_size = PANGO_PIXELS(pango_font_description_get_size (desc));
  /* add ! needed by scintilla for pango render */
  if (prefdet->font_name) g_free(prefdet->font_name);
  prefdet->font_name = g_strdup_printf("!%s",pango_font_description_get_family (desc));
  pango_font_description_free (desc);
}

/**
* set_style_name
* set a new style name.
* this function don't save new values to gconf. 
* you must call "preferences_manager_save_data_full" to update gconf values.
*/

void set_style_name (PreferencesManager *preferences_manager, gchar *newstyle)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  if (!newstyle) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);

  if (newstyle) g_free(prefdet->style_name);
  prefdet->style_name = g_strdup(newstyle);
}
/*
* load_styles (internal)
* loads lexer styles
*/
void load_styles(PreferencesManagerDetails *prefdet)
{ 
  gphpedit_debug(DEBUG_PREFS);
  load_font_settings(prefdet);
  prefdet->style_name = get_string("/gPHPEdit/defaults/style", "mixer");
}
