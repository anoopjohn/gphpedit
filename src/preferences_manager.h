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
#ifndef PREFERENCES_MANAGER_H
#define PREFERENCES_MANAGER_H

#include <gtk/gtk.h>

#define PREFERENCES_MANAGER_TYPE preferences_manager_get_type()
#define PREFERENCES_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFERENCES_MANAGER_TYPE, PreferencesManager))
#define PREFERENCES_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), PREFERENCES_MANAGER_TYPE, PreferencesManagerClass))
#define OBJECT_IS_PREFERENCES_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PREFERENCES_MANAGER_TYPE))
#define OBJECT_IS_PREFERENCES_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), PREFERENCES_MANAGER_TYPE))
#define PREFERENCES_MANAGER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), PREFERENCES_MANAGER_TYPE, PreferencesManagerClass))

typedef struct PreferencesManagerDetails PreferencesManagerDetails;

typedef struct
{
	GObject object;
	PreferencesManagerDetails *details;
} PreferencesManager;

typedef struct
{
	GObjectClass parent_class;

} PreferencesManagerClass;

/* Basic GObject requirements. */
GType preferences_manager_get_type (void);
PreferencesManager *preferences_manager_new (void);
/*main_window session preferences*/
void get_preferences_manager_window_size (PreferencesManager *preferences_manager, gint *width, gint *height);
void set_preferences_manager_window_size (PreferencesManager *preferences_manager, gint width, gint height);
void get_preferences_manager_window_position (PreferencesManager *preferences_manager, gint *top, gint *left);
void set_preferences_manager_window_position (PreferencesManager *preferences_manager, gint top, gint left);
gboolean get_preferences_manager_window_maximized(PreferencesManager *preferences_manager);
void set_preferences_manager_window_maximized(PreferencesManager *preferences_manager, gboolean newstate);
/**/
gboolean get_preferences_manager_show_filebrowser(PreferencesManager *preferences_manager);
void set_preferences_manager_show_filebrowser(PreferencesManager *preferences_manager, gboolean new_status);
gboolean get_preferences_manager_show_statusbar(PreferencesManager *preferences_manager);
void set_preferences_manager_show_statusbar(PreferencesManager *preferences_manager, gboolean new_status);
gboolean get_preferences_manager_show_maintoolbar(PreferencesManager *preferences_manager);
void set_preferences_manager_show_maintoolbar(PreferencesManager *preferences_manager, gboolean new_status);
/**/
GSList *get_preferences_manager_search_history(PreferencesManager *preferences_manager);
void set_preferences_manager_new_search_history_item(PreferencesManager *preferences_manager, gint pos, const gchar *newtext);
/*plugins */
gboolean get_plugin_is_active(PreferencesManager *preferences_manager, const gchar *name);
void set_plugin_is_active(PreferencesManager *preferences_manager, const gchar *name, gboolean status);
/*styles */
void set_font_settings (PreferencesManager *preferences_manager, gchar *font_desc);
/* session files */
GSList *get_preferences_manager_session_files(PreferencesManager *preferences_manager);
void set_preferences_manager_session_files(PreferencesManager *preferences_manager, GSList *files);
/**/
void preferences_manager_save_data(PreferencesManager *preferences_manager);
void preferences_manager_save_data_full(PreferencesManager *preferences_manager);
void preferences_manager_restore_data(PreferencesManager *preferences_manager);
#endif /* PREFERENCES_MANAGER_H */

