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
#ifndef PREFERENCES_MANAGER_H
#define PREFERENCES_MANAGER_H

#include <gtk/gtk.h>

#define PREFERENCES_MANAGER_TYPE preferences_manager_get_type()
#define PREFERENCES_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFERENCES_MANAGER_TYPE, Preferences_Manager))
#define PREFERENCES_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), PREFERENCES_MANAGER_TYPE, Preferences_ManagerClass))
#define OBJECT_IS_PREFERENCES_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PREFERENCES_MANAGER_TYPE))
#define OBJECT_IS_PREFERENCES_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), PREFERENCES_MANAGER_TYPE))
#define PREFERENCES_MANAGER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), PREFERENCES_MANAGER_TYPE, Preferences_ManagerClass))

typedef struct Preferences_ManagerDetails Preferences_ManagerDetails;

typedef struct
{
	GObject object;
	Preferences_ManagerDetails *details;
} Preferences_Manager;

typedef struct
{
	GObjectClass parent_class;

} Preferences_ManagerClass;

/* Basic GObject requirements. */
GType preferences_manager_get_type (void);
Preferences_Manager *preferences_manager_new (void);
/*main_window session preferences*/
void get_preferences_manager_window_size (Preferences_Manager *preferences_manager, gint *width, gint *height);
void set_preferences_manager_window_size (Preferences_Manager *preferences_manager, gint width, gint height);
void get_preferences_manager_window_position (Preferences_Manager *preferences_manager, gint *top, gint *left);
void set_preferences_manager_window_position (Preferences_Manager *preferences_manager, gint top, gint left);
gboolean get_preferences_manager_window_maximized(Preferences_Manager *preferences_manager);
void set_preferences_manager_window_maximized(Preferences_Manager *preferences_manager, gboolean newstate);
/**/
GSList *get_preferences_manager_search_history(Preferences_Manager *preferences_manager);
gboolean get_preferences_manager_show_filebrowser(Preferences_Manager *preferences_manager);
void set_preferences_manager_show_filebrowser(Preferences_Manager *preferences_manager, gboolean new_status);
gboolean get_preferences_manager_show_statusbar(Preferences_Manager *preferences_manager);
void set_preferences_manager_show_statusbar(Preferences_Manager *preferences_manager, gboolean new_status);
gboolean get_preferences_manager_show_maintoolbar(Preferences_Manager *preferences_manager);
void set_preferences_manager_show_maintoolbar(Preferences_Manager *preferences_manager, gboolean new_status);
gboolean get_preferences_manager_show_findtoolbar(Preferences_Manager *preferences_manager);
void set_preferences_manager_show_findtoolbar(Preferences_Manager *preferences_manager, gboolean new_status);

