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

#include <stdlib.h>
#include "syntax_check_manager.h"
#include "gvfs_utils.h"
#include "main_window_callbacks.h"

/*
* syntax_check_manager private struct
*/
struct Syntax_Check_Manager_Details
{
  gchar *dummy;
};

#define SYNTAX_CHECK_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    SYNTAX_CHECK_MANAGER_TYPE,\
					    Syntax_Check_Manager_Details))

static gpointer parent_class;
static void               syntax_check_manager_finalize         (GObject                *object);
static void               syntax_check_manager_init             (gpointer                object,
							       gpointer                klass);
static void  syntax_check_manager_class_init (Syntax_Check_ManagerClass *klass);

/*
 * syntax_check_manager_get_type
 * register Syntax_Check_Manager type and returns a new GType
*/
GType
syntax_check_manager_get_type (void)
{
    static GType our_type = 0;
    
    if (!our_type) {
        static const GTypeInfo our_info =
        {
            sizeof (Syntax_Check_ManagerClass),
            NULL,               /* base_init */
            NULL,               /* base_finalize */
            (GClassInitFunc) syntax_check_manager_class_init,
            NULL,               /* class_finalize */
            NULL,               /* class_data */
            sizeof (Syntax_Check_Manager),
            0,                  /* n_preallocs */
            (GInstanceInitFunc) syntax_check_manager_init,
        };

        our_type = g_type_register_static (G_TYPE_OBJECT, "Syntax_Check_Manager",
                                           &our_info, 0);
  }
    
    return our_type;
}
static void
syntax_check_manager_class_init (Syntax_Check_ManagerClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
  parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = syntax_check_manager_finalize;
	g_type_class_add_private (klass, sizeof (Syntax_Check_Manager_Details));
}

static void
syntax_check_manager_init (gpointer object, gpointer klass)
{
	Syntax_Check_Manager_Details *synmgdet;
	synmgdet = SYNTAX_CHECK_MANAGER_GET_PRIVATE(object);
  /* init plugins table*/
//  synmgdet->plugins_table= g_hash_table_new_full (g_str_hash, g_str_equal,NULL, g_object_unref);
}

static void
syntax_check_manager_finalize (GObject *object)
{
  Syntax_Check_Manager *plugmg = SYNTAX_CHECK_MANAGER(object);
  Syntax_Check_Manager_Details *synmgdet;
	synmgdet = SYNTAX_CHECK_MANAGER_GET_PRIVATE(plugmg);
  /* free object resources*/
	G_OBJECT_CLASS (parent_class)->finalize (object);
}


Syntax_Check_Manager *syntax_check_manager_new (void)
{
	Syntax_Check_Manager *plugmg;
  plugmg = g_object_new (SYNTAX_CHECK_MANAGER_TYPE, NULL);
  
	return plugmg; /* return new object */
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

  if (!result) {
    return NULL;
  }
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
//  g_print("res:%s\n", result->str);
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
//  g_print("syntax:\n%s\n", output);
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
GString *save_as_temp_file(Document *document)
{
  gchar *write_buffer = document_get_text(document);
  GString *filename = text_save_as_temp_file(write_buffer);
  g_free(write_buffer);
  return filename;
}

gchar *syntax_check_manager_run(Document *document)
{
  GString *command_line=NULL;
  gchar *output;
  gboolean using_temp;
  GString *filename;
  gint ftype = document_get_document_type(document);
  gchar *docfilename = document_get_filename(document);
  if (document_get_saved_status(document) && filename_is_native(docfilename) && document_get_untitled(document)) {
      gchar *local_path=filename_get_scaped_path(docfilename);
      filename = g_string_new(local_path);
      g_free(local_path);
      using_temp = FALSE;
    }
    else {
      filename = save_as_temp_file(document);
      using_temp = TRUE;
    }
    g_free(docfilename);
    if(ftype==TAB_PHP){
    Preferences_Manager *pref = preferences_manager_new ();
//    command_line = g_string_new(get_preferences_manager_php_binary_location(main_window.prefmg));
    command_line = g_string_new(get_preferences_manager_php_binary_location(pref));
    command_line = g_string_append(command_line, " -q -l -d html_errors=Off -f '");
    command_line = g_string_append(command_line, filename->str);
    command_line = g_string_append(command_line, "'");
    g_object_unref(pref);
//    g_print("eject:%s\n", command_line->str);
    } else if (ftype==TAB_PERL){
    command_line = g_string_new("perl -c ");
    command_line = g_string_append(command_line, "'");
    command_line = g_string_append(command_line, filename->str);
    command_line = g_string_append(command_line, "'");
//    g_print("eject:%s\n", command_line->str);
    } else {

      g_string_free(filename, TRUE);
      return NULL;
    }
    output = run_php_lint(command_line->str);
    g_string_free(command_line, TRUE);
    gchar *result=NULL;

    if (output) {
      if (ftype==TAB_PHP)
      result = process_php_lines(output);
      else
      result = process_perl_lines(output);
      g_free(output);
    } else {
      result =g_strdup(_("Error calling PHP CLI (is PHP command line binary installed? If so, check if it's in your path or set php_binary in Preferences)\n"));
    }
    if (using_temp) {
      release_temp_file (filename->str);
    }
    g_string_free(filename, TRUE);
    return result;
}
