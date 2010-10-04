/* This file is part of gPHPEdit, a GNOME2 PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009 José Rostagno (for vijona.com.ar) 

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

#include <config.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include "debug.h"
#include "calltip.h"
#include "tab.h"
#include "main_window.h"
#include "classbrowser_ui.h"

/*
* calltip_manager private struct
*/
struct CalltipManagerDetails
{
  GTree *php_api_tree;
  GTree *css_api_tree;
  GTree *cobol_api_tree;
  GTree *cxx_api_tree;
  GTree *sql_api_tree;

  GString *completion_list_tree;
  gchar *prefix; 

  char cache_str[200]; /* cached value */
  gchar *cache_completion; /* cached list*/
};

#define MAX_API_LINE_LENGTH 16384

#define CALLTIP_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    CALLTIP_MANAGER_TYPE,\
					    CalltipManagerDetails))

static void calltip_manager_finalize (GObject  *object);
static void  calltip_manager_class_init (CalltipManagerClass *klass);
static void calltip_manager_load_api_file(const gchar *api_filename, GTree **api_tree);
static void calltip_manager_function_list_from_array_prepare(gchar **keywords, GTree **api_tree);
gboolean calltip_manager_has_cache(gchar *buffer, gchar *cache_str, gchar *cache_completion);
GString *calltip_manager_get_autocomp_from_cache(gchar *buffer, gchar *cache_str, gchar *cache_completion);
static void calltip_manager_save_result_in_cache(CalltipManagerDetails *calltipmgdet, gchar *result, gchar *search_word);

/* http://library.gnome.org/devel/gobject/unstable/gobject-Type-Information.html#G-DEFINE-TYPE:CAPS */
G_DEFINE_TYPE(CalltipManager, calltip_manager, G_TYPE_OBJECT);  

/*
* overide default contructor to make a singleton.
* see http://blogs.gnome.org/xclaesse/2010/02/11/how-to-make-a-gobject-singleton/
*/
static GObject* 
calltip_manager_constructor (GType type,
                 guint n_construct_params,
                 GObjectConstructParam *construct_params)
{
  static GObject *self = NULL;

  if (self == NULL)
    {
      self = G_OBJECT_CLASS (calltip_manager_parent_class)->constructor (
          type, n_construct_params, construct_params);
      g_object_add_weak_pointer (self, (gpointer) &self);
      return self;
    }

  return g_object_ref (self);
}

static void
calltip_manager_class_init (CalltipManagerClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = calltip_manager_finalize;
  object_class->constructor = calltip_manager_constructor;
	g_type_class_add_private (klass, sizeof (CalltipManagerDetails));
}

