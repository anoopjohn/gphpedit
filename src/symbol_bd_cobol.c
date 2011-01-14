/* This file is part of gPHPEdit, a GNOME PHP Editor.

   Copyright (C) 2010 José Rostagno (for vijona.com.ar)

   For more information or to find the latest release, visit our 
   website at http://www.gphpedit.org/

   gCOBOLEdit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   gCOBOLEdit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with gCOBOLEdit. If not, see <http://www.gnu.org/licenses/>.

   The GNU General Public License is contained in the file COPYING.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "symbolizable.h"
#include "symbol_bd_cobol.h"
#include "symbol_bd_utils.h"
#include "tab_cobol.h"
#include "gvfs_utils.h"

/*
* symbol_bd_cobol private struct
*/
struct SymbolBdCOBOLDetails
{
  /* API symbols list */
  GTree *cobol_api_tree;
  /* custom symbols lists */
  GHashTable *functionlist;
  GHashTable *cobol_variables_tree;
  GHashTable *cobol_class_tree;
  
  /* virtual class */
  gchar *current_program;
  guint current_program_id;

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
 * symbol_bd_cobol_get_type
 * register SymbolBdCOBOL type and returns a new GType
*/

static void symbol_bd_cobol_class_init (SymbolBdCOBOLClass *klass);
static void symbol_bd_cobol_dispose (GObject *gobject);
static void symbol_bd_cobol_do_parse_file(SymbolBdCOBOL *symbolbd, const gchar *filename);
static void symbol_bd_cobol_functionlist_add(SymbolBdCOBOL *symbolbd, gchar *classname, gchar *funcname, gchar *filename, guint line_number);
static void symbol_bd_cobol_varlist_add(SymbolBdCOBOL *symbolbd, gchar *varname, gchar *filename);
static void symbol_bd_cobol_classlist_add(SymbolBdCOBOL *symbolbd, gchar *classname, gchar *filename, gint line_number);

#define SYMBOL_BD_COBOL_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    SYMBOL_BD_COBOL_TYPE,\
					    SymbolBdCOBOLDetails))

static void symbol_bd_cobol_symbolizable_init(SymbolizableIface *iface, gpointer user_data);

G_DEFINE_TYPE_WITH_CODE(SymbolBdCOBOL, symbol_bd_cobol, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE (IFACE_TYPE_SYMBOLIZABLE,
                                                 symbol_bd_cobol_symbolizable_init));

static gboolean make_result_string (gpointer key, gpointer value, gpointer user_data)
{
  gchar *function_name = (gchar *)value;
  SymbolBdCOBOLDetails *symbolbddet = (SymbolBdCOBOLDetails *) user_data;
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
  SymbolBdCOBOLDetails *symbolbddet = (SymbolBdCOBOLDetails *) user_data;
  if (g_str_has_prefix(function->functionname, symbolbddet->completion_prefix)) {
    g_tree_insert (symbolbddet->completion_tree, key, g_strdup_printf("%s?1",function->functionname));
  }
}

static gboolean add_api_item (gpointer key, gpointer value, gpointer user_data)
{
  SymbolBdCOBOLDetails *symbolbddet = (SymbolBdCOBOLDetails *) user_data;
  if (g_str_has_prefix(key, symbolbddet->completion_prefix)) {
    g_tree_insert (symbolbddet->completion_tree, key, g_strdup_printf("%s?2", (gchar *)key));
  }
  if (strncmp(key, symbolbddet->completion_prefix, MIN(strlen(key),strlen(symbolbddet->completion_prefix)))>0){
    return TRUE;
  }
  return FALSE;
}

static void add_var_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserVar *var = (ClassBrowserVar *) value;
  SymbolBdCOBOLDetails *symbolbddet = (SymbolBdCOBOLDetails *) user_data;
  if (g_str_has_prefix(var->varname, symbolbddet->completion_prefix)) {
    g_tree_insert (symbolbddet->completion_tree, key, g_strdup_printf("%s?3",var->varname));
  }
}

