/* This file is part of gPHPEdit, a GNOME2 PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009 José Rostagno(for vijona.com.ar)
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
/* ******* FOLDERBROWSER DESIGN ********
Folderbrowser has a treeview to show the directory struct. This treeview has a column for the pixmap
and the name (both shown to the user), but also the mimetype of the file (not shown). 

Folderbrowser has the following features:
-Remember last folder
-Autorefresh when directory contents changes
-Popup menu
-AutoSort (directories go before files, files are sorted by name and at last by extension)
-Double click in treeview open files and expand/collapse treerow for directories
-Keypress capture: press delete will delete current selected file/folder; press enter will open current selected file/folder
-Only displays files which could be opened by the editor
-Drag and drop: if you drop uri into the folderbrowser these files will be copied to current folderbrowser folder
*/
#include "folderbrowser.h"
#include "tab.h"
#include "gvfs_utils.h"
#include <gdk/gdkkeysyms.h>
//#define DEBUGFOLDERBROWSER

typedef struct {
  const gchar *filename;
  const gchar *mime;
} POPUPDATA;
POPUPDATA pop;
GFileMonitor *monitor;

enum {
  TARGET_URI_LIST,
  TARGET_STRING
};
GtkTreeModel *cache_model;
GSList *filesinfolder;
typedef struct {
  GFile *file;
  const gchar *uri;
  const gchar *mime;
  GIcon *icon;
  GFileType filetype;
  const char *display_name;
  const char *edit_name;
} FOLDERFILE;

static void debug_gfile(GFile * uri, gboolean newline) {
  if (uri) {
    gchar *name = g_file_get_uri(uri);
    g_print("%s%s", name, newline?"\n":"");
    g_free(name);
  } else {
    g_print("(GFile=NULL)%s", newline?"\n":"");
  }
}

#define DEBUG_GFILE debug_gfile


static void create_tree_async(GFile *file);
void enumerate_files_async (GObject *source_object, GAsyncResult *res, gpointer user_data);
void finish_enumerate (GObject *source_object, GAsyncResult *res, gpointer user_data);
void create_tree_fill_async(GtkTreeStore *pTree, FOLDERFILE *current, GtkTreeIter *iter, GtkTreeIter *iter2);
void fb_file_v_drag_data_received(GtkWidget * widget, GdkDragContext * context, gint x,  gint y, GtkSelectionData * data, guint info, guint time,gpointer user_data);
void copy_async_lcb(GObject *source_object,GAsyncResult *res,gpointer user_data);
void copy_uris_async(GFile *destdir, GSList *sources);
void copy_files_async(GFile *destdir, gchar *sources);

static FOLDERFILE *new_folderfile(void)

{
  FOLDERFILE *file;
  file = (FOLDERFILE *)g_slice_new0(FOLDERFILE);
  filesinfolder = g_slist_append(filesinfolder, file);
  return file;
}

static inline void clear_folderbrowser(void){
  /* clear tree and cache data*/
  gtk_tree_store_clear(main_window.pTree);
  if (filesinfolder) g_slist_free (filesinfolder);
  filesinfolder=NULL;
}

static inline void store_last_folder(gchar *newpath){
  if (newpath){
  /*store folder in config*/
  GConfClient *config;
  config=gconf_client_get_default ();
  gconf_client_set_string (config,"/gPHPEdit/main_window/folderbrowser/folder", newpath,NULL);
  g_object_unref(config);
  }
}
static inline void search_control_sensible(gboolean status){
    gtk_widget_set_sensitive (main_window.searchentry, status);
    gtk_entry_set_icon_sensitive (GTK_ENTRY(main_window.searchentry), GTK_ENTRY_ICON_PRIMARY, status);
    gtk_entry_set_icon_sensitive (GTK_ENTRY(main_window.searchentry), GTK_ENTRY_ICON_SECONDARY, status);
}
static gboolean visible_func (GtkTreeModel *model, GtkTreeIter  *iter, gpointer data) {
  /* Visible if row is non-empty and name column contain filename as prefix */
  gchar *str;
  gboolean visible = FALSE;
  const char *filename= (const char *) data;
  gtk_tree_model_get (model, iter, 1, &str, -1);
  if (str && g_str_has_prefix(str,filename)) visible = TRUE;
  g_free (str);
  return visible;
}

/**
* on_search_press
* process icon search press signals
*/
static void on_search_press (const gchar *filename){
  GtkTreeModel *new_model= gtk_tree_model_filter_new (cache_model,NULL);
  gtk_tree_model_filter_set_visible_func ((GtkTreeModelFilter *) new_model, visible_func, (gpointer) filename, NULL);
  gtk_tree_view_set_model (GTK_TREE_VIEW(main_window.pListView),new_model);
}

/**
* on_cleanicon_press
* process icon press signals 
*/
static void on_cleanicon_press (GtkEntry *entry, GtkEntryIconPosition icon_pos, GdkEvent *event, gpointer user_data){
  if (icon_pos==GTK_ENTRY_ICON_SECONDARY){    
    gtk_entry_set_text (entry,"");
  } else {
    const gchar *find_string=gtk_entry_get_text (entry);
    #ifdef DEBUGFOLDERBROWSER
    g_print("DEBUG::Search for:%s in folderbrowser\n",find_string);
    #endif
    if (find_string && !IS_DEFAULT_DIR(CURRENTFOLDER))
      on_search_press (gtk_entry_get_text (entry));
    else
      /* nothing to search sets default model */
      gtk_tree_view_set_model (GTK_TREE_VIEW(main_window.pListView),cache_model);
  }
}

static void search_typed (GtkEntry *entry, const gchar *text, gint length, gint *position, gpointer data) {
  on_cleanicon_press (entry, GTK_ENTRY_ICON_PRIMARY, NULL, NULL);
}

static void search_activate(GtkEntry *entry,gpointer user_data) {
  on_cleanicon_press (entry, GTK_ENTRY_ICON_PRIMARY, NULL, NULL);
}

