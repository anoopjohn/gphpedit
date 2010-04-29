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
#include "calltip.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
//#define DEBUGCALLTIP
GSList *api_list;
GString *calltip=NULL;
#define MAX_API_LINE_LENGTH 16384


gchar *css_keywords[] = {"color","background-color", "background-image", "background-repeat", "background-attachment","background-position"," background", "font-family", "font-style","font-variant","font-weight", "font-size","font","word-spacing", "letter-spacing","text-decoration","vertical-align","text-transform","text-align","text-indent","line-height","margin-top","margin-right","margin-bottom","margin-left","margin","padding-top","padding-right","padding-bottom","padding-left","padding","border-top-width","border-right-width","border-bottom-width","border-left-width","border-width","border-top","border-right","border-bottom","border-left","border","border-color","border-style","width","height","float","clear","display","white-space","list-style-type","list-style-image","list-style-position","list-style","border-top-color","border-right-color","border-bottom-color","border-left-color","border-color","border-top-style","border-right-style","border-bottom-style","border-left-style","top","right","bottom left","position","z-index","direction","unicode-bidi","min-width","max-width","min-height","max-height","overflow","clip","visibility","content","quotes","counter-reset","counter-increment","marker-offset","size marks","page-break-before","page-break-after","page-break-inside","page","orphans", "widows","font-stretch","font-size-adjust","unicode-range","units-per-em","src","stemv","stemh","slope","cap-height","x-height","ascent","descent","widths","bbox","definition-src","baseline","centerline","mathline","topline","text-shadow","caption-side","table-layout","border-collapse","border-spacing","empty-cells","speak-header","cursor","outline","outline-width","outline-style","outline-color","volume","speak","pause-before","pause-after","pause","cue-before","cue-after","cue","play-during","azimuth","elevation","speech-rate","voice-family","pitch","pitch-range","stress","richness","speak-punctuation","speak-numeral",NULL};

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

void function_list_prepare(void)
{
	FILE *apifile;
	char buffer[MAX_API_LINE_LENGTH];
        gchar *api_dir = NULL;
	/* use autoconf macro to build api file path */
	api_dir = g_build_path (G_DIR_SEPARATOR_S, API_DIR, "php-gphpedit.api", NULL);
	#ifdef DEBUGCALLTIP
	g_print("DEBUG::API PATH:'%s'\n",api_dir);
	#endif
	apifile = fopen(api_dir, "r");
	if( apifile != NULL ) {
		while( fgets( buffer, MAX_API_LINE_LENGTH, apifile ) != NULL ) {
			gchar *line=g_strdup(buffer);
			/* From glib docs. Prepend and reverse list it's more eficient */
			api_list = g_slist_prepend(api_list, line);
		}
		api_list= g_slist_reverse (api_list);
		fclose( apifile );
	}
	else {
		g_print(_("WARNING: Could not open php-gphpedit.api file\n"));
	}
	g_free(api_dir);
}

GString *get_api_line(GtkWidget *scintilla, gint wordStart, gint wordEnd)
{
	GSList *walk;
	gchar *line;
	gchar *buffer = NULL;
	gchar *function_name;
	gchar *return_value;
	gchar *params;
	gchar *description;
	gint length;
	gchar *token_line, *copy_line;

	buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(scintilla), wordStart, wordEnd, &length);

	for (walk = api_list; walk != NULL; walk = g_slist_next (walk)) {
		line = walk->data;
		token_line = g_malloc(strlen(line)+1);
		copy_line = token_line;
		strcpy(token_line, line); // Line has trailing \n
		function_name = strtok(token_line, "|");
		return_value = strtok(NULL, "|");
		params = strtok(NULL, "|");
		description = strtok(NULL, "|");
		/* A full comparison of the function name is required for the tool tip
		a partial match will result in an incorrect tooltip. So we 
		have to use strcmp and not strncasecmp */
		if (strcmp(function_name, buffer)==0) {
			calltip = g_string_new(NULL);
			g_string_printf(calltip, "%s %s %s\n%s", return_value, function_name, params, description);
			g_free (buffer);
			g_free(copy_line);
			return calltip;
		}
		g_free(copy_line);
	}

	g_free (buffer);
	return NULL;
}

GString *complete_function_list(gchar *original_list)
{
	GSList *walk;
	gchar *line;
	GString *result;
	gchar *buffer = NULL;
	gchar *function_name;
	gchar *token_line, *copy_line;

	result = g_string_new(original_list);

	for (walk = api_list; walk != NULL; walk = g_slist_next (walk)) {
		line = walk->data;
		token_line = g_malloc(strlen(line)+1);
		copy_line = token_line;
		strcpy(token_line, line); // Line has trailing \n
		function_name = strtok(token_line, "|");
		if (result == NULL) {
			if (original_list==NULL) {
				result = g_string_new(function_name);
				result = g_string_append(result, " ");
			}
			else {
				result = g_string_new("");
			}
		}
		else {
			result = g_string_append(result, " ");
		}
		result = g_string_append(result, function_name);
		g_free(copy_line);
	}

	g_free (buffer);
	return result;
}