static GString *symbol_bd_cobol_get_autocomp_from_cache(gchar *cache_str, gchar *cache_completion, const gchar *symbol_prefix)
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

gboolean symbol_bd_cobol_has_cache(gchar *cache_str, gchar *cache_completion, gint cache_flags, const gchar *symbol_prefix, gint flags)
{
  if (cache_flags != flags) return FALSE;
  gint len = strlen(cache_str);
  return (len !=0 && strlen(symbol_prefix) > len && g_str_has_prefix(symbol_prefix, cache_str));
}

static void symbol_bd_cobol_save_result_in_cache(SymbolBdCOBOLDetails *symbolbddet, gchar *result, const gchar *search_word)
{
    if (symbolbddet->cache_completion) g_free(symbolbddet->cache_completion);
    symbolbddet->cache_completion = g_strdup(result);
    strncpy(symbolbddet->cache_str, search_word, MIN(strlen(search_word),200));
}

static gchar *symbol_bd_cobol_get_symbols_matches (Symbolizable *self, const gchar *symbol_prefix, gint flags)
{
  SymbolBdCOBOLDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_COBOL_GET_PRIVATE(self);
  symbolbddet->completion_prefix = (gchar *) symbol_prefix;
  symbolbddet->completion_string = NULL;

  if (symbol_bd_cobol_has_cache(symbolbddet->cache_str, symbolbddet->cache_completion, symbolbddet->cache_flags, symbol_prefix, flags)){
    symbolbddet->completion_string = symbol_bd_cobol_get_autocomp_from_cache(symbolbddet->cache_str, symbolbddet->cache_completion, symbol_prefix);
  } else {
    symbolbddet->completion_tree = g_tree_new_full ((GCompareDataFunc) g_strcmp0, NULL, NULL,(GDestroyNotify) g_free);

    if (((flags & SYMBOL_ALL) == SYMBOL_ALL) || ((flags & SYMBOL_FUNCTION) == SYMBOL_FUNCTION)) {
      g_hash_table_foreach (symbolbddet->functionlist, add_result_item, symbolbddet);
      /* add api functions */
      g_tree_foreach (symbolbddet->cobol_api_tree, add_api_item, symbolbddet);
    }
    if (((flags & SYMBOL_ALL) == SYMBOL_ALL) || ((flags & SYMBOL_VAR) == SYMBOL_VAR)) {
      g_hash_table_foreach (symbolbddet->cobol_variables_tree, add_var_item, symbolbddet);
    }

    g_tree_foreach (symbolbddet->completion_tree, make_result_string, symbolbddet);
    g_tree_destroy (symbolbddet->completion_tree);
    if (symbolbddet->completion_string) symbol_bd_cobol_save_result_in_cache(symbolbddet, symbolbddet->completion_string->str, symbol_prefix);
  }

  if (symbolbddet->completion_string){
    gphpedit_debug_message(DEBUG_CLASSBROWSER, "prefix: %s autocomplete list:%s\n", symbol_prefix, symbolbddet->completion_string->str);
    return g_string_free(symbolbddet->completion_string, FALSE);
  }

  gphpedit_debug_message(DEBUG_CLASSBROWSER, "prefix: %s autocomplete list:%s\n", symbol_prefix, "null");
  return NULL;
}

static gchar *symbol_bd_cobol_get_class_symbols (Symbolizable *self, const gchar *class_name)
{
  return NULL;
}

static gchar *symbol_bd_cobol_get_classes (Symbolizable *self)
{
  return NULL;
}

static gchar *symbol_bd_cobol_get_calltip (Symbolizable *self, const gchar *symbol_name)
{
  return NULL;
}

