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
#include <config.h>
#include "main.h"

#include "debug.h"
#include "filebrowser_backend.h"
#include "tab.h"
#include "preferences_manager.h"
#include "document_manager.h"
#include "gvfs_utils.h"

#define MIME_ISDIR(string) (g_strcmp0(string, "inode/directory")==0)
#define IS_MIME(stringa,stringb) (g_content_type_equals (stringa, stringb))
#define IS_TEXT(stringa) (g_content_type_is_a (stringa, "text/*"))
#define IS_APPLICATION(stringa) (g_content_type_is_a (stringa, "application/*") && !IS_MIME(stringa,"application/x-php") && !IS_MIME(stringa,"application/javascript") && !IS_MIME(stringa,"application/x-perl"))
#define DEFAULT_DIR (N_("Workspace's directory"))
#define IS_DEFAULT_DIR(a) (g_strcmp0(a,DEFAULT_DIR)==0)
#define FOLDER_INFOFLAGS "standard::is-backup,standard::display-name,standard::icon,standard::content-type"

/* object signal enumeration */
enum {
	DONE_LOADING,
	CHANGE_FOLDER,
	LAST_SIGNAL
};

struct FilebrowserBackendDetails
{
 GFileEnumerator * enumerator;
 GCancellable *cancellable;
 guint idle_id;
 GSList *filesinfolder;
 gchar *current_folder;
 GFileMonitor *monitor;
};

#define FILEBROWSER_BACKEND_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    FILEBROWSER_TYPE_BACKEND,\
					    FilebrowserBackendDetails))

static guint signals[LAST_SIGNAL];

static void               filebrowser_backend_finalize         (GObject                *object);
static void               filebrowser_backend_init             (gpointer                object,
							       gpointer                klass);
static void               filebrowser_backend_class_init (FilebrowserBackendClass *klass);
static gpointer filebrowser_backend_parent_class;

static inline void change_current_folder(FilebrowserBackend *filebackend, const gchar *new_folder);

gboolean populate_files (gpointer data);
void clean_enumerate (gpointer data);
void init_enumeration( FilebrowserBackend *filebackend);

static FOLDERFILE *new_folderfile(void)
{
  FOLDERFILE *file;
  file = (FOLDERFILE *) g_slice_new0(FOLDERFILE);
  return file;
}

GType
filebrowser_backend_get_type (void)
{
    static GType our_type = 0;
    
    if (!our_type) {
        static const GTypeInfo our_info =
        {
            sizeof (FilebrowserBackendClass),
            NULL,               /* base_init */
            NULL,               /* base_finalize */
            (GClassInitFunc) filebrowser_backend_class_init,
            NULL,               /* class_finalize */
            NULL,               /* class_data */
            sizeof (FilebrowserBackend),
            0,                  /* n_preallocs */
            (GInstanceInitFunc) filebrowser_backend_init,
        };

        our_type = g_type_register_static (G_TYPE_OBJECT, "FilebrowserBackend",
                                           &our_info, 0);
  }
    
    return our_type;
}

static void
filebrowser_backend_class_init (FilebrowserBackendClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	
	object_class->finalize = filebrowser_backend_finalize;
	filebrowser_backend_parent_class = g_type_class_peek_parent (klass);

	signals[DONE_LOADING] =
		g_signal_new ("done_loading",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (FilebrowserBackendClass, done_loading),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);
 
	signals[CHANGE_FOLDER] =
		g_signal_new ("change_folder",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (FilebrowserBackendClass, change_folder),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__STRING,
		              G_TYPE_NONE, 1, G_TYPE_STRING);

	g_type_class_add_private (klass, sizeof (FilebrowserBackendDetails));
}

static void
filebrowser_backend_init (gpointer object, gpointer klass)
{
//	FilebrowserBackendDetails *directory;
//	directory = FILEBROWSER_BACKEND_GET_PRIVATE(object);
}

