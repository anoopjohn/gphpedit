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
#include "symbol_bd_cxx.h"
#include "symbol_bd_utils.h"
#include "gvfs_utils.h"


/*
* symbol_bd_cxx private struct
*/
struct SymbolBdCXXDetails
{
  /* API symbols list */
  GTree *cxx_api_tree;
  /* custom symbols lists */
  GHashTable *cxx_function_tree;
  GHashTable *cxx_variables_tree;
  GHashTable *cxx_class_tree;

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
 * symbol_bd_cxx_get_type
 * register SymbolBdCXX type and returns a new GType
*/

static void symbol_bd_cxx_class_init (SymbolBdCXXClass *klass);
static void symbol_bd_cxx_dispose (GObject *gobject);
static void symbol_bd_cxx_do_parse_file(SymbolBdCXX *symbolbd, const gchar *filename);
static gchar *symbol_bd_cxx_custom_function_calltip(SymbolBdCXXDetails *symbolbddet, const gchar *function_name);
void symbol_bd_cxx_functionlist_add(SymbolBdCXX *symbolbd, gchar *classname, 
            gchar *funcname, const gchar *filename, guint line_number, gchar *param_list);
void symbol_bd_cxx_classlist_add(SymbolBdCXX *symbolbd, gchar *classname, 
            const gchar *filename, gint line_number);
void symbol_bd_cxx_varlist_add(SymbolBdCXX *symbolbd, gchar *varname, 
            gchar *funcname, const gchar *filename);

#define SYMBOL_BD_CXX_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    SYMBOL_BD_CXX_TYPE,\
					    SymbolBdCXXDetails))

static void symbol_bd_cxx_symbolizable_init(SymbolizableIface *iface, gpointer user_data);

G_DEFINE_TYPE_WITH_CODE(SymbolBdCXX, symbol_bd_cxx, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE (IFACE_TYPE_SYMBOLIZABLE,
                                                 symbol_bd_cxx_symbolizable_init));

static gboolean make_result_string (gpointer key, gpointer value, gpointer user_data)
{
  gchar *function_name = (gchar *)value;
  SymbolBdCXXDetails *symbolbddet = (SymbolBdCXXDetails *) user_data;
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
  SymbolBdCXXDetails *symbolbddet = (SymbolBdCXXDetails *) user_data;
  if (g_str_has_prefix(function->functionname, symbolbddet->completion_prefix)) {
    g_tree_insert (symbolbddet->completion_tree, key, g_strdup_printf("%s?1",function->functionname));
  }
}

static gboolean add_api_item (gpointer key, gpointer value, gpointer user_data)
{
  SymbolBdCXXDetails *symbolbddet = (SymbolBdCXXDetails *) user_data;
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
  SymbolBdCXXDetails *symbolbddet = (SymbolBdCXXDetails *) user_data;
  if (g_str_has_prefix(class->classname, symbolbddet->completion_prefix)) {
    g_tree_insert (symbolbddet->completion_tree, key, g_strdup_printf("%s?4",class->classname));
  }
}

static void add_var_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserVar *var = (ClassBrowserVar *) value;
  SymbolBdCXXDetails *symbolbddet = (SymbolBdCXXDetails *) user_data;
  if (g_str_has_prefix(var->varname, symbolbddet->completion_prefix)) {
    g_tree_insert (symbolbddet->completion_tree, key, g_strdup_printf("%s?3",var->varname));
  }
}

static void add_member_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserFunction *function = (ClassBrowserFunction *) value;
  SymbolBdCXXDetails *symbolbddet = (SymbolBdCXXDetails *) user_data;
  if (function->classname!=NULL && g_str_has_prefix(function->functionname, symbolbddet->completion_prefix)) {
      g_tree_insert (symbolbddet->completion_tree, key, g_strdup_printf("%s?1",function->functionname));
  }
}

gboolean symbol_bd_cxx_has_cache(gchar *cache_str, gchar *cache_completion, gint cache_flags, const gchar *symbol_prefix, gint flags)
{
  if (cache_flags != flags) return FALSE;
  gint len = strlen(cache_str);
  return (len !=0 && strlen(symbol_prefix) > len && g_str_has_prefix(symbol_prefix, cache_str));
}

