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
#ifndef DOCUMENT_SCINTILLA_H
#define DOCUMENT_SCINTILLA_H

#include <gtk/gtk.h>
#include <gtkscintilla.h>
#include "documentable.h"
#define DOCUMENT_SCINTILLA_TYPE document_scintilla_get_type()
#define DOCUMENT_SCINTILLA(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), DOCUMENT_SCINTILLA_TYPE, Document_Scintilla))
#define DOCUMENT_SCINTILLA_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), DOCUMENT_SCINTILLA_TYPE, Document_ScintillaClass))
#define OBJECT_IS_DOCUMENT_SCINTILLA(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DOCUMENT_SCINTILLA_TYPE))
#define OBJECT_IS_DOCUMENT_SCINTILLA_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOCUMENT_SCINTILLA_TYPE))
#define DOCUMENT_SCINTILLA_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), DOCUMENT_SCINTILLA_TYPE, Document_ScintillaClass))

typedef struct Document_ScintillaDetails Document_ScintillaDetails;

typedef struct
{
	GObject object;
	Document_ScintillaDetails *details;
} Document_Scintilla;

typedef struct
{
	GObjectClass parent_class;

	void (* load_complete) (Document_Scintilla *doc, gboolean result, gpointer user_data);
	void (* save_update) (Document_Scintilla *doc, gpointer user_data); /* emited when document_scintilla save state change*/
	void (* type_changed) (Document_Scintilla *doc, gint type, gpointer user_data);

} Document_ScintillaClass;

/* Basic GObject requirements. */
GType document_scintilla_get_type (void);
Document_Scintilla *document_scintilla_new (gint type, GFile *file, gint goto_line, gchar *contents);
void document_scintilla_refresh_properties(Document_Scintilla *doc);
void document_scintilla_clear_sintax_style(Document_Scintilla *doc);
void document_scintilla_set_sintax_annotation(Document_Scintilla *doc);
void document_scintilla_set_sintax_line(Document_Scintilla *doc, guint current_line_number);
void document_scintilla_add_sintax_annotation(Document_Scintilla *doc, guint current_line_number, gchar *token, gint style);
void document_scintilla_keyboard_macro_startstop(Document_Scintilla *document_scintilla);
void document_scintilla_keyboard_macro_playback(Document_Scintilla *document_scintilla);
void document_scintilla_insert_template(Document_Scintilla *document_scintilla, gchar *template);
#endif /* DOCUMENT_SCINTILLA_H */