static inline void clear_filebrowser_backend(FilebrowserBackendDetails *directory, gboolean clear_current_folder){
  if (directory->filesinfolder) g_slist_free (directory->filesinfolder);
  directory->filesinfolder=NULL;
  if (directory->current_folder && clear_current_folder){
    g_free(directory->current_folder);
    directory->current_folder=NULL;
  }
}

static inline void filebrowser_backend_restore(FilebrowserBackend *filebackend){
  FilebrowserBackendDetails *directory;
  directory = FILEBROWSER_BACKEND_GET_PRIVATE(filebackend);
  clear_filebrowser_backend(directory, TRUE);
  change_current_folder(filebackend, DEFAULT_DIR);
  g_signal_emit (G_OBJECT (filebackend), signals[DONE_LOADING], 0); /* needed to update ui */
}

static void
filebrowser_backend_finalize (GObject *object)
{
	FilebrowserBackendDetails *directory;

	directory = FILEBROWSER_BACKEND_GET_PRIVATE(object);
	g_cancellable_cancel (directory->cancellable);
  
	clear_filebrowser_backend(directory, TRUE);
	g_object_unref(directory->cancellable);
	G_OBJECT_CLASS (filebrowser_backend_parent_class)->finalize (object);
}

/*
* init object and start process
*/
gboolean init_filebrowser(FilebrowserBackend *filebackend){
  FilebrowserBackendDetails *directory;
  directory = FILEBROWSER_BACKEND_GET_PRIVATE(filebackend);

  directory->cancellable = g_cancellable_new ();
  gphpedit_debug_message(DEBUG_FILEBROWSER, "init with :%s\n",directory->current_folder);
  GFile *file = get_gfile_from_filename (directory->current_folder);
  //file don't exist?
  if (!g_file_query_exists (file,directory->cancellable)){
    gphpedit_debug_message(DEBUG_FILEBROWSER, "file '%s' don't exist?\n",directory->current_folder);
    g_object_unref(file);
    filebrowser_backend_restore(filebackend);
    return FALSE;
  }
  GError *error=NULL;
  GFileInfo *info =g_file_query_info (file,G_FILE_ATTRIBUTE_ACCESS_CAN_READ, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,directory->cancellable,&error);
  if (!info){
    g_print("ERROR initing file browser:%s\n",error->message);
    g_error_free(error);
    gphpedit_debug_message(DEBUG_FILEBROWSER, "folder %s\n",directory->current_folder);
    g_object_unref(file);
    filebrowser_backend_restore(filebackend);
    return FALSE;
  }
  if (!g_file_info_get_attribute_boolean (info, G_FILE_ATTRIBUTE_ACCESS_CAN_READ)){
    gphpedit_debug_message(DEBUG_FILEBROWSER, "Don't have read access for %s.\n", directory->current_folder);
    filebrowser_backend_restore(filebackend);
    return FALSE;
  }
  g_object_unref(info);
  /* clear cache data*/
  clear_filebrowser_backend(directory, FALSE);
  gphpedit_debug_message(DEBUG_FILEBROWSER, "%s","clear tree and cache data\n");
  /* enumerate folder files */
  g_object_unref(file);
  if (g_cancellable_is_cancelled (directory->cancellable)) return FALSE;
  init_enumeration(filebackend);
  return TRUE;
}

void init_enumeration( FilebrowserBackend *filebackend)
{
  FilebrowserBackendDetails *directory = FILEBROWSER_BACKEND_GET_PRIVATE(filebackend);
  if (g_cancellable_is_cancelled (directory->cancellable)) return ; /* remove source */
  GFile *file = get_gfile_from_filename (directory->current_folder);
  GError *error=NULL;
  directory->enumerator =  g_file_enumerate_children (file,FOLDER_INFOFLAGS,G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, directory->cancellable,&error);
  if (!directory->enumerator){
    g_print(_("Error getting file browser files. GIO Error:%s\t"),error->message);
    g_error_free (error);
    filebrowser_backend_restore(filebackend);
    return ;
  }
 directory->idle_id = gdk_threads_add_idle_full (G_PRIORITY_DEFAULT, populate_files,  filebackend,  clean_enumerate);
}

