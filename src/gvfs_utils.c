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
    if (!filename) return NULL;
    GFile *file= get_gfile_from_filename(filename);
    if (!g_file_query_exists (file,NULL)){
     g_object_unref(file);
     return NULL;
    }
    GFile *parent= g_file_get_parent (file);
    gchar *parent_path= g_file_get_uri (parent);
    g_object_unref(file);
    g_object_unref(parent);
    return parent_path;
}
/* 
*
*/
gchar *filename_get_relative_path(gchar *filename){
  if (!filename || strlen(filename)==0) return NULL;
  GFile *file= get_gfile_from_filename(filename);
  GFile *home= get_gfile_from_filename((gchar *) g_get_home_dir());
  gchar *rel =g_file_get_relative_path (home,file);
  g_object_unref(file);
  g_object_unref(home);
  if (rel) {
  gchar *relpath=g_strdup_printf("~/%s",rel);
  g_free(rel);
  return relpath;
  }
  return g_strdup(filename);
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
    if (!filename) return NULL;
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
* get_file_modified
* return TRUE if the file has been modified or FALSE otherwise
* if updatemark = TRUE, the GTimeVal parameter will be updated will lastest mark
*/
gboolean get_file_modified(gchar *filename,GTimeVal *act, gboolean update_mark){
  GFileInfo *info;
  GError *error=NULL;
  GFile *file;
  if (!filename) return FALSE;
  file= get_gfile_from_filename (filename);
  info=g_file_query_info (file,"time::modified,time::modified-usec",0,NULL,&error);
  if (error){
  g_error_free(error);
  g_object_unref(file);
  return FALSE;  
  }
  GTimeVal result;
  g_file_info_get_modification_time (info,&result);
  gboolean hr=FALSE;
  if ((result.tv_sec > act->tv_sec) || (result.tv_sec == act->tv_sec && result.tv_usec > act->tv_usec)) hr=TRUE;
  if (update_mark){
    /*make current mark as file mark*/
    act->tv_sec=result.tv_sec;
    act->tv_usec =result.tv_usec;
  }
  g_object_unref(info);  
  g_object_unref(file);
  return hr;
}

gboolean filename_is_local_or_http(gchar *filename){
  GFile *file;
  if (!filename) return FALSE;
  file= get_gfile_from_filename (filename);
  gchar *scheme= g_file_get_uri_scheme (file);
  gboolean result= g_str_has_prefix(filename, "file://") || g_str_has_prefix(filename, "http://") || g_str_has_prefix(filename, "https://");
  g_free(scheme);
  g_object_unref(file);
  return result;
}

gboolean filename_is_native(gchar *filename)
{
  gboolean result=FALSE;
  gchar *ret=NULL;
  if (!filename) return TRUE;
  ret = filename_get_path(filename);
  if (ret){
    g_free(ret);
    result=TRUE;
  }

  return result;
}

gchar *filename_get_display_name(gchar *filename)
{
  if (!filename) return NULL;
  GFile *fi;
  GError *error=NULL;
  fi=get_gfile_from_filename(filename);
  GFileInfo *info= g_file_query_info (fi, "standard::display-name",G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL,&error);
  if (!info){
    g_print(_("Error renaming file. GIO Error:%s\n"),error->message);
    g_error_free (error);
    return NULL;
  }
  gchar *result = g_strdup((gchar *)g_file_info_get_display_name (info));
  g_object_unref(info);
  g_object_unref(fi);
  g_print("%s (%s)", filename, result);
  return result;
}

gboolean filename_rename(gchar *filename, gchar *new_name)
{
  gboolean result=FALSE;
  if (!filename || !new_name) return result;
  GError *error=NULL;
  GFile *fi= get_gfile_from_filename(filename);
  fi=g_file_set_display_name (fi,new_name,NULL,&error);
  if (error){
    g_print(_("Error renaming file. GIO Error:%s\n"),error->message);
    g_error_free (error);
  } else {
    result=TRUE;
  }
  g_object_unref(fi);
  return result;
}
gboolean filename_delete_file(gchar *filename)
{
  GFile *fi;
  GError *error=NULL;
  fi=get_gfile_from_filename(filename);
  if (!g_file_trash (fi,NULL,&error)){
    if (error->code == G_IO_ERROR_NOT_SUPPORTED){
      if (!g_file_delete (fi,NULL,&error)){
        g_print(_("GIO Error deleting file: %s\n"),error->message);
        g_error_free (error);
        return FALSE;
        }
    } else {
      g_print(_("GIO Error deleting file: %s\n"),error->message);
      g_error_free (error);
      return FALSE;
    }
  }
  g_object_unref (fi);
  return TRUE;
}
/**
* get_absolute_from_relative
* return an absolute uri from a relatice path or NULL. must free the returned value with gfree when no longer needed
* if base = NULL then home dir will be used as base path
*/

gchar *get_absolute_from_relative(gchar *path, gchar *base){
  if (!path) return NULL;
  GFile *parent=NULL;
  if (!base){
    parent= get_gfile_from_filename((gchar *) g_get_home_dir());
  }else{
    parent= get_gfile_from_filename (base);
  }
  GFile *result= g_file_resolve_relative_path (parent,path);
  gchar *file_uri= g_file_get_uri (result);
  g_object_unref(parent);
  g_object_unref(result);
  return file_uri;
}

// function from http://devpinoy.org/blogs/cvega/archive/2006/06/19/xtoi-hex-to-integer-c-function.aspx
// Converts a hexadecimal string to integer
int xtoi(const char* xs, unsigned int* result)
{
  size_t szlen = strlen(xs);
  int i, xv, fact;

 if (szlen > 0)
 {

  // Converting more than 32bit hexadecimal value?
  if (szlen>8) return 2; // exit

  // Begin conversion here
  *result = 0;
  fact = 1;

  // Run until no more character to convert
  for(i=szlen-1; i>=0 ;i--)
  {
   if (g_ascii_isxdigit(*(xs+i)))
   {
    if (*(xs+i)>=97)
    {
     xv = ( *(xs+i) - 97) + 10;
    }
    else if ( *(xs+i) >= 65)
    {
     xv = (*(xs+i) - 65) + 10;
    }
    else
    {
     xv = *(xs+i) - 48;
    }
    *result += (xv * fact);
    fact *= 16;
   }
   else
   {
    // Conversion was abnormally terminated
    // by non hexadecimal digit, hence
    // returning only the converted with
    // an error value 4 (illegal hex character)
    return 4;
   }
  }
 }

 // Nothing to convert
 return 1;
}
/**
 * Replace any %xx escapes by their single-character equivalent.
 */
void unquote(char *s) {
	char *o = s;
	while (*s) {
		if ((*s == '%') && s[1] && s[2]) {
      guint a;
      char xl[3]={*(s+1),*(s+2),0};
      const char *t=xl;
      xtoi(t, &a);
			*o = a;
			s += 2;
		} else {
			*o = *s;
		}
		o++;
		s++;
	}
	*o = '\0';
}