/*
 * icon_name_from_icon
 *
 *   this function returns the icon name of a Gicon
 *   for the current gtk default icon theme
 */

static gchar *icon_name_from_icon(GIcon *icon, GtkIconTheme *icon_theme) {
  gchar *icon_name=NULL;
  if (icon && G_IS_THEMED_ICON(icon)) {
    GStrv names;
    g_object_get(icon, "names", &names, NULL);
    if (names && names[0]) {
      int i;
      for (i = 0; i < g_strv_length (names); i++) {
        if (gtk_icon_theme_has_icon(icon_theme, names[i])) {
          icon_name = g_strdup(names[i]);
          break;
        }
      }
      g_strfreev (names);
    }
  } else {
    icon_name = g_strdup("folder");
  }
  if (!icon_name){
    icon_name=g_strdup("folder");
  }
  return icon_name;
}

static void print_files(gchar *path){
  #ifdef DEBUGFOLDERBROWSER
  g_print("DEBUG::printing files\n");
  #endif
  GSList *l;
  GtkTreeIter iter2;
  GtkTreeIter* iter=NULL;
  gtk_tree_store_clear(main_window.pTree);
  /* get default icon theme */
  GtkIconTheme *theme= gtk_icon_theme_get_default();
    for (l = filesinfolder; l != NULL; l = g_slist_next (l)) {
      FOLDERFILE *current=(FOLDERFILE *) l->data;
      #ifdef DEBUGFOLDERBROWSER
      g_print("DEBUG:::File added -> name: '%s' \tmime: '%s'\n",current->display_name,current->mime);
      #endif
      GdkPixbuf *p_file_image = NULL;
      /* get icon of size menu */
      gchar *icon_name=icon_name_from_icon(current->icon,theme);
      p_file_image = gtk_icon_theme_load_icon (theme, icon_name, GTK_ICON_SIZE_MENU, 0, NULL);
      g_free(icon_name);
      gtk_tree_store_insert_with_values(GTK_TREE_STORE(main_window.pTree), &iter2, iter, 0, ICON_COLUMN, p_file_image, FILE_COLUMN, current->display_name,MIME_COLUMN,current->mime,-1);
      g_object_unref(p_file_image);
        while (gtk_events_pending ())
          gtk_main_iteration ();
    }
    //GError *error=NULL;
    //gchar *filepath =convert_to_full(path);
    //GFile *file=g_file_new_for_uri(filepath);
    //g_free(filepath);
    //Start file monitor for folderbrowser autorefresh
    //monitor= g_file_monitor_directory (file,G_FILE_MONITOR_NONE,NULL,&error);
    //if (!monitor){
    //g_print(_("Error initing folderbrowser autorefresh. GIO Error:%s\n"),error->message);
    //g_error_free (error);
    //}else{
    //#ifdef DEBUGFOLDERBROWSER
    //g_print("DEBUG:: initing folderbrowser update for:%s\n",path);
    //#endif
    //g_signal_connect(monitor, "changed", (GCallback) update_folderbrowser_signal, NULL);
    //}
  if (path && !IS_DEFAULT_DIR(path)){
    cache_model=gtk_tree_view_get_model (GTK_TREE_VIEW(main_window.pListView));
    search_control_sensible(TRUE);
  } else {
    search_control_sensible(FALSE);
  }
  store_last_folder(path);
}

static void create_tree_async(GFile *file){
  g_file_enumerate_children_async  (file,FOLDER_INFOFLAGS,G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,G_PRIORITY_DEFAULT,NULL, enumerate_files_async,(gpointer)CURRENTFOLDER); 
}

static gchar *get_mime_from_tree(GtkTreeView *tree_view){
  GtkTreeModel *model;
  GtkTreeSelection *select;
  GtkTreeIter iter;
  gchar *tmime;
  select = gtk_tree_view_get_selection(tree_view);
  if(gtk_tree_selection_get_selected (select, &model, &iter)) {
    gtk_tree_model_get (model, &iter,MIME_COLUMN,&tmime, -1);
  return tmime;
  }
  return NULL;
}

static gchar *get_path_from_tree(GtkTreeView *tree_view, gchar *root_path){
  GtkTreeModel *model;
  GtkTreeSelection *select;
  GtkTreeIter iter;
  select = gtk_tree_view_get_selection(tree_view);
  if(gtk_tree_selection_get_selected (select, &model, &iter)) {
    gchar *nfile;
    gtk_tree_model_get (model, &iter,FILE_COLUMN, &nfile, -1);
    GtkTreeIter* parentiter=(GtkTreeIter*)g_malloc(sizeof(GtkTreeIter));
    while(gtk_tree_model_iter_parent(model,parentiter,&iter)){
      gchar *rom;
      gtk_tree_model_get (model, parentiter, FILE_COLUMN, &rom, -1);
      nfile = g_build_path (G_DIR_SEPARATOR_S, rom, nfile, NULL);
      iter=*parentiter;
      g_free(parentiter);
      parentiter=(GtkTreeIter*)g_malloc(sizeof(GtkTreeIter));
     g_free(rom);
    }
    g_free(parentiter);
    gchar* file_name = g_build_path (G_DIR_SEPARATOR_S, root_path, nfile, NULL);
    g_free(nfile);
    return file_name;
  }
  return NULL;
}

void enumerate_files_async (GObject *source_object, GAsyncResult *res, gpointer user_data){
  GError *error=NULL;
  GFileEnumerator *enumerator= g_file_enumerate_children_finish    ( (GFile *)source_object,res,&error);
  if (!enumerator){
    g_print(_("Error getting folderbrowser files. GIO Error:%s\t"),error->message);
    g_error_free (error);
    DEBUG_GFILE((GFile *)source_object,TRUE);
    gtk_button_set_label(GTK_BUTTON(main_window.button_dialog), DEFAULT_DIR);
    clear_folderbrowser();
    search_control_sensible(FALSE);
    return;
  }
  while (gtk_events_pending ())
    gtk_main_iteration ();
  g_file_enumerator_next_files_async  (enumerator,30,G_PRIORITY_DEFAULT,NULL, finish_enumerate,user_data);
}

