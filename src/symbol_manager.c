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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "debug.h"
#include "document.h"
#include "symbol_manager.h"
#include "symbol_bd_php.h"
#include "symbol_bd_cobol.h"
#include "symbol_bd_css.h"
#include "symbol_bd_cxx.h"
#include "symbol_bd_sql.h"
#include "symbol_bd_python.h"
#include "symbol_bd_perl.h"

/*
* symbol_manager private struct
*/
struct SymbolManagerDetails
{
  SymbolBdPHP *sbd_php;
  SymbolBdCOBOL *sbd_cobol;
  SymbolBdSQL *sbd_sql;
  SymbolBdCSS *sbd_css;
  SymbolBdCXX *sbd_cxx;
  SymbolBdPYTHON *sbd_python;
  SymbolBdPERL *sbd_perl;
};

/* object signal enumeration */
enum {
	UPDATE,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

/*
 * symbol_manager_get_type
 * register SymbolManager type and returns a new GType
*/

static void symbol_manager_class_init (SymbolManagerClass *klass);

#define SYMBOL_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    SYMBOL_MANAGER_TYPE,\
					    SymbolManagerDetails))

/* http://library.gnome.org/devel/gobject/unstable/gobject-Type-Information.html#G-DEFINE-TYPE:CAPS */
G_DEFINE_TYPE(SymbolManager, symbol_manager, G_TYPE_OBJECT);

/*
* overide default contructor to make a singleton.
* see http://blogs.gnome.org/xclaesse/2010/02/11/how-to-make-a-gobject-singleton/
*/
static GObject* 
symbol_manager_constructor (GType type,
                 guint n_construct_params,
                 GObjectConstructParam *construct_params)
{
  static GObject *self = NULL;

  if (self == NULL)
    {
      self = G_OBJECT_CLASS (symbol_manager_parent_class)->constructor (
          type, n_construct_params, construct_params);
      g_object_add_weak_pointer (self, (gpointer) &self);
      return self;
    }

  return g_object_ref (self);
}

void symbol_manager_finalize (GObject *object)
{
  SymbolManager *symbolmg = SYMBOL_MANAGER(object);
  SymbolManagerDetails *symbolmgdet;
	symbolmgdet = SYMBOL_MANAGER_GET_PRIVATE(symbolmg);

  if (symbolmgdet->sbd_php) g_object_unref(symbolmgdet->sbd_php);
  if (symbolmgdet->sbd_cobol) g_object_unref(symbolmgdet->sbd_cobol);
  if (symbolmgdet->sbd_sql) g_object_unref(symbolmgdet->sbd_sql);
  if (symbolmgdet->sbd_css) g_object_unref(symbolmgdet->sbd_css);
  if (symbolmgdet->sbd_cxx) g_object_unref(symbolmgdet->sbd_cxx);
  if (symbolmgdet->sbd_python) g_object_unref(symbolmgdet->sbd_python);
  if (symbolmgdet->sbd_perl) g_object_unref(symbolmgdet->sbd_perl);

  /* Chain up to the parent class */
  G_OBJECT_CLASS (symbol_manager_parent_class)->finalize (object);
}

void
symbol_manager_class_init (SymbolManagerClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
  object_class->constructor = symbol_manager_constructor;
	object_class->finalize = symbol_manager_finalize;

	signals[UPDATE] =
		g_signal_new ("update",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (SymbolManagerClass, update),
		              NULL, NULL,
		               g_cclosure_marshal_VOID__VOID ,
		               G_TYPE_NONE, 0);

	g_type_class_add_private (klass, sizeof (SymbolManagerDetails));
}

static void sdb_symbolizable_update_cb (Symbolizable *sbd, gpointer user_data)
{
    g_signal_emit (G_OBJECT (user_data), signals[UPDATE], 0);
}

void
symbol_manager_init (SymbolManager *symbolmg)
{
  SymbolManagerDetails *symbolmgdet;
	symbolmgdet = SYMBOL_MANAGER_GET_PRIVATE(symbolmg);

  symbolmgdet->sbd_php = symbol_bd_php_new ();
  g_signal_connect(SYMBOLIZABLE(symbolmgdet->sbd_php), "update", G_CALLBACK(sdb_symbolizable_update_cb), symbolmg);
  symbolmgdet->sbd_cobol = symbol_bd_cobol_new ();
  g_signal_connect(SYMBOLIZABLE(symbolmgdet->sbd_cobol), "update", G_CALLBACK(sdb_symbolizable_update_cb), symbolmg);
  symbolmgdet->sbd_sql = symbol_bd_sql_new ();
  g_signal_connect(SYMBOLIZABLE(symbolmgdet->sbd_sql), "update", G_CALLBACK(sdb_symbolizable_update_cb), symbolmg);
  symbolmgdet->sbd_css = symbol_bd_css_new ();
  g_signal_connect(SYMBOLIZABLE(symbolmgdet->sbd_css), "update", G_CALLBACK(sdb_symbolizable_update_cb), symbolmg);
  symbolmgdet->sbd_cxx = symbol_bd_cxx_new ();
  g_signal_connect(SYMBOLIZABLE(symbolmgdet->sbd_cxx), "update", G_CALLBACK(sdb_symbolizable_update_cb), symbolmg);
  symbolmgdet->sbd_python = symbol_bd_python_new ();
  g_signal_connect(SYMBOLIZABLE(symbolmgdet->sbd_python), "update", G_CALLBACK(sdb_symbolizable_update_cb), symbolmg);
  symbolmgdet->sbd_perl = symbol_bd_perl_new ();
  g_signal_connect(SYMBOLIZABLE(symbolmgdet->sbd_perl), "update", G_CALLBACK(sdb_symbolizable_update_cb), symbolmg);
}

