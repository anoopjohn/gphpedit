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

#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "symbolizable.h"
#include "symbol_bd_php.h"
#include "symbol_bd_utils.h"
#include "symbol_bd_php_parse.h"


/*
* symbol_bd_php private struct
*/
struct SymbolBdPHPDetails
{
  /* API symbols list */
  GTree *php_api_tree;
  /* custom symbols lists */
  GHashTable *php_function_tree;
  GHashTable *php_variables_tree;
  GHashTable *php_class_tree;

  guint identifierid;
  /* file table */
  GHashTable *db_file_table;

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
 * symbol_bd_php_get_type
 * register SymbolBdPHP type and returns a new GType
*/

static void symbol_bd_php_class_init (SymbolBdPHPClass *klass);
static void symbol_bd_php_dispose (GObject *gobject);
static void symbol_bd_php_do_parse_file(SymbolBdPHP *symbolbd, const gchar *filename);
static void symbol_bd_php_add_global_var(SymbolBdPHPDetails *symbolbddet, const gchar *var_name);
static gchar *symbol_bd_php_custom_function_calltip(SymbolBdPHPDetails *symbolbddet, const gchar *function_name);

#define SYMBOL_BD_PHP_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    SYMBOL_BD_PHP_TYPE,\
					    SymbolBdPHPDetails))

static void symbol_bd_php_symbolizable_init(SymbolizableIface *iface, gpointer user_data);

G_DEFINE_TYPE_WITH_CODE(SymbolBdPHP, symbol_bd_php, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE (IFACE_TYPE_SYMBOLIZABLE,
                                                 symbol_bd_php_symbolizable_init));

static gboolean make_result_string (gpointer key, gpointer value, gpointer user_data)
{
  gchar *function_name = (gchar *)value;
  SymbolBdPHPDetails *symbolbddet = (SymbolBdPHPDetails *) user_data;
  if (!symbolbddet->completion_string) {
    symbolbddet->completion_string = g_string_new(function_name);
  } else {
    symbolbddet->completion_string = g_string_append(symbolbddet->completion_string, " ");
    symbolbddet->completion_string = g_string_append(symbolbddet->completion_string, function_name);
  }
  return FALSE;
}

static void add_result_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserFunction *function = (ClassBrowserFunction *) value;
  SymbolBdPHPDetails *symbolbddet = (SymbolBdPHPDetails *) user_data;
  if (g_str_has_prefix(function->functionname, symbolbddet->completion_prefix)) {
      g_tree_insert (symbolbddet->completion_tree, key, g_strdup_printf("%s?1",function->functionname));
  }
}

static void add_member_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserFunction *function = (ClassBrowserFunction *) value;
  SymbolBdPHPDetails *symbolbddet = (SymbolBdPHPDetails *) user_data;
  if (function->classname!=NULL && g_str_has_prefix(function->functionname, symbolbddet->completion_prefix)) {
      g_tree_insert (symbolbddet->completion_tree, key, g_strdup_printf("%s?1",function->functionname));
  }
}

static gboolean add_api_item (gpointer key, gpointer value, gpointer user_data)
{
  SymbolBdPHPDetails *symbolbddet = (SymbolBdPHPDetails *) user_data;
  if (g_str_has_prefix(key, symbolbddet->completion_prefix)) {
      g_tree_insert (symbolbddet->completion_tree, key, g_strdup_printf("%s?2", (gchar *)key));
  }
  if (strncmp(key, symbolbddet->completion_prefix, MIN(strlen(key),strlen(symbolbddet->completion_prefix)))>0){
    return TRUE;
  }
  return FALSE;
}

static void add_class_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserClass *class = (ClassBrowserClass *) value;
  SymbolBdPHPDetails *symbolbddet = (SymbolBdPHPDetails *) user_data;
  if (g_str_has_prefix(class->classname, symbolbddet->completion_prefix)) {
      g_tree_insert (symbolbddet->completion_tree, key, g_strdup_printf("%s?4",class->classname));
  }
}

static void add_var_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserVar *var = (ClassBrowserVar *) value;
  SymbolBdPHPDetails *symbolbddet = (SymbolBdPHPDetails *) user_data;
  if (g_str_has_prefix(var->varname, symbolbddet->completion_prefix)) {
      g_tree_insert (symbolbddet->completion_tree, key, g_strdup_printf("%s?3",var->varname));
  }
}

