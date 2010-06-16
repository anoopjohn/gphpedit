#ifndef GVFS_UTILS
#define GVFS_UTILS
#include <gio/gio.h>

GFile *get_gfile_from_filename(gchar *filename);
gboolean filename_file_exist(gchar *filename);
gchar *read_text_file_sync(gchar *filename);
gchar *filename_parent_uri(gchar *filename);
gchar *filename_get_uri(gchar *filename);
gchar *filename_get_path(gchar *filename);
gchar *filename_get_basename (gchar *filename);
gboolean get_file_modified(gchar *filename,GTimeVal *act, gboolean update_mark);
gchar *filename_get_relative_path(gchar *filename);
gboolean filename_is_local_or_http(gchar *filename);
gboolean filename_is_native(gchar *filename);
#endif
