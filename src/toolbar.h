/* This file is part of gPHPEdit, a GNOME PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
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
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with gPHPEdit. If not, see <http://www.gnu.org/licenses/>.

   The GNU General Public License is contained in the file COPYING.
*/


#ifndef __TOOLBAR_H__
#define __TOOLBAR_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GOBJECT_TYPE_TOOLBAR              (TOOLBAR_get_type())
#define TOOLBAR(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GOBJECT_TYPE_TOOLBAR, ToolBar))
#define TOOLBAR_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GOBJECT_TYPE_TOOLBAR, ToolBarClass))
#define OBJECT_IS_TOOLBAR(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GOBJECT_TYPE_TOOLBAR))
#define OBJECT_IS_TOOLBAR_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GOBJECT_TYPE_TOOLBAR))
#define TOOLBAR_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GOBJECT_TYPE_TOOLBAR, ToolBarClass))

/* Private structure type */
typedef struct _ToolBarPrivate ToolBarPrivate;
typedef struct _ToolBar ToolBar;

struct _ToolBar 
{
	GtkBox dialog;
	ToolBarPrivate *priv;
};

typedef struct _ToolBarClass ToolBarClass;
struct _ToolBarClass
{
	GtkBoxClass parent_class;
};

GType 		 TOOLBAR_get_type 		(void) G_GNUC_CONST;
GtkWidget *toolbar_new (void);
void toolbar_update_controls(ToolBar *toolbar, gboolean is_scintilla, gboolean isreadonly);

G_END_DECLS

#endif  /* __TOOLBAR_H__  */
