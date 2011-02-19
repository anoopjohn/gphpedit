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

#ifndef LANGUAGE_CSS_H
#define LANGUAGE_CSS_H

#include "document_scintilla.h"
#define LANGUAGE_CSS_TYPE language_css_get_type()
#define LANGUAGE_CSS(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), LANGUAGE_CSS_TYPE, Language_CSS))
#define LANGUAGE_CSS_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), LANGUAGE_CSS_TYPE, Language_CSSClass))
#define OBJECT_IS_LANGUAGE_CSS(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LANGUAGE_CSS_TYPE))
#define OBJECT_IS_LANGUAGE_CSS_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), LANGUAGE_CSS_TYPE))
#define LANGUAGE_CSS_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), LANGUAGE_CSS_TYPE, Language_CSSClass))

typedef struct Language_CSSDetails Language_CSSDetails;

typedef struct
{
	GObject object;
	Language_CSSDetails *details;
} Language_CSS;

typedef struct
{
	GObjectClass parent_class;
} Language_CSSClass;

/* Basic GObject requirements. */
GType language_css_get_type (void);
Language_CSS *language_css_new (Document_Scintilla *doc);
#endif /* LANGUAGE_CSS_H */

