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
#include <string.h>

#include "debug.h"
#include "symbol_bd_php_parse.h"
#include "gvfs_utils.h"
#include "document_manager.h"

static gchar *read_text_file(gchar *filename) 
{
  gchar *buffer=NULL;

  Documentable *document;
  gphpedit_debug(DEBUG_CLASSBROWSER);
  DocumentManager *docmg = document_manager_new();
  document = document_manager_find_documentable_from_filename (docmg, filename);
  if (document) buffer = documentable_get_text (document);
  g_object_unref(docmg);
  if (!buffer){
    buffer = read_text_file_sync(filename);
  }
//  g_print("buffer:<---\n%s\n--->",buffer);
  return buffer;
}

static gboolean is_whitespace(gchar character)
{
/*
  Determines whether a character is a control character or a white-space character
*/
  return (g_ascii_iscntrl(character) || g_ascii_isspace(character));
}


static gboolean is_identifier_char(gchar character)
{
  return (g_ascii_isalnum (character) || (character == '_'));
}


static gboolean is_opening_brace(gchar character)
{
  return (character == '{');
}


static gboolean is_closing_brace(gchar character)
{
  return (character == '}');
}


static gboolean is_opening_parenthesis(gchar character)
{
  return (character == '(');
}


static gboolean is_closing_parenthesis(gchar character)
{
  return (character == ')');
}


static gboolean is_newline(gchar character)
{
  return (character == '\n');
}

//Check if the search string is found from the current position backwards
//returns true if found else false
static gboolean check_previous(gchar *original, gchar *current, gchar *search)
{
  gint search_len;
  
  //Move forward 1 position so that the search will include the current position
  current = current + 1;

  search_len = strlen(search);

  // First, make sure we're not going to go out of bounds.
  if (current - original < search_len) {
    return FALSE;
  }

  // Second, go back from current search_len positions
  current = current - search_len ;

  // Then check to see if we match
  if (g_ascii_strncasecmp(current, search, search_len)==0) {
    return TRUE;
  }

  return FALSE;
}

// Check if the character before the search string is a valid character that will
// make the character + search string a valid token. Used to find out if the char
// before the occurrence of strings 'class' and 'function' is a non-identifier char
// and non-$ char
// Eg: xfunction should return false so should $class
static gboolean non_letter_before(gchar *original, gchar *current, gchar *search)
{
  gint search_len;

  search_len = strlen(search);

  // First, make sure we're not going to go out of bounds.
  if (current-original < (search_len+1)) {
    return FALSE;
  }

  // Second, go back from current search_len positions
  current = current - (search_len);

  // Then check to see if we match
  if (!is_identifier_char(*current) && *current != '$') {
    return TRUE;
  }

  return FALSE;
}

