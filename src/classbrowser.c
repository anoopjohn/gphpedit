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
#include "classbrowser.h"
#include "main_window_callbacks.h"
#include "gvfs_utils.h"
//#define DEBUGCLASSBROWSER 
static GSList *functionlist = NULL;
static GTree *php_variables_tree;
static GTree *php_files_tree=NULL;
static GTree *php_class_tree;
guint identifierid = 0;
GString *completion_result=NULL;

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
#ifdef HAVE_CTAGS_EXUBERANT
void call_ctags(gchar *filename){
  if (!filename){
    g_print("skip\n");
    return;
  }
  gboolean result;
  gchar *stdout;
  gint exit_status;
  GError *error=NULL;
  gchar *stdouterr;
  gchar *path=filename_get_path(filename);
  gchar *command_line=g_strdup_printf("ctags -x '%s'",path);
  result = g_spawn_command_line_sync (command_line, &stdout, &stdouterr, &exit_status, &error);
  g_free(command_line);
  g_free(path);
  if (result) {
  // g_print("ctags:%s ->(%s)\n",stdout,stdouterr);

  gchar *copy;
  gchar *token;
  gchar *name;
  gchar *type;
  gchar *line;
  copy = stdout;
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
        if (is_cobol_file(filename))
            process_cobol_word(name,filename,type,line);
        g_free(name);
        g_free(line);
        g_free(type);
        copy = NULL;
      }
    //we have all functions in the same GTree and we distinguish by filetype (PHP,COBOL,C/C++,PERL,PYTHON,ect).
    g_free(stdouterr);
    g_free(stdout);
  }
}
#endif
gboolean free_php_files_tree_item (gpointer key, gpointer value, gpointer data){
  ClassBrowserFile *file=(ClassBrowserFile *)value;
  if (php_files_tree && key && value) g_tree_steal (php_files_tree, key);
  if (file->filename) g_free(file->filename);
  //if (file) g_slice_free(ClassBrowserFile, file);
//  if (key){
//  g_tree_remove(php_files_tree, key);
//  }
//  g_free (key);

  return FALSE;	
}

static gboolean visible_func (GtkTreeModel *model, GtkTreeIter  *iter, gpointer data) {
  /* Visible if row is non-empty and name column contain filename as prefix */
  guint file_type;
  gboolean visible = FALSE;
  guint data_type= main_window.current_editor->type;
  gtk_tree_model_get (model, iter, FILE_TYPE, &file_type, -1);
  if (data_type==file_type) visible = TRUE;
 // g_print("%d -> %d (%s)\n",data_type,file_type,main_window.current_editor->filename->str);
  return visible;
}


void classbrowser_filelist_clear(void)
{
    g_tree_foreach (php_files_tree,free_php_files_tree_item,NULL);
}
void classbrowser_filelist_add(gchar *filename)
{
  ClassBrowserFile *file;
  if (!g_tree_lookup (php_files_tree,filename)){
#ifdef DEBUGCLASSBROWSER
    g_print("Added filename to classbrowser:%s\n",filename);
#endif
    file = g_slice_new(ClassBrowserFile);
    file->filename = g_strdup(filename);
    file->accessible = TRUE;
    file->modified_time.tv_sec = 0;
    file->modified_time.tv_usec = 0;
    g_tree_insert (php_files_tree,g_strdup(file->filename),file);
  }
}
gboolean classbrowser_file_accessible(gchar *filename)
{
  GFileInfo *info;
  GError *error=NULL;
  GFile *file;
  if (!filename) return FALSE;
#ifdef DEBUGCLASSBROWSER
  g_print("can read filename:'%s'?\n",filename);
#endif
  file= get_gfile_from_filename(filename);
  
  info=g_file_query_info (file,G_FILE_ATTRIBUTE_ACCESS_CAN_READ,0,NULL,&error);
  if (error){
  g_object_unref(file);
  return FALSE;  
  }
  gboolean hr= g_file_info_get_attribute_boolean (info, G_FILE_ATTRIBUTE_ACCESS_CAN_READ);
  g_object_unref(info);  
  g_object_unref(file);
  
  return hr;
}