static void symbol_bd_cxx_save_result_in_cache(SymbolBdCXXDetails *symbolbddet, gchar *result, const gchar *search_word)
{
    if (symbolbddet->cache_completion) g_free(symbolbddet->cache_completion);
    symbolbddet->cache_completion = g_strdup(result);
    strncpy(symbolbddet->cache_str, search_word, MIN(strlen(search_word),200));
}

static gchar *symbol_bd_cxx_get_symbols_matches (Symbolizable *self, const gchar *symbol_prefix, gint flags)
{
  SymbolBdCXXDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_CXX_GET_PRIVATE(self);
  symbolbddet->completion_prefix = (gchar *) symbol_prefix;
  symbolbddet->completion_string = NULL;

  if (symbol_bd_cxx_has_cache(symbolbddet->cache_str, symbolbddet->cache_completion, symbolbddet->cache_flags, symbol_prefix, flags)){
    symbolbddet->completion_string = symbol_bd_get_autocomp_from_cache(symbolbddet->cache_str, symbolbddet->cache_completion, symbol_prefix);
  } else {
    symbolbddet->completion_tree = g_tree_new_full ((GCompareDataFunc) g_strcmp0, NULL, NULL,(GDestroyNotify) g_free);

    if (((flags & SYMBOL_ALL) == SYMBOL_ALL) || ((flags & SYMBOL_FUNCTION) == SYMBOL_FUNCTION)) {
      g_hash_table_foreach (symbolbddet->cxx_function_tree, add_result_item, symbolbddet);
      /* add api functions */
      g_tree_foreach (symbolbddet->cxx_api_tree, add_api_item, symbolbddet);
    }
    if (((flags & SYMBOL_MEMBER) == SYMBOL_MEMBER) && !((flags & SYMBOL_FUNCTION) == SYMBOL_FUNCTION)) {
      g_hash_table_foreach (symbolbddet->cxx_function_tree, add_member_item, symbolbddet);
    }
    if (((flags & SYMBOL_ALL) == SYMBOL_ALL) || ((flags & SYMBOL_CLASS) == SYMBOL_CLASS)) {
      g_hash_table_foreach (symbolbddet->cxx_class_tree, add_class_item, symbolbddet);
    }
    if (((flags & SYMBOL_ALL) == SYMBOL_ALL) || ((flags & SYMBOL_VAR) == SYMBOL_VAR)) {
      g_hash_table_foreach (symbolbddet->cxx_variables_tree, add_var_item, symbolbddet);
    }

    g_tree_foreach (symbolbddet->completion_tree, make_result_string, symbolbddet);
    g_tree_destroy (symbolbddet->completion_tree);
    if (symbolbddet->completion_string) symbol_bd_cxx_save_result_in_cache(symbolbddet, symbolbddet->completion_string->str, symbol_prefix);
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
  SymbolBdCXXDetails *symbolbddet = (SymbolBdCXXDetails *) user_data;

  if (g_str_has_prefix(function->functionname, symbolbddet->completion_prefix)) {
    if (!symbolbddet->completion_string) {
      symbolbddet->completion_string = g_string_new(function->functionname);
      symbolbddet->completion_string = g_string_append(symbolbddet->completion_string, "?1");
    } else {
       g_string_append_printf(symbolbddet->completion_string, " %s?1", function->functionname);
    }
  }
}

static gchar *symbol_bd_cxx_get_class_symbols (Symbolizable *self, const gchar *class_name)
{
  SymbolBdCXXDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_CXX_GET_PRIVATE(self);
  symbolbddet->completion_string = NULL;
  symbolbddet->completion_prefix = (gchar *) class_name;

  g_hash_table_foreach (symbolbddet->cxx_function_tree, make_result_member_string, symbolbddet);
  if (symbolbddet->completion_string){
    gphpedit_debug_message(DEBUG_CLASSBROWSER, "prefix: %s autocomplete list:%s\n", class_name, symbolbddet->completion_string->str);
    return g_string_free(symbolbddet->completion_string, FALSE);
  }

  gphpedit_debug_message(DEBUG_CLASSBROWSER, "prefix: %s autocomplete list:%s\n", class_name, "null");
  return NULL;
}

static void make_class_completion_string (gpointer key, gpointer value, gpointer data)
{
  SymbolBdCXXDetails *symbolbddet = (SymbolBdCXXDetails *) data;
  ClassBrowserClass *class;
  class=(ClassBrowserClass *)value;

  if (!symbolbddet->completion_string) {
    symbolbddet->completion_string = g_string_new(g_strchug(class->classname));
    symbolbddet->completion_string = g_string_append(symbolbddet->completion_string, "?4"); /* add corresponding image*/
  } else {
    g_string_append_printf (symbolbddet->completion_string," %s?4", g_strchug(class->classname)); /* add corresponding image*/
  }
}

static gchar *symbol_bd_cxx_get_classes (Symbolizable *self)
{
  SymbolBdCXXDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_CXX_GET_PRIVATE(self);
  symbolbddet->completion_string = NULL;

  g_hash_table_foreach (symbolbddet->cxx_class_tree, make_class_completion_string, symbolbddet);

  if (!symbolbddet->completion_string) return NULL;
  return g_string_free(symbolbddet->completion_string, FALSE);
}

static gchar *symbol_bd_cxx_get_calltip (Symbolizable *self, const gchar *symbol_name)
{
  SymbolBdCXXDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_CXX_GET_PRIVATE(self);

  gchar *return_value;
  gchar *params;
  gchar *description;
  gchar *token_line, *copy_line;
  gchar *result = NULL;
  gchar *value = g_tree_lookup (symbolbddet->cxx_api_tree, symbol_name);
  if (value){
    token_line = g_strdup (value);
    copy_line = token_line;
    return_value = strtok(token_line, "|");
    params = strtok(NULL, "|");
    description = strtok(NULL, "|");
    /* make calltip */
    result = g_strdup_printf ("%s %s %s\n%s", return_value, symbol_name, params, description);
    gphpedit_debug_message(DEBUG_CALLTIP, "calltip: %s\n",result);
    g_free(copy_line);	
  } else {
    /*maybe a custom function*/
    result = symbol_bd_cxx_custom_function_calltip(symbolbddet, symbol_name);
  }
  return result;
}

static GList *symbol_bd_cxx_get_custom_symbols_list (Symbolizable *self, gint flags)
{
  SymbolBdCXXDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_CXX_GET_PRIVATE(self);
  if (((flags & SYMBOL_FUNCTION) == SYMBOL_FUNCTION)) {
    return g_hash_table_get_values (symbolbddet->cxx_function_tree);
  }
  if (((flags & SYMBOL_CLASS) == SYMBOL_CLASS)) {
    return g_hash_table_get_values (symbolbddet->cxx_class_tree);
  }
  if (((flags & SYMBOL_VAR) == SYMBOL_VAR)) {
    return g_hash_table_get_values (symbolbddet->cxx_variables_tree);
  }
  return NULL;
}

static void add_custom_function_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserFunction *function = (ClassBrowserFunction *) value;
  SymbolBdCXXDetails *symbolbddet = (SymbolBdCXXDetails *) user_data;
  if (g_strcmp0(function->filename, symbolbddet->completion_prefix) == 0 ) {
    g_hash_table_insert (symbolbddet->completion_list, key, function);
  }
}

