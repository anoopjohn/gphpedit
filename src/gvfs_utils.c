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

