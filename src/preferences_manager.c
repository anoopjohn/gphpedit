/* This file is part of gPHPEdit, a GNOME PHP Editor.

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
/*
 * PREFERENCES_MANAGER SYSTEM:
 */
/*
* manage all aplication preferences in a clean way, so aplication don't need to know how it's settings are stored.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>

#include "debug.h"
#include "preferences_manager.h"
#include "document_scintilla.h"

#include "gphpedit-session.h"

#define MAXHISTORY 16

/*
* preferences_manager private struct
*/
struct PreferencesManagerDetails
{
  /* session object */
  GphpeditSession *session;
  /*  */
  GSettings *gs;
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
  gboolean side_panel_hidden;
  gint side_panel_size;
  gchar *filebrowser_last_folder;
  gboolean showfilebrowser;
  guint showstatusbar:1;
  gboolean showmaintoolbar;
  // Default settings
  gint indentation_size;
  gint tab_size;
  gboolean show_indentation_guides;
  gboolean edge_mode;
  gint edge_column;
  gchar *php_binary_location;
  gchar *shared_source_location;
  gchar *php_file_extensions;

  gboolean auto_complete_braces;
  gboolean higthlightcaretline;
  //gint auto_indent_after_brace;
  guint auto_complete_delay;
  guint calltip_delay;
  gboolean line_wrapping;
  gboolean use_tabs_instead_spaces;
  gboolean single_instance_only;
  gint font_quality;  
  GSList *search_history; /* for incremental search history */

  gchar *style_name;
  gint font_size;
  gchar *font_name;

  GSList *session_files;
};

#define PREFERENCES_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
              PREFERENCES_MANAGER_TYPE,\
              PreferencesManagerDetails))