void finish_enumerate (GObject *source_object, GAsyncResult *res, gpointer user_data){
GError *error=NULL;
GList *filesinfo=NULL;
filesinfo= g_file_enumerator_next_files_finish ((GFileEnumerator *)source_object, res,&error);
if (error){
  g_print(_("Error::%s"),error->message);
  g_error_free (error);
}
//two options: finish or error?
if (!filesinfo){
  g_file_enumerator_close((GFileEnumerator *)source_object,NULL,NULL);
  g_object_unref((GFileEnumerator *)source_object);
  //if list isn't empty print files
  print_files(user_data);
  return;
}
GList *l;
for (l = filesinfo; l != NULL; l = g_list_next (l)) {
//store files
  GFileInfo *info;
  info=(GFileInfo *)l->data;
  const gchar *mime= g_file_info_get_content_type (info);
  if (!g_file_info_get_is_hidden (info)  && !g_file_info_get_is_backup (info)){
    if (MIME_ISDIR(mime)){
        //if has dot in name pos 0 don't process
        const gchar *folder=g_file_info_get_display_name(info);
        if(folder[0]!='.'){
          FOLDERFILE *current;
          current=new_folderfile();
          current->mime=mime;
          current->icon=g_file_info_get_icon(info); 
          current->filetype=g_file_info_get_file_type(info);
          current->display_name=folder;
          current->edit_name=g_file_info_get_edit_name(info);
          //gchar *sChemin=(gchar *)user_data;
          //gchar *next_dir = NULL;
          //next_dir = g_build_path (G_DIR_SEPARATOR_S, sChemin, current->display_name, NULL);
          //g_print("Next:'%s'\n",next_dir);
          //GFile *nextfile=g_file_new_for_uri (next_dir);
          //create_tree_async(nextfile);
          }
  } else {
    if (IS_TEXT(mime) && !IS_APPLICATION(mime)){
      //files
      FOLDERFILE *current;
      current=new_folderfile();
      current->mime=mime;
      current->icon=g_file_info_get_icon(info); 
      current->filetype=g_file_info_get_file_type(info);
      current->display_name=g_file_info_get_display_name(info);
      current->edit_name=g_file_info_get_edit_name(info);
      }
    }
  }
   while (gtk_events_pending ())
    gtk_main_iteration ();
}
g_list_free(filesinfo);
/* next files */
g_file_enumerator_next_files_async  ((GFileEnumerator *)source_object,30,G_PRIORITY_DEFAULT,NULL, finish_enumerate,user_data);
}

void update_folderbrowser (gchar *newpath){
  #ifdef DEBUGFOLDERBROWSER
  g_print("DEBUG::UPDATING FOLDERBROWSER\n");
  g_print("New path:%s\n",newpath);
  #endif
  GtkTreeIter iter2;
  GtkTreeIter* iter=NULL;
  if (newpath && !IS_DEFAULT_DIR(newpath)){
    gtk_button_set_label(GTK_BUTTON(main_window.button_dialog), newpath);
    clear_folderbrowser();
    init_folderbrowser(GTK_TREE_STORE(main_window.pTree),newpath,iter,&iter2);
  } else {
    gtk_button_set_label(GTK_BUTTON(main_window.button_dialog), DEFAULT_DIR);
    /* clear tree and cache data*/
    clear_folderbrowser();
    search_control_sensible(FALSE);
  }
}
static void button_refresh(void){
  gchar *folder=g_strdup((gchar *)CURRENTFOLDER);
  update_folderbrowser (folder);
  g_free(folder);
}
static void go_home_cb(void){
  if(main_window.current_editor){
  /*if there is a file open set file folder as home dir*/
    gchar *folderpath;
    if (main_window.current_editor->is_untitled==FALSE){
      if (editor_is_local(main_window.current_editor)){
        folderpath=filename_parent_uri(main_window.current_editor->filename->str);
      }else{
        /* set default dir as home dir*/
        folderpath= DEFAULT_DIR;
      }
    } else {
      /* set default dir as home dir*/
      folderpath= DEFAULT_DIR;
    }
    search_control_sensible(FALSE);
    #ifdef DEBUGFOLDERBROWSER
    g_print("DEBUG:::home dir:%s\n",folderpath);
    #endif
    update_folderbrowser (folderpath);
    if (folderpath!= DEFAULT_DIR) g_free(folderpath);
  }
}

/*
* go up one level in directory tree
*/
static void go_up_cb(void){
  gchar *folderpath= (gchar*)CURRENTFOLDER;
  if (folderpath && !IS_DEFAULT_DIR(folderpath)){
    gchar *fullfolderpath=filename_parent_uri(folderpath);
    #ifdef DEBUGFOLDERBROWSER
    g_print("DEBUG:::Up dir:%s\n",fullfolderpath);
    #endif
    if (fullfolderpath){
      update_folderbrowser (fullfolderpath);
      g_free(fullfolderpath);
    } else {
      update_folderbrowser (DEFAULT_DIR);
    }
  } else {
    search_control_sensible(FALSE);
  }
}

/*
 * tree_double_clicked
 *
 *   this function handles double click signal of folderbrowser treeview:
 *   if selected item is a file then open it
 *   if selected file is a directory force expand/collapse treerow
 */
