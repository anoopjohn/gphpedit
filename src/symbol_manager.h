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
#ifndef SYMBOL_MANAGER_H
#define SYMBOL_MANAGER_H

#include <gtk/gtk.h>
#include "symbolizable.h"

#define SYMBOL_MANAGER_TYPE symbol_manager_get_type()
#define SYMBOL_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), SYMBOL_MANAGER_TYPE, SymbolManager))
#define SYMBOL_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), SYMBOL_MANAGER_TYPE, SymbolManagerClass))
#define OBJECT_IS_SYMBOL_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SYMBOL_MANAGER_TYPE))
#define OBJECT_IS_SYMBOL_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), SYMBOL_MANAGER_TYPE))
#define SYMBOL_MANAGER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), SYMBOL_MANAGER_TYPE, SymbolManagerClass))

typedef struct SymbolManagerDetails SymbolManagerDetails;

typedef struct
{
	GObject object;
	SymbolManagerDetails *details;
} SymbolManager;

typedef struct
{
	GObjectClass parent_class;

	void (* update) (SymbolManager *symbolmg, gpointer user_data);

} SymbolManagerClass;

/* Basic GObject requirements. */
GType symbol_manager_get_type (void);
SymbolManager *symbol_manager_new (void);
gchar *symbol_manager_get_symbols_matches (SymbolManager *symbolmg, const gchar *symbol_prefix, gint flags, gint ftype);
gchar *symbol_manager_get_class_symbols (SymbolManager *symbolmg, const gchar *class_name, gint ftype);
gchar *symbol_manager_get_classes (SymbolManager *symbolmg, gint ftype);
gchar *symbol_manager_get_calltip (SymbolManager *symbolmg, const gchar *symbol_name, gint ftype);
GList *symbol_manager_get_custom_symbols_list (SymbolManager *symbolmg, gint symbol_type, gint ftype);
GList *symbol_manager_get_custom_symbols_list_by_filename (SymbolManager *symbolmg, gint symbol_type, gchar *filename, gint ftype);
void symbol_manager_rescan_file (SymbolManager *symbolmg, gchar *filename, gint ftype);
void symbol_manager_purge_file (SymbolManager *symbolmg, gchar *filename, gint ftype);
void symbol_manager_add_file (SymbolManager *symbolmg, gchar *filename, gint ftype);

#endif /* SYMBOL_MANAGER_H */
