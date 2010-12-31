/* This file is part of gPHPEdit, a GNOME PHP Editor.

   Copyright (C) 2010 José Rostagno (for vijona.com.ar)

   For more information or to find the latest release, visit our 
   website at http://www.gphpedit.org/

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "symbolizable.h"
#include "symbol_bd_sql.h"
#include "symbol_bd_utils.h"
#include "tab_sql.h"

/*
* symbol_bd_sql private struct
*/
struct SymbolBdSQLDetails
{
  /* API symbols list */
  GTree *sql_api_tree;
  
  guint identifierid;

  gchar *completion_prefix;
  GString *completion_string;
  GHashTable *completion_list;

  /* cache items */
  char cache_str[200]; /* cached value */
  gchar *cache_completion; /* cached list*/
  gint cache_flags;
};

/*
 * symbol_bd_sql_get_type
 * register SymbolBdSQL type and returns a new GType
*/

static void symbol_bd_sql_class_init (SymbolBdSQLClass *klass);
static void symbol_bd_sql_dispose (GObject *gobject);

#define SYMBOL_BD_SQL_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    SYMBOL_BD_SQL_TYPE,\
					    SymbolBdSQLDetails))

static void symbol_bd_sql_symbolizable_init(SymbolizableIface *iface, gpointer user_data);

G_DEFINE_TYPE_WITH_CODE(SymbolBdSQL, symbol_bd_sql, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE (IFACE_TYPE_SYMBOLIZABLE,
                                                 symbol_bd_sql_symbolizable_init));

static void make_result_string (gpointer key, gpointer value, gpointer user_data)
{
  gchar *function_name = (gchar *)value;
  SymbolBdSQLDetails *symbolbddet = (SymbolBdSQLDetails *) user_data;
  if (!symbolbddet->completion_string) {
    symbolbddet->completion_string = g_string_new(function_name);
  } else {
    symbolbddet->completion_string = g_string_append(symbolbddet->completion_string, " ");
    symbolbddet->completion_string = g_string_append(symbolbddet->completion_string, function_name);
  }
}

static gboolean add_api_item (gpointer key, gpointer value, gpointer user_data)
{
  SymbolBdSQLDetails *symbolbddet = (SymbolBdSQLDetails *) user_data;
  if (g_str_has_prefix(key, symbolbddet->completion_prefix)) {
    g_hash_table_insert (symbolbddet->completion_list, key, g_strdup_printf("%s?2", (gchar *)key));
  }
  if (strncmp(key, symbolbddet->completion_prefix, MIN(strlen(key),strlen(symbolbddet->completion_prefix)))>0){
    return TRUE;
  }
  return FALSE;
}

static GString *symbol_bd_sql_get_autocomp_from_cache(gchar *cache_str, gchar *cache_completion, const gchar *symbol_prefix)
{
  GString *result=NULL;

  /*  Autocompletion optimization:
  *   we store last text typed and we compare with actual text. If current text typed
  *   refine last search we take that search and remove words that don't match new text
  *   so we improve performance a lot because we don't make another full search.
  */
  gchar **strings;
  strings = g_strsplit (cache_completion," ",0);
  int i=0;
  result = g_string_new(NULL);
  while (strings[i]!=0){
    if (g_str_has_prefix(strings[i], symbol_prefix)){
       result = g_string_append(result, strings[i]);
       result = g_string_append(result, " ");
    }    
    i++;    
  }
  g_strfreev (strings);
  g_free(cache_completion);
  cache_completion = g_strdup(result->str);
  strncpy(cache_str,symbol_prefix,MIN(strlen(symbol_prefix),200));

  return result;
}

gboolean symbol_bd_sql_has_cache(gchar *cache_str, gchar *cache_completion, gint cache_flags, const gchar *symbol_prefix, gint flags)
{
  if (cache_flags != flags) return FALSE;
  gint len = strlen(cache_str);
  return (len !=0 && strlen(symbol_prefix) > len && g_str_has_prefix(symbol_prefix, cache_str));
}

static void symbol_bd_sql_save_result_in_cache(SymbolBdSQLDetails *symbolbddet, gchar *result, const gchar *search_word)
{
    if (symbolbddet->cache_completion) g_free(symbolbddet->cache_completion);
    symbolbddet->cache_completion = g_strdup(result);
    strncpy(symbolbddet->cache_str, search_word, MIN(strlen(search_word),200));
}

