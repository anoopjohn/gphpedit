/* This file is part of gPHPEdit, a GNOME PHP Editor.
 
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009, 2011 José Rostagno (for vijona.com.ar)
	  
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

#ifndef LANGUAGE_COBOL_H
#define LANGUAGE_COBOL_H

#include "document_scintilla.h"
#define LANGUAGE_COBOL_TYPE language_cobol_get_type()
#define LANGUAGE_COBOL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), LANGUAGE_COBOL_TYPE, Language_COBOL))
#define LANGUAGE_COBOL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), LANGUAGE_COBOL_TYPE, Language_COBOLClass))
#define OBJECT_IS_LANGUAGE_COBOL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LANGUAGE_COBOL_TYPE))
#define OBJECT_IS_LANGUAGE_COBOL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), LANGUAGE_COBOL_TYPE))
#define LANGUAGE_COBOL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), LANGUAGE_COBOL_TYPE, Language_COBOLClass))

typedef struct Language_COBOLDetails Language_COBOLDetails;

typedef struct
{
	GObject object;
	Language_COBOLDetails *details;
} Language_COBOL;

typedef struct
{
	GObjectClass parent_class;
} Language_COBOLClass;

/* Basic GObject requirements. */
GType language_cobol_get_type (void);
Language_COBOL *language_cobol_new (Document_Scintilla *doc);
#endif /* LANGUAGE_COBOL_H */