static void add_custom_class_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserClass *class = (ClassBrowserClass *) value;
  SymbolBdCXXDetails *symbolbddet = (SymbolBdCXXDetails *) user_data;
  if (g_strcmp0(class->filename, symbolbddet->completion_prefix)==0) {
    g_hash_table_insert (symbolbddet->completion_list, key, class);
  }
}

static void add_custom_var_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserVar *var = (ClassBrowserVar *) value;
  SymbolBdCXXDetails *symbolbddet = (SymbolBdCXXDetails *) user_data;
  if (g_strcmp0(var->filename, symbolbddet->completion_prefix)==0) {
    g_hash_table_insert (symbolbddet->completion_list, key, var);
  }
}

static GList *symbol_bd_cxx_get_custom_symbols_list_by_filename (Symbolizable *self, gint symbol_type, gchar *filename)
{
  SymbolBdCXXDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_CXX_GET_PRIVATE(self);
  if(!filename) return NULL;
  g_return_val_if_fail(self, NULL);
  if (!g_hash_table_lookup (symbolbddet->db_file_table, filename)) return NULL;
  
  symbolbddet->completion_list = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, NULL);
  symbolbddet->completion_prefix = filename;
  if (((symbol_type & SYMBOL_FUNCTION) == SYMBOL_FUNCTION)) {
    g_hash_table_foreach (symbolbddet->cxx_function_tree, add_custom_function_item, symbolbddet);
  }
  if (((symbol_type & SYMBOL_CLASS) == SYMBOL_CLASS)) {
    g_hash_table_foreach (symbolbddet->cxx_class_tree, add_custom_class_item, symbolbddet);
  }
  if (((symbol_type & SYMBOL_VAR) == SYMBOL_VAR)) {
    g_hash_table_foreach (symbolbddet->cxx_variables_tree, add_custom_var_item, symbolbddet);
  }
  GList *result = g_hash_table_get_values (symbolbddet->completion_list);
  g_hash_table_destroy (symbolbddet->completion_list);

  return result;
}

