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


#ifndef __SEARCH_DIALOG_H__
#define __SEARCH_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GOBJECT_TYPE_SEARCH_DIALOG              (SEARCH_DIALOG_get_type())
#define SEARCH_DIALOG(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GOBJECT_TYPE_SEARCH_DIALOG, SearchDialog))
#define SEARCH_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GOBJECT_TYPE_SEARCH_DIALOG, SearchDialogClass))
#define OBJECT_IS_SEARCH_DIALOG(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GOBJECT_TYPE_SEARCH_DIALOG))
#define OBJECT_IS_SEARCH_DIALOG_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GOBJECT_TYPE_SEARCH_DIALOG))
#define SEARCH_DIALOG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GOBJECT_TYPE_SEARCH_DIALOG, SearchDialogClass))

/* Private structure type */
typedef struct _SearchDialogPrivate SearchDialogPrivate;

/*
 * Main object structure
 */
typedef struct _SearchDialog SearchDialog;

struct _SearchDialog 
{
	GtkDialog dialog;

	/*< private > */
	SearchDialogPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _SearchDialogClass SearchDialogClass;

struct _SearchDialogClass 
{
	GtkDialogClass parent_class;
	
};

/*
 * Public methods
 */
GType 		 SEARCH_DIALOG_get_type 		(void) G_GNUC_CONST;

GtkWidget *search_dialog_new (GtkWindow *parent);
   
G_END_DECLS

#endif  /* __SEARCH_DIALOG_H__  */
