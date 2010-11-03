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
#ifndef DOCUMENT_SAVER_H
#define DOCUMENT_SAVER_H

#include <gtk/gtk.h>
#include "document.h"

#define DOCUMENT_SAVER_TYPE document_saver_get_type()
#define DOCUMENT_SAVER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), DOCUMENT_SAVER_TYPE, DocumentSaver))
#define DOCUMENT_SAVER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), DOCUMENT_SAVER_TYPE, DocumentSaverClass))
#define OBJECT_IS_DOCUMENT_SAVER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DOCUMENT_SAVER_TYPE))
#define OBJECT_IS_DOCUMENT_SAVER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOCUMENT_SAVER_TYPE))
#define DOCUMENT_SAVER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), DOCUMENT_SAVER_TYPE, DocumentSaverClass))

typedef struct DocumentSaverDetails DocumentSaverDetails;

typedef struct
{
	GObject object;

	DocumentSaverDetails *details;
} DocumentSaver;

typedef struct
{
	GObjectClass parent_class;

	void (* done_saving) (DocumentSaver *docsav, Document *document, gpointer user_data);
} DocumentSaverClass;


/* Basic GObject requirements. */
GType document_saver_get_type (void);
DocumentSaver *document_saver_new (void);
void document_saver_save_document(DocumentSaver *docsav, Document *doc);
#endif /* DOCUMENT_SAVER_H */

