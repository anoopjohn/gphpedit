/* This file is part of gPHPEdit, a GNOME2 PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009, 2010 José Rostagno (for vijona.com.ar) 

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

#include <stdlib.h>
#include "debug.h"
#include "syntax_check_manager.h"
#include "gvfs_utils.h"
#include "main_window_callbacks.h"

#define SYNTAX_CHECK_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    SYNTAX_CHECK_MANAGER_TYPE,\
					    SyntaxCheckManager_Details))

static void  syntax_check_manager_class_init (SyntaxCheckManagerClass *klass);
static void  syntax_check_manager_init (SyntaxCheckManager *klass);

/* http://library.gnome.org/devel/gobject/unstable/gobject-Type-Information.html#G-DEFINE-TYPE:CAPS */
G_DEFINE_TYPE(SyntaxCheckManager, syntax_check_manager, G_TYPE_OBJECT);

static void
syntax_check_manager_class_init (SyntaxCheckManagerClass *klass)
{
}

static void
syntax_check_manager_init (SyntaxCheckManager *object)
{
}

SyntaxCheckManager *syntax_check_manager_new (void)
{
	return g_object_new (SYNTAX_CHECK_MANAGER_TYPE, NULL); /* return new object */
}


/*
* (internal)
*/
gchar *run_php_lint(gchar *command_line)
{
  gboolean result;
  gchar *stdout;
  gint exit_status;
  GError *error;
  gchar *stdouterr;
  error = NULL;

  result = g_spawn_command_line_sync (command_line,
                                      &stdout, &stdouterr, &exit_status, &error);

  if (!result) return NULL;

  gchar *res =g_strdup_printf ("%s\n%s",stdouterr,stdout);
  g_free(stdouterr);
  g_free(stdout);
  return res;
}
/*
* (internal)
*/

gchar *process_php_lines(gchar *output)
{
  gchar *copy;
  gchar *token;
  gchar *line_number;
  GString *result;
  copy = output;
  result = g_string_new (NULL);
  copy=g_strdup_printf("\n%s", output);
  while ((token = strtok(copy, "\n"))) {
      if (g_str_has_prefix(token, "PHP Warning:  ")){
        token+=14; //skip 'PHP Warning:  '
        line_number = strstr(token, "line ");
        if (line_number){
        line_number+=5; /* len of 'line '*/
        }
        gint num=atoi(line_number);
        if (num>0) {
          g_string_append_printf (result, "%d W %s\n", num, token);
        } else {
          g_string_append_printf (result, "%s\n", token);
        }
      } else if (g_str_has_prefix(token,"PHP Parse error:  syntax error, ")){
        token+=32; // skip 'PHP Parse error:  syntax error, '
        line_number = strrchr(token, ' ');
        line_number++;
        if (atoi(line_number)>0) {
          g_string_append_printf (result, "%d E %s\n", atoi(line_number), token);
        } else {
          g_string_append_printf (result, "%s\n", token);
        }
      } else {
         if (!g_str_has_prefix(token, "Content-type")){
           g_string_append_printf (result, "%s\n", token);
         } 
      }
      copy = NULL;
    }
  gphpedit_debug_message(DEBUG_SYNTAX, "result:%s\n", result->str);
  return g_string_free (result,FALSE);

}
/*
* (internal)
*/

gchar *process_perl_lines(gchar *output)
{
  gchar *copy;
  gchar *token;
  gchar *line_number;
  copy = output;
  GString *result;
  result = g_string_new (NULL);
  gphpedit_debug_message(DEBUG_SYNTAX, "syntax:\n%s\n", output);
  gint quote=0;
  gint a=0;
  gchar *cop=copy;
  while (*cop!='\0'){
      if(*cop=='"' && quote==0) quote++;
      else if(*cop=='"' && quote!=0) quote--;
      if (*cop=='\n' && quote==1) *(copy +a)=' ';
      cop++;
      a++;
//      g_print("char:%c, quote:%d,pos:%d\n",*cop,quote,a);
      }      
      while ((token = strtok(copy, "\n"))) {
        gchar number[15];  
        int i=15;
        line_number = strstr(token, "line ");
        if (line_number){
        line_number+=5;
        while (*line_number!=',' && *line_number!='.' && i!=0){
        number[15-i]=*line_number;
        line_number++;
        i--;
        }
        number[i]='\0';
        }
        gint num=atoi(number);
        if (num>0) {
          if (g_str_has_prefix(token, "syntax error") || g_str_has_prefix(token, "Unrecognized character")){
            g_string_append_printf (result, "%d E %s\n", num, token);
          } else {
            g_string_append_printf (result, "%d %s\n", num, token);
          }
        }
        else {
           if (g_str_has_suffix(token, "syntax OK")) g_string_append_printf (result, "%s", "syntax OK\n");
           else g_string_append_printf (result, "%s\n", token);
        }
      number[0]='a'; /*force new number */
      copy = NULL;
    }
  return g_string_free (result,FALSE);
}


/*
* save_as_temp_file (internal)
* save the content of an editor and return the filename of the temp file or NULL on error.
*/
GString *save_as_temp_file(Documentable *document)
{
  gphpedit_debug(DEBUG_SYNTAX);
  gchar *write_buffer = documentable_get_text(document);
  GString *filename = text_save_as_temp_file(write_buffer);
  g_free(write_buffer);
  return filename;
}

GString *get_syntax_filename(Documentable *document, gboolean *using_temp)
{
  GString *filename = NULL;
  gchar *docfilename = documentable_get_filename(document);
  gboolean untitled, saved;
  g_object_get(document, "untitled", &untitled, "saved", &saved, NULL);
  if (saved && filename_is_native(docfilename) && !untitled) {
    gchar *local_path=filename_get_scaped_path(docfilename);
    filename = g_string_new(local_path);
    g_free(local_path);
    *using_temp = FALSE;
  } else {
    filename = save_as_temp_file(document);
    *using_temp = TRUE;
  }
  g_free(docfilename);
  return filename;
}
gchar *syntax_check_manager_run(Documentable *document)
{
  GString *command_line=NULL;
  gchar *output;
  gboolean using_temp;
  GString *filename = NULL;
  const gchar *php_binary_location;
  PreferencesManager *pref;
  gint type;
  g_object_get(document, "type", &type, NULL);

  command_line = g_string_new(NULL);
  switch(type) {
    case TAB_PHP:
      pref = preferences_manager_new ();
      g_object_get(main_window.prefmg, "php_binary_location", &php_binary_location, NULL);
      filename = get_syntax_filename(document, &using_temp);
      g_string_append_printf (command_line, "%s -q -l -d html_errors=Off -f '%s'", php_binary_location, filename->str);
      g_object_unref(pref);
      break;
    case TAB_PERL:
      filename = get_syntax_filename(document, &using_temp);
      g_string_append_printf(command_line, "perl -c '%s'", filename->str);
      break;
    default:
      return NULL;
      break;
  }
  gphpedit_debug_message(DEBUG_SYNTAX, "eject:%s\n", command_line->str);
  if (using_temp) release_temp_file (filename->str);
  g_string_free(filename, TRUE);

  output = run_php_lint(command_line->str);
  g_string_free(command_line, TRUE);
  gchar *result=NULL;
  if (output) {
    switch(type) {
      case TAB_PHP:
        result = process_php_lines(output);
        break;
      case TAB_PERL:
        result = process_perl_lines(output);
        break;
      default:
        break;
    }
    g_free(output);
  } else {
    result = g_strdup(_("Error calling PHP CLI (is PHP command line binary installed? If so, check if it's in your path or set php_binary in Preferences)\n"));
  }
  return result;
}