gboolean get_preferences_manager_side_panel_status(Preferences_Manager *preferences_manager);
void set_preferences_manager_parse_side_panel_status(Preferences_Manager *preferences_manager, gint new_status);
gint get_preferences_manager_side_panel_get_size(Preferences_Manager *preferences_manager);
void set_preferences_manager_side_panel_size(Preferences_Manager *preferences_manager, gint new_size);
/**/
gint get_preferences_manager_parse_only_current_file(Preferences_Manager *preferences_manager);
void set_preferences_manager_parse_only_current_file(Preferences_Manager *preferences_manager, gint new_status);
/**/
const gchar *get_preferences_manager_last_opened_folder(Preferences_Manager *preferences_manager);
void set_preferences_manager_last_opened_folder(Preferences_Manager *preferences_manager, const gchar *new_last_folder);
void set_preferences_manager_filebrowser_last_folder(Preferences_Manager *preferences_manager, const gchar *new_last_folder);
const gchar *get_preferences_manager_filebrowser_last_folder(Preferences_Manager *preferences_manager);
gint get_preferences_manager_indentation_size (Preferences_Manager *preferences_manager);
void set_preferences_manager_indentation_size(Preferences_Manager *preferences_manager, gint newstate);
const gchar *get_preferences_manager_php_binary_location(Preferences_Manager *preferences_manager);
void set_preferences_manager_php_binary_location(Preferences_Manager *preferences_manager, gchar *newstate);
gboolean get_preferences_manager_saved_session(Preferences_Manager *preferences_manager);
void set_preferences_manager_saved_session(Preferences_Manager *preferences_manager, gboolean newstate);
gint get_preferences_manager_font_quality(Preferences_Manager *preferences_manager);
gboolean get_preferences_manager_line_wrapping(Preferences_Manager *preferences_manager);
void set_preferences_manager_line_wrapping(Preferences_Manager *preferences_manager, gboolean newstate);
gint get_preferences_manager_calltip_delay(Preferences_Manager *preferences_manager);
void set_preferences_manager_calltip_delay(Preferences_Manager *preferences_manager, gint newstate);
gint get_preferences_manager_auto_complete_delay(Preferences_Manager *preferences_manager);
void set_preferences_manager_auto_complete_delay(Preferences_Manager *preferences_manager, gint newstate);
gboolean get_preferences_manager_use_tabs_instead_spaces(Preferences_Manager *preferences_manager);
void set_preferences_manager_use_tabs_instead_spaces(Preferences_Manager *preferences_manager, gboolean newstate);
gboolean get_preferences_manager_auto_complete_braces(Preferences_Manager *preferences_manager);
void set_preferences_manager_auto_complete_braces(Preferences_Manager *preferences_manager, gboolean newstate);
gboolean get_preferences_manager_show_folding(Preferences_Manager *preferences_manager);
gboolean get_preferences_manager_single_instance_only(Preferences_Manager *preferences_manager);
void set_preferences_manager_single_instance_only(Preferences_Manager *preferences_manager, gboolean newstate);
gboolean get_preferences_manager_higthlight_caret_line(Preferences_Manager *preferences_manager);
void set_preferences_manager_higthlight_caret_line(Preferences_Manager *preferences_manager, gboolean newstate);
gboolean get_preferences_manager_show_indentation_guides(Preferences_Manager *preferences_manager);
void set_preferences_manager_show_indentation_guides(Preferences_Manager *preferences_manager, gboolean newstate);
gboolean get_preferences_manager_edge_mode(Preferences_Manager *preferences_manager);
void set_preferences_manager_edge_mode(Preferences_Manager *preferences_manager, gboolean newstate);
gint get_preferences_manager_edge_column(Preferences_Manager *preferences_manager);
void set_preferences_manager_edge_column(Preferences_Manager *preferences_manager, gint newstate);
gint get_preferences_manager_tab_size(Preferences_Manager *preferences_manager);
void set_preferences_manager_tab_size(Preferences_Manager *preferences_manager, gint newstate);
gchar *get_preferences_manager_shared_source_location(Preferences_Manager *preferences_manager);
void set_preferences_manager_shared_source_location(Preferences_Manager *preferences_manager, gchar *newstate); /* */
gchar *get_preferences_manager_php_file_extensions(Preferences_Manager *preferences_manager);
void set_preferences_manager_php_file_extensions(Preferences_Manager *preferences_manager, gchar *newstate);
GSList *get_preferences_manager_php_search_history(Preferences_Manager *preferences_manager);
void set_preferences_manager_new_search_history_item(Preferences_Manager *preferences_manager, gint pos, const gchar *newtext);
/*plugins */
gboolean get_plugin_is_active(Preferences_Manager *preferences_manager, const gchar *name);
void set_plugin_is_active(Preferences_Manager *preferences_manager, const gchar *name, gboolean status);
/*styles */
gchar *get_preferences_manager_style_name(Preferences_Manager *preferences_manager);
void set_preferences_manager_style_name(Preferences_Manager *preferences_manager, gchar *newstate);
guint get_preferences_manager_style_size(Preferences_Manager *preferences_manager);
gchar *get_preferences_manager_style_font(Preferences_Manager *preferences_manager);
void set_font_settings (Preferences_Manager *preferences_manager, gchar *font_desc);
void set_style_name (Preferences_Manager *preferences_manager, gchar *newstyle);

void preferences_manager_save_data(Preferences_Manager *preferences_manager);
void preferences_manager_save_data_full(Preferences_Manager *preferences_manager);
void preferences_manager_restore_data(Preferences_Manager *preferences_manager);
#endif /* PREFERENCES_MANAGER_H */