gchar *cobol_keyword[] = {"ACCEPT","ACCESS","ACQUIRE","ACTUAL","ADD","ADDRESS","ADVANCING","AFTER","ALL","ALLOWING","ALPHABET","ALPHABETIC","ALPHABETIC-LOWER","ALPHABETIC-UPPER","ALPHANUMERIC","ALPHANUMERIC-EDITED","ALSO","ALTER","ALTERNATE","AND","ANY","APPLY","ARE","AREA","AREAS","ASCENDING","ASSIGN","ATTRIBUTE","AUTHOR","AUTO","AUTO-HYPHEN-SKIP","AUTO-SKIP","AUTOMATIC","AUTOTERMINATE","BACKGROUND-COLOR","BACKGROUND-COLOUR","BACKWARD","BASIS","BEEP","BEFORE","BEGINNING","BELL","BINARY","BLANK","BLINK","BLINKING","BLOCK","BOLD","BOTTOM","BOX","BOXED","CALL","CANCEL","CBL","CENTERED","CHAIN","CHAINING","CHANGED","CHARACTER","CHARACTERS","CHART","CLASS","CLOCK-UNITS","CLOSE","COBOL","CODE","CODE-SET","COL","COLLATING","COLOR","COLOUR","COLUMN","COM-REG","COMMA","COMMAND-LINE","COMMIT","COMMITMENT","COMMON","COMMUNICATION","COMP","COMP-0","COMP-1","COMP-2","COMP-3","COMP-4","COMP-5","COMP-6","COMP-X","COMPRESSION","COMPUTATIONAL","COMPUTATIONAL","COMPUTATIONAL-1","COMPUTATIONAL-2","COMPUTATIONAL-3","COMPUTATIONAL-4","COMPUTATIONAL-5","COMPUTATIONAL-6","COMPUTATIONAL-X","COMPUTE","CONFIGURATION","CONSOLE","CONTAINS","CONTENT","CONTINUE","CONTROL","CONTROL-AREA","CONTROLS","CONVERSION","CONVERT","CONVERTING","COPY","CORE-INDEX","CORR","CORRESPONDING","COUNT","CRT","CRT-UNDER","CSP","CURRENCY","CURRENT-DATE","CURSOR","CYCLE","CYL-INDEX","CYL-OVERFLOW","DATA","DATE","DATE-COMPILED","DATE-WRITTEN","DAY","DAY-OF-WEEK","DBCS","DEBUG","DEBUG-CONTENTS","DEBUG-ITEM","DEBUG-LINE","DEBUG-NAME","DEBUG-SUB-1","DEBUG-SUB-2","DEBUG-SUB-3","DEBUGGING","DECIMAL-POINT","DECLARATIVES","DEFAULT","DELETE","DELIMITED","DELIMITER","DEPENDING","DESCENDING","DESTINATION","DETAIL","DISABLE","DISK","DISP","DISPLAY","DISPLAY-1","DISPLAY-ST","DIVIDE","DIVISION","DOWN","DRAW","DROP","DUPLICATES","DYNAMIC","ECHO","EGCS","EGI","EJECT","ELSE","EMI","EMPTY-CHECK","ENABLE","ENCRYPTION","END","END-ACCEPT","END-ADD","END-CALL","END-CHAIN","END-COMPUTE","END-DELETE","END-DISPLAY","END-DIVIDE","END-EVALUATE","END-IF","END-INVOKE","END-MULTIPLY","END-OF-PAGE","END-PERFORM","END-READ","END-RECEIVE","END-RETURN","END-REWRITE","END-SEARCH","END-START","END-STRING","END-SUBTRACT","END-UNSTRING","END-WRITE","ENDING","ENTER","ENTRY","ENVIRONMENT","ENVIRONMENT-DIVISION","EOL","EOP","EOS","EQUAL","EQUALS","ERASE","ERROR","ESCAPE","ESI","EVALUATE","EVERY","EXAMINE","EXCEEDS","EXCEPTION","EXCESS-3","EXCLUSIVE","EXEC","EXECUTE","EXHIBIT","EXIT","EXTEND","EXTENDED-SEARCH","EXTERNAL","EXTERNALLY-DESCRIBED-KEY","FACTORY","FALSE","FH--FCD","FH--KEYDEF","FILE","FILE-CONTROL","FILE-ID","FILE-LIMIT","FILE-LIMITS","FILE-PREFIX","FILLER","FINAL","FIRST","FIXED","FOOTING","FOOTING","FOR","FOREGROUND-COLOR","FOREGROUND-COLOUR","FORMAT","FROM","FULL","FUNCTION","GENERATE","GIVING","GLOBAL","GO","GOBACK","GREATER","GRID","GROUP","HEADING","HIGH","HIGH-VALUE","HIGH-VALUES","HIGHLIGHT","I-O-CONTROL","IDENTIFICATION","IGNORE","INDEX","INDEXED","INDIC","INDICATE","INDICATOR","INDICATORS","INHERITING","INITIAL","INITIALIZE","INITIATE","INPUT","INPUT-OUTPUT","INSERT","INSPECT","INSTALLATION","INTO","INVALID","INVOKE","INVOKED","JAPANESE","JUST","JUSTIFIED","KANJI","KEPT","KEY","KEYBOARD","LABEL","LAST","LEADING","LEAVE","LEFT","LEFT-JUSTIFY","LEFTLINE","LENGTH","LENGTH-CHECK","LESS","LIMIT","LIMITS","LIN","LINAGE","LINAGE-COUNTER","LINE","LINE-COUNTER","LINES","LINKAGE","LOCAL-STORAGE","LOCK","LOCK-HOLDING","LOCKING","LOW","LOW-VALUE","LOW-VALUES","LOWER","LOWLIGHT","MANUAL","MASS-UPDATE","MASTER-INDEX","MEMORY","MERGE","MESSAGE","METHOD","MODE","MODIFIED","MODULES","MORE-LABELS","MOVE","MULTIPLE","MULTIPLY","NAME","NAMED","NATIONAL","NATIONAL-EDITED","NATIVE","NCHAR","NEGATIVE","NEXT","NO-ECHO","NOMINAL","NOT","NOTE","NSTD-REELS","NULL","NULLS","NUMBER","NUMERIC","NUMERIC-EDITED","NUMERIC-FILL","O-FILL","OBJECT","OBJECT-COMPUTER","OBJECT-STORAGE","OCCURS","OFF","OMITTED","OOSTACKPTR","OPEN","OPTIONAL","ORDER","ORGANIZATION","OTHER","OTHERS","OTHERWISE","OUTPUT","OVERFLOW","OVERLINE","PACKED-DECIMAL","PACKED-DECIMAL","PADDING","PAGE","PAGE-COUNTER","PARAGRAPH","PASSWORD","PERFORM","PIC","PICTURE","PLUS","POINTER","POP-UP","POS","POSITION","POSITIONING","POSITIVE","PREVIOUS","PRINT","PRINT-CONTROL","PRINT-SWITCH","PRINTER","PRINTER-1","PRINTING","PRIOR","PRIVATE","PROCEDURE","PROCEDURE-POINTER","PROCEDURES","PROCEED","PROCESS","PROCESSING","PROGRAM","PROGRAM-ID","PROMPT","PROTECTED","PUBLIC","PURGE","QUEUE","QUOTE","QUOTES","RANDOM","RANGE","READ","READERS","READY","RECEIVE","RECEIVE-CONTROL","RECORD","RECORD-OVERFLOW","RECORDING","RECORDS","REDEFINES","REEL","REFERENCE","REFERENCES","RELATIVE","RELEASE","RELOAD","REMAINDER","REMARKS","REMOVAL","RENAMES","REORG-CRITERIA","REPEATED","REPLACE","REPLACING","REPORT","REPORTING","REPORTS","REQUIRED","REREAD","RERUN","RESERVE","RESET","RESIDENT","RETURN","RETURN-CODE","RETURNING","REVERSE","REVERSE-VIDEO","REVERSED","REWIND","REWRITE","RIGHT","RIGHT-JUSTIFY","ROLLBACK","ROLLING","ROUNDED","RUN","S01","S02","S03","S04","S05","SAME","SCREEN","SCROLL","SEARCH","SECTION","SECURE","SECURITY","SEEK","SEGMENT","SEGMENT-LIMIT","SELECT","SELECTIVE","SELF","SELFCLASS","SEND","SENTENCE","SEPARATE","SEQUENCE","SEQUENTIAL","SERVICE","SET","SETSHADOW","SHIFT-IN","SHIFT-OUT","SIGN","SIZE","SKIP1","SKIP2","SKIP3","SORT","SORT-CONTROL","SORT-CORE-SIZE","SORT-FILE-SIZE","SORT-MERGE","SORT-MESSAGE","SORT-MODE-SIZE","SORT-OPTION","SORT-RETURN","SOURCE","SOURCE-COMPUTER","SPACE","SPACE-FILL","SPACES","SPACES","SPECIAL-NAMES","STANDARD","STANDARD-1","STANDARD-2","START","STARTING","STATUS","STOP","STORE","STRING","SUB-QUEUE-1","SUB-QUEUE-2","SUB-QUEUE-3","SUBFILE","SUBTRACT","SUM","SUPER","SUPPRESS","SYMBOLIC","SYNC","SYNCHRONIZED","SYSIN","SYSIPT","SYSLST","SYSOUT","SYSPCH","SYSPUNCH","SYSTEM-INFO","TAB","TALLYING","TAPE","TERMINAL","TERMINAL-INFO","TERMINATE","TEST","TEXT","THAN","THEN","THROUGH","THRU","TIME","TIME-OF-DAY","TIME-OUT","TIMEOUT","TIMES","TITLE","TOP","TOTALED","TOTALING","TRACE","TRACK-AREA","TRACK-LIMIT","TRACKS","TRAILING","TRAILING-SIGN","TRANSACTION","TRANSFORM","TRUE","TYPE","TYPEDEF","UNDERLINE","UNDERLINED","UNEQUAL","UNIT","UNLOCK","UNSTRING","UNTIL","UPDATE","UPDATERS","UPON","UPPER","UPSI-0","UPSI-1","UPSI-2","UPSI-3","UPSI-4","UPSI-5","UPSI-6","UPSI-7","USAGE","USE","USER","USING","VALUE","VALUES","VARIABLE","VARYING","WAIT","WHEN","WHEN-COMPILED","WINDOW","WITH","WORDS","WORKING-STORAGE","WRAP","WRITE","WRITE-ONLY","WRITE-VERIFY","WRITERSZERO","ZERO","ZERO-FILL","ZEROES","ZEROS",NULL};

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

