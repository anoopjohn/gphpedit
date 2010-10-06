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
#ifndef CLASSBROWSER_BACKEND_H
#define CLASSBROWSER_BACKEND_H

#include <gtk/gtk.h>

#define CLASSBROWSER_BACKEND_TYPE classbrowser_backend_get_type()
#define CLASSBROWSER_BACKEND(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), CLASSBROWSER_BACKEND_TYPE, Classbrowser_Backend))
#define CLASSBROWSER_BACKEND_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), CLASSBROWSER_BACKEND_TYPE, Classbrowser_BackendClass))
#define OBJECT_IS_CLASSBROWSER_BACKEND(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CLASSBROWSER_BACKEND_TYPE))
#define OBJECT_IS_CLASSBROWSER_BACKEND_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), CLASSBROWSER_BACKEND_TYPE))
#define CLASSBROWSER_BACKEND_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), CLASSBROWSER_BACKEND_TYPE, Classbrowser_BackendClass))

typedef struct Classbrowser_BackendDetails Classbrowser_BackendDetails;

typedef struct
{
	GObject object;
	Classbrowser_BackendDetails *details;
} Classbrowser_Backend;

typedef struct
{
	GObjectClass parent_class;

	void (* done_refresh) (Classbrowser_Backend *doclod, gboolean result, gpointer user_data);

} Classbrowser_BackendClass;

/* needed types */
typedef struct
{
  gchar *functionname;
  gchar *paramlist;
  gchar *filename;
  guint line_number;
  guint class_id;
  gchar *classname;
  gboolean remove;
  guint identifierid;
  gint file_type;
}
ClassBrowserFunction;

typedef struct
{
  gchar *classname;
  gchar *filename;
  guint line_number;
  gboolean remove;
  guint identifierid;
  gint file_type;
}
ClassBrowserClass;

/* Basic GObject requirements. */
GType classbrowser_backend_get_type (void);
Classbrowser_Backend *classbrowser_backend_new (void);
void classbrowser_backend_update(Classbrowser_Backend *classback, gboolean only_current_file);
GSList *classbrowser_backend_get_function_list(Classbrowser_Backend *classback);
GTree *classbrowser_backend_get_class_list(Classbrowser_Backend *classback);
void classbrowser_functionlist_add(Classbrowser_Backend *classback, gchar *classname, gchar *funcname, gchar *filename, gint file_type, guint line_number, gchar *param_list);
void classbrowser_classlist_add(Classbrowser_Backend *classback, gchar *classname, gchar *filename, gint line_number,gint file_type);
void classbrowser_varlist_add(Classbrowser_Backend *classback, gchar *varname, gchar *funcname, gchar *filename, gint file_type);
GString *classbrowser_backend_get_selected_label(Classbrowser_Backend *classback, gchar *filename, gint line);
GString *classbrowser_backend_get_autocomplete_php_classes_string(Classbrowser_Backend *classback);
gchar *classbrowser_backend_autocomplete_php_variables(Classbrowser_Backend *classback, gchar *buffer);
gchar *classbrowser_backend_autocomplete_member_function(Classbrowser_Backend *classback, gchar *prefix);
gchar *classbrowser_backend_custom_function_calltip(Classbrowser_Backend *classback, gchar *function_name, gint file_type);
gchar *classbrowser_backend_add_custom_autocompletion(Classbrowser_Backend *classback, gchar *prefix, gint file_type, GSList *list);
#endif /* CLASSBROWSER_BACKEND_H */

