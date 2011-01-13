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
#ifndef DOCUMENT_MANAGER_H
#define DOCUMENT_MANAGER_H

#include <gtk/gtk.h>
#include "document.h"

#define DOCUMENT_MANAGER_TYPE document_manager_get_type()
#define DOCUMENT_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), DOCUMENT_MANAGER_TYPE, DocumentManager))
#define DOCUMENT_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), DOCUMENT_MANAGER_TYPE, DocumentManagerClass))
#define OBJECT_IS_DOCUMENT_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DOCUMENT_MANAGER_TYPE))
#define OBJECT_IS_DOCUMENT_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOCUMENT_MANAGER_TYPE))
#define DOCUMENT_MANAGER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), DOCUMENT_MANAGER_TYPE, DocumentManagerClass))

typedef struct DocumentManagerDetails DocumentManagerDetails;

typedef struct
{
	GObject object;
	DocumentManagerDetails *details;
} DocumentManager;

typedef struct
{
	GObjectClass parent_class;

  void (* new_document) (DocumentManager *docmg, Documentable *doc, gpointer user_data);

} DocumentManagerClass;

/* Basic GObject requirements. */
GType document_manager_get_type (void);
DocumentManager *document_manager_new (void);
DocumentManager *document_manager_new_full (char **argv, gint argc);
void document_manager_add_new_document(DocumentManager *docmg, gint type, const gchar *filename, gint goto_line);
void document_manager_document_reload(DocumentManager *docmg);
Document *document_manager_find_document_from_widget (DocumentManager *docmg, void *widget);
Document *document_manager_find_document_from_filename (DocumentManager *docmg, gchar *filename);
Document *document_manager_get_current_document (DocumentManager *docmg);
Documentable *document_manager_get_current_documentable (DocumentManager *docmg);
void document_manager_get_context_help(DocumentManager *docmg);
gboolean document_manager_set_current_document_from_widget (DocumentManager *docmg, GtkWidget *child);
void document_manager_get_document_preview(DocumentManager *docmg);
void document_manager_session_save(DocumentManager *docmg);
void document_manager_session_reopen(DocumentManager *docmg);
void document_manager_switch_to_file_or_open(DocumentManager *docmg, gchar *filename, gint line_number);
void document_manager_open_selected(DocumentManager *docmg);
GSList *document_manager_get_document_list (DocumentManager *docmg);
gint document_manager_get_document_count (DocumentManager *docmg);
void document_manager_close_all_tabs(DocumentManager *docmg);
gboolean document_manager_can_all_tabs_be_saved(DocumentManager *docmg);
void document_manager_save_all(DocumentManager *docmg);
gboolean document_manager_set_current_document_from_position(DocumentManager *docmg, gint page_num);
gboolean document_manager_try_save_page(DocumentManager *docmg, Document *document, gboolean close_if_can);
void document_manager_close_page(DocumentManager *docmg, Document *document);
gboolean document_manager_try_close_document(DocumentManager *docmg, Document *document);
gboolean document_manager_try_close_current_document(DocumentManager *docmg);
void document_manager_refresh_properties_all(DocumentManager *docmg);
#endif /* DOCUMENT_MANAGER_H */