static GList *symbol_bd_cobol_get_custom_symbols_list (Symbolizable *self, gint flags)
{
  SymbolBdCOBOLDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_COBOL_GET_PRIVATE(self);
  if (((flags & SYMBOL_FUNCTION) == SYMBOL_FUNCTION)) {
    return g_hash_table_get_values (symbolbddet->functionlist);
  }
  if (((flags & SYMBOL_CLASS) == SYMBOL_CLASS)) {
    return g_hash_table_get_values (symbolbddet->cobol_class_tree);
  }
  if (((flags & SYMBOL_VAR) == SYMBOL_VAR)) {
    return g_hash_table_get_values (symbolbddet->cobol_variables_tree);
  }
  return NULL;
}

static void add_custom_function_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserFunction *function = (ClassBrowserFunction *) value;
  SymbolBdCOBOLDetails *symbolbddet = (SymbolBdCOBOLDetails *) user_data;
  if (g_strcmp0(function->filename, symbolbddet->completion_prefix) == 0 ) {
    g_hash_table_insert (symbolbddet->completion_list, key, function);
  }
}

static void add_custom_class_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserClass *class = (ClassBrowserClass *) value;
  SymbolBdCOBOLDetails *symbolbddet = (SymbolBdCOBOLDetails *) user_data;
  if (g_strcmp0(class->filename, symbolbddet->completion_prefix)==0) {
    g_hash_table_insert (symbolbddet->completion_list, key, class);
  }
}

static void add_custom_var_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserVar *var = (ClassBrowserVar *) value;
  SymbolBdCOBOLDetails *symbolbddet = (SymbolBdCOBOLDetails *) user_data;
  if (g_strcmp0(var->filename, symbolbddet->completion_prefix)==0) {
    g_hash_table_insert (symbolbddet->completion_list, key, var);
  }
}

static GList *symbol_bd_cobol_get_custom_symbols_list_by_filename (Symbolizable *self, gint symbol_type, gchar *filename)
{
  SymbolBdCOBOLDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_COBOL_GET_PRIVATE(self);
  if(!filename) return NULL;
  g_return_val_if_fail(self, NULL);
  if (!g_hash_table_lookup (symbolbddet->db_file_table, filename)) return NULL;
  
  symbolbddet->completion_list = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, NULL);
  symbolbddet->completion_prefix = filename;
  if (((symbol_type & SYMBOL_FUNCTION) == SYMBOL_FUNCTION)) {
    g_hash_table_foreach (symbolbddet->functionlist, add_custom_function_item, symbolbddet);
  }
  if (((symbol_type & SYMBOL_CLASS) == SYMBOL_CLASS)) {
    g_hash_table_foreach (symbolbddet->cobol_class_tree, add_custom_class_item, symbolbddet);
  }
  if (((symbol_type & SYMBOL_VAR) == SYMBOL_VAR)) {
    g_hash_table_foreach (symbolbddet->cobol_variables_tree, add_custom_var_item, symbolbddet);
  }
  GList *result = g_hash_table_get_values (symbolbddet->completion_list);
  g_hash_table_destroy (symbolbddet->completion_list);

  return result;
}

static void symbol_bd_cobol_rescan_file (Symbolizable *self, gchar *filename)
{
}

static void remove_custom_function_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserFunction *function = (ClassBrowserFunction *) value;
  SymbolBdCOBOLDetails *symbolbddet = (SymbolBdCOBOLDetails *) user_data;
  if (g_strcmp0(function->filename, symbolbddet->completion_prefix) == 0) {
    g_hash_table_remove (symbolbddet->functionlist, key);
  }
}

static void remove_custom_class_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserClass *class = (ClassBrowserClass *) value;
  SymbolBdCOBOLDetails *symbolbddet = (SymbolBdCOBOLDetails *) user_data;
  if (g_strcmp0(class->filename, symbolbddet->completion_prefix)==0) {
    g_hash_table_remove (symbolbddet->cobol_class_tree, key);
  }
}

static void remove_custom_var_item (gpointer key, gpointer value, gpointer user_data)
{
  ClassBrowserVar *var = (ClassBrowserVar *) value;
  SymbolBdCOBOLDetails *symbolbddet = (SymbolBdCOBOLDetails *) user_data;
  if (g_strcmp0(var->filename, symbolbddet->completion_prefix)==0) {
    g_hash_table_remove (symbolbddet->cobol_variables_tree, key);
  }
}