static void
calltip_manager_init (CalltipManager  *object)
{
	CalltipManagerDetails *calltipmgdet;
	calltipmgdet = CALLTIP_MANAGER_GET_PRIVATE(object);
  /* init calltips table*/
  calltip_manager_load_api_file("php-gphpedit.api", &calltipmgdet->php_api_tree);
  calltip_manager_load_api_file("css.api", &calltipmgdet->css_api_tree);
  calltip_manager_load_api_file("c.api", &calltipmgdet->cxx_api_tree);
  calltip_manager_function_list_from_array_prepare(cobol_keyword, &calltipmgdet->cobol_api_tree);
  calltip_manager_function_list_from_array_prepare(sql_keywords, &calltipmgdet->sql_api_tree);

  calltipmgdet->cache_str[0]=0;
  calltipmgdet->cache_completion = NULL;
}

static void
calltip_manager_finalize (GObject *object)
{
  CalltipManager *calltipmg = CALLTIP_MANAGER(object);
  CalltipManagerDetails *calltipmgdet;
	calltipmgdet = CALLTIP_MANAGER_GET_PRIVATE(calltipmg);

  if (calltipmgdet->css_api_tree){
     g_tree_destroy(calltipmgdet->css_api_tree);
  }
  if (calltipmgdet->php_api_tree){
     g_tree_destroy(calltipmgdet->php_api_tree);
  }
  if (calltipmgdet->cxx_api_tree){
     g_tree_destroy(calltipmgdet->cxx_api_tree);
  }
  if (calltipmgdet->completion_list_tree != NULL) {

    g_string_free (calltipmgdet->completion_list_tree,TRUE);	
  }
  if (calltipmgdet->cache_completion) g_free(calltipmgdet->cache_completion);

	G_OBJECT_CLASS (calltip_manager_parent_class)->finalize (object);
}