gboolean classbrowser_file_exist(gchar *filename)
{
#ifdef DEBUGCLASSBROWSER
  g_print("exist filename?:%s\n",filename);
#endif
  return filename_file_exist(filename);
}

gboolean classbrowser_file_set_remove (gpointer key, gpointer value, gpointer data){
  ClassBrowserFile *file=  (ClassBrowserFile *)value;
    if (file) {
      if (!classbrowser_file_exist(file->filename)){
        classbrowser_filelist_remove(file);
      }
      else if (!classbrowser_file_accessible(file->filename)) {
        file->accessible = FALSE;
      }
      else {
        file->accessible = TRUE;
      }
    }
  return FALSE;
}

void classbrowser_filelist_update(void)
{
g_tree_foreach (php_files_tree, classbrowser_file_set_remove,NULL);
}

gboolean classbrowser_file_modified(gchar *filename,GTimeVal *act){
  gboolean hr=get_file_modified(filename,act, TRUE);
#ifdef DEBUGCLASSBROWSER
    g_print("filename:'%s' returned %d modified status\n",filename,hr);
#endif
  return hr;
}

/* release resources used by classbrowser */
gboolean classbrowser_php_class_set_remove_item (gpointer key, gpointer value, gpointer data){
  ClassBrowserClass *class=(ClassBrowserClass *)value;
  if (class) {
    class->remove = TRUE;
  }
  return FALSE;	
}

void classbrowser_start_update(void)
{
  GSList *li;
  ClassBrowserFunction *function;

  for(li = functionlist; li!= NULL; li = g_slist_next(li)) {
    function = li->data;
    if (function) {
      function->remove = TRUE;
    }
  }
  if (!php_class_tree){
     /* create new tree */
     php_class_tree=g_tree_new ((GCompareFunc)g_utf8_collate);
  } else {
    g_tree_foreach (php_class_tree,classbrowser_php_class_set_remove_item,NULL);
  }
}


gboolean classbrowser_safe_equality(gchar *a, gchar *b)
{
  /*
  * g_strcmp0
  * Compares str1 and str2 like strcmp(). Handles NULL gracefully by sorting it before non-NULL strings. 
  */
  return (g_strcmp0(a,b)==0);
}

ClassBrowserFunction *classbrowser_functionlist_find(gchar *funcname, gchar *param_list, gchar *filename, gchar *classname)
{
  GSList *li;
  ClassBrowserFunction *function;
  gboolean found;

  for(li = functionlist; li!= NULL; li = g_slist_next(li)) {
    function = li->data;
    if (function) {
      found = TRUE;
      if (classbrowser_safe_equality(function->functionname, funcname) &&
              classbrowser_safe_equality(function->filename, filename) &&
              classbrowser_safe_equality(function->paramlist, param_list) &&
              classbrowser_safe_equality(function->classname, classname)) {
        return function;
      }
    }
  }

  return NULL;
}


void classbrowser_functionlist_free(ClassBrowserFunction *function, GtkTreeIter *iter)
{
  gtk_tree_store_remove(GTK_TREE_STORE(main_window.classtreestore),iter);
  g_free(function->filename);
  g_free(function->functionname);
  if (function->paramlist) {
    g_free(function->paramlist);
  }
  if (function->classname) {
    g_free(function->classname);
  }
  functionlist = g_slist_remove(functionlist, function);
  g_slice_free(ClassBrowserFunction,function);
}