static void symbol_bd_cxx_rescan_file (Symbolizable *self, gchar *filename)
{
}

static void remove_custom_function_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserFunction *function = (ClassBrowserFunction *) value;
  SymbolBdCXXDetails *symbolbddet = (SymbolBdCXXDetails *) user_data;
  if (g_strcmp0(function->filename, symbolbddet->completion_prefix) == 0) {
    g_hash_table_remove (symbolbddet->cxx_function_tree, key);
  }
}

static void remove_custom_class_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserClass *class = (ClassBrowserClass *) value;
  SymbolBdCXXDetails *symbolbddet = (SymbolBdCXXDetails *) user_data;
  if (g_strcmp0(class->filename, symbolbddet->completion_prefix)==0) {
    g_hash_table_remove (symbolbddet->cxx_class_tree, key);
  }
}

static void remove_custom_var_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserVar *var = (ClassBrowserVar *) value;
  SymbolBdCXXDetails *symbolbddet = (SymbolBdCXXDetails *) user_data;
  if (g_strcmp0(var->filename, symbolbddet->completion_prefix)==0) {
    g_hash_table_remove (symbolbddet->cxx_variables_tree, key);
  }
}

static void symbol_bd_cxx_purge_file (Symbolizable *self, gchar *filename)
{
  SymbolBdCXXDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_CXX_GET_PRIVATE(self);
  if(!filename) return ;
  g_return_if_fail(self);

  if (!g_hash_table_remove (symbolbddet->db_file_table, filename)) return ;

  symbolbddet->completion_prefix = filename;

  g_hash_table_foreach (symbolbddet->cxx_function_tree, remove_custom_function_item, symbolbddet);
  g_hash_table_foreach (symbolbddet->cxx_class_tree, remove_custom_class_item, symbolbddet);
  g_hash_table_foreach (symbolbddet->cxx_variables_tree, remove_custom_var_item, symbolbddet);
}

/*
* symbol_bd_cxx_add_file:
* search file in file table, if file isn't found add it and scan it for symbols otherwise
* rescan the file.
*/
static void symbol_bd_cxx_add_file (Symbolizable *self, gchar *filename)
{
  SymbolBdCXXDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_CXX_GET_PRIVATE(self);
  if(!filename) return ;
  g_return_if_fail(self);

  if (g_hash_table_lookup (symbolbddet->db_file_table, filename)){
    symbol_bd_cxx_rescan_file (self, filename);
  } else {
    /* add file to table */
    g_hash_table_insert (symbolbddet->db_file_table, g_strdup(filename), g_strdup(filename));
    /* scan file for symbols */
    symbol_bd_cxx_do_parse_file(SYMBOL_BD_CXX(self), filename);
  }
}

static void symbol_bd_cxx_symbolizable_init(SymbolizableIface *iface, gpointer user_data)
{
  iface->get_symbols_matches = symbol_bd_cxx_get_symbols_matches;
  iface->get_class_symbols = symbol_bd_cxx_get_class_symbols;
  iface->get_classes = symbol_bd_cxx_get_classes;
  iface->get_calltip = symbol_bd_cxx_get_calltip;
  iface->get_custom_symbols_list = symbol_bd_cxx_get_custom_symbols_list;
  iface->get_custom_symbols_list_by_filename = symbol_bd_cxx_get_custom_symbols_list_by_filename;
  iface->rescan_file = symbol_bd_cxx_rescan_file;
  iface->purge_file = symbol_bd_cxx_purge_file;
  iface->add_file = symbol_bd_cxx_add_file;
}

