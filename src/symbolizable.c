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

#include "symbolizable.h"

G_DEFINE_INTERFACE(Symbolizable, symbolizable, G_TYPE_INVALID)

/* signal enumeration */
enum {
	UPDATE,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static void
symbolizable_default_init (SymbolizableIface *iface)
{
  /*
  * UPDATE:
  * Emited when symbols are added or deleted.
  * When a file is added, rescaned or purged this signal is emited.
  */
	signals[UPDATE] =
		g_signal_new ("update",
		              IFACE_TYPE_SYMBOLIZABLE,
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (SymbolizableIface, update),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID ,
		              G_TYPE_NONE, 0);
}

/*
* symbolizable_get_symbols_matches
* return a string with all symbol that starts with @symbol_prefix, 
* following the symbol types from @flags parameter.
* for example:
  gchar *result;
  result = symbolizable_get_symbols_matches (symbol, "str", SYMBOL_FUNCTION | SYMBOL_CLASS);
* will retreive all classes and functions that start with "str".
* NOTE: return value must be free with g_free when no longer needed.
*/
gchar *symbolizable_get_symbols_matches (Symbolizable *self, const gchar *symbol_prefix, gint flags)
{
  if (!self) return NULL;
	return SYMBOLIZABLE_GET_IFACE (self)->get_symbols_matches (self, symbol_prefix, flags);
}

/*
* symbolizable_get_class_symbols
* return a string with symbol members for @class_name.
* NOTE: return value must be free with g_free when no longer needed.
*/
gchar *symbolizable_get_class_symbols (Symbolizable *self, const gchar *class_name)
{
  if (!self) return NULL;
	return SYMBOLIZABLE_GET_IFACE (self)->get_class_symbols (self, class_name);
}

/*
* symbolizable_get_classes
* return a string containing all classes.
* NOTE: return value must be free with g_free when no longer needed.
*/
gchar *symbolizable_get_classes (Symbolizable *self)
{
  if (!self) return NULL;
	return SYMBOLIZABLE_GET_IFACE (self)->get_classes (self);
}

/*
* symbolizable_get_calltip
* return a string containing the calltip for @symbol_name.
* @symbol_name must be a function name.
* NOTE: return value must be free with g_free when no longer needed.
*/
gchar *symbolizable_get_calltip (Symbolizable *self, const gchar *symbol_name)
{
  if (!self) return NULL;
	return SYMBOLIZABLE_GET_IFACE (self)->get_calltip (self, symbol_name);
}

/*
* symbolizable_get_custom_symbols_list
* return the list of symbols that match @symbol_type.
* Only one kind of symbol are allowed at a time.
* NOTE: the returned items are owned by the object and must not be freed.
* the return value must be free with g_list_free when no longer needed.
*/
GList *symbolizable_get_custom_symbols_list (Symbolizable *self, gint symbol_type)
{
  if (!self) return NULL;
	return SYMBOLIZABLE_GET_IFACE (self)->get_custom_symbols_list (self, symbol_type);
}

/*
* symbolizable_get_custom_symbols_list_by_filename
* return the list of symbols that match @symbol_type for a filename.
* Only one kind of symbol are allowed at a time.
* NOTE: the returned items are owned by the object and must not be freed.
* the return value must be free with g_list_free when no longer needed.
*/
GList *symbolizable_get_custom_symbols_list_by_filename (Symbolizable *self, gint symbol_type, gchar *filename)
{
  if (!self) return NULL;
	return SYMBOLIZABLE_GET_IFACE (self)->get_custom_symbols_list_by_filename (self, symbol_type, filename);
}

/*
*
*/
void symbolizable_rescan_file (Symbolizable *self, gchar *filename)
{
  g_return_if_fail (self);
  g_return_if_fail (filename);
	SYMBOLIZABLE_GET_IFACE (self)->rescan_file (self, filename);
  g_signal_emit (G_OBJECT (self), signals[UPDATE], 0);
}

/*
* symbolizable_add_file
* Add a new file into the DB.
*/
void symbolizable_add_file (Symbolizable *self, gchar *filename)
{
  g_return_if_fail (self);
  g_return_if_fail (filename);
	SYMBOLIZABLE_GET_IFACE (self)->add_file (self, filename);
  g_signal_emit (G_OBJECT (self), signals[UPDATE], 0);
}

/*
* symbolizable_purge_file
* Remove a file and it's symbols from the DB.
*/
void symbolizable_purge_file (Symbolizable *self, gchar *filename)
{
  g_return_if_fail (self);
  g_return_if_fail (filename);
	SYMBOLIZABLE_GET_IFACE (self)->purge_file (self, filename);
  g_signal_emit (G_OBJECT (self), signals[UPDATE], 0);
}