void tree_double_clicked(GtkTreeView *tree_view,GtkTreePath *path,GtkTreeViewColumn *column,gpointer user_data)
 {
  gchar *mime=get_mime_from_tree(tree_view);
  gchar *folderpath= (gchar*)CURRENTFOLDER;
  gchar *file_name=get_path_from_tree(tree_view,folderpath);
  #ifdef DEBUGFOLDERBROWSER
  g_print("DEBUG:::DOUBLECLICK\t mime:%s\tname:%s\n",mime,file_name);
  #endif
  if (!MIME_ISDIR(mime)){
    g_free(mime);
    switch_to_file_or_open(file_name,0);
  } else {
    /*if(gtk_tree_view_row_expanded (tree_view,path))
        gtk_tree_view_collapse_row (tree_view,path);
      else
        gtk_tree_view_expand_row (tree_view,path,0);
    */
    g_free(mime);
    GtkTreeIter iter2;
    GtkTreeIter* iter=NULL;
    gtk_button_set_label(GTK_BUTTON(main_window.button_dialog), file_name);
    #ifdef DEBUGFOLDERBROWSER
    g_print("DEBUG::initing folderbrowser path:%s\n",file_name);
    #endif
    init_folderbrowser(GTK_TREE_STORE(main_window.pTree),file_name,iter,&iter2);
  }
  g_free(file_name);
}

/*
 * filebrowser_sort_func
 *
 * this function is the sort function, and has the following  features:
 * - directories go before files
 * - files are first sorted without extension, only equal names are sorted by extension
 *
 */
gint filebrowser_sort_func(GtkTreeModel * model, GtkTreeIter * a, GtkTreeIter * b,
               gpointer user_data)
{
  gchar *namea, *nameb, *mimea, *mimeb;
  gboolean isdira, isdirb;
  gint retval = 0;
  gtk_tree_model_get(GTK_TREE_MODEL (model), a, FILE_COLUMN, &namea,MIME_COLUMN, &mimea,-1);
  gtk_tree_model_get(GTK_TREE_MODEL (model), b, FILE_COLUMN, &nameb,MIME_COLUMN, &mimeb,-1);
  isdira = (mimea && MIME_ISDIR(mimea));
  isdirb = (mimeb && MIME_ISDIR(mimeb));
  #ifdef DEBUGFOLDERBROWSER
  g_print("DEBUG::SORTING FOLDERBROWSER\n");
  g_print("isdira=%d, mimea=%s, isdirb=%d, mimeb=%s\n",isdira,mimea,isdirb,mimeb);
  #endif
  if (isdira == isdirb) {    /* both files, or both directories */
    if (namea == nameb) {
      retval = 0;      /* both NULL */
    } else if (namea == NULL || nameb == NULL) {
      retval = (namea - nameb);
    } else {        /* sort by name, first without extension */
      gchar *dota, *dotb;
      dota = strrchr(namea, '.');
      dotb = strrchr(nameb, '.');
      if (dota)
        *dota = '\0';
      if (dotb)
        *dotb = '\0';
      retval = strcmp(namea, nameb);
      if (retval == 0) {
        if (dota)
          *dota = '.';
        if (dotb)
          *dotb = '.';
        retval = strcmp(namea, nameb);
      }
    }
  } else {          /* a directory and a file */
    retval = (isdirb - isdira);
  }
  g_free(namea);
  g_free(nameb);
  g_free(mimea);
  g_free(mimeb);
  return retval;
}

/*
 *   create tree
 *
 *   this function fill folderbrowser tree and start folderbrowser autorefresh
 */

void update_folderbrowser_signal (GFileMonitor *monitor,GFile *file,GFile *other_file, GFileMonitorEvent event_type, gpointer user_data){
  gtk_tree_store_clear(main_window.pTree);
  g_slist_free (filesinfolder);
  filesinfolder=NULL;
  update_folderbrowser ((gchar*)CURRENTFOLDER);
}
void popup_open_file(gchar *filename){
  switch_to_file_or_open(filename, 0);
}

/*
 * popup_delete_file
 *
 *   This function is the delete function of the folderbrowser popup menu, and has the following  features:
 * - Promp before delete the file
 * - Send file to trash if filesystem support that feature
 * - Delete file if filesystem don't support send to trash feature
 *
 */

void popup_delete_file(void){
 GtkWidget *dialog;
 gint button;
 dialog = gtk_message_dialog_new(GTK_WINDOW(main_window.window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_INFO,GTK_BUTTONS_YES_NO,"%s",
            _("Are you sure you wish to delete this file?"));
 gtk_window_set_title(GTK_WINDOW(dialog), _("Question"));
 button = gtk_dialog_run (GTK_DIALOG (dialog));
 gtk_widget_destroy(dialog);
  if (button == GTK_RESPONSE_YES){
    GFile *fi;
    GError *error=NULL;
    fi=get_gfile_from_filename((gchar *)pop.filename);
      if (!g_file_trash (fi,NULL,&error)){
        if (error->code == G_IO_ERROR_NOT_SUPPORTED){
          if (!g_file_delete (fi,NULL,&error)){
            g_print(_("GIO Error deleting file: %s\n"),error->message);
            g_error_free (error);
            }
        } else {
          g_print(_("GIO Error deleting file: %s\n"),error->message);
          g_error_free (error);
        }
      }
    g_object_unref (fi);
  }
}

void popup_rename_file(gchar *file){
  GFile *fi;
  GError *error=NULL;
  fi=get_gfile_from_filename(file);
  GFileInfo *info= g_file_query_info (fi, "standard::display-name",G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL,&error);
  if (!info){
    g_print(_("Error renaming file. GIO Error:%s\n"),error->message);
    g_error_free (error);
    return;
  }
  GtkWidget *window;
  window = gtk_dialog_new_with_buttons(_("Rename File"), GTK_WINDOW(main_window.window), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
  GtkWidget *vbox1 = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area(GTK_DIALOG(window))),vbox1);
  GtkWidget *hbox1 = gtk_hbox_new (FALSE, 8);
  gtk_widget_show (hbox1);
  gtk_container_add (GTK_CONTAINER (vbox1),hbox1);
  GtkWidget *label1 = gtk_label_new (_("New Filename"));
  gtk_widget_show (label1);
  gtk_container_add (GTK_CONTAINER (hbox1),label1);
  GtkWidget *text_filename = gtk_entry_new();
  gtk_entry_set_max_length (GTK_ENTRY(text_filename),20);
  gtk_entry_set_width_chars(GTK_ENTRY(text_filename),21);
  gtk_entry_set_text (GTK_ENTRY(text_filename),(gchar *)g_file_info_get_display_name (info));
  gtk_widget_show (text_filename);
  gtk_container_add (GTK_CONTAINER (hbox1),text_filename);
  gint res=gtk_dialog_run(GTK_DIALOG(window));
  const char *name=gtk_entry_get_text (GTK_ENTRY(text_filename));
  if (res==GTK_RESPONSE_ACCEPT){
    if (strcmp(name,g_file_info_get_display_name (info))!=0){
    fi=g_file_set_display_name (fi,name,NULL,&error);
    }
  }
  gtk_widget_destroy(window);
  g_object_unref(fi);
}