gboolean populate_files (gpointer data)     //TODO:: show an spinner while loading
{
  FilebrowserBackend *filebackend= FILEBROWSER_BACKEND(data);
  FilebrowserBackendDetails *directory = FILEBROWSER_BACKEND_GET_PRIVATE(filebackend);
  GDK_THREADS_ENTER();
  if (g_cancellable_is_cancelled (directory->cancellable)){
  GDK_THREADS_LEAVE();
  return FALSE; /* remove source */
  }
  GError *error=NULL;
  GFileInfo *info = g_file_enumerator_next_file (directory->enumerator, directory->cancellable, &error);
  if (info){
	  const gchar *mime= g_file_info_get_content_type (info);
	  if (!g_file_info_get_is_hidden (info)  && !g_file_info_get_is_backup (info)){
	    if (MIME_ISDIR(mime)){
		//if has dot in name pos 0 don't process
		const gchar *folder=g_file_info_get_display_name(info);
		if(folder[0]!='.'){
		  FOLDERFILE *current;
		  current=new_folderfile();
		  current->mime=g_strdup(mime);
    	          GIcon *icon =g_file_info_get_icon(info); 
	          current->icon= g_icon_to_string (icon);
		  current->display_name=g_strdup(folder);
		  /* add to list */
		 directory->filesinfolder = g_slist_append(directory->filesinfolder, current);
		  }
	  } else {
	    if (IS_TEXT(mime) && !IS_APPLICATION(mime)){
	      //files
	      FOLDERFILE *current;
	      current=new_folderfile();
	      current->mime=g_strdup(mime);
	      GIcon *icon =g_file_info_get_icon(info); 
	      current->icon= g_icon_to_string (icon);
	      current->display_name=g_strdup(g_file_info_get_display_name(info));
	      /* add to list */
	      directory->filesinfolder = g_slist_append(directory->filesinfolder, current);
	      }
	    }
	  }	
	g_object_unref(info);
   } else {
   	if (error){
   		g_print(_("Error::%s"),error->message);
   		g_error_free (error);
   	}
	GDK_THREADS_LEAVE();
	return FALSE; /* remove source */
   }
    GDK_THREADS_LEAVE();
    return TRUE;
}

void clean_enumerate (gpointer data){
  FilebrowserBackend *filebackend= FILEBROWSER_BACKEND(data);
  FilebrowserBackendDetails *directory = FILEBROWSER_BACKEND_GET_PRIVATE(filebackend);
  if (directory->enumerator){
    g_file_enumerator_close(directory->enumerator,directory->cancellable,NULL);
    g_object_unref(directory->enumerator);
  }
  g_cancellable_reset (directory->cancellable);
  g_signal_emit (G_OBJECT (filebackend), signals[DONE_LOADING], 0);
}

FilebrowserBackend *filebrowser_backend_new (const gchar *folder)
{
  gphpedit_debug(DEBUG_FILEBROWSER);
	FilebrowserBackend *fbback= g_object_new (FILEBROWSER_TYPE_BACKEND, NULL);

	if (folder && strlen(folder)!=0){
	  change_current_folder(fbback, folder);
	} else {
	  change_current_folder(fbback, DEFAULT_DIR);
	}
	init_filebrowser(fbback);
	return fbback;
}

const gchar *get_filebrowser_backend_current_folder(FilebrowserBackend *fbback){
	FilebrowserBackendDetails *directory = FILEBROWSER_BACKEND_GET_PRIVATE(fbback);
	return directory->current_folder;
}

guint get_filebrowser_backend_number_files(FilebrowserBackend *fbback){
  if (!fbback) return 0;
	FilebrowserBackendDetails *directory = FILEBROWSER_BACKEND_GET_PRIVATE(fbback);
	return g_slist_length (directory->filesinfolder);
}