CalltipManager *calltip_manager_new (void)
{
	CalltipManager *calltipmg;
  calltipmg = g_object_new (CALLTIP_MANAGER_TYPE, NULL);
  
	return calltipmg; /* return new object */
}
/*
* calltip_manager_load_api_file (internal function)
* loads APi file content in the given GTree
*/
static void calltip_manager_load_api_file(const gchar *api_filename, GTree **api_tree)
{
  FILE *apifile;
  char buffer[MAX_API_LINE_LENGTH];
  gchar *api_dir = NULL;
  /* use autoconf macro to build api file path */
  api_dir = g_build_path (G_DIR_SEPARATOR_S, API_DIR, api_filename, NULL);
  gphpedit_debug_message(DEBUG_CALLTIP, "API PATH:'%s'", api_dir);
  apifile = fopen(api_dir, "r");
  if(apifile) {
    *api_tree=g_tree_new_full((GCompareDataFunc) g_utf8_collate, NULL, g_free, NULL);
    while(fgets( buffer, MAX_API_LINE_LENGTH, apifile ) != NULL ) {
      gchar *line=g_strdup(buffer);
      gchar *token_line = line;
      gchar *function_name = strtok(token_line, "|");
      gphpedit_debug_message(DEBUG_CALLTIP, "function name:%s",function_name);
      g_tree_insert (*api_tree, function_name, (line + strlen(function_name)+1));
      //g_free(line);
    }
    fclose( apifile );
  } else {
    g_warning(_("Could not open %s file\n"), api_filename);
  }
  g_free(api_dir);
}

/*
* load keywords string in a GTree to make search faster.
*/
static void calltip_manager_function_list_from_array_prepare(gchar **keywords, GTree **api_tree)
{
  gphpedit_debug(DEBUG_CALLTIP);
  guint n;

  *api_tree=g_tree_new_full((GCompareDataFunc) g_utf8_collate, NULL, g_free, NULL);
  for (n = 0; keywords[n]!=NULL; n++) {
    g_tree_insert (*api_tree, g_strdup(keywords[n]), ""); /* we only need the key */
  }
}