void popup_create_dir(void){
  GtkWidget *window;
  window = gtk_dialog_new_with_buttons(_("New Dir"), GTK_WINDOW(main_window.window), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
  GtkWidget *vbox1 = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area(GTK_DIALOG(window))),vbox1);
  GtkWidget *hbox1 = gtk_hbox_new (FALSE, 8);
  gtk_widget_show (hbox1);
  gtk_container_add (GTK_CONTAINER (vbox1),hbox1);
  GtkWidget *label1 = gtk_label_new (_("Directory Name"));
  gtk_widget_show (label1);
  gtk_container_add (GTK_CONTAINER (hbox1),label1);
  GtkWidget *text_filename = gtk_entry_new();
  gtk_entry_set_max_length (GTK_ENTRY(text_filename),20);
  gtk_entry_set_width_chars(GTK_ENTRY(text_filename),21);
  gtk_widget_show (text_filename);
  gtk_container_add (GTK_CONTAINER (hbox1),text_filename);
  gint res=gtk_dialog_run(GTK_DIALOG(window));
  const char *name=gtk_entry_get_text (GTK_ENTRY(text_filename));
  if (res==GTK_RESPONSE_ACCEPT && name){
    GFile *config;
    GError *error=NULL;
    gchar *filename;
    if (!MIME_ISDIR(pop.mime)){
      gchar *parent=filename_parent_uri((gchar *)pop.filename);
      filename= g_strdup_printf("%s/%s",parent,name);
      g_free(parent);
      config= get_gfile_from_filename(filename);
      g_free(filename);
    } else {
      filename= g_build_path (G_DIR_SEPARATOR_S, (gchar *)pop.filename, name, NULL);
      config=get_gfile_from_filename(filename);
      g_free(filename);
    }
    #ifdef DEBUGFOLDERBROWSER
    g_print("DEBUG::New directory:%s",filename);
    #endif
    if (!g_file_make_directory (config, NULL, &error)){
      g_print(_("Error creating folder. GIO error:%s\n"), error->message);
      g_error_free (error);
    }
    g_object_unref(config);
    gtk_widget_destroy(window);
    }
}

/*
 * view_popup_menu
 *
 *   This function shows a popup menu with the following features:
 * - Open File
 * - Rename File
 * - Delete File
 * - Create New directory
 *
 */

void view_popup_menu (GtkWidget *treeview, GdkEventButton *event, gpointer userdata) {
  GtkWidget *menu, *menuopen,*menurename,*menudelete,*menucreate,*sep;
  menu = gtk_menu_new();

  menuopen = gtk_menu_item_new_with_label(_("Open file"));
  if (MIME_ISDIR(pop.mime)){
    gtk_widget_set_state (menuopen,GTK_STATE_INSENSITIVE);
  }else {
    g_signal_connect(menuopen, "activate", (GCallback) popup_open_file, (gpointer) pop.filename);
  }
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuopen);

  menurename = gtk_menu_item_new_with_label(_("Rename File"));
  g_signal_connect(menurename, "activate", (GCallback) popup_rename_file, (gpointer) pop.filename);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menurename);

  menudelete = gtk_menu_item_new_with_label(_("Delete file"));
  g_signal_connect(menudelete, "activate", (GCallback) popup_delete_file, NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menudelete);

  sep = gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);
  
  menucreate = gtk_menu_item_new_with_label(_("Create New Directory"));
  g_signal_connect(menucreate, "activate", (GCallback) popup_create_dir, NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menucreate);

  gtk_widget_show_all(menu);

  /* Note: event can be NULL here when called from view_onPopupMenu;
   *  gdk_event_get_time() accepts a NULL argument */
  gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                   (event != NULL) ? event->button : 0,
                   gdk_event_get_time((GdkEvent*)event));
}


gboolean view_onButtonPressed (GtkWidget *treeview, GdkEventButton *event, gpointer userdata) {
  /* single click with the right mouse button? */
  if (event->type == GDK_BUTTON_PRESS  &&  event->button == 3){
    /* select row if no row is selected */
    GtkTreeSelection *selection;
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

    /* Note: gtk_tree_selection_count_selected_rows() does not
    *   exist in gtk+-2.0, only in gtk+ >= v2.2 ! */
    if (gtk_tree_selection_count_selected_rows(selection)  <= 1){
      GtkTreePath *path;
      /* Get tree path for row that was clicked */
      if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview),
                                             (gint) event->x,
                                             (gint) event->y,
                                             &path, NULL, NULL, NULL)) {
      gtk_tree_selection_unselect_all(selection);
      gtk_tree_selection_select_path(selection, path);
      gtk_tree_path_free(path);
      }
    }
    gchar *mime=get_mime_from_tree(GTK_TREE_VIEW(treeview));
    gchar *path=(gchar*)CURRENTFOLDER;
    gchar *file_name=get_path_from_tree(GTK_TREE_VIEW(treeview),path);
    pop.filename=file_name;
    pop.mime=mime;
    view_popup_menu(treeview, event, NULL);
      return TRUE; /* we handled this */
    }
  return FALSE; /* we did not handle this */
}