#ifdef DEBUG
static void str_sec_print(gchar *label, gchar *pstart, gchar *pend, guint line_number)
{
  gchar *teststring;
  guint length;
  length = pend - pstart + 1;
  teststring = g_malloc(length + 1);
  strncpy(teststring, pstart, length);
  teststring[length]='\0';
  gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE, "%5d:%s: %s", line_number, label, teststring);
  g_free(teststring);
}
#endif
void classbrowser_parse_file(SymbolBdPHP *symbolbd, gchar *filename)
{
  gchar *file_contents;
  gchar *o; // original pointer to start of contents
  gchar *c; // current position within contents
  #ifdef DEBUG
    //debug var
    gchar *sss, *dss, *scs, *mcs, *hss;
  #endif  

  gboolean within_php;
  gboolean within_single_line_comment;
  gboolean within_multi_line_comment;
  gboolean within_heredoc;
  //gboolean within_nowdoc;
  gboolean within_single_string;
  gboolean within_double_string;
  guint brace_count;
  guint parenthesis_count;
  guint line_number;

  gchar *heredoc_tag_start;
  gchar *heredoc_closingtag;
  guint heredoctag_length = 0;
  gboolean looking_for_heredocident;

  gchar *within_class;
  guint class_length = 0;
  gboolean looking_for_class_name;
  gboolean within_class_name;
  gchar *start_class_name = NULL;
  gchar *within_function;
  guint function_length = 0;
  gboolean looking_for_function_name;
  gboolean within_function_name;
  gchar *start_function_name = NULL;
  gboolean within_function_param_list;
  gchar *start_param_list;
  gchar *param_list;
  guint param_list_length;
  gboolean function_awaiting_brace_or_parenthesis;

  gboolean posiblevar=FALSE;
  gchar *startvarname=NULL;
  gchar *posvarname=NULL;
  gchar *varname=NULL;
  gchar *beforevarname=NULL;

  within_php = FALSE;
  within_single_line_comment = FALSE;
  within_multi_line_comment = FALSE;
  within_single_string = FALSE;
  within_double_string = FALSE;
  within_heredoc = FALSE;
  looking_for_heredocident = FALSE;
  heredoc_closingtag = NULL;
  heredoc_tag_start = NULL;
  
  brace_count = 0;
  line_number = 1;
  within_class = NULL;
  looking_for_class_name = FALSE;
  within_class_name = FALSE;
  within_function = NULL;
  looking_for_function_name = FALSE;
  within_function_name = FALSE;
  within_function_param_list = FALSE;
  start_param_list = NULL;
  param_list = NULL;
  function_awaiting_brace_or_parenthesis = FALSE;

  g_return_if_fail(filename);
  file_contents = read_text_file(filename);
  g_return_if_fail(file_contents);
  o = file_contents;
  c = o;

  while (*c) {
    if (!within_php) {
      if (check_previous(o, c, "<?")) {
        within_php=TRUE;
      }
    } else {
      if (within_single_line_comment && is_newline(*c)) {
        #ifdef DEBUG
        str_sec_print("SLC", scs, c, line_number);
        #endif
        within_single_line_comment = FALSE;
      }
      else if (within_multi_line_comment && check_previous(o, c, "*/")) {
      #ifdef DEBUG
        str_sec_print("MLC", mcs, c, line_number);
      #endif
        within_multi_line_comment = FALSE;
      }
      //escaped single quote within single quoted string does not end the string 
      //single quote ends single quoted string
      if (within_single_string && *c=='\'' && !check_previous(o, c, "\\'")) {
      #ifdef DEBUG
        str_sec_print("SQS", sss, c, line_number);
      #endif
        within_single_string = FALSE;
      }
      //escaped double quote within double quoted string does not end the string 
      //double quote ends double quoted string
      else if (within_double_string && *c=='"' && !check_previous(o, c, "\\\"")) {
      #ifdef DEBUG
        str_sec_print("DQS", dss, c, line_number);
      #endif
        within_double_string = FALSE;
      }
      ///heredocs have custom closing tags. check it from the opening tag
      else if (within_heredoc && !looking_for_heredocident && *c=='\n' && (check_previous(o, c-1, heredoc_closingtag) || (*(c-1) == ';' && check_previous(o, c-2, heredoc_closingtag)))) {
      #ifdef DEBUG
        gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE, "%s(%d): End Heredoc", filename, line_number);
        str_sec_print("HDS", hss, c, line_number);
      #endif
        g_free(heredoc_closingtag);
        within_heredoc = FALSE;
      }
      else if (within_heredoc && looking_for_heredocident && *c == '\n') {
        //if nowdoc
        if (*heredoc_tag_start == '\'') {
          //-2 for the two single quotes
          heredoctag_length = c - heredoc_tag_start - 2;           
          heredoc_closingtag = g_malloc(heredoctag_length + 1);
          strncpy(heredoc_closingtag, heredoc_tag_start + 1, heredoctag_length);
          heredoc_closingtag[heredoctag_length]='\0';
          #ifdef DEBUG
            gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE,"Expecting Nowdoc closing tag: %s\n", heredoc_closingtag);
          #endif
        }
        else {
          heredoctag_length = c - heredoc_tag_start;           
          heredoc_closingtag = g_malloc(heredoctag_length + 1);
          strncpy(heredoc_closingtag, heredoc_tag_start, heredoctag_length);
          heredoc_closingtag[heredoctag_length]='\0';
          #ifdef DEBUG
          gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE, "Expecting Heredoc closing tag: %s", heredoc_closingtag);
          #endif
        }
        looking_for_heredocident = FALSE;
      }
      //if not within comments or strings or heredocs
      else if (!within_multi_line_comment && !within_single_line_comment && !within_double_string && !within_single_string && !within_heredoc) {
        if (check_previous(o, c, "?>")) {
          within_php = FALSE;
        }
        //when does the second condition happen?
        //you are already outside a string. you can't have a backslash 
        //just before a new opening single quote
        else if (*c == '\'' && !check_previous(o, c, "\\'")) {
          within_single_string=TRUE;
          #ifdef DEBUG
          sss = c;
          gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE,"Found Single Quoted String: %d", line_number);
          #endif
        }
        //when does the second condition happen?
        else if (*c == '"' && !check_previous(o, c, "\\\"")) {
          within_double_string=TRUE;
          #ifdef DEBUG
          dss = c;
          gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE, "Found Double Quoted String: %d", line_number);
          #endif
        }
        //more efficient to call function only when needed hence the first check
        else if (*c == '<' && check_previous(o, c, "<<<")) {
          #ifdef DEBUG
          hss = c-2;
          gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE, "%s(%d): Found Heredoc", filename, line_number);
          #endif
          within_heredoc=TRUE;
          heredoc_tag_start = c+1;
          looking_for_heredocident = TRUE;
        }
        //more efficient to call function only when needed hence the first check
        else if (*c == '/' && check_previous(o, c, "//")) {
          #ifdef DEBUG
          scs = c-1;
          gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE, "%s(%d): Found Single Line Comment", filename, line_number);
          #endif
          within_single_line_comment = TRUE;
        }
        //more efficient to call function only when needed hence the first check
        else if (*c == '*' && check_previous(o, c, "/*")) {
          #ifdef DEBUG
          mcs = c-1;
          gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE, "%s(%d): Found Multi Line Comment", filename, line_number);
          #endif
          within_multi_line_comment = TRUE;
        }
        else {
          if (check_previous(o, c, "class ") && non_letter_before(o, c, "class ")) {
            #ifdef DEBUG
            gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE, "%s(%d): Found Class", filename, line_number);
            #endif
            looking_for_class_name = TRUE;
          }
          else if (is_identifier_char(*c) && looking_for_class_name && !within_class_name) {
            start_class_name = c-1;
            looking_for_class_name = FALSE;
            within_class_name = TRUE;
          }
          else if ( (is_whitespace(*c) || is_opening_brace(*c)) && within_class_name) {
            class_length = (c - start_class_name);
            if (within_class) {
              g_free(within_class);
            }
            within_class = g_malloc(class_length+1);
            strncpy(within_class, start_class_name, class_length);
            within_class[class_length]='\0';
            #ifdef DEBUG
              gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE, "%s(%d): Class '%s'", filename, line_number, within_class);
            #endif
            symbol_bd_php_classlist_add(symbolbd, within_class, filename, line_number);
            within_class_name = FALSE;
          }
          else if (check_previous(o, c, "function ") && non_letter_before(o, c, "function ")) {
            #ifdef DEBUG
              gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE,"%s","Looking for function name");
            #endif
            looking_for_function_name = TRUE;
          }
          if (is_identifier_char(*c) && looking_for_function_name && !within_function_name) {
            #ifdef DEBUG
              gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE,"%s", "Storing function name");
            #endif
            start_function_name = c;
            function_length = 0;
            looking_for_function_name = FALSE;
            within_function_name = TRUE;
          }
          if ( (is_whitespace(*c) || is_opening_brace(*c) || is_opening_parenthesis(*c)) && within_function_name && function_length==0) {
            #ifdef DEBUG
              gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE,"%s", "Found function");
            #endif
            function_length = (c - start_function_name);
            if (within_function) {
              g_free(within_function);
            }
            within_function = g_malloc(function_length+1);
            strncpy(within_function, start_function_name, function_length);
            within_function[function_length]='\0';
            function_awaiting_brace_or_parenthesis = TRUE;
            within_function_name = FALSE;
          }

          if ( function_awaiting_brace_or_parenthesis && is_opening_brace(*c)) {
            function_awaiting_brace_or_parenthesis = FALSE;
            if (within_class) {
              symbol_bd_php_functionlist_add(symbolbd,within_class, within_function, filename, line_number, NULL);
              #ifdef DEBUG
                gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE, "%s(%d): Class method %s::%s", filename, line_number, within_class, within_function);
              #endif
            }
            else {
              symbol_bd_php_functionlist_add(symbolbd,NULL, within_function, filename, line_number, NULL);
              #ifdef DEBUG
                gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE, "%s(%d): Function %s", filename, line_number, within_function);
              #endif
            }
          }
          else if (function_awaiting_brace_or_parenthesis && is_opening_parenthesis(*c)) {
            within_function_param_list = TRUE;
            start_param_list = c+1;
            function_awaiting_brace_or_parenthesis = FALSE;
          }
          else if (is_closing_parenthesis(*c) && within_function_param_list) {
            param_list_length = (c - start_param_list);
            if (param_list) {
              g_free(param_list);
            }
            param_list = g_malloc(param_list_length+1);
            strncpy(param_list, start_param_list, param_list_length);
            param_list[param_list_length]='\0';
            if (within_class) {
              symbol_bd_php_functionlist_add(symbolbd, within_class, within_function, filename, line_number, param_list);
              #ifdef DEBUG
                gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE, "%s(%d): Class method %s::%s(%s)", filename, line_number, within_class, within_function, param_list);
              #endif
            }
            else {
              symbol_bd_php_functionlist_add(symbolbd, NULL, within_function, filename,line_number, param_list);
              #ifdef DEBUG
                gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE, "%s(%d): Function %s(%s)", filename, line_number, within_function, param_list);
              #endif
            }
            within_function_param_list = FALSE;
          }
          if (posiblevar){
            if (is_identifier_char(*c)){
              posvarname=c;
            } else {
              posiblevar=FALSE;
            int len=posvarname - startvarname +1; /*include initial $*/
              if (len>1){ /*only if we have $ and something more */
                varname = g_malloc(len +1);
                strncpy(varname,startvarname,len);
                varname[len]='\0';
                if (!beforevarname){ beforevarname=g_strdup(varname); /*store last variable name found*/
                } else {
                  if (strcmp(beforevarname,varname)==0){
                    #ifdef DEBUG
                    gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE, "Duplicate variable: %s",varname);
                    #endif
                  } else {
                    #ifdef DEBUG
                    gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE, "Classbrowser var added:%s",varname);
                    #endif
                    symbol_bd_php_varlist_add(symbolbd, varname, within_function, filename);
                    g_free(beforevarname);
                    beforevarname=g_strdup(varname);
                  }
                }
                g_free(varname);
              }
            }
          }
          if (*c=='$' && !within_function_param_list && !within_multi_line_comment && !within_single_line_comment){ /* skip params vars */
            posiblevar=TRUE;
            startvarname=c;
          }
          if (is_opening_brace(*c)) {
            brace_count++;
            #ifdef DEBUG
              gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE,"Brace count %d:%c", brace_count, *c);
            #endif
          }
          else if (is_closing_brace(*c)) {
            brace_count--;
            #ifdef DEBUG
              gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE,"Brace count %d:%c", brace_count, *c);
            #endif
            if (brace_count == 0) {
              if (within_class) {
                #ifdef DEBUG
                  gphpedit_debug_message(DEBUG_CLASSBROWSER_PARSE, "Freeing class %s", within_class);
                #endif
                g_free(within_class);
                within_class = NULL;
              }
            }
          }
          else if (is_opening_parenthesis(*c)) {
            parenthesis_count++;
          }
          else if (is_closing_parenthesis(*c)) {
            parenthesis_count--;
          }
        }
      }
    }
    if (is_newline(*c)) {
      line_number++;
    }
    c++;
  }
  if (param_list) g_free(param_list);
  if (within_function) g_free(within_function);
  if (beforevarname) g_free(beforevarname);
  g_free(file_contents);
}
