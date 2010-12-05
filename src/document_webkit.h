/* This file is part of gPHPEdit, a GNOME PHP Editor.

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
#ifndef DOCUMENT_WEBKIT_H
#define DOCUMENT_WEBKIT_H

#include <gtk/gtk.h>

#define DOCUMENT_WEBKIT_TYPE document_webkit_get_type()
#define DOCUMENT_WEBKIT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), DOCUMENT_WEBKIT_TYPE, Document_Webkit))
#define DOCUMENT_WEBKIT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), DOCUMENT_WEBKIT_TYPE, Document_WebkitClass))
#define OBJECT_IS_DOCUMENT_WEBKIT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DOCUMENT_WEBKIT_TYPE))
#define OBJECT_IS_DOCUMENT_WEBKIT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOCUMENT_WEBKIT_TYPE))
#define DOCUMENT_WEBKIT_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), DOCUMENT_WEBKIT_TYPE, Document_WebkitClass))

typedef struct Document_WebkitDetails Document_WebkitDetails;

typedef struct
{
	GObject object;
	Document_WebkitDetails *details;
} Document_Webkit;

typedef struct
{
	GObjectClass parent_class;

	void (* load_complete) (Document_Webkit *doc, gboolean result, gpointer user_data);
	void (* save_update) (Document_Webkit *doc, gpointer user_data); /* emited when document_webkit save state change*/

} Document_WebkitClass;

/* Basic GObject requirements. */
GType document_webkit_get_type (void);
Document_Webkit *document_webkit_new (gint type, GFile *file);
void document_webkit_activate_incremental_search(Document_Webkit *doc);
#endif /* DOCUMENT_WEBKIT_H */