static gchar *symbol_bd_sql_get_symbols_matches (Symbolizable *self, const gchar *symbol_prefix, gint flags)
{
  SymbolBdSQLDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_SQL_GET_PRIVATE(self);
  symbolbddet->completion_prefix = (gchar *) symbol_prefix;
  symbolbddet->completion_string = NULL;

  if (symbol_bd_sql_has_cache(symbolbddet->cache_str, symbolbddet->cache_completion, symbolbddet->cache_flags, symbol_prefix, flags)){
    symbolbddet->completion_string = symbol_bd_sql_get_autocomp_from_cache(symbolbddet->cache_str, symbolbddet->cache_completion, symbol_prefix);
  } else {
    symbolbddet->completion_list = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, (GDestroyNotify) g_free);

    if (((flags & SYMBOL_ALL) == SYMBOL_ALL) || ((flags & SYMBOL_FUNCTION) == SYMBOL_FUNCTION)) {
      /* add api functions */
      g_tree_foreach (symbolbddet->sql_api_tree, add_api_item, symbolbddet);
    }
    g_hash_table_foreach (symbolbddet->completion_list, make_result_string, symbolbddet);
    g_hash_table_destroy (symbolbddet->completion_list);
    if (symbolbddet->completion_string) symbol_bd_sql_save_result_in_cache(symbolbddet, symbolbddet->completion_string->str, symbol_prefix);
  }

  if (symbolbddet->completion_string){
    gphpedit_debug_message(DEBUG_CLASSBROWSER, "prefix: %s autocomplete list:%s\n", symbol_prefix, symbolbddet->completion_string->str);
    return g_string_free(symbolbddet->completion_string, FALSE);
  }

  gphpedit_debug_message(DEBUG_CLASSBROWSER, "prefix: %s autocomplete list:%s\n", symbol_prefix, "null");
  return NULL;
}

static gchar *symbol_bd_sql_get_class_symbols (Symbolizable *self, const gchar *class_name)
{
  return NULL;
}

static gchar *symbol_bd_sql_get_classes (Symbolizable *self)
{
  return NULL;
}

static gchar *symbol_bd_sql_get_calltip (Symbolizable *self, const gchar *symbol_name)
{
  return NULL;
}

static GList *symbol_bd_sql_get_custom_symbols_list (Symbolizable *self, gint flags)
{
  return NULL;
}

static GList *symbol_bd_sql_get_custom_symbols_list_by_filename (Symbolizable *self, gint symbol_type, gchar *filename)
{
  return NULL;
}

static void symbol_bd_sql_rescan_file (Symbolizable *self, gchar *filename)
{
}

static void symbol_bd_sql_purge_file (Symbolizable *self, gchar *filename)
{
}

static void symbol_bd_sql_add_file (Symbolizable *self, gchar *filename)
{
}

static void symbol_bd_sql_symbolizable_init(SymbolizableIface *iface, gpointer user_data)
{
  iface->get_symbols_matches = symbol_bd_sql_get_symbols_matches;
  iface->get_class_symbols = symbol_bd_sql_get_class_symbols;
  iface->get_classes = symbol_bd_sql_get_classes;
  iface->get_calltip = symbol_bd_sql_get_calltip;
  iface->get_custom_symbols_list = symbol_bd_sql_get_custom_symbols_list;
  iface->get_custom_symbols_list_by_filename = symbol_bd_sql_get_custom_symbols_list_by_filename;
  iface->rescan_file = symbol_bd_sql_rescan_file;
  iface->purge_file = symbol_bd_sql_purge_file;
  iface->add_file = symbol_bd_sql_add_file;
}

void
symbol_bd_sql_class_init (SymbolBdSQLClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = symbol_bd_sql_dispose;

	g_type_class_add_private (klass, sizeof (SymbolBdSQLDetails));
}