gboolean classbrowser_tree_find_iter(GtkTreeIter *iter, ClassBrowserFunction *function)
{
  guint id;
  gboolean found;
  GtkTreeIter child;

  found = TRUE;
  while (found) {
    if (gtk_tree_model_iter_has_child(GTK_TREE_MODEL(main_window.classtreestore), iter)) {
      gtk_tree_model_iter_children(GTK_TREE_MODEL(main_window.classtreestore), &child, iter);
      if (classbrowser_tree_find_iter(&child, function)) {
        memcpy(iter, &child, sizeof(child));
        return TRUE;
      }
    }

    gtk_tree_model_get(GTK_TREE_MODEL(main_window.classtreestore),iter, ID_COLUMN, &id,-1);

    if (id == function->identifierid) {
      return TRUE;
    }
    // if we aren't looking at the right id, move on
    found = gtk_tree_model_iter_next(GTK_TREE_MODEL(main_window.classtreestore), iter);
  }
  return FALSE;
}


void classbrowser_functionlist_remove(ClassBrowserFunction *function)
{
  GtkTreeIter iter;

  if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(main_window.classtreestore), &iter)) {
    if (classbrowser_tree_find_iter(&iter, function)) {
      classbrowser_functionlist_free(function, &iter);
    }
  }
}


void classbrowser_classlist_remove(ClassBrowserClass *class)
{
  GtkTreeIter iter;
  guint id;
  gboolean found;

  if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(main_window.classtreestore), &iter))
  {
    found = TRUE;
    while (found) {
      gtk_tree_model_get(GTK_TREE_MODEL(main_window.classtreestore),&iter, ID_COLUMN, &id,-1);
      if (id == class->identifierid) {
        gtk_tree_store_remove(GTK_TREE_STORE(main_window.classtreestore),&iter);
        break;
      } else {
        /* if we aren't looking at the right id, move on */
        found = gtk_tree_model_iter_next(GTK_TREE_MODEL(main_window.classtreestore), &iter);
      }
    }
  }
  gchar *keyname=g_strdup_printf("%s%s",class->classname,class->filename);
  g_free(class->filename);
  g_free(class->classname);
  g_tree_remove (php_class_tree,keyname);
  g_slice_free(ClassBrowserClass, class);
  g_free(keyname);
}


gboolean classbrowser_class_find_before_position(gchar *classname, GtkTreeIter *iter)
{
  gboolean found;
  gchar *classnamefound;
  guint type;

  if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(main_window.classtreestore), iter)) {
    found = TRUE;
    while (found) {
      gtk_tree_model_get(GTK_TREE_MODEL(main_window.classtreestore),iter,
                         NAME_COLUMN, &classnamefound, TYPE_COLUMN, &type, -1);
      if ((type==CB_ITEM_TYPE_CLASS) && g_ascii_strcasecmp(classname, classnamefound)<0) {
        g_free(classnamefound);
        return TRUE;
      }
      if (type==CB_ITEM_TYPE_FUNCTION) {
        g_free(classnamefound);
        return TRUE;
      }
      found = gtk_tree_model_iter_next(GTK_TREE_MODEL(main_window.classtreestore), iter);
      g_free(classnamefound);
    }
  }
  return FALSE;
}


void classbrowser_classlist_add(gchar *classname, gchar *filename, gint line_number,gint file_type)
{
  ClassBrowserClass *class;
  GtkTreeIter iter;
  GtkTreeIter before;
  gchar *keyname=g_strdup_printf("%s%s",classname,filename);
  class=g_tree_lookup (php_class_tree, keyname);
  if ((class)){
    class->line_number = line_number;
    class->remove= FALSE;
    g_free(keyname);
  } else {
    class = g_slice_new(ClassBrowserClass);
    class->classname = g_strdup(classname);
    class->filename = g_strdup(filename);
    class->line_number = line_number;
    class->remove = FALSE;
    class->identifierid = identifierid++;
    class->file_type=file_type;
    g_tree_insert (php_class_tree,keyname,class);

    if (classbrowser_class_find_before_position(classname, &before)) {
      gtk_tree_store_insert_before(main_window.classtreestore, &iter, NULL, &before);
    } else {
      gtk_tree_store_append (main_window.classtreestore, &iter, NULL);
    }

    gtk_tree_store_set (GTK_TREE_STORE(main_window.classtreestore), &iter,
                        NAME_COLUMN, (class->classname), FILENAME_COLUMN, (class->filename),
                        LINE_NUMBER_COLUMN, line_number, TYPE_COLUMN, CB_ITEM_TYPE_CLASS, ID_COLUMN, (class->identifierid), FILE_TYPE, file_type,-1);
  }

}