void
symbol_bd_cxx_class_init (SymbolBdCXXClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = symbol_bd_cxx_dispose;

	g_type_class_add_private (klass, sizeof (SymbolBdCXXDetails));
}

void
symbol_bd_cxx_init (SymbolBdCXX *symbolbd)
{
  SymbolBdCXXDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_CXX_GET_PRIVATE(symbolbd);

  symbolbddet->identifierid = 0;

  /* init API tables */
  symbol_bd_load_api_file("c.api", &symbolbddet->cxx_api_tree);

  /*init file table */
  symbolbddet->db_file_table = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  /* custom symbols tables */
  symbolbddet->cxx_function_tree= g_hash_table_new_full (g_str_hash, g_str_equal, g_free, free_function_item);
  symbolbddet->cxx_class_tree= g_hash_table_new_full (g_str_hash, g_str_equal, g_free, free_class_item);
  symbolbddet->cxx_variables_tree = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, free_variable_item);

}

/*
* disposes the Gobject
*/
void symbol_bd_cxx_dispose (GObject *object)
{
  SymbolBdCXX *symbolbd = SYMBOL_BD_CXX(object);
  SymbolBdCXXDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_CXX_GET_PRIVATE(symbolbd);

  if (symbolbddet->cxx_api_tree) g_tree_destroy(symbolbddet->cxx_api_tree);

  if (symbolbddet->cxx_function_tree) g_hash_table_destroy (symbolbddet->cxx_function_tree);
  if (symbolbddet->cxx_variables_tree) g_hash_table_destroy (symbolbddet->cxx_variables_tree);
  if (symbolbddet->cxx_class_tree) g_hash_table_destroy (symbolbddet->cxx_class_tree);

  if (symbolbddet->db_file_table) g_hash_table_destroy (symbolbddet->db_file_table);

  if (symbolbddet->cache_completion) g_free(symbolbddet->cache_completion);

  /* Chain up to the parent class */
  G_OBJECT_CLASS (symbol_bd_cxx_parent_class)->dispose (object);
}

SymbolBdCXX *symbol_bd_cxx_new (void)
{
	SymbolBdCXX *symbolbd;
  symbolbd = g_object_new (SYMBOL_BD_CXX_TYPE, NULL);

	return symbolbd; /* return new object */
}

static gchar *get_fixed_custom_param(gchar *param)
{
  gchar *parameters = NULL;
  /* some function came with a trailing ')' we must remove it */
  gchar *par = param;
  if (param[strlen(param)-2]==')' || param[strlen(param)-2]=='{'){
    gint len = strlen(param) - 2;
    if (len > 0) {
      parameters = g_strndup(param, len);
      par = parameters;
    } else {
      par = NULL;
    }
  } else {
    par = g_strdup(param);
  }
  return par;
}

static void process_cxx_word(GObject *symbolbd, gchar *name, const gchar *filename, gchar *type, gchar *line, gchar *param)
{
 SymbolBdCXX *symbol = SYMBOL_BD_CXX(symbolbd);
 SymbolBdCXXDetails *symbolbddet;
 symbolbddet = SYMBOL_BD_CXX_GET_PRIVATE(symbol);

 if (g_strcmp0(type,"function")==0) {
    gchar *parameters = NULL;
    parameters = get_fixed_custom_param(param);
    gchar *end = strstr(name, "::");
    if (end) {
      gchar *class = g_strndup(name, end - name);
      gchar *nam = g_strndup(end + 2, strlen(name) - strlen(class) - 2);
      gchar *keyname=g_strdup_printf("%s%s",class,filename);
      if (g_hash_table_lookup (symbolbddet->cxx_class_tree, keyname)) {
        symbol_bd_cxx_functionlist_add(symbol, class, nam, filename, atoi(line), parameters);
      } else {
        symbol_bd_cxx_functionlist_add(symbol, NULL, nam, filename, atoi(line), parameters);
      }
      g_free(keyname);
      g_free(nam);
      g_free(class);
    } else {
      symbol_bd_cxx_functionlist_add(symbol, NULL, name, filename, atoi(line), parameters);
    }
    g_free(parameters);
 } else if (g_strcmp0(type,"macro")==0) {
    symbol_bd_cxx_functionlist_add(symbol, NULL, name, filename, atoi(line), NULL);
 } else if (g_strcmp0(type,"struct")==0 && g_strcmp0(type,"member")==0 && g_strcmp0(type,"variable")==0) {
    symbol_bd_cxx_varlist_add(symbol, name, NULL, filename);
 } else if (g_strcmp0(type,"class")==0) {
    symbol_bd_cxx_classlist_add(symbol, name, filename, atoi(line));
 }
}