static void symbol_bd_cobol_purge_file (Symbolizable *self, gchar *filename)
{
  SymbolBdCOBOLDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_COBOL_GET_PRIVATE(self);
  if(!filename) return ;
  g_return_if_fail(self);

  symbolbddet->completion_prefix = filename;

  if (!g_hash_table_remove (symbolbddet->db_file_table, filename)) return ;

  g_hash_table_foreach (symbolbddet->functionlist, remove_custom_function_item, symbolbddet);
  g_hash_table_foreach (symbolbddet->cobol_class_tree, remove_custom_class_item, symbolbddet);
  g_hash_table_foreach (symbolbddet->cobol_variables_tree, remove_custom_var_item, symbolbddet);
}

/*
* symbol_bd_cobol_add_file:
* search file in file table, if file isn't found add it and scan it for symbols otherwise
* rescan the file.
*/
static void symbol_bd_cobol_add_file (Symbolizable *self, gchar *filename)
{
  SymbolBdCOBOLDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_COBOL_GET_PRIVATE(self);
  if(!filename) return ;
  g_return_if_fail(self);

  if (g_hash_table_lookup (symbolbddet->db_file_table, filename)){
    symbol_bd_cobol_rescan_file (self, filename);
  } else {
    /* add file to table */
    g_hash_table_insert (symbolbddet->db_file_table, g_strdup(filename), g_strdup(filename));
    /* scan file for symbols */
    symbol_bd_cobol_do_parse_file(SYMBOL_BD_COBOL(self), filename);
  }
}

static void symbol_bd_cobol_symbolizable_init(SymbolizableIface *iface, gpointer user_data)
{
  iface->get_symbols_matches = symbol_bd_cobol_get_symbols_matches;
  iface->get_class_symbols = symbol_bd_cobol_get_class_symbols;
  iface->get_classes = symbol_bd_cobol_get_classes;
  iface->get_calltip = symbol_bd_cobol_get_calltip;
  iface->get_custom_symbols_list = symbol_bd_cobol_get_custom_symbols_list;
  iface->get_custom_symbols_list_by_filename = symbol_bd_cobol_get_custom_symbols_list_by_filename;
  iface->rescan_file = symbol_bd_cobol_rescan_file;
  iface->purge_file = symbol_bd_cobol_purge_file;
  iface->add_file = symbol_bd_cobol_add_file;
}

void
symbol_bd_cobol_class_init (SymbolBdCOBOLClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = symbol_bd_cobol_dispose;

	g_type_class_add_private (klass, sizeof (SymbolBdCOBOLDetails));
}

