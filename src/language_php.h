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

#ifndef LANGUAGE_PHP_H
#define LANGUAGE_PHP_H

#include "document_scintilla.h"
#define LANGUAGE_PHP_TYPE language_php_get_type()
#define LANGUAGE_PHP(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), LANGUAGE_PHP_TYPE, Language_PHP))
#define LANGUAGE_PHP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), LANGUAGE_PHP_TYPE, Language_PHPClass))
#define OBJECT_IS_LANGUAGE_PHP(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LANGUAGE_PHP_TYPE))
#define OBJECT_IS_LANGUAGE_PHP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), LANGUAGE_PHP_TYPE))
#define LANGUAGE_PHP_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), LANGUAGE_PHP_TYPE, Language_PHPClass))

typedef struct Language_PHPDetails Language_PHPDetails;

typedef struct
{
	GObject object;
	Language_PHPDetails *details;
} Language_PHP;

typedef struct
{
	GObjectClass parent_class;
} Language_PHPClass;

/* Basic GObject requirements. */
GType language_php_get_type (void);
Language_PHP *language_php_new (Document_Scintilla *doc);
#endif /* LANGUAGE_PHP_H */