static gchar *sql_keywords[] = {"ADD", "ALL", "ALTER", "ANALYZE", "AND", "AS", "ASC", "ASENSITIVE", "AUTO_INCREMENT", 
  "BDB", "BEFORE", "BERKELEYDB", "BETWEEN", "BIGINT", "BINARY", "BLOB", "BOTH", "BTREE",
  "BY", "CALL", "CASCADE", "CASE", "CHANGE", "CHAR", "CHARACTER", "CHECK", "COLLATE", 
  "COLUMN", "COLUMNS", "CONNECTION", "CONSTRAINT", "CREATE", "CROSS", "CURRENT_DATE", 
  "CURRENT_TIME", "CURRENT_TIMESTAMP", "CURSOR", "DATABASE", "DATABASES", "DAY_HOUR", 
  "DAY_MINUTE", "DAY_SECOND", "DEC", "DECIMAL", "DECLARE", "DEFAULT", "DELAYED", 
  "DELETE", "DESC", "DESCRIBE", "DISTINCT", "DISTINCTROW", "DIV", "DOUBLE", "DROP", 
  "ELSE", "ELSEIF", "ENCLOSED", "ERRORS", "ESCAPED", "EXISTS", "EXPLAIN", "FALSE", 
  "FIELDS", "FLOAT", "FOR", "FORCE", "FOREIGN", "FROM", "FULLTEXT", "GRANT", "GROUP", 
  "HASH", "HAVING", "HIGH_PRIORITY", "HOUR_MINUTE", "HOUR_SECOND", "IF", "IGNORE", 
  "IN", "INDEX", "INFILE", "INNER", "INNODB", "INOUT", "INSENSITIVE", "INSERT", "INT", 
  "INTEGER", "INTERVAL", "INTO", "IS", "ITERATE", "JOIN", "KEY", "KEYS", "KILL", 
  "LEADING", "LEAVE", "LEFT", "LIKE", "LIMIT", "LINES", "LOAD", "LOCALTIME", 
  "LOCALTIMESTAMP", "LOCK", "LONG", "LONGBLOB", "LONGTEXT", "LOOP", "LOW_PRIORITY", 
  "MASTER_SERVER_ID", "MATCH", "MEDIUMBLOB", "MEDIUMINT", "MEDIUMTEXT", "MIDDLEINT", 
  "MINUTE_SECOND", "MOD", "MRG_MYISAM", "NATURAL", "NOT", "NULL", "NUMERIC", "ON",
  "OPTIMIZE", "OPTION", "OPTIONALLY", "OR", "ORDER", "OUT", "OUTER", "OUTFILE", 
  "PRECISION", "PRIMARY", "PRIVILEGES", "PROCEDURE", "PURGE", "READ", "REAL", 
  "REFERENCES", "REGEXP", "RENAME", "REPEAT", "REPLACE", "REQUIRE", "RESTRICT", 
  "RETURN", "RETURNS", "REVOKE", "RIGHT", "RLIKE", "RTREE", "SELECT", "SENSITIVE", 
  "SEPARATOR", "SET", "SHOW", "SMALLINT", "SOME", "SONAME", "SPATIAL", "SPECIFIC", 
  "SQL_BIG_RESULT", "SQL_CALC_FOUND_ROWS", "SQL_SMALL_RESULT", "SSL", "STARTING", 
  "STRAIGHT_JOIN STRIPED", "TABLE", "TABLES", "TERMINATED", "THEN", "TINYBLOB", 
  "TINYINT", "TINYTEXT", "TO", "TRAILING", "TRUE", "TYPES", "UNION", "UNIQUE", 
  "UNLOCK", "UNSIGNED", "UNTIL", "UPDATE", "USAGE", "USE", "USER_RESOURCES", 
  "USING", "VALUES", "VARBINARY", "VARCHAR", "VARCHARACTER", "VARYING", "WARNINGS", 
  "WHEN", "WHERE", "WHILE", "WITH", "WRITE", "XOR", "YEAR_MONTH", "ZEROFILL", NULL};

void
symbol_bd_sql_init (SymbolBdSQL *symbolbd)
{
  SymbolBdSQLDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_SQL_GET_PRIVATE(symbolbd);

  symbolbddet->identifierid = 0;

  /* init API tables */
  symbol_bd_function_list_from_array_prepare(sql_keywords, &symbolbddet->sql_api_tree);
}

/*
* disposes the Gobject
*/
void symbol_bd_sql_dispose (GObject *object)
{
  SymbolBdSQL *symbolbd = SYMBOL_BD_SQL(object);
  SymbolBdSQLDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_SQL_GET_PRIVATE(symbolbd);

  if (symbolbddet->sql_api_tree) g_tree_destroy(symbolbddet->sql_api_tree);

  if (symbolbddet->cache_completion) g_free(symbolbddet->cache_completion);

  /* Chain up to the parent class */
  G_OBJECT_CLASS (symbol_bd_sql_parent_class)->dispose (object);
}

SymbolBdSQL *symbol_bd_sql_new (void)
{
	SymbolBdSQL *symbolbd;
  symbolbd = g_object_new (SYMBOL_BD_SQL_TYPE, NULL);

	return symbolbd; /* return new object */
}