SymbolManager *symbol_manager_new (void)
{
	SymbolManager *symbolmg;
  symbolmg = g_object_new (SYMBOL_MANAGER_TYPE, NULL);

	return symbolmg; /* return new object */
}

static Symbolizable *symbol_manager_get_symbolizable_for_type(SymbolManager *symbolmg, gint ftype)
{
  SymbolManagerDetails *symbolmgdet;
	symbolmgdet = SYMBOL_MANAGER_GET_PRIVATE(symbolmg);

  Symbolizable *result;
  switch(ftype) {
    case TAB_PHP:
      result = SYMBOLIZABLE(symbolmgdet->sbd_php);
      break;
    case TAB_CSS:
      result = SYMBOLIZABLE(symbolmgdet->sbd_css);
      break;
    case TAB_SQL:
      result = SYMBOLIZABLE(symbolmgdet->sbd_sql);
      break;
    case TAB_COBOL:
      result = SYMBOLIZABLE(symbolmgdet->sbd_cobol);
      break;
    case TAB_CXX:
      result = SYMBOLIZABLE(symbolmgdet->sbd_cxx);
      break;
    case TAB_PYTHON:
      result = SYMBOLIZABLE(symbolmgdet->sbd_python);
      break;
    case TAB_PERL:
      result = SYMBOLIZABLE(symbolmgdet->sbd_perl);
      break;
    default:
      result = NULL;
      break;
  }
  return result;
}

gchar *symbol_manager_get_symbols_matches (SymbolManager *symbolmg, const gchar *symbol_prefix, gint flags, gint ftype)
{
  if(!symbolmg || !symbol_prefix) return NULL;
  Symbolizable *result = symbol_manager_get_symbolizable_for_type(symbolmg, ftype);
  if (!result) return NULL;
  return symbolizable_get_symbols_matches (result, symbol_prefix, flags);
}

gchar *symbol_manager_get_class_symbols (SymbolManager *symbolmg, const gchar *class_name, gint ftype)
{
  if(!symbolmg || !class_name) return NULL;
  Symbolizable *result = symbol_manager_get_symbolizable_for_type(symbolmg, ftype);
  if (!result) return NULL;
  return symbolizable_get_class_symbols (result, class_name);
}

gchar *symbol_manager_get_classes (SymbolManager *symbolmg, gint ftype)
{
  if(!symbolmg) return NULL;
  Symbolizable *result = symbol_manager_get_symbolizable_for_type(symbolmg, ftype);
  if (!result) return NULL;
  return symbolizable_get_classes (result);
}

gchar *symbol_manager_get_calltip (SymbolManager *symbolmg, const gchar *symbol_name, gint ftype)
{
  if(!symbolmg || !symbol_name) return NULL;
  Symbolizable *result = symbol_manager_get_symbolizable_for_type(symbolmg, ftype);
  if (!result) return NULL;
  return symbolizable_get_calltip (result, symbol_name);
}

GList *symbol_manager_get_custom_symbols_list (SymbolManager *symbolmg, gint symbol_type, gint ftype)
{
  if(!symbolmg) return NULL;
  Symbolizable *result = symbol_manager_get_symbolizable_for_type(symbolmg, ftype);
  if (!result) return NULL;
  return symbolizable_get_custom_symbols_list (result, symbol_type);
}

GList *symbol_manager_get_custom_symbols_list_by_filename (SymbolManager *symbolmg, gint symbol_type, gchar *filename, gint ftype)
{
  if(!symbolmg || !filename) return NULL;
  Symbolizable *result = symbol_manager_get_symbolizable_for_type(symbolmg, ftype);
  if (!result) return NULL;
  return symbolizable_get_custom_symbols_list_by_filename (result, symbol_type, filename);
}

void symbol_manager_rescan_file (SymbolManager *symbolmg, gchar *filename, gint ftype)
{
  if(!symbolmg || !filename) return ;
  Symbolizable *result = symbol_manager_get_symbolizable_for_type(symbolmg, ftype);
  if (!result) return ;
  symbolizable_rescan_file (result, filename);
}

void symbol_manager_purge_file (SymbolManager *symbolmg, gchar *filename, gint ftype)
{
  if(!symbolmg || !filename) return ;
  Symbolizable *result = symbol_manager_get_symbolizable_for_type(symbolmg, ftype);
  if (!result) return ;
  symbolizable_purge_file (result, filename);
}

void symbol_manager_add_file (SymbolManager *symbolmg, gchar *filename, gint ftype)
{
  if(!symbolmg || !filename) return ;
  Symbolizable *result = symbol_manager_get_symbolizable_for_type(symbolmg, ftype);
  if (!result) return ;
  symbolizable_add_file (result, filename);
}

