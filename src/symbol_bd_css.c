/* This file is part of gPHPEdit, a GNOME PHP Editor.

   Copyright (C) 2010 José Rostagno (for vijona.com.ar)

   For more information or to find the latest release, visit our 
   website at http://www.gphpedit.org/

   gCSSEdit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   gCSSEdit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with gCSSEdit. If not, see <http://www.gnu.org/licenses/>.

   The GNU General Public License is contained in the file COPYING.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "symbolizable.h"
#include "symbol_bd_css.h"
#include "symbol_bd_utils.h"

/*
* symbol_bd_css private struct
*/
struct SymbolBdCSSDetails
{
  /* API symbols list */
  GTree *css_api_tree;
  
  guint identifierid;

  gchar *completion_prefix;
  GString *completion_string;
  GHashTable *completion_list;
  GTree *completion_tree;

  /* cache items */
  char cache_str[200]; /* cached value */
  gchar *cache_completion; /* cached list*/
  gint cache_flags;
};

/*
 * symbol_bd_css_get_type
 * register SymbolBdCSS type and returns a new GType
*/

static void symbol_bd_css_class_init (SymbolBdCSSClass *klass);
static void symbol_bd_css_dispose (GObject *gobject);

#define SYMBOL_BD_CSS_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    SYMBOL_BD_CSS_TYPE,\
					    SymbolBdCSSDetails))

static void symbol_bd_css_symbolizable_init(SymbolizableIface *iface, gpointer user_data);

G_DEFINE_TYPE_WITH_CODE(SymbolBdCSS, symbol_bd_css, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE (IFACE_TYPE_SYMBOLIZABLE,
                                                 symbol_bd_css_symbolizable_init));

static gboolean make_result_string (gpointer key, gpointer value, gpointer user_data)
{
  gchar *function_name = (gchar *)value;
  SymbolBdCSSDetails *symbolbddet = (SymbolBdCSSDetails *) user_data;
  if (!symbolbddet->completion_string) {
    symbolbddet->completion_string = g_string_new(function_name);
  } else {
    symbolbddet->completion_string = g_string_append(symbolbddet->completion_string, " ");
    symbolbddet->completion_string = g_string_append(symbolbddet->completion_string, function_name);
  }
  return FALSE;
}

static gboolean add_api_item (gpointer key, gpointer value, gpointer user_data)
{
  SymbolBdCSSDetails *symbolbddet = (SymbolBdCSSDetails *) user_data;
  if (g_str_has_prefix(key, symbolbddet->completion_prefix)) {
    g_tree_insert (symbolbddet->completion_tree, key, g_strdup_printf("%s?2", (gchar *)key));
  }
  if (strncmp(key, symbolbddet->completion_prefix, MIN(strlen(key),strlen(symbolbddet->completion_prefix)))>0){
    return TRUE;
  }
  return FALSE;
}

gboolean symbol_bd_css_has_cache(gchar *cache_str, gchar *cache_completion, gint cache_flags, const gchar *symbol_prefix, gint flags)
{
  if (cache_flags != flags) return FALSE;
  gint len = strlen(cache_str);
  return (len !=0 && strlen(symbol_prefix) > len && g_str_has_prefix(symbol_prefix, cache_str));
}

static void symbol_bd_css_save_result_in_cache(SymbolBdCSSDetails *symbolbddet, gchar *result, const gchar *search_word)
{
    if (symbolbddet->cache_completion) g_free(symbolbddet->cache_completion);
    symbolbddet->cache_completion = g_strdup(result);
    strncpy(symbolbddet->cache_str, search_word, MIN(strlen(search_word),200));
}

static gchar *symbol_bd_css_get_symbols_matches (Symbolizable *self, const gchar *symbol_prefix, gint flags)
{
  gphpedit_debug (DEBUG_SYMBOLIZABLE);
  SymbolBdCSSDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_CSS_GET_PRIVATE(self);
  symbolbddet->completion_prefix = (gchar *) symbol_prefix;
  symbolbddet->completion_string = NULL;

  if (symbol_bd_css_has_cache(symbolbddet->cache_str, symbolbddet->cache_completion, symbolbddet->cache_flags, symbol_prefix, flags)){
    symbolbddet->completion_string = symbol_bd_get_autocomp_from_cache(symbolbddet->cache_str, symbolbddet->cache_completion, symbol_prefix);
  } else {
    symbolbddet->completion_tree = g_tree_new_full ((GCompareDataFunc) g_strcmp0, NULL, NULL,(GDestroyNotify) g_free);
    if (((flags & SYMBOL_ALL) == SYMBOL_ALL) || ((flags & SYMBOL_FUNCTION) == SYMBOL_FUNCTION)) {
      /* add api functions */
      g_tree_foreach (symbolbddet->css_api_tree, add_api_item, symbolbddet);
    }
    g_tree_foreach (symbolbddet->completion_tree, make_result_string, symbolbddet);
    g_tree_destroy (symbolbddet->completion_tree);
    if (symbolbddet->completion_string) symbol_bd_css_save_result_in_cache(symbolbddet, symbolbddet->completion_string->str, symbol_prefix);
  }

  if (symbolbddet->completion_string){
    gphpedit_debug_message(DEBUG_CLASSBROWSER, "prefix: %s autocomplete list:%s\n", symbol_prefix, symbolbddet->completion_string->str);
    return g_string_free(symbolbddet->completion_string, FALSE);
  }

  gphpedit_debug_message(DEBUG_CLASSBROWSER, "prefix: %s autocomplete list:%s\n", symbol_prefix, "null");
  return NULL;
}

