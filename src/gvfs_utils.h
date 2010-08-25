#ifndef GVFS_UTILS
#define GVFS_UTILS
#include <gio/gio.h>

GFile *get_gfile_from_filename(gchar *filename);
gboolean filename_file_exist(gchar *filename);
gchar *read_text_file_sync(gchar *filename);
gchar *filename_parent_uri(gchar *filename);
gchar *filename_get_uri(gchar *filename);
gchar *filename_get_path(gchar *filename);
gchar *filename_get_scaped_path(gchar *filename);
gchar *filename_get_basename (gchar *filename);
gboolean GFile_get_modified(GFile *file,GTimeVal *act, gboolean update_mark);
gboolean get_file_modified(gchar *filename,GTimeVal *act, gboolean update_mark);
gchar *filename_get_relative_path(gchar *filename);
gboolean filename_is_local_or_http(gchar *filename);
gboolean GFile_is_local_or_http(GFile *file);
gboolean filename_is_native(gchar *filename);
gboolean filename_delete_file(gchar *filename);
gchar *get_absolute_from_relative(gchar *path, gchar *base);
gchar *filename_get_display_name(gchar *filename);
gboolean filename_rename(gchar *filename, gchar *new_name);
GString *text_save_as_temp_file(gchar *text);
void release_temp_file (const gchar *filename);
#endif