gboolean symbol_bd_php_has_cache(gchar *cache_str, gchar *cache_completion, gint cache_flags, const gchar *symbol_prefix, gint flags)
{
  if (cache_flags != flags) return FALSE;
  gint len = strlen(cache_str);
  return (len !=0 && strlen(symbol_prefix) > len && g_str_has_prefix(symbol_prefix, cache_str));
}

static void symbol_bd_php_save_result_in_cache(SymbolBdPHPDetails *symbolbddet, gchar *result, const gchar *search_word)
{
    if (symbolbddet->cache_completion) g_free(symbolbddet->cache_completion);
    symbolbddet->cache_completion = g_strdup(result);
    strncpy(symbolbddet->cache_str, search_word, MIN(strlen(search_word),200));
}

static gchar *symbol_bd_php_get_symbols_matches (Symbolizable *self, const gchar *symbol_prefix, gint flags)
{
  gphpedit_debug (DEBUG_SYMBOLIZABLE);
  SymbolBdPHPDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_PHP_GET_PRIVATE(self);
  symbolbddet->completion_prefix = (gchar *) symbol_prefix;
  symbolbddet->completion_string = NULL;

  if (symbol_bd_php_has_cache(symbolbddet->cache_str, symbolbddet->cache_completion, symbolbddet->cache_flags, symbol_prefix, flags)){
    symbolbddet->completion_string = symbol_bd_get_autocomp_from_cache(symbolbddet->cache_str, symbolbddet->cache_completion, symbol_prefix);
  } else {
    symbolbddet->completion_tree = g_tree_new_full ((GCompareDataFunc) g_strcmp0, NULL, NULL,(GDestroyNotify) g_free);
    if (((flags & SYMBOL_ALL) == SYMBOL_ALL) || ((flags & SYMBOL_FUNCTION) == SYMBOL_FUNCTION)) {
      g_hash_table_foreach (symbolbddet->php_function_tree, add_result_item, symbolbddet);
      /* add api functions */
      g_tree_foreach (symbolbddet->php_api_tree, add_api_item, symbolbddet);
    }

    if (((flags & SYMBOL_MEMBER) == SYMBOL_MEMBER) && !((flags & SYMBOL_FUNCTION) == SYMBOL_FUNCTION)) {
      g_hash_table_foreach (symbolbddet->php_function_tree, add_member_item, symbolbddet);
    }
    if (((flags & SYMBOL_ALL) == SYMBOL_ALL) || ((flags & SYMBOL_CLASS) == SYMBOL_CLASS)) {
      g_hash_table_foreach (symbolbddet->php_class_tree, add_class_item, symbolbddet);
    }
    if (((flags & SYMBOL_ALL) == SYMBOL_ALL) || ((flags & SYMBOL_VAR) == SYMBOL_VAR)) {
      g_hash_table_foreach (symbolbddet->php_variables_tree, add_var_item, symbolbddet);
    }

    g_tree_foreach (symbolbddet->completion_tree, make_result_string, symbolbddet);
    g_tree_destroy (symbolbddet->completion_tree);
    if (symbolbddet->completion_string) symbol_bd_php_save_result_in_cache(symbolbddet, symbolbddet->completion_string->str, symbol_prefix);
  }

  if (symbolbddet->completion_string){
    gphpedit_debug_message(DEBUG_CLASSBROWSER, "prefix: %s autocomplete list:%s\n", symbol_prefix, symbolbddet->completion_string->str);
    return g_string_free(symbolbddet->completion_string, FALSE);
  }

  gphpedit_debug_message(DEBUG_CLASSBROWSER, "prefix: %s autocomplete list:%s\n", symbol_prefix, "null");
  return NULL;
}

static void make_result_member_string (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserFunction *function = (ClassBrowserFunction *) value;
  SymbolBdPHPDetails *symbolbddet = (SymbolBdPHPDetails *) user_data;

  if (g_str_has_prefix(function->functionname, symbolbddet->completion_prefix)) {
    if (!symbolbddet->completion_string) {
      symbolbddet->completion_string = g_string_new(function->functionname);
      symbolbddet->completion_string = g_string_append(symbolbddet->completion_string, "?1");
    } else {
       g_string_append_printf(symbolbddet->completion_string, " %s?1", function->functionname);
    }
  }
}

