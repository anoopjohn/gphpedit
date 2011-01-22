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
#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <gtk/gtk.h>
#include "documentable.h"
#include "document_scintilla.h"
#include "document_webkit.h"

#define DOCUMENT_TYPE document_get_type()
#define DOCUMENT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), DOCUMENT_TYPE, Document))
#define DOCUMENT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), DOCUMENT_TYPE, DocumentClass))
#define OBJECT_IS_DOCUMENT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DOCUMENT_TYPE))
#define OBJECT_IS_DOCUMENT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOCUMENT_TYPE))
#define DOCUMENT_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), DOCUMENT_TYPE, DocumentClass))

typedef struct DocumentDetails DocumentDetails;

typedef struct
{
	GObject object;
	DocumentDetails *details;
} Document;

typedef struct
{
	GObjectClass parent_class;

} DocumentClass;

/* document types enum */

enum {
TAB_FILE,
TAB_PHP,
TAB_CSS,
TAB_SQL,
TAB_HELP,
TAB_CXX,
TAB_PERL,
TAB_PYTHON,
TAB_COBOL,
TAB_PREVIEW
};

/* Basic GObject requirements. */
GType document_get_type (void);
#endif /* DOCUMENT_H */