GSList *get_filebrowser_backend_files(FilebrowserBackend *fbback){
  if (!fbback) return NULL;
	FilebrowserBackendDetails *directory = FILEBROWSER_BACKEND_GET_PRIVATE(fbback);
	return directory->filesinfolder;
}

void filebrowser_backend_update_folder (FilebrowserBackend *fbback, const gchar *newfolder){
  gphpedit_debug_message(DEBUG_FILEBROWSER, "New path:%s\n",newfolder);

  if (newfolder && !IS_DEFAULT_DIR(newfolder)){
    gchar *real_path=filename_get_path((gchar *)newfolder);
    change_current_folder(fbback, real_path);
    init_filebrowser(fbback);
    g_free(real_path);
  } else {
    filebrowser_backend_restore(fbback);
  }
}

void filebrowser_backend_go_folder_up (FilebrowserBackend *fbback){
  FilebrowserBackendDetails *directory = FILEBROWSER_BACKEND_GET_PRIVATE(fbback);

  gchar *fullfolderpath=filename_parent_uri(directory->current_folder);
  gphpedit_debug_message(DEBUG_FILEBROWSER, "Up dir:%s\n",fullfolderpath);
  filebrowser_backend_update_folder (fbback,fullfolderpath); /*update with new uri */
  if (fullfolderpath) g_free(fullfolderpath);
}

void filebrowser_backend_go_folder_home (FilebrowserBackend *fbback, gchar *filename){
//  FilebrowserBackendDetails *directory = FILEBROWSER_BACKEND_GET_PRIVATE(fbback);
  gchar *folderpath=NULL;
  if (filename_is_native(filename)){
    folderpath=filename_parent_uri(filename);
    gphpedit_debug_message(DEBUG_FILEBROWSER, "HOME dir:%s\n",folderpath);
  }
  filebrowser_backend_update_folder (fbback,folderpath);
  if (folderpath) g_free(folderpath);
}
void filebrowser_backend_refresh_folder (FilebrowserBackend *fbback){
  gphpedit_debug(DEBUG_FILEBROWSER);
  FilebrowserBackendDetails *directory = FILEBROWSER_BACKEND_GET_PRIVATE(fbback);
  filebrowser_backend_update_folder (fbback,directory->current_folder); /*update with new uri */
}

void filebrowser_backend_open_file (FilebrowserBackend *fbback, gchar *filename){
  gphpedit_debug(DEBUG_FILEBROWSER);
  DocumentManager *docmg = document_manager_new();
  document_manager_switch_to_file_or_open(docmg,filename, 0);
  g_object_unref(docmg);
}

void filebrowser_backend_delete_file(FilebrowserBackend *filebackend, gchar *filename){
  if (!filebackend || !filename) return;
 gint button = yes_no_dialog (_("Question"), _("Are you sure you wish to delete this file?"));
  if (button == GTK_RESPONSE_YES){
	filename_delete_file(filename);
	filebrowser_backend_refresh_folder (filebackend);
  }
}

void filebrowser_backend_create_dir(FilebrowserBackend *filebackend, gchar *filename, gchar *name, gboolean isdir){
  FilebrowserBackendDetails *directory = FILEBROWSER_BACKEND_GET_PRIVATE(filebackend);
  GFile *config;
  GError *error=NULL;
  gchar *filename_int;
  if (isdir){
      gchar *parent=filename_parent_uri(filename);
      filename_int= g_strdup_printf("%s/%s",parent,name);
      g_free(parent);
      config= get_gfile_from_filename(filename_int);
  } else {
      gchar *parent;
      if (filename) {
        parent=filename_parent_uri(filename);
      } else {
        parent=g_strdup(directory->current_folder);
      }
      filename_int= g_build_path (G_DIR_SEPARATOR_S, parent, name, NULL);
      g_free(parent);
      config=get_gfile_from_filename(filename_int);
  }
  gphpedit_debug_message(DEBUG_FILEBROWSER, "New directory:%s",filename_int);

  g_free(filename_int);
  if (!g_file_make_directory (config, directory->cancellable, &error)){
     g_print(_("Error creating folder. GIO error:%s\n"), error->message);
     g_error_free (error);
  }
  g_object_unref(config);
	filebrowser_backend_refresh_folder (filebackend);
}

