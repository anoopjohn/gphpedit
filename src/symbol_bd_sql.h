/* This file is part of gPHPEdit, a GNOME PHP Editor.

   Copyright (C) 2010 José Rostagno (for vijona.com.ar)

   For more information or to find the latest release, visit our 
   website at http://www.gsqledit.org/

   gSQLEdit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   gSQLEdit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with gSQLEdit. If not, see <http://www.gnu.org/licenses/>.

   The GNU General Public License is contained in the file COPYING.
*/
#ifndef SYMBOL_BD_SQL_H
#define SYMBOL_BD_SQL_H

#include <glib-object.h>

#define SYMBOL_BD_SQL_TYPE symbol_bd_sql_get_type()
#define SYMBOL_BD_SQL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), SYMBOL_BD_SQL_TYPE, SymbolBdSQL))
#define SYMBOL_BD_SQL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), SYMBOL_BD_SQL_TYPE, SymbolBdSQLClass))
#define OBJECT_IS_SYMBOL_BD_SQL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SYMBOL_BD_SQL_TYPE))
#define OBJECT_IS_SYMBOL_BD_SQL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), SYMBOL_BD_SQL_TYPE))
#define SYMBOL_BD_SQL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), SYMBOL_BD_SQL_TYPE, SymbolBdSQLClass))

typedef struct SymbolBdSQLDetails SymbolBdSQLDetails;

typedef struct
{
	GObject object;
	SymbolBdSQLDetails *details;
} SymbolBdSQL;

typedef struct
{
	GObjectClass parent_class;

} SymbolBdSQLClass;

/* Basic GObject requirements. */
GType symbol_bd_sql_get_type (void);
SymbolBdSQL *symbol_bd_sql_new (void);
#endif /* SYMBOL_BD_SQL_H */