gchar *cobol_keyword[] = {"ACCEPT","ACCESS","ACQUIRE","ACTUAL","ADD","ADDRESS","ADVANCING","AFTER","ALL","ALLOWING","ALPHABET",
"ALPHABETIC","ALPHABETIC-LOWER","ALPHABETIC-UPPER","ALPHANUMERIC","ALPHANUMERIC-EDITED","ALSO","ALTER","ALTERNATE","AND","ANY","APPLY","ARE",
"AREA","AREAS","ASCENDING","ASSIGN","ATTRIBUTE","AUTHOR","AUTO","AUTO-HYPHEN-SKIP","AUTO-SKIP","AUTOMATIC","AUTOTERMINATE",
"BACKGROUND-COLOR","BACKGROUND-COLOUR","BACKWARD","BASIS","BEEP","BEFORE","BEGINNING","BELL","BINARY","BLANK","BLINK","BLINKING",
"BLOCK","BOLD","BOTTOM","BOX","BOXED","CALL","CANCEL","CBL","CENTERED","CHAIN","CHAINING","CHANGED","CHARACTER","CHARACTERS","CHART",
"CLASS","CLOCK-UNITS","CLOSE","COBOL","CODE","CODE-SET","COL","COLLATING","COLOR","COLOUR","COLUMN","COM-REG","COMMA","COMMAND-LINE","COMMIT",
"COMMITMENT","COMMON","COMMUNICATION","COMP","COMP-0","COMP-1","COMP-2","COMP-3","COMP-4","COMP-5","COMP-6","COMP-X","COMPRESSION",
"COMPUTATIONAL","COMPUTATIONAL","COMPUTATIONAL-1","COMPUTATIONAL-2","COMPUTATIONAL-3","COMPUTATIONAL-4","COMPUTATIONAL-5","COMPUTATIONAL-6",
"COMPUTATIONAL-X","COMPUTE","CONFIGURATION","CONSOLE","CONTAINS","CONTENT","CONTINUE","CONTROL","CONTROL-AREA","CONTROLS","CONVERSION","CONVERT",
"CONVERTING","COPY","CORE-INDEX","CORR","CORRESPONDING","COUNT","CRT","CRT-UNDER","CSP","CURRENCY","CURRENT-DATE","CURSOR","CYCLE","CYL-INDEX",
"CYL-OVERFLOW","DATA","DATE","DATE-COMPILED","DATE-WRITTEN","DAY","DAY-OF-WEEK","DBCS","DEBUG","DEBUG-CONTENTS","DEBUG-ITEM","DEBUG-LINE",
"DEBUG-NAME","DEBUG-SUB-1","DEBUG-SUB-2","DEBUG-SUB-3","DEBUGGING","DECIMAL-POINT","DECLARATIVES","DEFAULT","DELETE","DELIMITED",
"DELIMITER","DEPENDING","DESCENDING","DESTINATION","DETAIL","DISABLE","DISK","DISP","DISPLAY","DISPLAY-1","DISPLAY-ST","DIVIDE","DIVISION",
"DOWN","DRAW","DROP","DUPLICATES","DYNAMIC","ECHO","EGCS","EGI","EJECT","ELSE","EMI","EMPTY-CHECK","ENABLE","ENCRYPTION","END","END-ACCEPT",
"END-ADD","END-CALL","END-CHAIN","END-COMPUTE","END-DELETE","END-DISPLAY","END-DIVIDE","END-EVALUATE","END-IF","END-INVOKE","END-MULTIPLY",
"END-OF-PAGE","END-PERFORM","END-READ","END-RECEIVE","END-RETURN","END-REWRITE","END-SEARCH","END-START","END-STRING","END-SUBTRACT",
"END-UNSTRING","END-WRITE","ENDING","ENTER","ENTRY","ENVIRONMENT","ENVIRONMENT-DIVISION","EOL","EOP","EOS","EQUAL","EQUALS","ERASE",
"ERROR","ESCAPE","ESI","EVALUATE","EVERY","EXAMINE","EXCEEDS","EXCEPTION","EXCESS-3","EXCLUSIVE","EXEC","EXECUTE","EXHIBIT","EXIT","EXTEND",
"EXTENDED-SEARCH","EXTERNAL","EXTERNALLY-DESCRIBED-KEY","FACTORY","FALSE","FH--FCD","FH--KEYDEF","FILE","FILE-CONTROL","FILE-ID","FILE-LIMIT",
"FILE-LIMITS","FILE-PREFIX","FILLER","FINAL","FIRST","FIXED","FOOTING","FOOTING","FOR","FOREGROUND-COLOR","FOREGROUND-COLOUR","FORMAT",
"FROM","FULL","FUNCTION","GENERATE","GIVING","GLOBAL","GO","GOBACK","GREATER","GRID","GROUP","HEADING","HIGH","HIGH-VALUE","HIGH-VALUES",
"HIGHLIGHT","I-O-CONTROL","IDENTIFICATION","IGNORE","INDEX","INDEXED","INDIC","INDICATE","INDICATOR","INDICATORS","INHERITING","INITIAL",
"INITIALIZE","INITIATE","INPUT","INPUT-OUTPUT","INSERT","INSPECT","INSTALLATION","INTO","INVALID","INVOKE","INVOKED","JAPANESE","JUST",
"JUSTIFIED","KANJI","KEPT","KEY","KEYBOARD","LABEL","LAST","LEADING","LEAVE","LEFT","LEFT-JUSTIFY","LEFTLINE","LENGTH",
"LENGTH-CHECK", "LESS", "LIMIT","LIMITS","LIN","LINAGE","LINAGE-COUNTER","LINE","LINE-COUNTER","LINES","LINKAGE","LOCAL-STORAGE","LOCK",
"LOCK-HOLDING","LOCKING","LOW","LOW-VALUE","LOW-VALUES","LOWER","LOWLIGHT","MANUAL","MASS-UPDATE","MASTER-INDEX","MEMORY",
"MERGE","MESSAGE","METHOD","MODE","MODIFIED","MODULES","MORE-LABELS","MOVE","MULTIPLE","MULTIPLY","NAME","NAMED","NATIONAL",
"NATIONAL-EDITED","NATIVE","NCHAR","NEGATIVE","NEXT","NO-ECHO","NOMINAL","NOT","NOTE","NSTD-REELS","NULL","NULLS","NUMBER","NUMERIC",
"NUMERIC-EDITED","NUMERIC-FILL","O-FILL","OBJECT","OBJECT-COMPUTER","OBJECT-STORAGE","OCCURS","OFF","OMITTED","OOSTACKPTR","OPEN","OPTIONAL",
"ORDER","ORGANIZATION","OTHER","OTHERS","OTHERWISE","OUTPUT","OVERFLOW","OVERLINE","PACKED-DECIMAL","PACKED-DECIMAL","PADDING","PAGE",
"PAGE-COUNTER","PARAGRAPH","PASSWORD","PERFORM","PIC","PICTURE","PLUS","POINTER","POP-UP","POS","POSITION","POSITIONING",
"POSITIVE","PREVIOUS","PRINT","PRINT-CONTROL","PRINT-SWITCH","PRINTER","PRINTER-1","PRINTING","PRIOR","PRIVATE","PROCEDURE",
"PROCEDURE-POINTER","PROCEDURES","PROCEED","PROCESS","PROCESSING","PROGRAM","PROGRAM-ID","PROMPT","PROTECTED","PUBLIC","PURGE",
"QUEUE","QUOTE","QUOTES","RANDOM","RANGE","READ","READERS","READY","RECEIVE","RECEIVE-CONTROL","RECORD","RECORD-OVERFLOW","RECORDING","RECORDS",
"REDEFINES","REEL","REFERENCE","REFERENCES","RELATIVE","RELEASE","RELOAD","REMAINDER","REMARKS","REMOVAL","RENAMES","REORG-CRITERIA","REPEATED",
"REPLACE","REPLACING","REPORT","REPORTING","REPORTS","REQUIRED","REREAD","RERUN","RESERVE","RESET","RESIDENT","RETURN","RETURN-CODE","RETURNING",
"REVERSE","REVERSE-VIDEO","REVERSED","REWIND","REWRITE","RIGHT","RIGHT-JUSTIFY","ROLLBACK","ROLLING","ROUNDED","RUN","S01","S02","S03","S04","S05",
"SAME","SCREEN","SCROLL","SEARCH","SECTION","SECURE","SECURITY","SEEK","SEGMENT","SEGMENT-LIMIT","SELECT","SELECTIVE","SELF","SELFCLASS","SEND",
"SENTENCE","SEPARATE","SEQUENCE","SEQUENTIAL","SERVICE","SET","SETSHADOW","SHIFT-IN","SHIFT-OUT","SIGN","SIZE","SKIP1","SKIP2","SKIP3","SORT",
"SORT-CONTROL","SORT-CORE-SIZE","SORT-FILE-SIZE","SORT-MERGE","SORT-MESSAGE","SORT-MODE-SIZE","SORT-OPTION","SORT-RETURN","SOURCE",
"SOURCE-COMPUTER","SPACE","SPACE-FILL","SPACES","SPACES","SPECIAL-NAMES","STANDARD","STANDARD-1","STANDARD-2","START","STARTING",
"STATUS","STOP","STORE","STRING","SUB-QUEUE-1","SUB-QUEUE-2","SUB-QUEUE-3","SUBFILE","SUBTRACT","SUM","SUPER","SUPPRESS","SYMBOLIC",
"SYNC","SYNCHRONIZED","SYSIN","SYSIPT","SYSLST","SYSOUT","SYSPCH","SYSPUNCH","SYSTEM-INFO","TAB","TALLYING","TAPE","TERMINAL",
"TERMINAL-INFO","TERMINATE","TEST","TEXT","THAN","THEN","THROUGH","THRU","TIME","TIME-OF-DAY","TIME-OUT","TIMEOUT","TIMES",
"TITLE","TOP","TOTALED","TOTALING","TRACE","TRACK-AREA","TRACK-LIMIT","TRACKS","TRAILING","TRAILING-SIGN","TRANSACTION","TRANSFORM","TRUE",
"TYPE","TYPEDEF","UNDERLINE","UNDERLINED","UNEQUAL","UNIT","UNLOCK","UNSTRING","UNTIL","UPDATE","UPDATERS","UPON","UPPER","UPSI-0","UPSI-1",
"UPSI-2","UPSI-3","UPSI-4","UPSI-5","UPSI-6","UPSI-7","USAGE","USE","USER","USING","VALUE","VALUES","VARIABLE","VARYING","WAIT","WHEN",
"WHEN-COMPILED","WINDOW","WITH","WORDS","WORKING-STORAGE","WRAP","WRITE","WRITE-ONLY","WRITE-VERIFY","WRITERSZERO","ZERO",
"ZERO-FILL","ZEROES","ZEROS",NULL};
void
symbol_bd_cobol_init (SymbolBdCOBOL *symbolbd)
{
  SymbolBdCOBOLDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_COBOL_GET_PRIVATE(symbolbd);

  symbolbddet->identifierid = 0;

  /* init API tables */
  symbol_bd_function_list_from_array_prepare(cobol_keyword, &symbolbddet->cobol_api_tree);
  /*init file table */
  symbolbddet->db_file_table = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  /* custom symbols tables */
  symbolbddet->functionlist= g_hash_table_new_full (g_str_hash, g_str_equal, g_free, free_function_item);
  symbolbddet->cobol_class_tree= g_hash_table_new_full (g_str_hash, g_str_equal, g_free, free_class_item);
  symbolbddet->cobol_variables_tree = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, free_variable_item);
}