gboolean classbrowser_classid_to_iter(guint classid, GtkTreeIter *iter)
{
  guint id;
  gboolean found;

  if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(main_window.classtreestore), iter)) {
    found = TRUE;
    while (found) {
      gtk_tree_model_get(GTK_TREE_MODEL(main_window.classtreestore),iter, ID_COLUMN, &id,-1);
      if (id == classid) {
        return TRUE;
      }
      found = gtk_tree_model_iter_next(GTK_TREE_MODEL(main_window.classtreestore), iter);
    }
  }
  return FALSE;
}

void classbrowser_varlist_add(gchar *varname, gchar *funcname, gchar *filename)
{
  ClassBrowserVar *var;
  var=g_tree_lookup (php_variables_tree, varname);
  if (var){
    var->remove = FALSE;
  } else {
    var = g_slice_new(ClassBrowserVar);
    var->varname = g_strdup(varname);
    if (funcname) {
      var->functionname = g_strdup(funcname);
    }
    var->filename = g_strdup(filename);
    var->remove = FALSE;
    var->identifierid = identifierid++;

    g_tree_insert (php_variables_tree, g_strdup(varname), var); /* key =variables name value var struct */

    #ifdef DEBUGCLASSBROWSER
      g_print("Filename: %s\n", filename);
    #endif
  }
}
static gboolean make_class_completion_string (gpointer key, gpointer value, gpointer data){
  ClassBrowserClass *class;
  class=(ClassBrowserClass *)value;
        if (!completion_result) {
        completion_result = g_string_new(g_strchug(class->classname));
        completion_result = g_string_append(completion_result, "?4"); /* add corresponding image*/
        } else {
        completion_result = g_string_append(completion_result, " ");
        completion_result = g_string_append(completion_result, g_strchug(class->classname));
        completion_result = g_string_append(completion_result, "?4"); /* add corresponding image*/
        }
  return FALSE;
}
void autocomplete_php_classes(GtkWidget *scintilla, gint wordStart, gint wordEnd){
  g_tree_foreach (php_class_tree, make_class_completion_string,NULL);
  if (completion_result){
    gtk_scintilla_autoc_show(GTK_SCINTILLA(scintilla), 0, completion_result->str);
    g_string_free(completion_result,TRUE); /*release resources*/
    completion_result=NULL;
  }  
}

static gboolean make_completion_string (gpointer key, gpointer value, gpointer data){
  gchar *prefix=(gchar *) data;
  ClassBrowserVar *var;
  var=(ClassBrowserVar *)value;
  if (g_str_has_prefix(key,prefix)){
        if (!completion_result) {
        completion_result = g_string_new(key);
        completion_result = g_string_append(completion_result, "?3");
        } else {
        completion_result = g_string_append(completion_result, " ");
        completion_result = g_string_append(completion_result, key);
        completion_result = g_string_append(completion_result, "?3"); /* add corresponding image*/
        }
  }
  return FALSE;
}
void autocomplete_php_variables(GtkWidget *scintilla, gint wordStart, gint wordEnd){
  gchar *buffer = NULL;
  gint length;
  buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(scintilla), wordStart, wordEnd, &length);
#ifdef DEBUGCLASSBROWSER
  g_print("var autoc:%s\n",buffer);
#endif
  g_tree_foreach (php_variables_tree, make_completion_string,buffer);
  g_free(buffer);
  if (completion_result){
    gtk_scintilla_autoc_show(GTK_SCINTILLA(scintilla), wordEnd-wordStart, completion_result->str);
    g_string_free(completion_result,TRUE); /*release resources*/
    completion_result=NULL;
  }  
}

