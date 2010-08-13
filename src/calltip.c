/* This file is part of gPHPEdit, a GNOME2 PHP Editor.
 
   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
	  
   For more information or to find the latest release, visit our 
   website at http://www.gphpedit.org/
 
   gPHPEdit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   gPHPEdit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with gPHPEdit.  If not, see <http://www.gnu.org/licenses/>.
 
   The GNU General Public License is contained in the file COPYING.
*/

#include <stdio.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "debug.h"

#include "calltip.h"
#include "tab.h"
#include "images.h"
#include "main_window.h"
#include "classbrowser_ui.h"

GTree *php_api_tree;
GTree *css_api_tree;
GString *completion_list_tree;
#define MAX_API_LINE_LENGTH 16384

gchar *cobol_keywords[] = {"ACCEPT","ACCESS","ACQUIRE","ACTUAL","ADD","ADDRESS","ADVANCING","AFTER","ALL","ALLOWING","ALPHABET","ALPHABETIC","ALPHABETIC-LOWER","ALPHABETIC-UPPER","ALPHANUMERIC","ALPHANUMERIC-EDITED","ALSO","ALTER","ALTERNATE","AND","ANY","APPLY","ARE","AREA","AREAS","ASCENDING","ASSIGN","ATTRIBUTE","AUTHOR","AUTO","AUTO-HYPHEN-SKIP","AUTO-SKIP","AUTOMATIC","AUTOTERMINATE","BACKGROUND-COLOR","BACKGROUND-COLOUR","BACKWARD","BASIS","BEEP","BEFORE","BEGINNING","BELL","BINARY","BLANK","BLINK","BLINKING","BLOCK","BOLD","BOTTOM","BOX","BOXED","CALL","CANCEL","CBL","CENTERED","CHAIN","CHAINING","CHANGED","CHARACTER","CHARACTERS","CHART","CLASS","CLOCK-UNITS","CLOSE","COBOL","CODE","CODE-SET","COL","COLLATING","COLOR","COLOUR","COLUMN","COM-REG","COMMA","COMMAND-LINE","COMMIT","COMMITMENT","COMMON","COMMUNICATION","COMP","COMP-0","COMP-1","COMP-2","COMP-3","COMP-4","COMP-5","COMP-6","COMP-X","COMPRESSION","COMPUTATIONAL","COMPUTATIONAL","COMPUTATIONAL-1","COMPUTATIONAL-2","COMPUTATIONAL-3","COMPUTATIONAL-4","COMPUTATIONAL-5","COMPUTATIONAL-6","COMPUTATIONAL-X","COMPUTE","CONFIGURATION","CONSOLE","CONTAINS","CONTENT","CONTINUE","CONTROL","CONTROL-AREA","CONTROLS","CONVERSION","CONVERT","CONVERTING","COPY","CORE-INDEX","CORR","CORRESPONDING","COUNT","CRT","CRT-UNDER","CSP","CURRENCY","CURRENT-DATE","CURSOR","CYCLE","CYL-INDEX","CYL-OVERFLOW","DATA","DATE","DATE-COMPILED","DATE-WRITTEN","DAY","DAY-OF-WEEK","DBCS","DEBUG","DEBUG-CONTENTS","DEBUG-ITEM","DEBUG-LINE","DEBUG-NAME","DEBUG-SUB-1","DEBUG-SUB-2","DEBUG-SUB-3","DEBUGGING","DECIMAL-POINT","DECLARATIVES","DEFAULT","DELETE","DELIMITED","DELIMITER","DEPENDING","DESCENDING","DESTINATION","DETAIL","DISABLE","DISK","DISP","DISPLAY","DISPLAY-1","DISPLAY-ST","DIVIDE","DIVISION","DOWN","DRAW","DROP","DUPLICATES","DYNAMIC","ECHO","EGCS","EGI","EJECT","ELSE","EMI","EMPTY-CHECK","ENABLE","ENCRYPTION","END","END-ACCEPT","END-ADD","END-CALL","END-CHAIN","END-COMPUTE","END-DELETE","END-DISPLAY","END-DIVIDE","END-EVALUATE","END-IF","END-INVOKE","END-MULTIPLY","END-OF-PAGE","END-PERFORM","END-READ","END-RECEIVE","END-RETURN","END-REWRITE","END-SEARCH","END-START","END-STRING","END-SUBTRACT","END-UNSTRING","END-WRITE","ENDING","ENTER","ENTRY","ENVIRONMENT","ENVIRONMENT-DIVISION","EOL","EOP","EOS","EQUAL","EQUALS","ERASE","ERROR","ESCAPE","ESI","EVALUATE","EVERY","EXAMINE","EXCEEDS","EXCEPTION","EXCESS-3","EXCLUSIVE","EXEC","EXECUTE","EXHIBIT","EXIT","EXTEND","EXTENDED-SEARCH","EXTERNAL","EXTERNALLY-DESCRIBED-KEY","FACTORY","FALSE","FH--FCD","FH--KEYDEF","FILE","FILE-CONTROL","FILE-ID","FILE-LIMIT","FILE-LIMITS","FILE-PREFIX","FILLER","FINAL","FIRST","FIXED","FOOTING","FOOTING","FOR","FOREGROUND-COLOR","FOREGROUND-COLOUR","FORMAT","FROM","FULL","FUNCTION","GENERATE","GIVING","GLOBAL","GO","GOBACK","GREATER","GRID","GROUP","HEADING","HIGH","HIGH-VALUE","HIGH-VALUES","HIGHLIGHT","I-O-CONTROL","IDENTIFICATION","IGNORE","INDEX","INDEXED","INDIC","INDICATE","INDICATOR","INDICATORS","INHERITING","INITIAL","INITIALIZE","INITIATE","INPUT","INPUT-OUTPUT","INSERT","INSPECT","INSTALLATION","INTO","INVALID","INVOKE","INVOKED","JAPANESE","JUST","JUSTIFIED","KANJI","KEPT","KEY","KEYBOARD","LABEL","LAST","LEADING","LEAVE","LEFT","LEFT-JUSTIFY","LEFTLINE","LENGTH","LENGTH-CHECK","LESS","LIMIT","LIMITS","LIN","LINAGE","LINAGE-COUNTER","LINE","LINE-COUNTER","LINES","LINKAGE","LOCAL-STORAGE","LOCK","LOCK-HOLDING","LOCKING","LOW","LOW-VALUE","LOW-VALUES","LOWER","LOWLIGHT","MANUAL","MASS-UPDATE","MASTER-INDEX","MEMORY","MERGE","MESSAGE","METHOD","MODE","MODIFIED","MODULES","MORE-LABELS","MOVE","MULTIPLE","MULTIPLY","NAME","NAMED","NATIONAL","NATIONAL-EDITED","NATIVE","NCHAR","NEGATIVE","NEXT","NO-ECHO","NOMINAL","NOT","NOTE","NSTD-REELS","NULL","NULLS","NUMBER","NUMERIC","NUMERIC-EDITED","NUMERIC-FILL","O-FILL","OBJECT","OBJECT-COMPUTER","OBJECT-STORAGE","OCCURS","OFF","OMITTED","OOSTACKPTR","OPEN","OPTIONAL","ORDER","ORGANIZATION","OTHER","OTHERS","OTHERWISE","OUTPUT","OVERFLOW","OVERLINE","PACKED-DECIMAL","PACKED-DECIMAL","PADDING","PAGE","PAGE-COUNTER","PARAGRAPH","PASSWORD","PERFORM","PIC","PICTURE","PLUS","POINTER","POP-UP","POS","POSITION","POSITIONING","POSITIVE","PREVIOUS","PRINT","PRINT-CONTROL","PRINT-SWITCH","PRINTER","PRINTER-1","PRINTING","PRIOR","PRIVATE","PROCEDURE","PROCEDURE-POINTER","PROCEDURES","PROCEED","PROCESS","PROCESSING","PROGRAM","PROGRAM-ID","PROMPT","PROTECTED","PUBLIC","PURGE","QUEUE","QUOTE","QUOTES","RANDOM","RANGE","READ","READERS","READY","RECEIVE","RECEIVE-CONTROL","RECORD","RECORD-OVERFLOW","RECORDING","RECORDS","REDEFINES","REEL","REFERENCE","REFERENCES","RELATIVE","RELEASE","RELOAD","REMAINDER","REMARKS","REMOVAL","RENAMES","REORG-CRITERIA","REPEATED","REPLACE","REPLACING","REPORT","REPORTING","REPORTS","REQUIRED","REREAD","RERUN","RESERVE","RESET","RESIDENT","RETURN","RETURN-CODE","RETURNING","REVERSE","REVERSE-VIDEO","REVERSED","REWIND","REWRITE","RIGHT","RIGHT-JUSTIFY","ROLLBACK","ROLLING","ROUNDED","RUN","S01","S02","S03","S04","S05","SAME","SCREEN","SCROLL","SEARCH","SECTION","SECURE","SECURITY","SEEK","SEGMENT","SEGMENT-LIMIT","SELECT","SELECTIVE","SELF","SELFCLASS","SEND","SENTENCE","SEPARATE","SEQUENCE","SEQUENTIAL","SERVICE","SET","SETSHADOW","SHIFT-IN","SHIFT-OUT","SIGN","SIZE","SKIP1","SKIP2","SKIP3","SORT","SORT-CONTROL","SORT-CORE-SIZE","SORT-FILE-SIZE","SORT-MERGE","SORT-MESSAGE","SORT-MODE-SIZE","SORT-OPTION","SORT-RETURN","SOURCE","SOURCE-COMPUTER","SPACE","SPACE-FILL","SPACES","SPACES","SPECIAL-NAMES","STANDARD","STANDARD-1","STANDARD-2","START","STARTING","STATUS","STOP","STORE","STRING","SUB-QUEUE-1","SUB-QUEUE-2","SUB-QUEUE-3","SUBFILE","SUBTRACT","SUM","SUPER","SUPPRESS","SYMBOLIC","SYNC","SYNCHRONIZED","SYSIN","SYSIPT","SYSLST","SYSOUT","SYSPCH","SYSPUNCH","SYSTEM-INFO","TAB","TALLYING","TAPE","TERMINAL","TERMINAL-INFO","TERMINATE","TEST","TEXT","THAN","THEN","THROUGH","THRU","TIME","TIME-OF-DAY","TIME-OUT","TIMEOUT","TIMES","TITLE","TOP","TOTALED","TOTALING","TRACE","TRACK-AREA","TRACK-LIMIT","TRACKS","TRAILING","TRAILING-SIGN","TRANSACTION","TRANSFORM","TRUE","TYPE","TYPEDEF","UNDERLINE","UNDERLINED","UNEQUAL","UNIT","UNLOCK","UNSTRING","UNTIL","UPDATE","UPDATERS","UPON","UPPER","UPSI-0","UPSI-1","UPSI-2","UPSI-3","UPSI-4","UPSI-5","UPSI-6","UPSI-7","USAGE","USE","USER","USING","VALUE","VALUES","VARIABLE","VARYING","WAIT","WHEN","WHEN-COMPILED","WINDOW","WITH","WORDS","WORKING-STORAGE","WRAP","WRITE","WRITE-ONLY","WRITE-VERIFY","WRITERSZERO","ZERO","ZERO-FILL","ZEROES","ZEROS",NULL};