static gchar *get_css_api_line(CalltipManagerDetails *calltipmgdet, gchar *buffer)
{
  gchar *callti = NULL;
  gchar *value = g_tree_lookup (calltipmgdet->css_api_tree, buffer);
  if (value){
  gchar *copy=g_strdup (value);
  /* replace | with \n */
  gchar *description = strstr(copy, "|");
  strncpy(description,"\n",1);
  /* make calltip */
  callti=g_strdup_printf ("%s %s", buffer,copy);

  gphpedit_debug_message(DEBUG_CALLTIP, "calltip:%s\n",callti);

  g_free(copy);	
  }
  return callti;
}

static gchar *get_php_api_line(CalltipManagerDetails *calltipmgdet, gchar *buffer)
{
  gchar *return_value;
  gchar *params;
  gchar *description;
  gchar *token_line, *copy_line;
  gchar *result = NULL;
  gchar *value=g_tree_lookup (calltipmgdet->php_api_tree, buffer);
  if (value){
    token_line = g_strdup (value);
    copy_line = token_line;
    return_value = strtok(token_line, "|");
    params = strtok(NULL, "|");
    description = strtok(NULL, "|");
    /* make calltip */
    result=g_strdup_printf ("%s %s %s\n%s",return_value,buffer,params,description);
    gphpedit_debug_message(DEBUG_CALLTIP, "calltip: %s\n",result);
    g_free(copy_line);	
  } else {
    /*maybe a custom function*/
    result= classbrowser_custom_function_calltip(GPHPEDIT_CLASSBROWSER(main_window.classbrowser), buffer, TAB_PHP);
  }
  return result;
}

static gchar *get_cxx_api_line(CalltipManagerDetails *calltipmgdet, gchar *buffer)
{
  gchar *return_value;
  gchar *params;
  gchar *description;
  gchar *token_line, *copy_line;
  gchar *result = NULL;
  gchar *value=g_tree_lookup (calltipmgdet->php_api_tree, buffer);
  if (value){
    token_line = g_strdup (value);
    copy_line = token_line;
    return_value = strtok(token_line, "|");
    params = strtok(NULL, "|");
    description = strtok(NULL, "|");
    /* make calltip */
    result=g_strdup_printf ("%s %s %s\n%s",return_value,buffer,params,description);
    gphpedit_debug_message(DEBUG_CALLTIP, "calltip: %s\n",result);
    g_free(copy_line);	
  } else {
    /*maybe a custom function*/
    result= classbrowser_custom_function_calltip(GPHPEDIT_CLASSBROWSER(main_window.classbrowser), buffer, TAB_CXX);
  }
  return result;
}

/*
* function to show the tool tip with a short description about the
* function. The current word at the cursor is used to find the
* corresponding function from the api file
*/
gchar *calltip_manager_show_call_tip(CalltipManager *calltipmg, gint type, gchar *prefix)
{
  CalltipManagerDetails *calltipmgdet;
	calltipmgdet = CALLTIP_MANAGER_GET_PRIVATE(calltipmg);
  gchar *result = NULL;
  switch(type) {
    case TAB_PHP:
      result = get_php_api_line(calltipmgdet, prefix);
      break;
    case TAB_CSS:
      result = get_css_api_line(calltipmgdet, prefix);
      break;
    case TAB_CXX:
      result = get_cxx_api_line(calltipmgdet, prefix);
    default:
      break;
  }
  return result;
}

static gboolean make_completion_string (gpointer key, gpointer value, gpointer data){
  CalltipManagerDetails *calltipmgdet = (CalltipManagerDetails *) data;
  if(g_str_has_prefix(key, calltipmgdet->prefix)){
	    if (!calltipmgdet->completion_list_tree) {
	      calltipmgdet->completion_list_tree = g_string_new(key);
        calltipmgdet->completion_list_tree = g_string_append(calltipmgdet->completion_list_tree, "?2");
	    } else {
        g_string_append_printf (calltipmgdet->completion_list_tree," %s?2", (gchar *)key);
      }
  }
  if (strncmp(key, calltipmgdet->prefix,MIN(strlen(key),strlen(calltipmgdet->prefix)))>0){
    return TRUE;
  }
  return FALSE;
}

GSList *list = NULL;
static gboolean make_completion_list (gpointer key, gpointer value, gpointer data)
{
  gchar *prefix = (gchar *)data;
  if(g_str_has_prefix(key, prefix)){
      gchar *string=g_strdup_printf("%s?2",(gchar *)key); /*must free when no longer needed*/
	    list = g_slist_prepend(list, string);
      }
  if (strncmp(key, prefix,MIN(strlen(key),strlen(prefix)))>0){
    return TRUE;
  }
  return FALSE;
}

