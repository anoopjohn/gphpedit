/* This file is part of gPHPEdit, a GNOME2 PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009, 2010 José Rostagno (for vijona.com.ar) 

   For more information or to find the latest release, visit our 
   website at http://www.gphpedit.org/

   gPHPEdit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   gPHPEdit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with gPHPEdit. If not, see <http://www.gnu.org/licenses/>.

   The GNU General Public License is contained in the file COPYING.
*/
#ifndef DOCUMENT_LOADER_H
#define DOCUMENT_LOADER_H

#include <gtk/gtk.h>
#include "document.h"

#define DOCUMENT_LOADER_TYPE document_loader_get_type()
#define DOCUMENT_LOADER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), DOCUMENT_LOADER_TYPE, DocumentLoader))
#define DOCUMENT_LOADER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), DOCUMENT_LOADER_TYPE, DocumentLoaderClass))
#define OBJECT_IS_DOCUMENT_LOADER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DOCUMENT_LOADER_TYPE))
#define OBJECT_IS_DOCUMENT_LOADER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOCUMENT_LOADER_TYPE))
#define DOCUMENT_LOADER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), DOCUMENT_LOADER_TYPE, DocumentLoaderClass))

typedef struct DocumentLoaderDetails DocumentLoaderDetails;

typedef struct
{
	GObject object;

	DocumentLoaderDetails *details;
} DocumentLoader;

typedef struct
{
	GObjectClass parent_class;

  void (*done_loading) (DocumentLoader *doclod, gboolean result, Document *doc, gpointer user_data);
	void (* done_navigate) (DocumentLoader *doclod, gboolean result, gpointer user_data);
	void (* need_mounting) (DocumentLoader *doclod, gpointer user_data);
} DocumentLoaderClass;


/* Basic GObject requirements. */
GType document_loader_get_type (void);
DocumentLoader *document_loader_new (GtkWindow *dialog_parent_window);
void document_loader_load (DocumentLoader *doclod, gint type, gchar *filename, gint goto_line);
void document_loader_reload_file(DocumentLoader *doclod, Document *doc);
#endif /* DOCUMENT_LOADER_H */