/*
* disposes the Gobject
*/
void symbol_bd_cobol_dispose (GObject *object)
{
  SymbolBdCOBOL *symbolbd = SYMBOL_BD_COBOL(object);
  SymbolBdCOBOLDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_COBOL_GET_PRIVATE(symbolbd);

  if (symbolbddet->cobol_api_tree) g_tree_destroy(symbolbddet->cobol_api_tree);

  if (symbolbddet->functionlist) g_hash_table_destroy (symbolbddet->functionlist);
  if (symbolbddet->cobol_variables_tree) g_hash_table_destroy (symbolbddet->cobol_variables_tree);
  if (symbolbddet->cobol_class_tree) g_hash_table_destroy (symbolbddet->cobol_class_tree);

  if (symbolbddet->db_file_table) g_hash_table_destroy (symbolbddet->db_file_table);

  if (symbolbddet->cache_completion) g_free(symbolbddet->cache_completion);

  /* Chain up to the parent class */
  G_OBJECT_CLASS (symbol_bd_cobol_parent_class)->dispose (object);
}

SymbolBdCOBOL *symbol_bd_cobol_new (void)
{
	SymbolBdCOBOL *symbolbd;
  symbolbd = g_object_new (SYMBOL_BD_COBOL_TYPE, NULL);

	return symbolbd; /* return new object */
}