static void preferences_manager_finalize  (GObject  *object);
static void  preferences_manager_class_init (PreferencesManagerClass *klass);
static void preferences_manager_dispose (GObject *gobject);
void load_default_settings(PreferencesManagerDetails *prefdet);
void load_styles(PreferencesManagerDetails *prefdet);
void load_session_settings(PreferencesManagerDetails *prefdet);

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
  PROP_SIDE_PANEL_HIDDEN,
  PROP_PARSE_ONLY_CURRENT_FILE,
  PROP_SIDE_PANEL_SIZE,
  PROP_FILEBROWSER_LAST_FOLDER,
  PROP_EDGE_MODE,
  PROP_EDGE_COLUMN,
  PROP_SHOW_FOLDING,
  PROP_SHOW_INDENTATION_GUIDES,
  PROP_AUTO_COMPLETE_BRACES,
  PROP_HIGTHLIGHT_CARET_LINE,
  PROP_LINE_WRAPPING,
  PROP_TABS_INSTEAD_SPACES,
  PROP_SINGLE_INSTANCE_ONLY,
  PROP_SHARED_SOURCE_LOCATION,
  PROP_FONT_QUALITY,
  PROP_PHP_BINARY_LOCATION,
  PROP_STYLE_NAME,
  PROP_INDENTATION_SIZE,
  PROP_PHP_FILE_EXTENSIONS,
  PROP_FONT_NAME,
  PROP_FONT_SIZE,
  PROP_TAB_SIZE,
  PROP_CALLTIP_DELAY,
  PROP_AUTOCOMPLETE_DELAY,
  PROP_SHOW_TOOLBAR
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
			break;
		case PROP_SHOW_TOOLBAR:
			prefdet->showmaintoolbar = g_value_get_boolean (value);
			break;
		case PROP_SIDE_PANEL_SIZE:
			prefdet->side_panel_size = g_value_get_int (value);
			break;
		case PROP_EDGE_MODE:
			prefdet->edge_mode = g_value_get_boolean (value);
			break;
		case PROP_EDGE_COLUMN:
			prefdet->edge_column = g_value_get_int (value);
			break;
		case PROP_INDENTATION_SIZE:
			prefdet->indentation_size = g_value_get_int (value);
			break;
		case PROP_TAB_SIZE:
			prefdet->tab_size = g_value_get_int (value);
			break;
		case PROP_FONT_SIZE:
			prefdet->font_size = g_value_get_uint (value);
			break;
		case PROP_CALLTIP_DELAY:
			prefdet->calltip_delay = g_value_get_uint (value);
			break;
		case PROP_AUTOCOMPLETE_DELAY:
			prefdet->auto_complete_delay = g_value_get_uint (value);
			break;
		case PROP_FONT_QUALITY:
			prefdet->font_quality = g_value_get_int (value);
			break;
		case PROP_SINGLE_INSTANCE_ONLY:
			prefdet->single_instance_only = g_value_get_boolean (value);
			break;
		case PROP_SHOW_INDENTATION_GUIDES:
			prefdet->show_indentation_guides = g_value_get_boolean (value);
			break;
		case PROP_AUTO_COMPLETE_BRACES:
			prefdet->auto_complete_braces = g_value_get_boolean (value);
			break;
		case PROP_HIGTHLIGHT_CARET_LINE:
			prefdet->higthlightcaretline = g_value_get_boolean (value);
			break;
		case PROP_LINE_WRAPPING:
			prefdet->line_wrapping = g_value_get_boolean (value);
			break;
		case PROP_TABS_INSTEAD_SPACES:
			prefdet->use_tabs_instead_spaces = g_value_get_boolean (value);
			break;
		case PROP_SIDE_PANEL_HIDDEN:
			prefdet->side_panel_hidden = g_value_get_boolean (value);
			break;
		case PROP_LAST_OPENED_FOLDER:
			if (prefdet->last_opened_folder) g_free(prefdet->last_opened_folder);
			prefdet->last_opened_folder = g_value_dup_string (value);
			break;
		case PROP_SHARED_SOURCE_LOCATION:
			if (prefdet->shared_source_location) g_free(prefdet->shared_source_location);
			prefdet->shared_source_location = g_value_dup_string (value);
			break;
		case PROP_PHP_BINARY_LOCATION:
			if (prefdet->php_binary_location) g_free(prefdet->php_binary_location);
			prefdet->php_binary_location = g_value_dup_string (value);
			break;
		case PROP_STYLE_NAME:
			if (prefdet->style_name) g_free(prefdet->style_name);
			prefdet->style_name = g_value_dup_string (value);
			break;
		case PROP_PHP_FILE_EXTENSIONS:
			if (prefdet->php_file_extensions) g_free(prefdet->php_file_extensions);
			prefdet->php_file_extensions = g_value_dup_string (value);
			break;
		case PROP_FONT_NAME:
			if (prefdet->font_name) g_free(prefdet->font_name);
			prefdet->font_name = g_value_dup_string (value);
			break;
		case PROP_FILEBROWSER_LAST_FOLDER:
			if (prefdet->filebrowser_last_folder) g_free(prefdet->filebrowser_last_folder);
			prefdet->filebrowser_last_folder = g_value_dup_string (value);
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
		case PROP_SHOW_TOOLBAR:
			g_value_set_boolean (value, prefdet->showmaintoolbar);
			break;
		case PROP_SIDE_PANEL_SIZE:
			g_value_set_int (value, prefdet->side_panel_size);
			break;
		case PROP_EDGE_MODE:
			g_value_set_boolean (value, prefdet->edge_mode);
			break;
		case PROP_EDGE_COLUMN:
			g_value_set_int (value, prefdet->edge_column);
			break;
		case PROP_FONT_QUALITY:
			g_value_set_int (value, prefdet->font_quality);
			break;
		case PROP_INDENTATION_SIZE:
			g_value_set_int (value, prefdet->indentation_size);
			break;
		case PROP_TAB_SIZE:
			g_value_set_int (value, prefdet->tab_size);
			break;
		case PROP_FONT_SIZE:
			g_value_set_uint (value, prefdet->font_size);
			break;
		case PROP_CALLTIP_DELAY:
			g_value_set_uint (value, prefdet->calltip_delay);
			break;
		case PROP_AUTOCOMPLETE_DELAY:
			g_value_set_uint (value, prefdet->auto_complete_delay);
			break;
		case PROP_SINGLE_INSTANCE_ONLY:
			g_value_set_boolean (value, prefdet->single_instance_only);
			break;
		case PROP_SHOW_INDENTATION_GUIDES:
			g_value_set_boolean (value, prefdet->show_indentation_guides);
			break;
		case PROP_AUTO_COMPLETE_BRACES:
			g_value_set_boolean (value, prefdet->auto_complete_braces);
			break;
		case PROP_HIGTHLIGHT_CARET_LINE:
			g_value_set_boolean (value, prefdet->higthlightcaretline);
			break;
		case PROP_LINE_WRAPPING:
			g_value_set_boolean (value, prefdet->line_wrapping);
			break;
		case PROP_TABS_INSTEAD_SPACES:
			g_value_set_boolean (value, prefdet->use_tabs_instead_spaces);
			break;
		case PROP_SHOW_FOLDING:
			g_value_set_boolean (value, TRUE);
			break;
		case PROP_SIDE_PANEL_HIDDEN:
			g_value_set_boolean (value, prefdet->side_panel_hidden);
			break;
		case PROP_LAST_OPENED_FOLDER:
			g_value_set_string (value, prefdet->last_opened_folder);
			break;
		case PROP_SHARED_SOURCE_LOCATION:
			g_value_set_string (value, prefdet->shared_source_location);
			break;
		case PROP_PHP_BINARY_LOCATION:
			g_value_set_string (value, prefdet->php_binary_location);
			break;
		case PROP_STYLE_NAME:
			g_value_set_string (value, prefdet->style_name);
			break;
		case PROP_PHP_FILE_EXTENSIONS:
			g_value_set_string (value, prefdet->php_file_extensions);
			break;
		case PROP_FONT_NAME:
			g_value_set_string (value, prefdet->font_name);
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

  g_object_class_install_property (object_class,
                              PROP_SHOW_TOOLBAR,
                              g_param_spec_boolean ("show_toolbar",
                              NULL, NULL,
                              TRUE, G_PARAM_READWRITE));

  /* return TRUE if side panel isn't visible */  
  g_object_class_install_property (object_class,
                              PROP_SIDE_PANEL_HIDDEN,
                              g_param_spec_boolean ("side_panel_hidden",
                              NULL, NULL,
                              TRUE, G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_SIDE_PANEL_SIZE,
                              g_param_spec_int ("side_panel_size",
                              NULL, NULL, 0, G_MAXINT, 
                              100, G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_EDGE_MODE,
                              g_param_spec_boolean ("edge_mode",
                              NULL, NULL,
                              FALSE, G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_EDGE_COLUMN,
                              g_param_spec_int ("edge_column",
                              NULL, NULL, 0, G_MAXINT, 
                              80, G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_INDENTATION_SIZE,
                              g_param_spec_int ("indentation_size",
                              NULL, NULL, 0, G_MAXINT, 
                              0, G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_TAB_SIZE,
                              g_param_spec_int ("tab_size",
                              NULL, NULL, 0, G_MAXINT, 
                              0, G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_FONT_SIZE,
                              g_param_spec_uint ("font_size",
                              NULL, NULL, 0, G_MAXUINT, 
                              0, G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_CALLTIP_DELAY,
                              g_param_spec_uint ("calltip_delay",
                              NULL, NULL, 0, G_MAXINT, 
                              0, G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_AUTOCOMPLETE_DELAY,
                              g_param_spec_uint ("autocomplete_delay",
                              NULL, NULL, 0, G_MAXINT, 
                              0, G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_FONT_QUALITY,
                              g_param_spec_int ("font_quality",
                              NULL, NULL, 0, G_MAXINT, 
                              0, G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_SINGLE_INSTANCE_ONLY,
                              g_param_spec_boolean ("single_instance_only",
                              NULL, NULL,
                              TRUE, G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_SHOW_FOLDING,
                              g_param_spec_boolean ("show_folding",
                              NULL, NULL,
                              TRUE, G_PARAM_READABLE));

  g_object_class_install_property (object_class,
                              PROP_SHOW_INDENTATION_GUIDES,
                              g_param_spec_boolean ("show_indentation_guides",
                              NULL, NULL,
                              FALSE, G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_AUTO_COMPLETE_BRACES,
                              g_param_spec_boolean ("auto_complete_braces",
                              NULL, NULL,
                              FALSE, G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_HIGTHLIGHT_CARET_LINE,
                              g_param_spec_boolean ("higthlight_caret_line",
                              NULL, NULL,
                              FALSE, G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_LINE_WRAPPING,
                              g_param_spec_boolean ("line_wrapping",
                              NULL, NULL,
                              FALSE, G_PARAM_READWRITE));
  g_object_class_install_property (object_class,
                              PROP_TABS_INSTEAD_SPACES,
                              g_param_spec_boolean ("tabs_instead_spaces",
                              NULL, NULL,
                              FALSE, G_PARAM_READWRITE));
  /* last folder gPHPEdit open property */
  g_object_class_install_property (object_class,
                              PROP_LAST_OPENED_FOLDER,
                              g_param_spec_string ("last_opened_folder",
                              "Last_Opened_Folder", NULL,
                              "", G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_SHARED_SOURCE_LOCATION,
                              g_param_spec_string ("shared_source_location",
                              NULL, NULL,
                              "", G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_PHP_BINARY_LOCATION,
                              g_param_spec_string ("php_binary_location",
                              NULL, NULL,
                              "", G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_STYLE_NAME,
                              g_param_spec_string ("style_name",
                              NULL, NULL,
                              "", G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_PHP_FILE_EXTENSIONS,
                              g_param_spec_string ("php_file_extensions",
                              NULL, NULL,
                              "", G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_FONT_NAME,
                              g_param_spec_string ("style_font_name",
                              NULL, NULL,
                              "", G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_FILEBROWSER_LAST_FOLDER,
                              g_param_spec_string ("filebrowser_last_folder",
                              NULL, NULL,
                              "", G_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (PreferencesManagerDetails));
}

void clean_default_settings(PreferencesManagerDetails *prefdet)
{
  /* free object resources*/
//FIXME: get double free error in these lines
//  if (prefdet->last_opened_folder) g_free(prefdet->last_opened_folder);
//  if (prefdet->filebrowser_last_folder) g_free(prefdet->filebrowser_last_folder);
  if (prefdet->php_binary_location) g_free(prefdet->php_binary_location);
  if (prefdet->shared_source_location) g_free(prefdet->shared_source_location);
  if (prefdet->php_file_extensions) g_free(prefdet->php_file_extensions);
}

static void force_config_folder(void)
{
  GError *error=NULL;
  GFile *config;
  gchar *uri = g_build_filename (g_get_user_config_dir (), "gphpedit", NULL);
  config = g_file_new_for_commandline_arg (uri);
  if (!g_file_query_exists(config, NULL)){
    if (!g_file_make_directory (config, NULL, &error)){
      if (error->code !=G_IO_ERROR_EXISTS){
        g_print(_("Unable to create %s (%d) %s"), uri, error->code, error->message);
        }
        g_error_free(error);
        g_object_unref(config);
      }
  }
  g_object_unref(config);
  g_free(uri);
}

static void
preferences_manager_init (PreferencesManager *object)
{
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(object);
  /* create config folder if doesn't exist */
  force_config_folder();
  
  /* init session object */
  gchar *uri = g_build_filename (g_get_user_config_dir (), "gphpedit", NULL);
  prefdet->session = gphpedit_session_new (uri);
  g_free(uri);

  load_default_settings(prefdet);
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
  g_object_unref(prefdet->session);
  if (prefdet->session_files){
   g_slist_foreach(prefdet->session_files,  (GFunc) g_free, NULL);
   g_slist_free(prefdet->session_files);
  }

  G_OBJECT_CLASS (preferences_manager_parent_class)->finalize (object);
}

PreferencesManager *preferences_manager_new (void)
{
  return g_object_new (PREFERENCES_MANAGER_TYPE, NULL); /* return new object */
}

/*
* preferences_manager_parse_font_quality
* reads font quality from gnome or from GtkSettings
* and return corresponding scintilla font quality
*/
gint preferences_manager_parse_font_quality(void) {
#if 0
  //FIXME: GSettings key for this??
  gchar *antialiasing = get_string("/desktop/gnome/font_rendering/antialiasing", "default");
  
  if (g_strcmp0(antialiasing,"none")==0){
    g_free(antialiasing);
    return SC_EFF_QUALITY_NON_ANTIALIASED;
  } else if (g_strcmp0(antialiasing,"grayscale")==0){
    g_free(antialiasing);
    return SC_EFF_QUALITY_ANTIALIASED;
  } else if (g_strcmp0(antialiasing,"rgba")==0){
    g_free(antialiasing);
    return SC_EFF_QUALITY_LCD_OPTIMIZED;
  }
#endif
  /* gconf key not found, try GtkSettings value */
  gint x;
  g_object_get(G_OBJECT(gtk_settings_get_default()), "gtk-xft-antialias", &x, NULL);
  if (x == 0) return SC_EFF_QUALITY_NON_ANTIALIASED;
  if (x == 1) return SC_EFF_QUALITY_ANTIALIASED;
  return SC_EFF_QUALITY_DEFAULT;
}

void load_default_settings(PreferencesManagerDetails *prefdet)
{
  gphpedit_debug(DEBUG_PREFS);
  prefdet->gs = g_settings_new ("org.gphpedit.preferences.editor");

  prefdet->php_binary_location= g_settings_get_string (prefdet->gs, "php-binary-location");
  prefdet->shared_source_location = g_settings_get_string (prefdet->gs, "shared-source-location");
  prefdet->php_file_extensions = g_settings_get_string (prefdet->gs, "php-file-extensions");
  g_settings_get (prefdet->gs, "indentation-size", "u", &prefdet->indentation_size);
  g_settings_get (prefdet->gs, "tabs-size", "u", &prefdet->tab_size);
  g_settings_get (prefdet->gs, "auto-complete-delay", "u", &prefdet->auto_complete_delay);
  g_settings_get (prefdet->gs, "calltip-delay", "u", &prefdet->calltip_delay);

  prefdet->show_indentation_guides = g_settings_get_boolean (prefdet->gs, "show-indentation-guides");
  prefdet->edge_mode = g_settings_get_boolean (prefdet->gs, "display-right-margin");
  g_settings_get (prefdet->gs, "right-margin-position", "u", &prefdet->edge_column);

  prefdet->line_wrapping = g_settings_get_boolean (prefdet->gs, "line-wrapping");
  /* font quality */
  prefdet->font_quality = preferences_manager_parse_font_quality();
  prefdet->auto_complete_braces = g_settings_get_boolean (prefdet->gs, "auto-complete-braces");
  prefdet->higthlightcaretline= g_settings_get_boolean (prefdet->gs, "highlight-current-line");
  prefdet->save_session = g_settings_get_boolean (prefdet->gs, "save-session");
  prefdet->use_tabs_instead_spaces = g_settings_get_boolean (prefdet->gs, "use-tabs-instead-spaces");
  prefdet->single_instance_only = g_settings_get_boolean (prefdet->gs, "single-instance-only");
  prefdet->showfilebrowser = g_settings_get_boolean (prefdet->gs, "showfilebrowser");
}

void load_session_settings(PreferencesManagerDetails *prefdet)
{
  gphpedit_debug(DEBUG_PREFS);

  /* main window settings */
  prefdet->left = gphpedit_session_get_int_with_default (prefdet->session, "main_window", "left", 20);
  prefdet->top = gphpedit_session_get_int_with_default (prefdet->session, "main_window", "top", 20);
  prefdet->width = gphpedit_session_get_int_with_default (prefdet->session, "main_window", "width", 400);
  prefdet->height = gphpedit_session_get_int_with_default (prefdet->session, "main_window", "height", 400);
  prefdet->maximized = gphpedit_session_get_boolean (prefdet->session, "main_window", "maximized");

  prefdet->showmaintoolbar = gphpedit_session_get_boolean_with_default (prefdet->session, "main_window", "showmaintoolbar", TRUE);
  prefdet->showstatusbar = gphpedit_session_get_boolean_with_default (prefdet->session, "main_window", "showstatusbar", TRUE);
  prefdet->search_history = gphpedit_session_get_string_list (prefdet->session, "search_history", "keys");
  prefdet->last_opened_folder = gphpedit_session_get_string_with_default (prefdet->session, "main_window", "last_opened_folder", (gchar *)g_get_home_dir());
  /* side panel settings */
  prefdet->side_panel_hidden = gphpedit_session_get_boolean_with_default (prefdet->session, "side_panel", "hidden", FALSE);
  prefdet->side_panel_size = gphpedit_session_get_int_with_default (prefdet->session, "side_panel", "size", 100);
  prefdet->parseonlycurrentfile = gphpedit_session_get_boolean (prefdet->session, "side_panel", "check_only_current_file");
  prefdet->filebrowser_last_folder = gphpedit_session_get_string_with_default (prefdet->session, "side_panel", "folder", (gchar *)g_get_home_dir());
  prefdet->session_files = gphpedit_session_get_string_list (prefdet->session, "session","files");

}

        /* accesor functions */
  /*
  * each propertly has accesor functions to get and set his corresponding value
  * set functions don't store new value in directly, instead store new value internally.
  * you must call "preferences_manager_save_data" in order to update values in GSettings.
  */

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

GSList *get_preferences_manager_search_history(PreferencesManager *preferences_manager)
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

GSList *get_preferences_manager_session_files(PreferencesManager *preferences_manager)
{
  g_return_val_if_fail (OBJECT_IS_PREFERENCES_MANAGER (preferences_manager), 0); /**/
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  return prefdet->session_files;
}

void set_preferences_manager_session_files(PreferencesManager *preferences_manager, GSList *files)
{
  if (!OBJECT_IS_PREFERENCES_MANAGER (preferences_manager)) return ;
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  if (prefdet->session_files){
   g_slist_foreach(prefdet->session_files,  (GFunc) g_free, NULL);
   g_slist_free(prefdet->session_files);
  }
  prefdet->session_files = g_slist_copy (files);
}

/*
* preferences_manager_save_data
* update session preferences data with new internal data
* this version only save a few preferences that can change often in the program.
* other preferences are only save when you call "preferences_manager_save_data_full"
* so we speed up process by not save unchanged data.
*/
void preferences_manager_save_data(PreferencesManager *preferences_manager)
{
  gphpedit_debug(DEBUG_PREFS);
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  /*store window settings*/
  if (!prefdet->maximized) {
    gphpedit_session_set_int (prefdet->session, "main_window", "left", prefdet->left);
    gphpedit_session_set_int (prefdet->session, "main_window", "top", prefdet->top);
    gphpedit_session_set_int (prefdet->session, "main_window", "width", prefdet->width);
    gphpedit_session_set_int (prefdet->session, "main_window", "height", prefdet->height);
  }
  gphpedit_session_set_boolean (prefdet->session, "main_window", "maximized", prefdet->maximized);
  gphpedit_session_set_boolean (prefdet->session, "main_window", "showmaintoolbar", prefdet->showmaintoolbar);
  gphpedit_session_set_boolean (prefdet->session, "main_window", "showstatusbar", prefdet->showstatusbar);
  gphpedit_session_set_string (prefdet->session, "main_window", "last_opened_folder", prefdet->last_opened_folder);

  /* side panel settings */
  gphpedit_session_set_boolean (prefdet->session, "side_panel", "hidden", prefdet->side_panel_hidden);
  gphpedit_session_set_int (prefdet->session, "side_panel", "size", prefdet->side_panel_size);
  gphpedit_session_set_boolean (prefdet->session, "side_panel", "check_only_current_file", prefdet->parseonlycurrentfile);
  gphpedit_session_set_string (prefdet->session, "side_panel", "folder", prefdet->filebrowser_last_folder);

  gphpedit_session_set_string_list (prefdet->session, "search_history", "keys", prefdet->search_history);

  gphpedit_session_set_string_list (prefdet->session, "session","files", prefdet->session_files);

  gphpedit_session_sync(prefdet->session);
}

/*
* preferences_manager_save_data_full
* update all preferences data with new internal data
*/
void preferences_manager_save_data_full(PreferencesManager *preferences_manager)
{
  gphpedit_debug(DEBUG_PREFS);
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  preferences_manager_save_data(preferences_manager);  /* save session data */

  g_settings_set (prefdet->gs, "indentation-size", "u", prefdet->indentation_size);
  g_settings_set (prefdet->gs, "tabs-size", "u", prefdet->tab_size);

  g_settings_set_boolean (prefdet->gs, "show-indentation-guides", prefdet->show_indentation_guides);
  g_settings_set_boolean (prefdet->gs, "display-right-margin", prefdet->edge_mode);
  g_settings_set (prefdet->gs, "right-margin-position", "u", prefdet->edge_column);
  g_settings_set_boolean (prefdet->gs, "line-wrapping", prefdet->line_wrapping);

  g_settings_set_boolean (prefdet->gs, "auto-complete-braces", prefdet->auto_complete_braces);
  g_settings_set_boolean (prefdet->gs, "highlight-current-line", prefdet->higthlightcaretline);
  g_settings_set_boolean (prefdet->gs, "save-session", prefdet->save_session);
  g_settings_set_boolean (prefdet->gs, "use-tabs-instead-spaces", prefdet->use_tabs_instead_spaces);
  g_settings_set_boolean (prefdet->gs, "single-instance-only", prefdet->single_instance_only);
  g_settings_set_boolean (prefdet->gs, "showfilebrowser", prefdet->showfilebrowser);

  g_settings_set_string (prefdet->gs, "php-binary-location", prefdet->php_binary_location);
  g_settings_set_string (prefdet->gs, "shared-source-location", prefdet->shared_source_location);
  g_settings_set_string (prefdet->gs, "php-file-extensions", prefdet->php_file_extensions);

  g_settings_set (prefdet->gs, "auto-complete-delay", "u", prefdet->auto_complete_delay);
  g_settings_set (prefdet->gs, "calltip-delay", "u", prefdet->calltip_delay);
  /* style settings */
  gchar *font_setting = g_strdup_printf("%s %d",prefdet->font_name + 1, prefdet->font_size);
  g_settings_set_string (prefdet->gs, "editor-font", font_setting);
  g_free(font_setting);
  g_settings_set_string (prefdet->gs, "scheme", prefdet->style_name);
}

/*
* preferences_manager_restore_data
* reload preferences data and discard internal data.
*/
void preferences_manager_restore_data(PreferencesManager *preferences_manager)
{
  gphpedit_debug(DEBUG_PREFS);
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  clean_default_settings(prefdet);

  load_default_settings(prefdet);
  load_styles(prefdet); /* load lexer styles */
}

/* plugin preferences functions */

gboolean get_plugin_is_active(PreferencesManager *preferences_manager, const gchar *name)
{
  if(!preferences_manager || !name) return FALSE;
  gphpedit_debug(DEBUG_PREFS);
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  
  gboolean result = gphpedit_session_get_boolean_with_default (prefdet->session,"plugins", name, TRUE);
  return result;
}

void set_plugin_is_active(PreferencesManager *preferences_manager, const gchar *name, gboolean status)
{
  if(!preferences_manager || !name) return ;
  gphpedit_debug(DEBUG_PREFS);
  PreferencesManagerDetails *prefdet;
  prefdet = PREFERENCES_MANAGER_GET_PRIVATE(preferences_manager);
  gphpedit_session_set_boolean (prefdet->session,"plugins", name, status);
  gphpedit_session_sync(prefdet->session);
}

  /**internal styles functions **/
/*
* get_default_font_settings
* read default font and default font size from GtkSettings
*/
gchar *get_default_font_settings(PreferencesManagerDetails *prefdet)
{
  gphpedit_debug(DEBUG_PREFS);
  gchar *string=NULL;
  g_object_get(G_OBJECT(gtk_settings_get_default()), "gtk-font-name", &string, NULL);
  return string;
}

void load_font_settings(PreferencesManagerDetails *prefdet)
{
  gphpedit_debug(DEBUG_PREFS);
  gchar *font_desc= g_settings_get_string (prefdet->gs, "editor-font");
  if (!font_desc) font_desc = g_strdup(get_default_font_settings(prefdet));
  PangoFontDescription *desc = pango_font_description_from_string (font_desc);
  prefdet->font_size = PANGO_PIXELS(pango_font_description_get_size (desc));
  /* add ! needed by scintilla for pango render */
  prefdet->font_name = g_strdup_printf("!%s",pango_font_description_get_family (desc));
  pango_font_description_free (desc);
  g_free(font_desc);
}

/**
* set_font_settings
* set a new font name and font size.
* font_desc must be a pango font description string like "Helvetica 12"
* this function don't save new values.
* you must call "preferences_manager_save_data_full" to update stored values.
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

/*
* load_styles (internal)
* loads lexer styles
*/
void load_styles(PreferencesManagerDetails *prefdet)
{
  gphpedit_debug(DEBUG_PREFS);
  load_font_settings(prefdet);
  prefdet->style_name = g_settings_get_string (prefdet->gs, "scheme");
}
