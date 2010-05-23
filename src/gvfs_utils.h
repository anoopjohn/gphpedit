#ifndef GVFS_UTILS
#define GVFS_UTILS
#include <gio/gio.h>

GFile *get_gfile_from_filename(gchar *filename);
gboolean filename_file_exist(gchar *filename);
gchar *read_text_file_sync(gchar *filename);
#endif