GString *get_completion_list(GtkWidget *scintilla, gint wordStart, gint wordEnd)
{
	GString *completion_list;
	GSList *walk;
	gchar *line;
	gchar *buffer = NULL;
	gchar *function_name;
	gint length;
	gchar *token_line, *copy_line;
	guint num_in_list;

	buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(scintilla), wordStart, wordEnd, &length);

	completion_list=NULL;
	num_in_list = 0;
	for (walk = api_list; (walk != NULL && num_in_list < 50); walk = g_slist_next (walk)) {
		line = walk->data;
		token_line = g_malloc(strlen(line)+1);
		copy_line = token_line;
		strcpy(token_line, line); // Line has trailing \n
		function_name = strtok(token_line, "|");
		if ((g_str_has_prefix(function_name, buffer)) || (wordStart==wordEnd)) {
			num_in_list++;
			if (completion_list == NULL) {
				completion_list = g_string_new(function_name);
			}
			else {
				completion_list = g_string_append(completion_list, " ");
				completion_list = g_string_append(completion_list, function_name);
			}
		}
		g_free(copy_line);
	}

	if (completion_list != NULL) {
		completion_list = g_string_append(completion_list, " ");
	}
	g_free (buffer);
	return completion_list;
}

void autocomplete_word(GtkWidget *scintilla, gint wordStart, gint wordEnd)
{
	GString *list;

	list = get_completion_list(scintilla, wordStart, wordEnd);

	if (list) {
		gtk_scintilla_autoc_show(GTK_SCINTILLA(scintilla), wordEnd-wordStart, list->str);
		g_string_free(list, FALSE);
	}
}


GString *get_css_completion_list(GtkWidget *scintilla, gint wordStart, gint wordEnd)
{
	GString *completion_list;
	gchar *buffer = NULL;
	gint length;
	guint n;

	buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(scintilla), wordStart, wordEnd, &length);

	completion_list=NULL;

	for (n = 0; css_keywords[n]!=NULL; n++) {
		if (g_str_has_prefix(css_keywords[n], buffer)) {
			if (completion_list == NULL) {
				completion_list = g_string_new(css_keywords[n]);
			}
			else {
				completion_list = g_string_append(completion_list, " ");
				completion_list = g_string_append(completion_list, css_keywords[n]);
			}
		}
	}

	if (completion_list != NULL) {
		completion_list = g_string_append(completion_list, " ");
	}
	g_free (buffer);
	return completion_list;
}


GString *get_sql_completion_list(GtkWidget *scintilla, gint wordStart, gint wordEnd)
{
	GString *completion_list;
	gchar *buffer = NULL;
	gint length;
	guint n;

	buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(scintilla), wordStart, wordEnd, &length);

	completion_list=NULL;

	for (n = 0; sql_keywords[n]!=NULL; n++) {
		if (g_str_has_prefix(sql_keywords[n], buffer)) {
			if (completion_list == NULL) {
				completion_list = g_string_new(sql_keywords[n]);
			}
			else {
				completion_list = g_string_append(completion_list, " ");
				completion_list = g_string_append(completion_list, sql_keywords[n]);
			}
		}
	}

	if (completion_list != NULL) {
		completion_list = g_string_append(completion_list, " ");
	}
	g_free (buffer);
	return completion_list;
}



void css_autocomplete_word(GtkWidget *scintilla, gint wordStart, gint wordEnd)
{
	GString *list;

	list = get_css_completion_list(scintilla, wordStart, wordEnd);

	if (list) {
		gtk_scintilla_autoc_show(GTK_SCINTILLA(scintilla), wordEnd-wordStart, list->str);
		g_string_free(list, FALSE);
	}
}


void sql_autocomplete_word(GtkWidget *scintilla, gint wordStart, gint wordEnd)
{
	GString *list;

	list = get_sql_completion_list(scintilla, wordStart, wordEnd);

	if (list) {
		gtk_scintilla_autoc_show(GTK_SCINTILLA(scintilla), wordEnd-wordStart, list->str);
		g_string_free(list, FALSE);
	}
}

//function to show the tool tip with a short description about the
//php function. The current word at the cursor is used to find the
//corresponding function from the php-gphpedit.api file
void show_call_tip(GtkWidget *scintilla, gint pos)
{
	gint wordStart;
	gint wordEnd;
	GString *api_line;

	wordStart = gtk_scintilla_word_start_position(GTK_SCINTILLA(scintilla), pos-1, TRUE);
	wordEnd = gtk_scintilla_word_end_position(GTK_SCINTILLA(scintilla), pos-1, TRUE);

        //function returns the global variable calltip. So does not have to free
	api_line = get_api_line(scintilla, wordStart, wordEnd);

	if (api_line != NULL) {
		gtk_scintilla_call_tip_show(GTK_SCINTILLA(scintilla), wordStart, api_line->str);
		// Fix as suggested by "urkle" in bug 55, comment out following line
		//g_string_free(api_line, TRUE);
	}
}
void clean_list_item (gpointer data, gpointer user_data){
     g_free (data);
}

void cleanup_calltip(void){
	if (api_list) {
	g_slist_foreach(api_list, (GFunc)clean_list_item, NULL);
	g_slist_free(api_list);
	}
}
