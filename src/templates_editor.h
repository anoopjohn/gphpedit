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

#ifndef __TEMPLATES_EDITOR_H__
#define __TEMPLATES_EDITOR_H__

#include "main_window"

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define OBJECT_TYPE_TEMPLATES_EDITOR              (templates_editor_get_type())
#define TEMPLATES_EDITOR(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), OBJECT_TYPE_TEMPLATES_EDITOR, TemplatesEditor))
#define TEMPLATES_EDITOR_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), OBJECT_TYPE_TEMPLATES_EDITOR, TemplatesEditorClass))
#define OBJECT_IS_TEMPLATES_EDITOR(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), OBJECT_TYPE_TEMPLATES_EDITOR))
#define OBJECT_IS_TEMPLATES_EDITOR_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), OBJECT_TYPE_TEMPLATES_EDITOR))
#define TEMPLATES_EDITOR_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), OBJECT_TYPE_TEMPLATES_EDITOR, TemplatesEditorClass))

/* Private structure type */
typedef struct _TemplatesEditorPrivate TemplatesEditorPrivate;

/*
 * Main object structure
 */
typedef struct _TemplatesEditor TemplatesEditor;

struct _TemplatesEditor 
{
	GtkVBox vbox;

	/*< private > */
	TemplatesEditorPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _TemplatesEditorClass TemplatesEditorClass;

struct _TemplatesEditorClass 
{
	GtkVBoxClass parent_class;
};

/*
 * Public methods
 */
GType		 templates_editor_get_type		(void) G_GNUC_CONST;

GtkWidget	*templates_editor_new		(void);
   
G_END_DECLS

#endif  /* __TEMPLATES_EDITOR_H__  */