void classbrowser_functionlist_add(gchar *classname, gchar *funcname, gchar *filename, gint file_type, guint line_number, gchar *param_list)
{
  ClassBrowserClass *class;
  ClassBrowserFunction *function;
  GtkTreeIter iter;
  GtkTreeIter class_iter;
  GString *function_decl;
  guint type;
  if ((function = classbrowser_functionlist_find(funcname, param_list, filename, classname))) {
    function->line_number = line_number;
    function->remove = FALSE;
  } else {
    function = g_slice_new0(ClassBrowserFunction);
    function->functionname = g_strdup(funcname);
    if (param_list) {
      function->paramlist = g_strdup(param_list);
    }
    function->filename = g_strdup(filename);
    function->line_number = line_number;
    function->remove = FALSE;
    function->identifierid = identifierid++;
    function->file_type = file_type;
    gchar *keyname=g_strdup_printf("%s%s",classname,filename);
    if (classname && (class = g_tree_lookup (php_class_tree,keyname))){
      function->class_id = class->identifierid;
      function->classname = g_strdup(classname);
      classbrowser_classid_to_iter(function->class_id, &class_iter);
      type = CB_ITEM_TYPE_CLASS_METHOD;
      gtk_tree_store_append (main_window.classtreestore, &iter, &class_iter);
    } else {
      type = CB_ITEM_TYPE_FUNCTION;
      gtk_tree_store_append (main_window.classtreestore, &iter, NULL);
    }
    g_free(keyname);
    functionlist = g_slist_append(functionlist, function);

    function_decl = g_string_new(funcname);
    if (file_type!=TAB_COBOL){ /* cobol paragraph don't have params */
      function_decl = g_string_append(function_decl, "(");
      if (param_list) {
        function_decl = g_string_append(function_decl, param_list);
      }
      function_decl = g_string_append(function_decl, ")");
    }
    #ifdef DEBUGCLASSBROWSER
      g_print("Filename: %s\n", filename);
    #endif
    gtk_tree_store_set (main_window.classtreestore, &iter,
                        NAME_COLUMN, function_decl->str, LINE_NUMBER_COLUMN, line_number, FILENAME_COLUMN, filename, TYPE_COLUMN, type, ID_COLUMN, function->identifierid,FILE_TYPE, file_type,-1);
    g_string_free(function_decl, TRUE);
  }
}
gboolean classbrowser_remove_class(gpointer key, gpointer value, gpointer data){
  ClassBrowserClass *class= (ClassBrowserClass *)value;
      if (class) {
      if (class->remove) {
        classbrowser_classlist_remove(class);
      }
    }
  return FALSE;
}

void classbrowser_remove_dead_wood(void)
{
  GSList *orig;
  GSList *li;
  ClassBrowserFunction *function;

  orig = g_slist_copy(functionlist);
  for(li = orig; li!= NULL; li = g_slist_next(li)) {
    function = li->data;
    if (function) {
      if (function->remove) {
        classbrowser_functionlist_remove(function);
      }
    }
  }
  g_slist_free(orig);
  g_tree_foreach (php_class_tree, classbrowser_remove_class, NULL);
}

void classbrowser_filelist_remove(ClassBrowserFile *file)
{
  g_tree_remove (php_files_tree,file->filename);
  g_free(file->filename);
  g_slice_free(ClassBrowserFile,file);
}

