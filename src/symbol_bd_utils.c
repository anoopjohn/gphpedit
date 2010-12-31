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
#include <stdio.h>
#include <string.h>
#include <glib/gi18n.h>
#include "symbol_bd_utils.h"
#include "gvfs_utils.h"

#define MAX_API_LINE_LENGTH 16384
/*
* symbol_bd_load_api_file (internal function)
* loads API file content in the given GTree
*/
void symbol_bd_load_api_file(const gchar *api_filename, GTree **api_tree)
{
//FIXME: async load
  FILE *apifile;
  char buffer[MAX_API_LINE_LENGTH];
  gchar *api_dir = NULL;
  /* use autoconf macro to build api file path */
  api_dir = g_build_path (G_DIR_SEPARATOR_S, API_DIR, api_filename, NULL);
  apifile = fopen(api_dir, "r");
  if(apifile) {
    *api_tree=g_tree_new_full((GCompareDataFunc) g_utf8_collate, NULL, g_free, NULL);
    while(fgets( buffer, MAX_API_LINE_LENGTH, apifile ) != NULL ) {
      gchar *line=g_strdup(buffer);
      gchar *token_line = line;
      gchar *function_name = strtok(token_line, "|");
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
void symbol_bd_function_list_from_array_prepare(gchar **keywords, GTree **api_tree)
{
  guint n;

  *api_tree=g_tree_new_full((GCompareDataFunc) g_utf8_collate, NULL, g_free, NULL);
  for (n = 0; keywords[n]!=NULL; n++) {
    g_tree_insert (*api_tree, g_strdup(keywords[n]), ""); /* we only need the key */
  }
}

void free_variable_item (gpointer data)
{
  ClassBrowserVar *var=(ClassBrowserVar *)data;
  if (var->varname) g_free(var->varname);
  if (var->functionname) g_free(var->functionname);
  if (var->filename) g_free(var->filename);
  if (var) g_slice_free(ClassBrowserVar, var);
}

void free_class_item (gpointer data)
{
  ClassBrowserClass *class = (ClassBrowserClass *) data;
  if (class->filename) g_free(class->filename);
  if (class->classname) g_free(class->classname);
  if (class) g_slice_free(ClassBrowserClass, class);
}

void free_function_item (gpointer data)
{
  ClassBrowserFunction *function = (ClassBrowserFunction *) data;
  g_free(function->filename);
  g_free(function->functionname);
  if (function->paramlist) g_free(function->paramlist);
  if (function->classname) g_free(function->classname);
  g_slice_free(ClassBrowserFunction,function);
}

gchar *get_ctags_token(gchar *text,gint *advancing){
  int i;
  int k=0;
  gchar *name;
  gchar *part = text;
  name=part;
  for (i=0;i<strlen(text);i++){
    /* process until get a space*/
    if (*(part+i)==' '){
      while (*(part+i+k)==' ') k++; /*count spaces*/
      break;
    }
  }
  name=g_malloc0(i+1);
  strncpy(name,part,i);
  *advancing=i+k; /* skip spaces*/
  return name;
}

gchar *get_ctags_param(gchar *text,gint *advancing){
  int i;
  gchar *name;
  gchar *part = text;
  name=part;
  for (i=0;i<strlen(text);i++){
    if (*(part+i)=='('){
      break;
    }
  }
  int len = strlen(part) - i -1;
  if (len < 0) return NULL;
  name = g_malloc0(strlen(part) - i);
  strncpy(name, part + i + 1, strlen(part) - i -2);
//  g_print("res: %s\n", name);
  return name;
}

gchar *call_ctags(const gchar *filename)
{
#ifdef HAVE_CTAGS_EXUBERANT
  if (!filename) return NULL;
  gboolean result;
  gchar *stdout=NULL;
  gint exit_status;
  GError *error=NULL;
  gchar *stdouterr;
  gchar *path = filename_get_path(filename);
  gchar *command_line = g_strdup_printf("ctags --extra=+q -x '%s'",path);
  result = g_spawn_command_line_sync (command_line, &stdout, &stdouterr, &exit_status, &error);
  g_free(command_line);
  g_free(path);
  g_free(stdouterr);

  return stdout;
#else
  return NULL;
#endif
}

void process_ctags_custom (GObject *symbolbd, gchar *result, const gchar *filename, void (*process_func)(GObject *symbolbd, gchar *name, const gchar *filename, gchar *type, gchar *line, gchar *param)) {
//   g_print("ctags:%s\n",result);
    gchar *copy;
    gchar *token;
    gchar *name;
    gchar *type;
    gchar *line;
    gchar *param;
    copy = result;
      while ((token = strtok(copy, "\n"))) {
          gint ad=0;
          name=get_ctags_token(token,&ad);
  //        g_print("name:%s ",name);
          token+=ad;
          type=get_ctags_token(token,&ad);
  //        g_print("type:%s ",type);
          token+=ad;
          line=get_ctags_token(token,&ad);
  //        g_print("line:%s\n",line);
          param = get_ctags_param(token,&ad);
          process_func(symbolbd, name, filename, type, line, param);
          g_free(name);
          g_free(line);
          g_free(type);
          g_free(param);
          copy = NULL;
        }
}

static ClassBrowserVar *get_new_var(gchar *varname, gchar *funcname, const gchar *filename, guint identifierid)
{
    ClassBrowserVar *var;
    var = g_slice_new0(ClassBrowserVar);
    var->varname = g_strdup(varname);
    var->functionname = NULL;
    if (funcname) var->functionname = g_strdup(funcname);
    var->filename = g_strdup(filename);
    var->remove = FALSE;
    var->identifierid = identifierid;
    return var;
}

void symbol_bd_varlist_add(GHashTable **variables_table, gchar *varname, gchar *funcname, const gchar *filename, guint *identifierid)
{
  ClassBrowserVar *var;
  var = g_hash_table_lookup (*variables_table, varname);
  if (var){
    var->remove = FALSE;
  } else {
    *identifierid=(*identifierid)++;
    var = get_new_var(varname, funcname, filename, *identifierid);
    g_hash_table_insert (*variables_table, g_strdup(varname), var); /* key =variables name value var struct */
  }
}

static ClassBrowserClass *get_new_class(gchar *classname, const gchar *filename, gint line_number, guint identifierid)
{
    ClassBrowserClass *class;
    class = g_slice_new0(ClassBrowserClass);
    class->classname = g_strdup(classname);
    class->filename = g_strdup(filename);
    class->line_number = line_number;
    class->remove = FALSE;
    class->identifierid = identifierid;
    return class;
}

void symbol_bd_classlist_add(GHashTable **class_table, gchar *classname, const gchar *filename, gint line_number, guint *identifierid)
{
  ClassBrowserClass *class;
  classname = g_strstrip(classname);
  gchar *keyname = g_strdup_printf("%s%s",classname,filename);
  class = g_hash_table_lookup (*class_table, keyname);
  if ((class)){
    class->line_number = line_number;
    class->remove= FALSE;
    g_free(keyname);
  } else {
    *identifierid=(*identifierid)++;
    class = get_new_class(classname, filename, line_number, *identifierid);
    g_hash_table_insert (*class_table, keyname, class);
  }
}

static ClassBrowserFunction *get_new_function(gchar *funcname, guint class_id, gchar *classname, const gchar *filename, guint line_number, gchar *param_list, guint identifierid)
{
    ClassBrowserFunction *function;
    function = g_slice_new0(ClassBrowserFunction);
    function->functionname = g_strdup(funcname);
    if (param_list){
      function->paramlist = g_strdup(param_list);
    } else {
      function->paramlist = NULL;
    }
    function->filename = g_strdup(filename);
    function->line_number = line_number;
    function->remove = FALSE;
    function->identifierid = identifierid;
    if (classname) {
      function->class_id = class_id;
      function->classname = g_strdup(classname);
    }
    return function;
}

void symbol_bd_functionlist_add(GHashTable **function_table, GHashTable **class_table, gchar *classname, gchar *funcname, const gchar *filename, 
                                      guint line_number, gchar *param_list, guint *identifierid)
{
  ClassBrowserClass *class;
  ClassBrowserFunction *function;
  if (classname) classname = g_strstrip(classname);
  funcname = g_strstrip(funcname);
  gchar *key = g_strdup_printf("%s%s%s",funcname,classname,filename);
  if ((function = g_hash_table_lookup (*function_table, key))) {
    if (function->paramlist){
      g_free(function->paramlist);
      function->paramlist = NULL;
    }
    if (param_list) function->paramlist = g_strdup(param_list);
    function->line_number = line_number;
    function->remove = FALSE;
    g_free(key);
  } else {
    *identifierid=(*identifierid)++;
    guint class_id = 0;
    gchar *keyname=g_strdup_printf("%s%s",classname,filename);
    if (classname && (class = g_hash_table_lookup (*class_table, keyname))) {
      class_id = class->identifierid;
    } else {
      classname = NULL;
    }
    g_free(keyname);
    function = get_new_function(funcname, class_id, classname, filename, line_number, param_list, *identifierid);
    g_hash_table_insert (*function_table, key, function);
  }
}