static gchar *symbol_bd_php_get_class_symbols (Symbolizable *self, const gchar *class_name)
{
  gphpedit_debug (DEBUG_SYMBOLIZABLE);
  SymbolBdPHPDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_PHP_GET_PRIVATE(self);
  symbolbddet->completion_string = NULL;
  symbolbddet->completion_prefix = (gchar *) class_name;

  g_hash_table_foreach (symbolbddet->php_function_tree, make_result_member_string, symbolbddet);
  if (symbolbddet->completion_string){
    gphpedit_debug_message(DEBUG_CLASSBROWSER, "prefix: %s autocomplete list:%s\n", class_name, symbolbddet->completion_string->str);
    return g_string_free(symbolbddet->completion_string, FALSE);
  }

  gphpedit_debug_message(DEBUG_CLASSBROWSER, "prefix: %s autocomplete list:%s\n", class_name, "null");
  return NULL;
}

static void make_class_completion_string (gpointer key, gpointer value, gpointer data)
{
  SymbolBdPHPDetails *symbolbddet = (SymbolBdPHPDetails *) data;
  ClassBrowserClass *class;
  class=(ClassBrowserClass *)value;

  if (!symbolbddet->completion_string) {
    symbolbddet->completion_string = g_string_new(g_strchug(class->classname));
    symbolbddet->completion_string = g_string_append(symbolbddet->completion_string, "?4"); /* add corresponding image*/
  } else {
    g_string_append_printf (symbolbddet->completion_string," %s?4", g_strchug(class->classname)); /* add corresponding image*/
  }
}

static gchar *symbol_bd_php_get_classes (Symbolizable *self)
{
  gphpedit_debug (DEBUG_SYMBOLIZABLE);
  SymbolBdPHPDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_PHP_GET_PRIVATE(self);
  symbolbddet->completion_string = NULL;

  g_hash_table_foreach (symbolbddet->php_class_tree, make_class_completion_string, symbolbddet);

  if (!symbolbddet->completion_string) return NULL;
  return g_string_free(symbolbddet->completion_string, FALSE);
}

static gchar *symbol_bd_php_get_calltip (Symbolizable *self, const gchar *symbol_name)
{
  gphpedit_debug (DEBUG_SYMBOLIZABLE);
  SymbolBdPHPDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_PHP_GET_PRIVATE(self);

  gchar *return_value;
  gchar *params;
  gchar *description;
  gchar *token_line, *copy_line;
  gchar *result = NULL;
  gchar *value = g_tree_lookup (symbolbddet->php_api_tree, symbol_name);
  if (value){
    token_line = g_strdup (value);
    copy_line = token_line;
    return_value = strtok(token_line, "|");
    params = strtok(NULL, "|");
    description = strtok(NULL, "|");
    /* make calltip */
    result = g_strdup_printf ("%s %s %s\n%s", return_value, symbol_name, params, description);
    gphpedit_debug_message(DEBUG_SYMBOLIZABLE, "calltip: %s\n",result);
    g_free(copy_line);	
  } else {
    /*maybe a custom function*/
      result = symbol_bd_php_custom_function_calltip(symbolbddet, symbol_name);
  }
  return result;
}

static GList *symbol_bd_php_get_custom_symbols_list (Symbolizable *self, gint flags)
{
  gphpedit_debug (DEBUG_SYMBOLIZABLE);
  SymbolBdPHPDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_PHP_GET_PRIVATE(self);
  if (((flags & SYMBOL_FUNCTION) == SYMBOL_FUNCTION)) {
    return g_hash_table_get_values (symbolbddet->php_function_tree);
  }
  if (((flags & SYMBOL_CLASS) == SYMBOL_CLASS)) {
    return g_hash_table_get_values (symbolbddet->php_class_tree);
  }
  if (((flags & SYMBOL_VAR) == SYMBOL_VAR)) {
    return g_hash_table_get_values (symbolbddet->php_variables_tree);
  }
  return NULL;
}

static void add_custom_function_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserFunction *function = (ClassBrowserFunction *) value;
  SymbolBdPHPDetails *symbolbddet = (SymbolBdPHPDetails *) user_data;
  if (g_strcmp0(function->filename, symbolbddet->completion_prefix) == 0 ) {
    g_hash_table_insert (symbolbddet->completion_list, key, function);
  }
}

static void add_custom_class_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserClass *class = (ClassBrowserClass *) value;
  SymbolBdPHPDetails *symbolbddet = (SymbolBdPHPDetails *) user_data;
  if (g_strcmp0(class->filename, symbolbddet->completion_prefix)==0) {
    g_hash_table_insert (symbolbddet->completion_list, key, class);
  }
}