void list_php_files_open(void){
  GSList *li;
  Editor *editor;
  for(li = editors; li!= NULL; li = g_slist_next(li)) {
    editor = li->data;
    if (editor) {
#ifdef CLASSBROWSER
      g_print("classbrowser found:%s\n",editor->filename->str);
#endif
      classbrowser_filelist_add(editor->filename->str);
    }
  }
}
void add_global_var(const gchar *var_name){
  ClassBrowserVar *var;
    var = g_slice_new(ClassBrowserVar);
    var->varname = g_strdup(var_name);
    var->functionname = NULL; /* NULL for global variables*/
    var->filename = NULL; /*NULL FOR PHP GLOBAL VARIABLES*/
    var->remove = FALSE;
    var->identifierid = identifierid++;

    g_tree_insert (php_variables_tree, g_strdup(var_name), var); /* key =variables name value var struct */

}

gboolean classbrowser_files_parse (gpointer key, gpointer value, gpointer data){
ClassBrowserFile *file=(ClassBrowserFile *)value;
    while (gtk_events_pending()) gtk_main_iteration(); /* update ui */
      if (!file->filename) return TRUE;
    #ifdef DEBUGCLASSBROWSER
      g_print("Parsing %s\n", file->filename);
    #endif
    if (classbrowser_file_modified(file->filename,&file->modified_time)){
      if (is_php_file_from_filename(file->filename)) {
      classbrowser_parse_file(file->filename);
#ifdef HAVE_CTAGS_EXUBERANT
      } else {
        /* CTAGS don't support CSS files */
        if (!is_css_file(file->filename)) call_ctags(file->filename);
#endif
      }
    }
  return FALSE;
}

//Note: this function can be optimized by not requesting to reparse files on tab change
//when the parse only selected tab is set - Anoop

void classbrowser_update(void)
{
  static guint press_event = 0;
  static guint release_event = 0;
  if (!php_files_tree){
     /* create new tree */
     php_files_tree=g_tree_new ((GCompareFunc)g_utf8_collate);
  }
  if (!php_variables_tree){
     /* create new tree */
     php_variables_tree=g_tree_new ((GCompareFunc)g_utf8_collate);

     /*add php global vars*/
     add_global_var("$GLOBALS");
     add_global_var("$HTTP_POST_VARS");
     add_global_var("$HTTP_RAW_POST_DATA");
     add_global_var("$http_response_header");
     add_global_var("$this");
     add_global_var("$_COOKIE");
     add_global_var("$_POST");
     add_global_var("$_REQUEST");
     add_global_var("$_SERVER");
     add_global_var("$_SESSION");
     add_global_var("$_GET");
     add_global_var("$_FILES");
     add_global_var("$_ENV");
     add_global_var("__CLASS__");
     add_global_var("__DIR__");
     add_global_var("__FILE__");
     add_global_var("__FUNCTION__");
     add_global_var("__METHOD__");
     add_global_var("__NAMESPACE__");
  }
  
  if (gtk_paned_get_position(GTK_PANED(main_window.main_horizontal_pane))==0) {
    return;
  }
  if (main_window.current_editor && php_files_tree){
    classbrowser_filelist_clear();
  }
  //if parse only current file is set then add only the file in the current tab
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (main_window.chkOnlyCurFileFuncs))){
    //add only if there is a current editor
    if (main_window.current_editor) {
      classbrowser_filelist_add(main_window.current_editor->filename->str);
    }
  } else { 
    list_php_files_open();
  }
  if (press_event) {
    g_signal_handler_disconnect(main_window.classtreeview, press_event);
  }
  if (release_event) {
    g_signal_handler_disconnect(main_window.classtreeview,release_event);
  }
  classbrowser_filelist_update();
  classbrowser_start_update();
  g_tree_foreach (php_files_tree,classbrowser_files_parse,NULL);
  classbrowser_remove_dead_wood();
  if (main_window.current_editor) {
    main_window.new_model= gtk_tree_model_filter_new (GTK_TREE_MODEL(main_window.classtreestore),NULL);
    gtk_tree_model_filter_set_visible_func ((GtkTreeModelFilter *) main_window.new_model, visible_func, NULL, NULL);
    gtk_tree_view_set_model (GTK_TREE_VIEW( main_window.classtreeview),main_window.new_model);
  }
  press_event = g_signal_connect(GTK_OBJECT(main_window.classtreeview), "button_press_event",
                                   G_CALLBACK(treeview_double_click), NULL);
  release_event = g_signal_connect(GTK_OBJECT(main_window.classtreeview), "button_release_event",
                                     G_CALLBACK(treeview_click_release), NULL);
}