static inline void clear_list(void){
  g_slist_foreach (list,(GFunc) g_free,NULL);
  g_slist_free(list);
  list=NULL;
}

gchar *calltip_manager_cobol_autocomplete_word(CalltipManager *calltipmg, gchar *buffer) 
{
  CalltipManagerDetails *calltipmgdet;
	calltipmgdet = CALLTIP_MANAGER_GET_PRIVATE(calltipmg);

  GString *result=NULL;

  calltipmgdet->prefix = g_ascii_strup (buffer,-1);
  if (calltip_manager_has_cache(calltipmgdet->prefix, calltipmgdet->cache_str, calltipmgdet->cache_completion)){
    result = calltip_manager_get_autocomp_from_cache(calltipmgdet->prefix, calltipmgdet->cache_str, calltipmgdet->cache_completion);
  } else {
    g_tree_foreach (calltipmgdet->cobol_api_tree, make_completion_list, calltipmgdet->prefix);
    gchar *custom = classbrowser_add_custom_autocompletion(GPHPEDIT_CLASSBROWSER(main_window.classbrowser), buffer, TAB_COBOL, list);
    //FIXME: add custom variables support
    result = g_string_new(custom);
    if (custom) g_free(custom);
    calltip_manager_save_result_in_cache(calltipmgdet, result->str, buffer);
    clear_list();
  }
  g_free(calltipmgdet->prefix);
  gphpedit_debug_message(DEBUG_CALLTIP,"Autocomplete list: %s\n", result->str);
  return g_string_free(result,FALSE);
}

gchar *calltip_manager_sql_autocomplete_word(CalltipManager *calltipmg, gchar *buffer)
{
  CalltipManagerDetails *calltipmgdet;
	calltipmgdet = CALLTIP_MANAGER_GET_PRIVATE(calltipmg);

  gchar *result = NULL;
  calltipmgdet->prefix = g_ascii_strup (buffer,-1);
  if (calltip_manager_has_cache(calltipmgdet->prefix, calltipmgdet->cache_str, calltipmgdet->cache_completion)){
    GString *res=NULL;
    res = calltip_manager_get_autocomp_from_cache(calltipmgdet->prefix, calltipmgdet->cache_str, calltipmgdet->cache_completion);
    result = g_string_free(res, FALSE);
  } else {
  g_tree_foreach (calltipmgdet->sql_api_tree, make_completion_string, calltipmgdet);
  if (calltipmgdet->completion_list_tree != NULL) {
    result = g_string_free (calltipmgdet->completion_list_tree, FALSE);
    calltipmgdet->completion_list_tree=NULL;
    calltip_manager_save_result_in_cache(calltipmgdet, result, calltipmgdet->prefix);
  }
  }
  g_free(calltipmgdet->prefix);

  gphpedit_debug_message(DEBUG_CALLTIP,"Autocomplete list: %s\n", result);

  return result;
}

gchar *calltip_manager_css_autocomplete_word(CalltipManager *calltipmg, gchar *buffer)
{
  CalltipManagerDetails *calltipmgdet;
	calltipmgdet = CALLTIP_MANAGER_GET_PRIVATE(calltipmg);

  gchar *result = NULL;
  calltipmgdet->prefix = g_strdup (buffer);
  if (calltip_manager_has_cache(calltipmgdet->prefix, calltipmgdet->cache_str, calltipmgdet->cache_completion)){
    GString *res=NULL;
    res = calltip_manager_get_autocomp_from_cache(calltipmgdet->prefix, calltipmgdet->cache_str, calltipmgdet->cache_completion);
    result = g_string_free(res, FALSE);
  } else {
  g_tree_foreach (calltipmgdet->css_api_tree, make_completion_string, calltipmgdet);
  if (calltipmgdet->completion_list_tree != NULL) {
    result = g_string_free (calltipmgdet->completion_list_tree, FALSE);
    calltipmgdet->completion_list_tree=NULL;
    calltip_manager_save_result_in_cache(calltipmgdet, result, calltipmgdet->prefix);
  }
  }

  g_free(calltipmgdet->prefix);

  gphpedit_debug_message(DEBUG_CALLTIP,"Autocomplete list: %s\n", result);

  return result;
}