static gchar *symbol_bd_css_get_class_symbols (Symbolizable *self, const gchar *class_name)
{
  gphpedit_debug (DEBUG_SYMBOLIZABLE);
  return NULL;
}

static gchar *symbol_bd_css_get_classes (Symbolizable *self)
{
  gphpedit_debug (DEBUG_SYMBOLIZABLE);
  return NULL;
}

static gchar *symbol_bd_css_get_calltip (Symbolizable *self, const gchar *symbol_name)
{
  gphpedit_debug (DEBUG_SYMBOLIZABLE);
  SymbolBdCSSDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_CSS_GET_PRIVATE(self);

  gchar *calltip = NULL;
  gchar *value = g_tree_lookup (symbolbddet->css_api_tree, symbol_name);
  if (value) {
    gchar *copy = g_strdup (value);
    /* replace | with \n */
    gchar *description = strstr(copy, "|");
    strncpy(description,"\n",1);
    /* make calltip */
    calltip = g_strdup_printf ("%s %s", symbol_name, copy);

    gphpedit_debug_message(DEBUG_CLASSBROWSER, "calltip: %s\n", calltip);

    g_free(copy);
  }
  return calltip;
}

static GList *symbol_bd_css_get_custom_symbols_list (Symbolizable *self, gint flags)
{
  gphpedit_debug (DEBUG_SYMBOLIZABLE);
  return NULL;
}

static GList *symbol_bd_css_get_custom_symbols_list_by_filename (Symbolizable *self, gint symbol_type, gchar *filename)
{
  gphpedit_debug (DEBUG_SYMBOLIZABLE);
  return NULL;
}

static void symbol_bd_css_rescan_file (Symbolizable *self, gchar *filename)
{
  gphpedit_debug (DEBUG_SYMBOLIZABLE);
}

static void symbol_bd_css_purge_file (Symbolizable *self, gchar *filename)
{
  gphpedit_debug (DEBUG_SYMBOLIZABLE);
}

static void symbol_bd_css_add_file (Symbolizable *self, gchar *filename)
{
  gphpedit_debug (DEBUG_SYMBOLIZABLE);
}

static void symbol_bd_css_symbolizable_init(SymbolizableIface *iface, gpointer user_data)
{
  iface->get_symbols_matches = symbol_bd_css_get_symbols_matches;
  iface->get_class_symbols = symbol_bd_css_get_class_symbols;
  iface->get_classes = symbol_bd_css_get_classes;
  iface->get_calltip = symbol_bd_css_get_calltip;
  iface->get_custom_symbols_list = symbol_bd_css_get_custom_symbols_list;
  iface->get_custom_symbols_list_by_filename = symbol_bd_css_get_custom_symbols_list_by_filename;
  iface->rescan_file = symbol_bd_css_rescan_file;
  iface->purge_file = symbol_bd_css_purge_file;
  iface->add_file = symbol_bd_css_add_file;
}

void
symbol_bd_css_class_init (SymbolBdCSSClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = symbol_bd_css_dispose;

	g_type_class_add_private (klass, sizeof (SymbolBdCSSDetails));
}

void
symbol_bd_css_init (SymbolBdCSS *symbolbd)
{
  SymbolBdCSSDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_CSS_GET_PRIVATE(symbolbd);

  symbolbddet->identifierid = 0;

  /* init API tables */
  symbol_bd_load_api_file("css.api", &symbolbddet->css_api_tree);
}

/*
* disposes the Gobject
*/
void symbol_bd_css_dispose (GObject *object)
{
  SymbolBdCSS *symbolbd = SYMBOL_BD_CSS(object);
  SymbolBdCSSDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_CSS_GET_PRIVATE(symbolbd);

  if (symbolbddet->css_api_tree) g_tree_destroy(symbolbddet->css_api_tree);

  if (symbolbddet->cache_completion) g_free(symbolbddet->cache_completion);

  /* Chain up to the parent class */
  G_OBJECT_CLASS (symbol_bd_css_parent_class)->dispose (object);
}

SymbolBdCSS *symbol_bd_css_new (void)
{
	SymbolBdCSS *symbolbd;
  symbolbd = g_object_new (SYMBOL_BD_CSS_TYPE, NULL);

	return symbolbd; /* return new object */
}