static void add_custom_var_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserVar *var = (ClassBrowserVar *) value;
  SymbolBdPHPDetails *symbolbddet = (SymbolBdPHPDetails *) user_data;
  if (g_strcmp0(var->filename, symbolbddet->completion_prefix)==0) {
    g_hash_table_insert (symbolbddet->completion_list, key, var);
  }
}

static GList *symbol_bd_php_get_custom_symbols_list_by_filename (Symbolizable *self, gint symbol_type, gchar *filename)
{
  gphpedit_debug (DEBUG_SYMBOLIZABLE);
  SymbolBdPHPDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_PHP_GET_PRIVATE(self);
  if(!filename) return NULL;
  g_return_val_if_fail(self, NULL);
  if (!g_hash_table_lookup (symbolbddet->db_file_table, filename)) return NULL;
  
  symbolbddet->completion_list = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, NULL);
  symbolbddet->completion_prefix = filename;
  if (((symbol_type & SYMBOL_FUNCTION) == SYMBOL_FUNCTION)) {
    g_hash_table_foreach (symbolbddet->php_function_tree, add_custom_function_item, symbolbddet);
  }
  if (((symbol_type & SYMBOL_CLASS) == SYMBOL_CLASS)) {
    g_hash_table_foreach (symbolbddet->php_class_tree, add_custom_class_item, symbolbddet);
  }
  if (((symbol_type & SYMBOL_VAR) == SYMBOL_VAR)) {
    g_hash_table_foreach (symbolbddet->php_variables_tree, add_custom_var_item, symbolbddet);
  }
  GList *result = g_hash_table_get_values (symbolbddet->completion_list);
  g_hash_table_destroy (symbolbddet->completion_list);

  return result;
}

static void symbol_bd_php_rescan_file (Symbolizable *self, gchar *filename)
{
  gphpedit_debug (DEBUG_SYMBOLIZABLE);
  symbolizable_purge_file (self, filename);
  symbolizable_add_file (self, filename);
}

static void remove_custom_function_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserFunction *function = (ClassBrowserFunction *) value;
  SymbolBdPHPDetails *symbolbddet = (SymbolBdPHPDetails *) user_data;
  if (g_strcmp0(function->filename, symbolbddet->completion_prefix) == 0) {
    g_hash_table_remove (symbolbddet->php_function_tree, key);
  }
}

static void remove_custom_class_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserClass *class = (ClassBrowserClass *) value;
  SymbolBdPHPDetails *symbolbddet = (SymbolBdPHPDetails *) user_data;
  if (g_strcmp0(class->filename, symbolbddet->completion_prefix)==0) {
    g_hash_table_remove (symbolbddet->php_class_tree, key);
  }
}

static void remove_custom_var_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserVar *var = (ClassBrowserVar *) value;
  SymbolBdPHPDetails *symbolbddet = (SymbolBdPHPDetails *) user_data;
  if (g_strcmp0(var->filename, symbolbddet->completion_prefix)==0) {
    g_hash_table_remove (symbolbddet->php_variables_tree, key);
  }
}

static void symbol_bd_php_purge_file (Symbolizable *self, gchar *filename)
{
  gphpedit_debug (DEBUG_SYMBOLIZABLE);
  SymbolBdPHPDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_PHP_GET_PRIVATE(self);
  if(!filename) return ;
  g_return_if_fail(self);

  if (!g_hash_table_remove (symbolbddet->db_file_table, filename)) return ;

  symbolbddet->completion_prefix = filename;

  g_hash_table_foreach (symbolbddet->php_function_tree, remove_custom_function_item, symbolbddet);
  g_hash_table_foreach (symbolbddet->php_class_tree, remove_custom_class_item, symbolbddet);
  g_hash_table_foreach (symbolbddet->php_variables_tree, remove_custom_var_item, symbolbddet);
}

