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

#ifndef FILEBROWSER_BACKEND_H
#define FILEBROWSER_BACKEND_H

#include <gtk/gtk.h>
#include <gio/gio.h>
#include "document_manager.h"

#define FILEBROWSER_TYPE_BACKEND filebrowser_backend_get_type()
#define FILEBROWSER_BACKEND(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), FILEBROWSER_TYPE_BACKEND, FilebrowserBackend))
#define FILEBROWSER_BACKEND_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), FILEBROWSER_TYPE_BACKEND, FilebrowserBackendClass))
#define FILEBROWSER_IS_BACKEND(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FILEBROWSER_TYPE_BACKEND))
#define FILEBROWSER_IS_BACKEND_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), FILEBROWSER_TYPE_BACKEND))
#define FILEBROWSER_BACKEND_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), FILEBROWSER_TYPE_BACKEND, FilebrowserBackendClass))

typedef struct {
  GFile *file;
  const gchar *uri;
  const gchar *mime;
  gchar *icon;
  const char *display_name;
} FOLDERFILE;

typedef struct FilebrowserBackendDetails FilebrowserBackendDetails;

typedef struct
{
	GObject object;
	FilebrowserBackendDetails *details;
} FilebrowserBackend;

typedef struct
{
	GObjectClass parent_class;

	/*** Notification signals for clients to connect to. ***/
	void (* done_loading) (FilebrowserBackend *directory, gpointer user_data);
	void (* change_folder) (gpointer instance, const gchar *current_folder, gpointer user_data);
} FilebrowserBackendClass;

/* Basic GObject requirements. */
GType filebrowser_backend_get_type (void);
FilebrowserBackend *filebrowser_backend_new (const gchar *folder);
const gchar *get_filebrowser_backend_current_folder(FilebrowserBackend *fbback);
guint get_filebrowser_backend_number_files(FilebrowserBackend *fbback);
GSList *get_filebrowser_backend_files(FilebrowserBackend *fbback);
void filebrowser_backend_update_folder (FilebrowserBackend *fbback, const gchar *newfolder);
void filebrowser_backend_go_folder_up (FilebrowserBackend *fbback);
void filebrowser_backend_refresh_folder (FilebrowserBackend *fbback);
void filebrowser_backend_go_folder_home (FilebrowserBackend *fbback, gchar *filename);
void filebrowser_backend_cancel (FilebrowserBackend *fbback);
void filebrowser_backend_delete_file(FilebrowserBackend *filebackend, gchar *filename);
void filebrowser_backend_create_dir(FilebrowserBackend *filebackend, gchar *filename, gchar *name, gboolean isdir);
gchar *filebrowser_backend_get_display_name(FilebrowserBackend *filebackend, gchar *filename);
void filebrowser_backend_rename_file(FilebrowserBackend *filebackend,gchar *filename, gchar *current_name, gchar *new_name);
gboolean filebrowser_backend_process_drag_drop(FilebrowserBackend *filebackend, gchar *stringdata);
#endif /* FILEBROWSER_BACKEND_H */

