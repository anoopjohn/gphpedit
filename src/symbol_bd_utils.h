/* This file is part of gPHPEdit, a GNOME PHP Editor.

   Copyright (C) 2010 José Rostagno (for vijona.com.ar)

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
#ifndef SYMBOL_BD_UTILS_H
#define SYMBOL_BD_UTILS_H

#include <glib-object.h>

void symbol_bd_load_api_file(const gchar *api_filename, GTree **api_tree);
void symbol_bd_function_list_from_array_prepare(gchar **keywords, GTree **api_tree);
void free_variable_item (gpointer data);
void free_class_item (gpointer data);
void free_function_item (gpointer data);
void process_ctags_custom (GObject *symbolbd, gchar *result, const gchar *filename, void (*process_func)(GObject *symbolbd, gchar *name, const gchar *filename, gchar *type, gchar *line, gchar *param));
void symbol_bd_varlist_add(GHashTable **variables_table, gchar *varname, gchar *funcname, const gchar *filename, guint *identifierid);
void symbol_bd_classlist_add(GHashTable **class_table, gchar *classname, const gchar *filename, gint line_number, guint *identifierid);
void symbol_bd_functionlist_add(GHashTable **function_table, GHashTable **class_table, gchar *classname, gchar *funcname, const gchar *filename, 
                                      guint line_number, gchar *param_list, guint *identifierid);
GString *symbol_bd_get_autocomp_from_cache(gchar *cache_str, gchar *cache_completion, const gchar *symbol_prefix);
#endif /* SYMBOL_BD_UTILS_H */
