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
#ifndef SYMBOL_BD_PHP_H
#define SYMBOL_BD_PHP_H

#include <glib-object.h>

#define SYMBOL_BD_PHP_TYPE symbol_bd_php_get_type()
#define SYMBOL_BD_PHP(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), SYMBOL_BD_PHP_TYPE, SymbolBdPHP))
#define SYMBOL_BD_PHP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), SYMBOL_BD_PHP_TYPE, SymbolBdPHPClass))
#define OBJECT_IS_SYMBOL_BD_PHP(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SYMBOL_BD_PHP_TYPE))
#define OBJECT_IS_SYMBOL_BD_PHP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), SYMBOL_BD_PHP_TYPE))
#define SYMBOL_BD_PHP_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), SYMBOL_BD_PHP_TYPE, SymbolBdPHPClass))

typedef struct SymbolBdPHPDetails SymbolBdPHPDetails;

typedef struct
{
	GObject object;
	SymbolBdPHPDetails *details;
} SymbolBdPHP;

typedef struct
{
	GObjectClass parent_class;

} SymbolBdPHPClass;

/* Basic GObject requirements. */
GType symbol_bd_php_get_type (void);
SymbolBdPHP *symbol_bd_php_new (void);
void symbol_bd_php_functionlist_add(SymbolBdPHP *symbolbd, gchar *classname, 
            gchar *funcname, gchar *filename, guint line_number, gchar *param_list);
void symbol_bd_php_classlist_add(SymbolBdPHP *symbolbd, gchar *classname, 
            gchar *filename, gint line_number);
void symbol_bd_php_varlist_add(SymbolBdPHP *symbolbd, gchar *varname, 
            gchar *funcname, gchar *filename);
#endif /* SYMBOL_BD_PHP_H */
