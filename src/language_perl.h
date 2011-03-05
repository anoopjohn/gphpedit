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

#ifndef LANGUAGE_PERL_H
#define LANGUAGE_PERL_H

#include "document_scintilla.h"
#define LANGUAGE_PERL_TYPE language_perl_get_type()
#define LANGUAGE_PERL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), LANGUAGE_PERL_TYPE, Language_PERL))
#define LANGUAGE_PERL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), LANGUAGE_PERL_TYPE, Language_PERLClass))
#define OBJECT_IS_LANGUAGE_PERL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LANGUAGE_PERL_TYPE))
#define OBJECT_IS_LANGUAGE_PERL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), LANGUAGE_PERL_TYPE))
#define LANGUAGE_PERL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), LANGUAGE_PERL_TYPE, Language_PERLClass))

typedef struct Language_PERLDetails Language_PERLDetails;

typedef struct
{
	GObject object;
	Language_PERLDetails *details;
} Language_PERL;

typedef struct
{
	GObjectClass parent_class;
} Language_PERLClass;

/* Basic GObject requirements. */
GType language_perl_get_type (void);
Language_PERL *language_perl_new (Document_Scintilla *doc);
#endif /* LANGUAGE_PERL_H */