static void process_cobol_word(GObject *symbolbd, gchar *name, const gchar *filename, gchar *type, gchar *line, gchar *param)
{
 if (is_cobol_banned_word(name)) return ;
  SymbolBdCOBOL *symbol = SYMBOL_BD_COBOL(symbolbd);
 SymbolBdCOBOLDetails *symbolbddet;
 symbolbddet = SYMBOL_BD_COBOL_GET_PRIVATE(symbolbd);
 if (g_strcmp0(type,"paragraph")==0) {
  symbol_bd_cobol_functionlist_add(symbol, symbolbddet->current_program, name, (gchar *)filename, atoi(line));
 } else if (g_strcmp0(type,"data")==0 || g_strcmp0(type,"group")==0) {
  symbol_bd_cobol_varlist_add(symbol, name, (gchar *)filename);
 } else if (g_strcmp0(type,"program")==0) {
  symbol_bd_cobol_classlist_add(symbol, name, (gchar *)filename, atoi(line));
  symbolbddet->current_program = g_strdup(name);
 }
}

/* scan a file for symbols */
static void symbol_bd_cobol_do_parse_file(SymbolBdCOBOL *symbolbd, const gchar *filename)
{
#ifdef HAVE_CTAGS_EXUBERANT
  g_return_if_fail(filename);
  SymbolBdCOBOLDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_COBOL_GET_PRIVATE(symbolbd);
  gphpedit_debug_message(DEBUG_CLASSBROWSER, "Parsing: %s\n", filename);
  gchar *path = filename_get_path(filename);
  gchar *command_line = g_strdup_printf("ctags --Cobol-kinds=-d-f-g-p-s -x '%s'", path);
  gchar *result = command_spawn(command_line);
  g_free(command_line);
  if (result) process_ctags_custom (G_OBJECT(symbolbd), result, filename, process_cobol_word);
  command_line = g_strdup_printf("ctags --Cobol-kinds=-f-P-s -x '%s'",path);
  result = command_spawn(command_line);
  g_free(command_line);
  if (result) process_ctags_custom (G_OBJECT(symbolbd), result, filename, process_cobol_word);
  g_free(symbolbddet->current_program);
  g_free(path);
#endif
}

