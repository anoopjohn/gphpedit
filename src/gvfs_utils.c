#include "main.h"
#include <gio/gio.h>
#include "gvfs_utils.h"

GFile *get_gfile_from_filename(gchar *filename){
  GFile *file;
  if (strstr(filename, "://") != NULL) {
    file=g_file_new_for_uri (filename);
  } else {
    file=g_file_new_for_path (filename);  
  }        
  return file;
}

gboolean filename_file_exist(gchar *filename)
{
  if (!filename) return FALSE;
  GFile *file= get_gfile_from_filename(filename);
  gboolean hr= g_file_query_exists (file,NULL);
  g_object_unref(file);
  return hr;
}
/*
* filename_parent_uri
* return a gchar with the parent uri of the Gfile
*/
gchar *filename_parent_uri(gchar *filename){
    GFile *file= get_gfile_from_filename(filename);
    if (!g_file_query_exists (file,NULL)) return NULL;
    GFile *parent= g_file_get_parent (file);
    gchar *parent_path= g_file_get_uri (parent);
    g_object_unref(file);
    g_object_unref(parent);
    return parent_path;
}
/*
* filename_get_basename
* return a gchar with the basename of the Gfile
*/
gchar *filename_get_basename (gchar *filename){
    GFile *file= get_gfile_from_filename(filename);
    gchar *basename= g_file_get_basename (file);
    g_object_unref(file);
    return basename;
}


/*
* filename_get_uri
* return a gchar with the uri of the Gfile
*/
gchar *filename_get_uri(gchar *filename){
    GFile *file= get_gfile_from_filename(filename);
    if (!g_file_query_exists (file,NULL)) return NULL;
    gchar *file_uri= g_file_get_uri (file);
    g_object_unref(file);
    return file_uri;
}
/*
* filename_get_path
* return a gchar with the local pathname for filename, if one exists
*/
gchar *filename_get_path(gchar *filename){
    GFile *file= get_gfile_from_filename(filename);
    gchar *file_path= g_file_get_path (file);
    g_object_unref(file);
    return file_path;
}
/*
* read_text_file_sync
* return a gchar with file contents or NULL in case of error. Must free with g_free when no longer needed
*/
gchar *read_text_file_sync( gchar *filename )
{
  GFile *file;
  GError *error=NULL;
  gchar *buffer=NULL;
  gsize nchars;

  file=get_gfile_from_filename(filename);
  if (!g_file_load_contents (file,NULL,&buffer, &nchars,NULL,&error)){
      g_print(_("Error reading file sync '%s'. GIO error:%s\n"),filename,error->message);
    }
  g_object_unref(file);
//  g_print("buffer:<---\n%s\n--->",buffer);
  return buffer;
}
/*
*
*
*/
gboolean get_file_modified(gchar *filename,GTimeVal *act, gboolean update_mark){
  GFileInfo *info;
  GError *error=NULL;
  GFile *file;
  if (!filename) return FALSE;
  file= get_gfile_from_filename (filename);
  info=g_file_query_info (file,"time::modified,time::modified-usec",0,NULL,&error);
  if (error){
  g_object_unref(file);
  return FALSE;  
  }
  GTimeVal result;
  g_file_info_get_modification_time (info,&result);
  gboolean hr=FALSE;
  if ((result.tv_sec > act->tv_sec) || (result.tv_sec == act->tv_sec && result.tv_usec > act->tv_usec)) hr=TRUE;
  if (update_mark){
    /*make current mark as file mark*/
    act=memcpy (act,&result, sizeof(GTimeVal));
  }
  g_object_unref(info);  
  g_object_unref(file);
  return hr;
}

