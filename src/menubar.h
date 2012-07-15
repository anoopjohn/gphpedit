/* This file is part of gPHPEdit, a GNOME2 PHP Editor.
 
   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
    
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


#ifndef __MENUBAR_H__
#define __MENUBAR_H__

#include <gtk/gtk.h>
#include "main_window.h"

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GOBJECT_TYPE_MENUBAR              (MENUBAR_get_type())
#define MENUBAR(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GOBJECT_TYPE_MENUBAR, MenuBar))
#define MENUBAR_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GOBJECT_TYPE_MENUBAR, MenuBarClass))
#define OBJECT_IS_MENUBAR(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GOBJECT_TYPE_MENUBAR))
#define OBJECT_IS_MENUBAR_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GOBJECT_TYPE_MENUBAR))
#define MENUBAR_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GOBJECT_TYPE_MENUBAR, MenuBarClass))

/* Private structure type */
typedef struct _MenuBarPrivate MenuBarPrivate;

/*
 * Main object structure
 */
typedef struct _MenuBar MenuBar;

struct _MenuBar 
{
	GtkMenuBar dialog;

	/*< private > */
	MenuBarPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _MenuBarClass MenuBarClass;

struct _MenuBarClass 
{
	GtkMenuBarClass parent_class;
	
};

/*
 * Public methods
 */
GType 		 MENUBAR_get_type 		(void) G_GNUC_CONST;

GtkWidget *menubar_new (gpointer main_window);
void install_menu_hint(GtkWidget *widget, gchar *message, gpointer main_window);
GtkWidget *menubar_get_menu_plugin(MenuBar *menubar);
void menubar_set_classbrowser_status(MenuBar *menubar, gboolean value);
void menubar_update_controls(MenuBar *menubar, gboolean is_scintilla, gboolean can_preview, gboolean isreadonly);
G_END_DECLS

#endif  /* __MENUBAR_H__  */