static void symbol_bd_cobol_varlist_add(SymbolBdCOBOL *symbolbd, gchar *varname, gchar *filename)
{
  SymbolBdCOBOLDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_COBOL_GET_PRIVATE(symbolbd);
  gphpedit_debug_message(DEBUG_CLASSBROWSER, "filename: %s var name: %s\n", filename, varname);
  symbol_bd_varlist_add(&symbolbddet->cobol_variables_tree, varname, NULL, filename, &symbolbddet->identifierid);
}

static void symbol_bd_cobol_classlist_add(SymbolBdCOBOL *symbolbd, gchar *classname, gchar *filename, gint line_number)
{
  SymbolBdCOBOLDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_COBOL_GET_PRIVATE(symbolbd);
  symbol_bd_classlist_add(&symbolbddet->cobol_class_tree, classname, filename, line_number, &symbolbddet->identifierid);
  gphpedit_debug_message(DEBUG_CLASSBROWSER,"filename: %s class name: %s\n", filename, classname);
}

static void symbol_bd_cobol_functionlist_add(SymbolBdCOBOL *symbolbd, gchar *classname, gchar *funcname, gchar *filename, guint line_number)
{
  SymbolBdCOBOLDetails *symbolbddet;
	symbolbddet = SYMBOL_BD_COBOL_GET_PRIVATE(symbolbd);
  symbol_bd_functionlist_add(&symbolbddet->functionlist, &symbolbddet->cobol_class_tree, classname, funcname, filename, 
                               line_number, NULL, &symbolbddet->identifierid);
  gphpedit_debug_message(DEBUG_CLASSBROWSER,"filename: %s fucntion: %s\n", filename, funcname);
}