gchar *sql_keywords[] = {"ADD", "ALL", "ALTER", "ANALYZE", "AND", "AS", "ASC", "ASENSITIVE", "AUTO_INCREMENT", 
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

void register_autoc_images(GtkScintilla *sci){
  gtk_scintilla_register_image(sci, 1, (const gchar *) function_xpm);
  gtk_scintilla_register_image(sci, 2, (const gchar *) bullet_blue_xpm);
  gtk_scintilla_register_image(sci, 3, (const gchar *) variable_xpm);
  gtk_scintilla_register_image(sci, 4, (const gchar *) class_xpm);
}

void function_list_prepare(void)
{
  FILE *apifile;
  char buffer[MAX_API_LINE_LENGTH];
  gchar *api_dir = NULL;
  /* use autoconf macro to build api file path */
  api_dir = g_build_path (G_DIR_SEPARATOR_S, API_DIR, "php-gphpedit.api", NULL);
  gphpedit_debug_message(DEBUG_CALLTIP, "API PATH:'%s'", api_dir);
  apifile = fopen(api_dir, "r");
  if( apifile != NULL ) {
    php_api_tree=g_tree_new_full((GCompareDataFunc) g_utf8_collate, NULL, g_free, NULL);
    while( fgets( buffer, MAX_API_LINE_LENGTH, apifile ) != NULL ) {
      gchar *line=g_strdup(buffer);
      gchar *token_line = line;
      gchar *function_name = strtok(token_line, "|");
      gphpedit_debug_message(DEBUG_CALLTIP, "function name:%s",function_name);
      g_tree_insert (php_api_tree, function_name, (line + strlen(function_name)+1));
      //g_free(token_line);
    }
    fclose( apifile );
  } else {
    g_warning(_("Could not open php-gphpedit.api file"));
  }
  g_free(api_dir);
}

void css_function_list_prepare(void)
{
  FILE *apifile;
  char buffer[MAX_API_LINE_LENGTH];
  gchar *api_dir = NULL;
  /* use autoconf macro to build api file path */
  api_dir = g_build_path (G_DIR_SEPARATOR_S, API_DIR, "css.api", NULL);
  gphpedit_debug_message(DEBUG_CALLTIP, "API PATH:'%s'", api_dir);
  apifile = fopen(api_dir, "r");
  if( apifile != NULL ) {
    css_api_tree=g_tree_new_full((GCompareDataFunc) g_utf8_collate, NULL, g_free, NULL);
    while( fgets( buffer, MAX_API_LINE_LENGTH, apifile ) != NULL ) {
      gchar *line=g_strdup(buffer);
      gchar *token_line = line;
      gchar *function_name = strtok(token_line, "|");
      gphpedit_debug_message(DEBUG_CALLTIP, "function name:%s",function_name);
      g_tree_insert (css_api_tree, function_name, (line + strlen(function_name)+1));
      //g_free(line);
    }
    fclose( apifile );
  } else {
    g_warning(_("Could not open php-gphpedit.api file"));
  }
  g_free(api_dir);
}
gchar *get_css_api_line(gchar *buffer)
{
  gchar *value=g_tree_lookup (css_api_tree, buffer);
  if (value){
  gchar *copy=g_strdup (value);
  /* replace | with \n */
  gchar *description = strstr(copy, "|");
  strncpy(description,"\n",1);
  /* make calltip */
  gchar *callti=g_strdup_printf ("%s %s", buffer,copy);

  gphpedit_debug_message(DEBUG_CALLTIP, "calltip:%s\n",callti);

  g_free(copy);	
  return callti;
  }
  return NULL;
}

gboolean make_completion_string (gpointer key, gpointer value, gpointer data){
  if(g_str_has_prefix(key, (gchar *)data)){
	    if (!completion_list_tree) {
	      completion_list_tree = g_string_new(key);
        completion_list_tree = g_string_append(completion_list_tree, "?2");
	    }else{
	      completion_list_tree = g_string_append(completion_list_tree, " ");
        completion_list_tree = g_string_append(completion_list_tree, key);
        completion_list_tree = g_string_append(completion_list_tree, "?2");
      }
  }
  if (strncmp(key, (gchar *)data,MIN(strlen(key),strlen(data)))>0){
    return TRUE;
  }
  return FALSE;
}

gchar *css_autocomplete_word(gchar *buffer)
{
  gchar *result = NULL;

  g_tree_foreach (css_api_tree, make_completion_string, buffer);
  if (completion_list_tree != NULL) {
    result = g_string_free (completion_list_tree, FALSE);
    completion_list_tree=NULL;
  }
  return result;
}

static gchar *get_api_line(gchar *buffer)
{
  gchar *return_value;
  gchar *params;
  gchar *description;
  gchar *token_line, *copy_line;

  gchar *value=g_tree_lookup (php_api_tree, buffer);
  if (value){
    token_line = g_strdup (value);
    copy_line = token_line;
    return_value = strtok(token_line, "|");
    params = strtok(NULL, "|");
    description = strtok(NULL, "|");
    /* make calltip */
    gchar *callti=g_strdup_printf ("%s %s %s\n%s",return_value,buffer,params,description);

    gphpedit_debug_message(DEBUG_CALLTIP, "calltip: %s\n",callti);

    g_free(copy_line);	
    return callti;
  } else {
  /*maybe a custom function*/
    gchar *result= classbrowser_custom_function_calltip(GPHPEDIT_CLASSBROWSER(main_window.classbrowser), buffer);
    return result;
  }
  return NULL;
}

GSList *list = NULL;
gboolean make_completion_list (gpointer key, gpointer value, gpointer data){
  if(g_str_has_prefix(key, (gchar *)data)){
      gchar *string=g_strdup_printf("%s?2",(gchar *)key); /*must free when no longer needed*/
	    list=g_slist_prepend(list,string);
      }
  if (strncmp(key, (gchar *)data,MIN(strlen(key),strlen(data)))>0){
    return TRUE;
  }
  return FALSE;
}
static inline void clear_list(void){
g_slist_foreach (list,(GFunc) g_free,NULL);
g_slist_free(list);
list=NULL;
}
char cache_str[200]={'1'}; /*is this enougth?*/
gchar *cache_completion;

gchar *autocomplete_word(gchar *buffer)
{
  GString *result=NULL;

  /*  Autocompletion optimization:
  *   we store last text typed and we compare with actual text. If current text typed
  *   refine last search we take that search and remove words that don't match new text
  *   so we improve performance a lot because we don't make another full search.
  */
  if (strlen(buffer) > strlen(cache_str) && g_str_has_prefix(buffer,cache_str)){ 
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
  }else{ 
  g_tree_foreach (php_api_tree, make_completion_list, buffer);
  /* add custom php functions */
  gchar *custom= classbrowser_add_custom_autocompletion(GPHPEDIT_CLASSBROWSER(main_window.classbrowser), buffer,list);
  result = g_string_new(custom);
  if (custom){
    if (cache_completion) g_free(cache_completion);
    cache_completion=g_strdup(custom);
    g_free(custom);
    strncpy(cache_str,buffer,MIN(strlen(buffer),200));
  }
  clear_list();
  }
  gphpedit_debug_message(DEBUG_CALLTIP,"Autocomplete list: %s\n", result->str);
  return g_string_free(result,FALSE);
}

gchar *cobol_autocomplete_word(gchar *buffer)
{
  GString *completion_list;
  gchar *result = NULL;
  guint n;

  completion_list=NULL;

  for (n = 0; cobol_keywords[n]!=NULL; n++) {
    if (g_str_has_prefix(cobol_keywords[n], buffer)) {
      if (completion_list == NULL) {
        completion_list = g_string_new(cobol_keywords[n]);
      } else {
        completion_list = g_string_append(completion_list, " ");
        completion_list = g_string_append(completion_list, cobol_keywords[n]);
      }
    }
  }

  if (completion_list != NULL) {
    completion_list = g_string_append(completion_list, " ");
    result = g_string_free(completion_list, FALSE);
  }

  gphpedit_debug_message(DEBUG_CALLTIP,"Autocomplete list: %s\n", result);

  return result;
}

gchar *sql_autocomplete_word(gchar *buffer)
{
  GString *completion_list;
  guint n;
  gchar *result = NULL;
  completion_list=NULL;

  for (n = 0; sql_keywords[n]!=NULL; n++) {
    if (g_str_has_prefix(sql_keywords[n], buffer)) {
        if (completion_list == NULL) {
          completion_list = g_string_new(sql_keywords[n]);
        } else {
          completion_list = g_string_append(completion_list, " ");
          completion_list = g_string_append(completion_list, sql_keywords[n]);
        }
    }
  }

  if (completion_list != NULL) {
    completion_list = g_string_append(completion_list, " ");
    result = g_string_free(completion_list, FALSE);
  }

  gphpedit_debug_message(DEBUG_CALLTIP,"Autocomplete list: %s\n", result);

  return result;
}

/*
* function to show the tool tip with a short description about the
* php function. The current word at the cursor is used to find the
* corresponding function from the api file
*/
gchar *show_call_tip(gint type, gchar *prefix)
{
  if (type==TAB_PHP){
    return get_api_line(prefix);
  } else {
    return get_css_api_line(prefix);
  }
}

void cleanup_calltip(void){
  
  gphpedit_debug(DEBUG_CALLTIP);

  if (php_api_tree){
     g_tree_destroy(php_api_tree);
  }
  if (completion_list_tree != NULL) {
    g_string_free (completion_list_tree,TRUE);	
  }
  if (css_api_tree){
     g_tree_destroy(css_api_tree);
  }
  if (cache_completion) g_free(cache_completion);
}