gboolean view_onPopupMenu (GtkWidget *treeview, gpointer userdata){
  view_popup_menu(treeview, NULL, userdata);
  return TRUE; /* we handled this */
}

gboolean key_press (GtkWidget *widget, GdkEventKey *event, gpointer user_data){
  #ifdef DEBUGFOLDERBROWSER
  g_print("DEBUG::folderbrowser keypress. keyval:%d \n",event->keyval);
  #endif
  if (event->keyval==GDK_Delete || event->keyval==GDK_Return){
    gchar *mime=get_mime_from_tree(GTK_TREE_VIEW(widget));
    gchar *path=(gchar*)CURRENTFOLDER;
    gchar *file_name=get_path_from_tree(GTK_TREE_VIEW(widget),path);
    if (event->keyval==GDK_Delete){
      //delete file
      pop.filename=file_name;
      pop.mime=mime;
      popup_delete_file();
    }else {
      //open file
      if (!MIME_ISDIR(mime))
      switch_to_file_or_open(file_name, 0);
    }
    return TRUE;
  }
  return FALSE;
}
/*
 * folderbrowser_create
 *
 *   Create folderbrowser widget and append it to the side bar
 *   Folderbrowser treeview has 3 columns:
 *   --icon column
 *   --filename column
 *   --mimetype column (hide for users)
 */

extern void folderbrowser_create(MainWindow *main_window)
 {
  main_window->folder = gtk_vbox_new(FALSE, 0);

  GtkTreeViewColumn *pColumn;
  GtkCellRenderer  *pCellRenderer;
  main_window->pTree = gtk_tree_store_new(N_COL, GDK_TYPE_PIXBUF, G_TYPE_STRING,G_TYPE_STRING);
  main_window->pListView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(main_window->pTree));
  pCellRenderer = gtk_cell_renderer_pixbuf_new();
  pColumn = gtk_tree_view_column_new_with_attributes("",pCellRenderer,"pixbuf",ICON_COLUMN,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(main_window->pListView), pColumn);
  main_window->pScrollbar = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(main_window->pScrollbar),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add(GTK_CONTAINER(main_window->pScrollbar), main_window->pListView);

  //renderer for text
  g_signal_connect(G_OBJECT(main_window->pListView), "row-activated", G_CALLBACK(tree_double_clicked), NULL);
  g_signal_connect(G_OBJECT(main_window->pListView), "button-press-event", (GCallback) view_onButtonPressed, NULL);
  g_signal_connect(G_OBJECT(main_window->pListView), "popup-menu", (GCallback) view_onPopupMenu, NULL);
  g_signal_connect(G_OBJECT(main_window->pListView), "key-press-event", G_CALLBACK(key_press), NULL);
  const GtkTargetEntry drag_dest_types[] = {
    {"text/uri-list", 0, TARGET_URI_LIST},
    {"STRING", 0, TARGET_STRING},
  };
  gtk_drag_dest_set(main_window->pListView, (GTK_DEST_DEFAULT_ALL), drag_dest_types, 2,
    (GDK_ACTION_DEFAULT | GDK_ACTION_COPY));
  g_signal_connect(G_OBJECT(main_window->pListView), "drag_data_received", G_CALLBACK(fb_file_v_drag_data_received),NULL);

  pColumn=NULL;
  pCellRenderer=NULL;
  pCellRenderer = gtk_cell_renderer_text_new();
  pColumn = gtk_tree_view_column_new_with_attributes(_("File"), pCellRenderer, "text",FILE_COLUMN,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(main_window->pListView), pColumn);

  pColumn=NULL;
  pCellRenderer=NULL;
  pCellRenderer = gtk_cell_renderer_text_new();
  pColumn = gtk_tree_view_column_new_with_attributes(_("Mime"), pCellRenderer,"text",MIME_COLUMN,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(main_window->pListView), pColumn);
  gtk_tree_view_column_set_visible    (pColumn,FALSE);
  gtk_widget_show(main_window->folder);

  //GtkWidget *label= gtk_label_new(_("Folder browser"));
  GtkWidget *label= gtk_image_new_from_file (PIXMAP_DIR "/folderbrowser.png");
  /* set tooltip */
  gtk_widget_set_tooltip_text (label,_("Folder Browser"));
  GConfClient *config;
  config=gconf_client_get_default ();
  /*load folder from config*/
  gchar *folderpath;
  folderpath=gconf_client_get_string(config,"/gPHPEdit/main_window/folderbrowser/folder",NULL);
  g_object_unref(config);
  if (!folderpath){
    main_window->button_dialog = gtk_button_new_with_label (DEFAULT_DIR);
  } else {
    main_window->button_dialog = gtk_button_new_with_label (folderpath);
    g_free(folderpath);
  }
  g_signal_connect(G_OBJECT(main_window->button_dialog), "pressed", G_CALLBACK(pressed_button_file_chooser), NULL);
  gtk_widget_set_size_request (main_window->pListView,80,450);

  /* home and up buttons*/
  GtkWidget *hbox2;
  hbox2 = gtk_hbox_new(FALSE, 0);
  /* home button */
  main_window->button_home= gtk_button_new();
  main_window->image_home= gtk_image_new_from_stock (GTK_STOCK_HOME,GTK_ICON_SIZE_BUTTON);
  gtk_widget_show(main_window->image_home);
  gtk_button_set_image(GTK_BUTTON(main_window->button_home), main_window->image_home);
  gtk_widget_show(main_window->button_home);
  gtk_widget_set_tooltip_text (main_window->button_home,_("Go Home Dir"));
  gtk_box_pack_start(GTK_BOX(hbox2), main_window->button_home, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(main_window->button_home), "clicked", G_CALLBACK (go_home_cb),NULL);

  /* up button */
  main_window->button_up= gtk_button_new();
  main_window->image_up= gtk_image_new_from_stock (GTK_STOCK_GO_UP,GTK_ICON_SIZE_BUTTON );
  gtk_widget_show(main_window->image_up);
  gtk_button_set_image(GTK_BUTTON(main_window->button_up), main_window->image_up);
  gtk_widget_show(main_window->button_up);
  gtk_widget_set_tooltip_text (main_window->button_up,_("Go Up one level"));
  gtk_box_pack_start(GTK_BOX(hbox2), main_window->button_up, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(main_window->button_up), "clicked", G_CALLBACK (go_up_cb),NULL);
  /* refresh button */
  main_window->button_refresh= gtk_button_new();
  main_window->image_refresh= gtk_image_new_from_stock (GTK_STOCK_REFRESH,GTK_ICON_SIZE_BUTTON );
  gtk_widget_show(main_window->image_refresh);
  gtk_button_set_image(GTK_BUTTON(main_window->button_refresh), main_window->image_refresh);
  gtk_widget_show(main_window->button_refresh);
  gtk_widget_set_tooltip_text (main_window->button_refresh,_("Refresh folderbrowser"));
  gtk_box_pack_start(GTK_BOX(hbox2), main_window->button_refresh, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(main_window->button_refresh), "clicked", G_CALLBACK (button_refresh),NULL);
  main_window->searchentry = gtk_entry_new();
  gtk_widget_show(main_window->searchentry);
  gtk_widget_set_tooltip_text (main_window->searchentry,_("Search Files in Folder Browser"));
  gtk_entry_set_icon_from_stock (GTK_ENTRY(main_window->searchentry),GTK_ENTRY_ICON_SECONDARY,GTK_STOCK_CLEAR);
  gtk_entry_set_icon_from_stock (GTK_ENTRY(main_window->searchentry),GTK_ENTRY_ICON_PRIMARY,GTK_STOCK_FIND);
  g_signal_connect (G_OBJECT (main_window->searchentry), "icon-press", G_CALLBACK (on_cleanicon_press), NULL);
  g_signal_connect_after(G_OBJECT(main_window->searchentry), "insert_text", G_CALLBACK(search_typed), NULL);
  g_signal_connect_after(G_OBJECT(main_window->searchentry), "backspace", G_CALLBACK(search_activate), NULL);
  g_signal_connect_after(G_OBJECT(main_window->searchentry), "activate", G_CALLBACK(search_activate), NULL);
  search_control_sensible(TRUE);
  gtk_box_pack_start(GTK_BOX(hbox2), main_window->searchentry, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(main_window->folder), hbox2, FALSE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(main_window->folder), main_window->button_dialog, FALSE, FALSE, 2);
  gtk_box_pack_start(GTK_BOX(main_window->folder), main_window->pScrollbar, TRUE, TRUE, 2);
  gtk_widget_show(main_window->button_dialog);
  gtk_widget_show_all(main_window->folder);
  gtk_notebook_insert_page (GTK_NOTEBOOK(main_window->notebook_manager), main_window->folder, label, 1);
  gchar *path=(gchar *)gtk_button_get_label(GTK_BUTTON(main_window->button_dialog));
    if(!IS_DEFAULT_DIR(path)){
    GtkTreeIter iter2;
    GtkTreeIter* iter=NULL;
    gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(main_window->pTree), 1,
    filebrowser_sort_func, NULL, NULL);
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(main_window->pTree), 1, GTK_SORT_ASCENDING);
    init_folderbrowser(GTK_TREE_STORE(main_window->pTree),path,iter,&iter2);
    #ifdef DEBUGFOLDERBROWSER
    g_print("Loading Folderbrowser for path:'%s'\n",path);
    #endif
  }
}

