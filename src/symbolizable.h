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
#ifndef __SYMBOLIZABLE_H__
#define __SYMBOLIZABLE_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define IFACE_TYPE_SYMBOLIZABLE             (symbolizable_get_type ())
#define SYMBOLIZABLE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), IFACE_TYPE_SYMBOLIZABLE, Symbolizable))
#define SYMBOLIZABLE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), IFACE_TYPE_SYMBOLIZABLE, SymbolizableIface))
#define IFACE_IS_SYMBOLIZABLE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IFACE_TYPE_SYMBOLIZABLE))
#define IFACE_IS_SYMBOLIZABLE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), IFACE_TYPE_SYMBOLIZABLE))
#define SYMBOLIZABLE_GET_IFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), IFACE_TYPE_SYMBOLIZABLE, SymbolizableIface))


typedef struct _Symbolizable       Symbolizable;         /* Dummy typedef */
typedef struct _SymbolizableIface  SymbolizableIface;
typedef struct _SymbolizableIface  SymbolizableInterface;

enum 
{
  SYMBOL_ALL            = 1 << 0,
  SYMBOL_FUNCTION       = 1 << 1,
  SYMBOL_CLASS          = 1 << 2,
  SYMBOL_VAR            = 1 << 3,
  SYMBOL_MEMBER         = 1 << 4
};
/*
* symbol_types:
* SYMBOL_ALL
* SYMBOL_FUNCTION
* SYMBOL_CLASS
* SYMBOL_VAR
* SYMBOL_MEMBER
*/
struct _SymbolizableIface
{
  GTypeInterface base_iface;

	void (* update) (Symbolizable *iface, gpointer user_data);

  /*vitual methods */
  gchar *(*get_symbols_matches) (Symbolizable *iface, const gchar *symbol_prefix, gint flags);
  gchar *(*get_class_symbols) (Symbolizable *iface, const gchar *class_name);
  gchar *(*get_classes) (Symbolizable *iface);
  gchar *(*get_calltip) (Symbolizable *iface, const gchar *symbol_name);
  GList *(*get_custom_symbols_list) (Symbolizable *iface, gint symbol_type);
  GList *(*get_custom_symbols_list_by_filename) (Symbolizable *iface, gint symbol_type, gchar *filename);
  void (*rescan_file) (Symbolizable *iface, gchar *filename);
  void (*purge_file) (Symbolizable *iface, gchar *filename);
  void (*add_file) (Symbolizable *iface, gchar *filename);
};

GType          symbolizable_get_type        (void) G_GNUC_CONST;


/* internal struct */
typedef struct
{
  gchar *varname;
  gchar *functionname;
  gchar *filename;
  gboolean remove;
  guint identifierid;
}
ClassBrowserVar;

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
}
ClassBrowserFunction;

typedef struct
{
  gchar *classname;
  gchar *filename;
  guint line_number;
  gboolean remove;
  guint identifierid;
}
ClassBrowserClass;

gchar *symbolizable_get_symbols_matches (Symbolizable *self, const gchar *symbol_prefix, gint flags);
gchar *symbolizable_get_class_symbols (Symbolizable *self, const gchar *class_name);
gchar *symbolizable_get_classes (Symbolizable *self);
gchar *symbolizable_get_calltip (Symbolizable *self, const gchar *symbol_name);
GList *symbolizable_get_custom_symbols_list (Symbolizable *self, gint symbol_type);
GList *symbolizable_get_custom_symbols_list_by_filename (Symbolizable *self, gint symbol_type, gchar *filename);
void symbolizable_rescan_file (Symbolizable *self, gchar *filename);
void symbolizable_purge_file (Symbolizable *self, gchar *filename);
void symbolizable_add_file (Symbolizable *self, gchar *filename);

G_END_DECLS

#endif /* __SYMBOLIZABLE_H__ */