gint member_function_list_sort(gconstpointer a, gconstpointer b)
{
  return (g_utf8_collate((gchar *)a, (gchar *)b));
}


GString *get_member_function_completion_list(GtkWidget *scintilla, gint wordStart, gint wordEnd)
{
  gchar *buffer = NULL;
  GSList *li;
  GList *li2;
  ClassBrowserFunction *function;
  GList* member_functions = NULL;
  GList* sorted_member_functions = NULL;
  GString *result = NULL;
  gint length;
  gchar *function_name;

  buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(scintilla), wordStart, wordEnd, &length);
  for(li = functionlist; li!= NULL; li = g_slist_next(li)) {
    function = li->data;
    if (function) {
      if (((strncmp(function->functionname, buffer, strlen(buffer))==0) || (wordStart==wordEnd)) && function->file_type==TAB_PHP ) {
        member_functions = g_list_append(member_functions, function->functionname);
      }
    }
  }

  sorted_member_functions = g_list_sort(member_functions, member_function_list_sort);
  member_functions = sorted_member_functions;

  for(li2 = member_functions; li2!= NULL; li2 = g_list_next(li2)) {
    function_name = li2->data;
    if (!result) {
      result = g_string_new(function_name);
      result = g_string_append(result, "?1");
    }
    else {
      result = g_string_append(result, " ");
      result = g_string_append(result, function_name);
      result = g_string_append(result, "?1");
    }
  }

  result = g_string_append(result, " ");
  g_free(buffer);
  return result;
}


void autocomplete_member_function(GtkWidget *scintilla, gint wordStart, gint wordEnd)
{
  GString *list;

  list = get_member_function_completion_list(scintilla, wordStart, wordEnd);

  if (list) {
    gtk_scintilla_autoc_show(GTK_SCINTILLA(scintilla), wordEnd-wordStart, list->str);
    g_string_free(list, FALSE);
  }
}

gchar *classbrowser_custom_function_calltip(gchar *function_name){
/*FIXME::two functions diferent classes same name =bad calltip */
  GSList *li;
  ClassBrowserFunction *function;
  gchar *calltip=NULL;
  for(li = functionlist; li!= NULL; li = g_slist_next(li)) {
    function = li->data;
    if (function) {
      if (g_utf8_collate(function->functionname, function_name)==0 && function->file_type==TAB_PHP) {
          calltip=g_strdup_printf("%s (%s)",function->functionname,function->paramlist);
          break;
      }
    }
  }
  return calltip;
}
gchar *classbrowser_add_custom_autocompletion(gchar *prefix,GSList *list){
  GSList *li;
  GList *li2;
  ClassBrowserFunction *function;
  GString *result=NULL;
  GList* member_functions = NULL;
  GList* sorted_member_functions = NULL;
  gchar *function_name;
  for(li = functionlist; li!= NULL; li = g_slist_next(li)) {
    function = li->data;
    if (function) {
      if ((g_str_has_prefix(function->functionname, prefix) && function->file_type==TAB_PHP)) {
        member_functions = g_list_prepend(member_functions, function->functionname);
      }
    }
  }
  /* add functions */
  for(li = list; li!= NULL; li = g_slist_next(li)) {
    function = li->data;
    if (function) {
        member_functions = g_list_prepend(member_functions, function);
    }
  }
  sorted_member_functions = g_list_sort(member_functions, member_function_list_sort);
  member_functions = sorted_member_functions;

  for(li2 = member_functions; li2!= NULL; li2 = g_list_next(li2)) {
    function_name = li2->data;
    if (!result) {
      result = g_string_new(function_name);
      if (!g_str_has_suffix(function_name,"?2"))
          result = g_string_append(result, "?1");
    }
    else {
      result = g_string_append(result, " ");
      result = g_string_append(result, function_name);
      if (!g_str_has_suffix(function_name,"?2") && !g_str_has_suffix(function_name,"?3"))
          result = g_string_append(result, "?1");
    }
  }
  if (result){
    result = g_string_append(result, " ");
    return result->str;
  } else {
    return NULL;
  }
}