void init_folderbrowser(GtkTreeStore *pTree, gchar *filename, GtkTreeIter *iter, GtkTreeIter *iter2){
  GFile *file;
  GError *error=NULL;
  file= g_file_new_for_uri (filename);
  //path don't exist?
  if (!g_file_query_exists (file,NULL)){
    #ifdef DEBUGFOLDERBROWSER
      g_print("DEBUG::: file '%s' don't exist?\n",filename);
    #endif
    gtk_button_set_label(GTK_BUTTON(main_window.button_dialog), DEFAULT_DIR);
    clear_folderbrowser();
    search_control_sensible(FALSE);
    store_last_folder(DEFAULT_DIR);
    return;
  }
  GFileInfo *info =g_file_query_info (file,G_FILE_ATTRIBUTE_ACCESS_CAN_READ, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,NULL,&error);
  if (!info){
    g_print("ERROR initing folderbrowser:%s\n",error->message);
    #ifdef DEBUGFOLDERBROWSER
      g_print("DEBUG::folder %s\n",filename);
    #endif
    gtk_button_set_label(GTK_BUTTON(main_window.button_dialog), DEFAULT_DIR);
    clear_folderbrowser();
    search_control_sensible(FALSE);
    store_last_folder(DEFAULT_DIR);
    return;
  }
  if (!g_file_info_get_attribute_boolean (info, G_FILE_ATTRIBUTE_ACCESS_CAN_READ)){
    g_print("Error Don't have read access for current folderbrowser path.\n");
    #ifdef DEBUGFOLDERBROWSER
      g_print("DEBUG::folder %s\n",filename);
    #endif
    gtk_button_set_label(GTK_BUTTON(main_window.button_dialog), DEFAULT_DIR);
    clear_folderbrowser();
    search_control_sensible(FALSE);
    store_last_folder(DEFAULT_DIR);
    return;
  }
  g_object_unref(info);
  /* clear tree and cache data*/
  clear_folderbrowser();
  #ifdef DEBUGFOLDERBROWSER
  g_print("DEBUG:: clear tree and cache data\n");
  #endif
  store_last_folder(filename);
  create_tree_async(file);
  g_object_unref(file);
}