/* scan a file for symbols */
static void symbol_bd_cxx_do_parse_file(SymbolBdCXX *symbolbd, const gchar *filename) {
#ifdef HAVE_CTAGS_EXUBERANT
  g_return_if_fail(filename);
  SymbolBdCXXDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_CXX_GET_PRIVATE(symbolbd);
  gphpedit_debug_message(DEBUG_CLASSBROWSER, "Parsing: %s\n", filename);
  gchar *path = filename_get_path(filename);
  gchar *command_line = g_strdup_printf("ctags --C++-kinds=-f-d-e-g-m-n-s-t-u-v -x '%s'", path);
  gchar *result = command_spawn(command_line);
  g_free(command_line);
  if (result) process_ctags_custom (G_OBJECT(symbolbd), result, filename, process_cxx_word);
  command_line = g_strdup_printf("ctags --extra=+q --C++-kinds=-c -x '%s'",path);
  result = command_spawn(command_line);
  g_free(command_line);
  if (result) process_ctags_custom (G_OBJECT(symbolbd), result, filename, process_cxx_word);
  g_free(path);
#endif
}

void symbol_bd_cxx_varlist_add(SymbolBdCXX *symbolbd, gchar *varname, gchar *funcname, const gchar *filename)
{
  SymbolBdCXXDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_CXX_GET_PRIVATE(symbolbd);
  gphpedit_debug_message(DEBUG_CLASSBROWSER, "filename: %s var name: %s\n", filename, varname);
  symbol_bd_varlist_add(&symbolbddet->cxx_variables_tree, varname, funcname, filename, &symbolbddet->identifierid);
}

void symbol_bd_cxx_classlist_add(SymbolBdCXX *symbolbd, gchar *classname, const gchar *filename, gint line_number)
{
  SymbolBdCXXDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_CXX_GET_PRIVATE(symbolbd);
  symbol_bd_classlist_add(&symbolbddet->cxx_class_tree, classname, filename, line_number, &symbolbddet->identifierid);
  gphpedit_debug_message(DEBUG_CLASSBROWSER,"filename: %s class name: %s\n", filename, classname);
}

void symbol_bd_cxx_functionlist_add(SymbolBdCXX *symbolbd, gchar *classname, gchar *funcname, const gchar *filename, guint line_number, gchar *param_list)
{
  SymbolBdCXXDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_CXX_GET_PRIVATE(symbolbd);
  symbol_bd_functionlist_add(&symbolbddet->cxx_function_tree, &symbolbddet->cxx_class_tree, classname, funcname, filename, 
                               line_number, param_list, &symbolbddet->identifierid);
  gphpedit_debug_message(DEBUG_CLASSBROWSER,"filename: %s fucntion: %s\n", filename, funcname);
}

static gboolean get_custom_calltip (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserFunction *function = (ClassBrowserFunction *) value;
  SymbolBdCXXDetails *symbolbddet = (SymbolBdCXXDetails *) user_data;

  return (g_utf8_collate(function->functionname, symbolbddet->completion_prefix)==0);
}

static gchar *symbol_bd_cxx_custom_function_calltip(SymbolBdCXXDetails *symbolbddet, const gchar *function_name)
{
/*FIXME::two functions diferent classes same name = bad calltip */
  ClassBrowserFunction *function;
  gchar *calltip = NULL;
  symbolbddet->completion_prefix = (gchar *)function_name;
  function = g_hash_table_find (symbolbddet->cxx_function_tree, get_custom_calltip, symbolbddet);
  if (function) {
    calltip = g_strdup_printf("%s (%s)", function->functionname, function->paramlist);
  }
  gphpedit_debug_message(DEBUG_CLASSBROWSER,"custom calltip: %s\n", calltip);
  return calltip;
}