/*
* symbol_bd_php_add_file:
* search file in file table, if file isn't found add it and scan it for symbols otherwise
* rescan the file.
*/
static void symbol_bd_php_add_file (Symbolizable *self, gchar *filename)
{
  gphpedit_debug (DEBUG_SYMBOLIZABLE);
  SymbolBdPHPDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_PHP_GET_PRIVATE(self);
  if(!filename) return ;
  g_return_if_fail(self);

  if (g_hash_table_lookup (symbolbddet->db_file_table, filename)){
    symbol_bd_php_rescan_file (self, filename);
  } else {
    /* add file to table */
    g_hash_table_insert (symbolbddet->db_file_table, g_strdup(filename), g_strdup(filename));
    /* scan file for symbols */
    symbol_bd_php_do_parse_file(SYMBOL_BD_PHP(self), filename);
  }
}

static void symbol_bd_php_symbolizable_init(SymbolizableIface *iface, gpointer user_data)
{
  iface->get_symbols_matches = symbol_bd_php_get_symbols_matches;
  iface->get_class_symbols = symbol_bd_php_get_class_symbols;
  iface->get_classes = symbol_bd_php_get_classes;
  iface->get_calltip = symbol_bd_php_get_calltip;
  iface->get_custom_symbols_list = symbol_bd_php_get_custom_symbols_list;
  iface->get_custom_symbols_list_by_filename = symbol_bd_php_get_custom_symbols_list_by_filename;
  iface->rescan_file = symbol_bd_php_rescan_file;
  iface->purge_file = symbol_bd_php_purge_file;
  iface->add_file = symbol_bd_php_add_file;
}

void
symbol_bd_php_class_init (SymbolBdPHPClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = symbol_bd_php_dispose;

	g_type_class_add_private (klass, sizeof (SymbolBdPHPDetails));
}

void
symbol_bd_php_init (SymbolBdPHP *symbolbd)
{
  SymbolBdPHPDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_PHP_GET_PRIVATE(symbolbd);

  symbolbddet->identifierid = 0;

  /* init API tables */
  symbol_bd_load_api_file("php-gphpedit.api", &symbolbddet->php_api_tree);

  /*init file table */
  symbolbddet->db_file_table = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  /* custom symbols tables */
  symbolbddet->php_function_tree= g_hash_table_new_full (g_str_hash, g_str_equal, g_free, free_function_item);
  symbolbddet->php_class_tree= g_hash_table_new_full (g_str_hash, g_str_equal, g_free, free_class_item);
  symbolbddet->php_variables_tree = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, free_variable_item);

  /* add php global vars*/
  symbol_bd_php_add_global_var(symbolbddet, "$GLOBALS");
  symbol_bd_php_add_global_var(symbolbddet, "$HTTP_POST_VARS");
  symbol_bd_php_add_global_var(symbolbddet, "$HTTP_RAW_POST_DATA");
  symbol_bd_php_add_global_var(symbolbddet, "$http_response_header");
  symbol_bd_php_add_global_var(symbolbddet, "$this");
  symbol_bd_php_add_global_var(symbolbddet, "$_COOKIE");
  symbol_bd_php_add_global_var(symbolbddet, "$_POST");
  symbol_bd_php_add_global_var(symbolbddet, "$_REQUEST");
  symbol_bd_php_add_global_var(symbolbddet, "$_SERVER");
  symbol_bd_php_add_global_var(symbolbddet, "$_SESSION");
  symbol_bd_php_add_global_var(symbolbddet, "$_GET");
  symbol_bd_php_add_global_var(symbolbddet, "$_FILES");
  symbol_bd_php_add_global_var(symbolbddet, "$_ENV");
  symbol_bd_php_add_global_var(symbolbddet, "__CLASS__");
  symbol_bd_php_add_global_var(symbolbddet, "__DIR__");
  symbol_bd_php_add_global_var(symbolbddet, "__FILE__");
  symbol_bd_php_add_global_var(symbolbddet, "__FUNCTION__");
  symbol_bd_php_add_global_var(symbolbddet, "__METHOD__");
  symbol_bd_php_add_global_var(symbolbddet, "__NAMESPACE__");
}

/*
* disposes the Gobject
*/
void symbol_bd_php_dispose (GObject *object)
{
  SymbolBdPHP *symbolbd = SYMBOL_BD_PHP(object);
  SymbolBdPHPDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_PHP_GET_PRIVATE(symbolbd);

  if (symbolbddet->php_api_tree) g_tree_destroy(symbolbddet->php_api_tree);

  if (symbolbddet->php_function_tree) g_hash_table_destroy (symbolbddet->php_function_tree);
  if (symbolbddet->php_variables_tree) g_hash_table_destroy (symbolbddet->php_variables_tree);
  if (symbolbddet->php_class_tree) g_hash_table_destroy (symbolbddet->php_class_tree);

  if (symbolbddet->db_file_table) g_hash_table_destroy (symbolbddet->db_file_table);

  if (symbolbddet->cache_completion) g_free(symbolbddet->cache_completion);

  /* Chain up to the parent class */
  G_OBJECT_CLASS (symbol_bd_php_parent_class)->dispose (object);
}