void fb_file_v_drag_data_received(GtkWidget * widget, GdkDragContext * context, gint x,  gint y, GtkSelectionData * data, guint info, guint time,gpointer user_data)
{
  gchar *stringdata;
  GFile *destdir = get_gfile_from_filename ((gchar*)CURRENTFOLDER);
  g_object_ref(destdir);

  g_signal_stop_emission_by_name(widget, "drag_data_received");
  if ((gtk_selection_data_get_length(data) == 0) || (gtk_selection_data_get_format(data) != 8)
    || ((info != TARGET_STRING) && (info != TARGET_URI_LIST))) {
    gtk_drag_finish(context, FALSE, TRUE, time);
    return;
  }
  stringdata = g_strndup((const gchar *) gtk_selection_data_get_data(data), gtk_selection_data_get_length(data));
  #ifdef DEBUGFOLDERBROWSER
  g_print("fb2_file_v_drag_data_received, stringdata='%s', len=%d\n", stringdata, gtk_selection_data_get_length(data));
  #endif
  if (destdir) {
    if (strchr(stringdata, '\n') == NULL) {  /* no newlines, probably a single file */
      GSList *list = NULL;
      GFile *uri;
      uri = g_file_new_for_commandline_arg(stringdata);
      list = g_slist_append(list, uri);
      copy_uris_async(destdir, list);
      g_slist_free(list);
      g_object_unref(uri);
    } else {
      /* there are newlines, probably this is a list of uri's */
      copy_files_async(destdir, stringdata);
    }
    g_object_unref(destdir);
    gtk_drag_finish(context, TRUE, TRUE, time);
  } else {
    gtk_drag_finish(context, FALSE, TRUE, time);
  }
  g_free(stringdata);
}
/************************/
/**
 * trunc_on_char:
 * @string: a #gchar * to truncate
 * @which_char: a #gchar with the char to truncate on
 *
 * Returns a pointer to the same string which is truncated at the first
 * occurence of which_char
 *
 * Return value: the same gchar * as passed to the function
 **/
gchar *trunc_on_char(gchar * string, gchar which_char)
{
  gchar *tmpchar = string;
  while(*tmpchar) {
    if (*tmpchar == which_char) {
      *tmpchar = '\0';
      return string;
    }
    tmpchar++;
  }
  return string;
}
/********************/
 typedef struct {
 GSList *sourcelist;
 GFile *destdir;
 GFile *curfile, *curdest;
 } Tcopyfile;

 static gboolean copy_uris_process_queue(Tcopyfile *cf);
 void copy_async_lcb(GObject *source_object,GAsyncResult *res,gpointer user_data) {
 Tcopyfile *cf = user_data;
 gboolean done;
 GError *error=NULL;
 /* fill in the blanks */
 done = g_file_copy_finish(cf->curfile,res,&error);
 if (!done) {
    if (error->code == G_IO_ERROR_EXISTS) {
      gint retval;
      gchar *tmpstr, *dispname;
      GFileInfo *info =g_file_query_info (cf->curfile,"standard::display-name", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,NULL,NULL);
      dispname = (gchar *)g_file_info_get_display_name (info);
      tmpstr = g_strdup_printf(_("%s cannot be copied, it already exists, overwrite?"),dispname);
      retval = yes_no_dialog (_("Overwrite file?"), tmpstr);
      g_free(tmpstr);
      g_free(dispname);
      if (retval != -8) {
        g_file_copy_async(cf->curfile,cf->curdest,G_FILE_COPY_OVERWRITE,
            G_PRIORITY_LOW,NULL,
            NULL,NULL,
            copy_async_lcb,cf);
        return;
      }
    }else {
      g_print("ERROR copying file::%s\n",error->message);
      g_error_free (error);
    }
  }
  g_object_unref(cf->curfile);
  g_object_unref(cf->curdest);
  if (!copy_uris_process_queue(cf)) {
    update_folderbrowser((gchar*)CURRENTFOLDER);
    g_object_unref(cf->destdir);
    g_slice_free(Tcopyfile,cf);
  }
}

static gboolean copy_uris_process_queue(Tcopyfile *cf) {
  if (cf->sourcelist) {
    GFile *uri, *dest;
    gchar *tmp;

    uri = cf->sourcelist->data;
    cf->sourcelist = g_slist_remove(cf->sourcelist, uri);
    tmp = g_file_get_basename(uri);
    dest = g_file_get_child(cf->destdir,tmp);
    g_free(tmp);

    cf->curfile = uri;
    cf->curdest = dest;
    g_file_copy_async(uri,dest,G_FILE_COPY_NONE,
          G_PRIORITY_LOW,NULL,
          NULL,NULL,
          copy_async_lcb,cf);
    return TRUE;
  }
  return FALSE;
}

void copy_uris_async(GFile *destdir, GSList *sources) {
    Tcopyfile *cf;
    GSList *tmplist;
    cf = g_slice_new0(Tcopyfile);
    cf->destdir = destdir;
    g_object_ref(cf->destdir);
    cf->sourcelist = g_slist_copy(sources);
    tmplist = cf->sourcelist;
    while (tmplist) {
      g_object_ref(tmplist->data);
      tmplist = tmplist->next;
    }
    copy_uris_process_queue(cf);
  }
void copy_files_async(GFile *destdir, gchar *sources) {
  Tcopyfile *cf;
  gchar **splitted, **tmp;
  cf = g_slice_new0(Tcopyfile);
  cf->destdir = destdir;
  g_object_ref(cf->destdir);
  /* create the source and destlist ! */
  tmp = splitted = g_strsplit(sources, "\n",0);
  while (*tmp) {
    trunc_on_char(trunc_on_char(*tmp, '\r'), '\n');
    if (strlen(*tmp) > 1) {
      GFile *src;
      src = g_file_new_for_commandline_arg(*tmp);
      cf->sourcelist = g_slist_append(cf->sourcelist, src);
    }
      tmp++;
  }
  g_strfreev(splitted);
  copy_uris_process_queue(cf);
}