gboolean calltip_manager_has_cache(gchar *buffer, gchar *cache_str, gchar *cache_completion)
{
  gint len = strlen(cache_str);
  return (len !=0 && strlen(buffer) > len && g_str_has_prefix(buffer, cache_str));
}

GString *calltip_manager_get_autocomp_from_cache(gchar *buffer, gchar *cache_str, gchar *cache_completion)
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
    if (g_str_has_prefix(strings[i],buffer)){
       result = g_string_append(result, strings[i]);
       result = g_string_append(result, " ");
    }    
    i++;    
  }
  g_strfreev (strings);
  g_free(cache_completion);
  cache_completion=g_strdup(result->str);
  strncpy(cache_str,buffer,MIN(strlen(buffer),200));

  return result;
}

static void calltip_manager_save_result_in_cache(CalltipManagerDetails *calltipmgdet, gchar *result, gchar *search_word){
    if (calltipmgdet->cache_completion) g_free(calltipmgdet->cache_completion);
    calltipmgdet->cache_completion = g_strdup(result);
    strncpy(calltipmgdet->cache_str,search_word,MIN(strlen(search_word),200));
}

gchar *calltip_manager_php_autocomplete_word(CalltipManager *calltipmg, gchar *buffer)
{
  CalltipManagerDetails *calltipmgdet;
	calltipmgdet = CALLTIP_MANAGER_GET_PRIVATE(calltipmg);

  GString *result=NULL;

  if (calltip_manager_has_cache(buffer, calltipmgdet->cache_str, calltipmgdet->cache_completion)){
    result = calltip_manager_get_autocomp_from_cache(buffer, calltipmgdet->cache_str, calltipmgdet->cache_completion);
  } else { 
    g_tree_foreach (calltipmgdet->php_api_tree, make_completion_list, buffer);
    /* add custom php functions */
    gchar *custom = classbrowser_add_custom_autocompletion(GPHPEDIT_CLASSBROWSER(main_window.classbrowser), buffer, TAB_PHP, list);
    result = g_string_new(custom);
    if (custom) g_free(custom);
    calltip_manager_save_result_in_cache(calltipmgdet, result->str, buffer);
    clear_list();
  }
  gphpedit_debug_message(DEBUG_CALLTIP,"Autocomplete list: %s\n", result->str);
  return g_string_free(result,FALSE);
}

gchar *calltip_manager_cxx_autocomplete_word(CalltipManager *calltipmg, gchar *buffer)
{

  CalltipManagerDetails *calltipmgdet;
	calltipmgdet = CALLTIP_MANAGER_GET_PRIVATE(calltipmg);

  GString *result=NULL;

  if (calltip_manager_has_cache(buffer, calltipmgdet->cache_str, calltipmgdet->cache_completion)){
    result = calltip_manager_get_autocomp_from_cache(buffer, calltipmgdet->cache_str, calltipmgdet->cache_completion);
  } else {
    g_tree_foreach (calltipmgdet->cxx_api_tree, make_completion_list, buffer);
    gchar *custom = classbrowser_add_custom_autocompletion(GPHPEDIT_CLASSBROWSER(main_window.classbrowser), buffer, TAB_CXX, list);
    //FIXME: add custom variables support
    result = g_string_new(custom);
    if (custom) g_free(custom);
    calltip_manager_save_result_in_cache(calltipmgdet, result->str, buffer);
    clear_list();
  }
  gphpedit_debug_message(DEBUG_CALLTIP,"Autocomplete list: %s\n", result->str);
  return g_string_free(result,FALSE);
}

gchar *calltip_manager_autocomplete_word(CalltipManager *calltipmg, gint type, gchar *buffer)
{
  if (!calltipmg || !buffer) return NULL;
  gchar *result = NULL;
  switch(type) {
    case TAB_PHP:
      result = calltip_manager_php_autocomplete_word(calltipmg, buffer);
      break;
    case TAB_CSS:
      result = calltip_manager_css_autocomplete_word(calltipmg, buffer);
      break;
    case TAB_SQL:
      result = calltip_manager_sql_autocomplete_word(calltipmg, buffer);
      break;
    case TAB_COBOL:
      result = calltip_manager_cobol_autocomplete_word(calltipmg, buffer);
      break;
    case TAB_CXX:
      result = calltip_manager_cxx_autocomplete_word(calltipmg, buffer);
      break;
    default:
      break;
  }
  return result;
}