gchar *filebrowser_backend_get_display_name(FilebrowserBackend *filebackend, gchar *filename){
  return filename_get_display_name(filename);
}

void filebrowser_backend_rename_file(FilebrowserBackend *filebackend,gchar *filename, gchar *current_name, gchar *new_name){
  if (g_strcmp0(new_name,current_name)!=0){
     filename_rename(filename,new_name);
    }
}

static inline void change_current_folder(FilebrowserBackend *filebackend, const gchar *new_folder){
  FilebrowserBackendDetails *directory;
  directory = FILEBROWSER_BACKEND_GET_PRIVATE(filebackend);
  if (!new_folder && *(new_folder)!=0) return;
  directory->current_folder= g_strdup(new_folder);
  PreferencesManager *prefmg = preferences_manager_new ();
  g_object_set(prefmg, "filebrowser_last_folder", directory->current_folder,NULL);
  g_object_unref(prefmg);
  gchar *real_path=filename_get_relative_path((gchar *)new_folder);
  if (!real_path) real_path=g_strdup(DEFAULT_DIR);
  g_signal_emit (G_OBJECT (filebackend), signals[CHANGE_FOLDER], 0, real_path); /* needed to update ui */
  g_free(real_path);
}

void filebrowser_backend_cancel (FilebrowserBackend *fbback){
  FilebrowserBackendDetails *directory;
  directory = FILEBROWSER_BACKEND_GET_PRIVATE(fbback);
  g_cancellable_cancel (directory->cancellable);
}


/********************/
 typedef struct {
 GSList *sourcelist;
 GFile *destdir;
 GFile *curfile, *curdest;
 FilebrowserBackend *fbback;
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
    filebrowser_backend_refresh_folder (cf->fbback);
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

void filebrowser_backend_copy_uris_async(FilebrowserBackend *filebackend, GFile *destdir, GSList *sources) {
    Tcopyfile *cf;
    GSList *tmplist;
    cf = g_slice_new0(Tcopyfile);
    cf->destdir = destdir;
    g_object_ref(cf->destdir);
    cf->sourcelist = g_slist_copy(sources);
    cf->fbback=filebackend;
    tmplist = cf->sourcelist;
    while (tmplist) {
      g_object_ref(tmplist->data);
      tmplist = tmplist->next;
    }
    copy_uris_process_queue(cf);
  }

void filebrowser_backend_copy_files_async(FilebrowserBackend *filebackend, GFile *destdir, gchar *sources) {
  Tcopyfile *cf;
  gchar **splitted, **tmp;
  cf = g_slice_new0(Tcopyfile);
  cf->destdir = destdir;
  cf->fbback=filebackend;
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

gboolean filebrowser_backend_process_drag_drop(FilebrowserBackend *filebackend, gchar *stringdata)
{
  FilebrowserBackendDetails *directory;
  directory = FILEBROWSER_BACKEND_GET_PRIVATE(filebackend);
  GFile *destdir = get_gfile_from_filename (directory->current_folder);
  g_object_ref(destdir);

  if (destdir) {
    if (strchr(stringdata, '\n') == NULL) {  /* no newlines, probably a single file */
      GSList *list = NULL;
      GFile *uri;
      uri = g_file_new_for_commandline_arg(stringdata);
      list = g_slist_append(list, uri);
      filebrowser_backend_copy_uris_async(filebackend, destdir, list);
      g_slist_free(list);
      g_object_unref(uri);
    } else {
      /* there are newlines, probably this is a list of uri's */
      filebrowser_backend_copy_files_async(filebackend,destdir, stringdata);
    }
    g_object_unref(destdir);
    return TRUE;
  } else {
    return FALSE;
  }
}