gboolean classbrowser_file_in_list_find(GSList *list, gchar *file)
{
  GSList *list_walk;
  gchar *data;

  for(list_walk = list; list_walk!= NULL; list_walk = g_slist_next(list_walk)) {
    data = list_walk->data;
    if (g_utf8_collate(data, file)==0) {
      return TRUE;
    }
  }
  return FALSE;
}


void classbrowser_update_selected_label(gchar *filename, gint line)
{
  GSList *filenames;
  GSList *function_walk;
  GString *new_label;
  ClassBrowserFunction *function;
  gchar *func_filename;
  gint num_files;

  filenames = NULL;
  num_files = 0;
  for(function_walk = functionlist; function_walk!= NULL; function_walk = g_slist_next(function_walk)) {
    num_files++;
    function = function_walk->data;
    if (function) {
      func_filename = function->filename;
      // g_slist_find and g_slist_index don't seem to work, always return NULL or -1 respec.
      if (!classbrowser_file_in_list_find(filenames, func_filename)) {
        filenames = g_slist_prepend(filenames, func_filename);
      }
    }
  }
  if(num_files < 2) {
    gchar *basename=filename_get_basename(filename);
    new_label = g_string_new(basename);
    g_free(basename);
  }
  else {
    new_label = get_differing_part(filenames, filename);
  }
  #ifdef DEBUGCLASSBROWSER
    g_print("%d :: %s\n", num_files, new_label->str);  
  #endif
  if (new_label) {
    new_label = g_string_prepend(new_label, _("FILE: "));
    g_string_append_printf(new_label, "(%d)", line);
    gtk_label_set_text(GTK_LABEL(main_window.treeviewlabel), new_label->str);
    g_string_free(new_label, TRUE);
    g_slist_free(filenames);
  }
}

//enable sorting of the list
void classbrowser_set_sortable(GtkTreeStore *classtreestore)
{
  gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(classtreestore), 
    classbrowser_compare_function_names,
    NULL,NULL);
  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(classtreestore), 
    0, classbrowser_compare_function_names, 
    NULL,NULL);
  gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(classtreestore),
    0,
    GTK_SORT_ASCENDING);
}
//compare function names of the iter of the gtktreeview
gint classbrowser_compare_function_names(GtkTreeModel *model,
                    GtkTreeIter *a,
                    GtkTreeIter *b,
                    gpointer user_data)
{
  gchar *aName, *bName;
  gint retVal;
  gtk_tree_model_get(model, a, 0, &aName, -1);
  gtk_tree_model_get(model, b, 0, &bName, -1);
  retVal = g_strcmp0(aName, bName);
  #ifdef DEBUGCLASSBROWSER
    g_message("* compare values %s and %s; return %d\n", aName, bName, retVal);
  #endif
  g_free(aName);
  g_free(bName);
  return retVal;
}

/* release resources used by classbrowser */
gboolean free_php_variables_tree_item (gpointer key, gpointer value, gpointer data){
  ClassBrowserVar *var=(ClassBrowserVar *)value;
  g_free(var->varname);
  if (var->functionname) g_free(var->functionname);
  if (var->filename) g_free(var->filename);
  g_slice_free(ClassBrowserVar, var);
  g_tree_remove(php_variables_tree, key);
  g_free (key);
  return FALSE;	
}

void cleanup_classbrowser(void){
  if (php_variables_tree)
    g_tree_foreach (php_variables_tree,free_php_variables_tree_item,NULL);
}