SymbolBdPHP *symbol_bd_php_new (void)
{
	SymbolBdPHP *symbolbd;
  symbolbd = g_object_new (SYMBOL_BD_PHP_TYPE, NULL);

	return symbolbd; /* return new object */
}

static void symbol_bd_php_add_global_var(SymbolBdPHPDetails *symbolbddet, const gchar *var_name)
{
  ClassBrowserVar *var;
    var = g_slice_new(ClassBrowserVar);
    var->varname = g_strdup(var_name);
    var->functionname = NULL; /* NULL for global variables*/
    var->filename = NULL; /*NULL FOR PHP GLOBAL VARIABLES*/
    var->remove = FALSE;
    var->identifierid = symbolbddet->identifierid++;
    g_hash_table_insert (symbolbddet->php_variables_tree, g_strdup(var_name), var); /* key = variables name value var struct */
    gphpedit_debug_message(DEBUG_CLASSBROWSER, "%s\n", var_name);
}

/* scan a file for symbols */
static void symbol_bd_php_do_parse_file(SymbolBdPHP *symbolbd, const gchar *filename) {
  g_return_if_fail(filename);
  SymbolBdPHPDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_PHP_GET_PRIVATE(symbolbd);
  gphpedit_debug_message(DEBUG_CLASSBROWSER, "Parsing: %s\n", filename);
  classbrowser_parse_file(symbolbd, (gchar *) filename);
}

void symbol_bd_php_varlist_add(SymbolBdPHP *symbolbd, gchar *varname, gchar *funcname, gchar *filename)
{
  SymbolBdPHPDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_PHP_GET_PRIVATE(symbolbd);
  gphpedit_debug_message(DEBUG_CLASSBROWSER, "filename: %s var name: %s\n", filename, varname);
  symbol_bd_varlist_add(&symbolbddet->php_variables_tree, varname, funcname, filename, &symbolbddet->identifierid);
}

void symbol_bd_php_classlist_add(SymbolBdPHP *symbolbd, gchar *classname, gchar *filename, gint line_number)
{
  SymbolBdPHPDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_PHP_GET_PRIVATE(symbolbd);
  symbol_bd_classlist_add(&symbolbddet->php_class_tree, classname, filename, line_number, &symbolbddet->identifierid);
  gphpedit_debug_message(DEBUG_CLASSBROWSER,"filename: %s class name: %s\n", filename, classname);
}

void symbol_bd_php_functionlist_add(SymbolBdPHP *symbolbd, gchar *classname, gchar *funcname, gchar *filename, guint line_number, gchar *param_list)
{
  SymbolBdPHPDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_PHP_GET_PRIVATE(symbolbd);
  symbol_bd_functionlist_add(&symbolbddet->php_function_tree, &symbolbddet->php_class_tree, classname, funcname, filename, 
                               line_number, param_list, &symbolbddet->identifierid);
  gphpedit_debug_message(DEBUG_CLASSBROWSER,"filename: %s fucntion: %s\n", filename, funcname);
}

static gboolean get_custom_calltip (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserFunction *function = (ClassBrowserFunction *) value;
  SymbolBdPHPDetails *symbolbddet = (SymbolBdPHPDetails *) user_data;

  return (g_utf8_collate(function->functionname, symbolbddet->completion_prefix)==0);
}

static gchar *symbol_bd_php_custom_function_calltip(SymbolBdPHPDetails *symbolbddet, const gchar *function_name)
{
/*FIXME::two functions diferent classes same name = bad calltip */
  ClassBrowserFunction *function;
  gchar *calltip = NULL;
  symbolbddet->completion_prefix = (gchar *)function_name;
  function = g_hash_table_find (symbolbddet->php_function_tree, get_custom_calltip, symbolbddet);
  if (function) {
    calltip = g_strdup_printf("%s (%s)", function->functionname, function->paramlist);
  }
  gphpedit_debug_message(DEBUG_CLASSBROWSER,"custom calltip: %s\n", calltip);
  return calltip;
}

