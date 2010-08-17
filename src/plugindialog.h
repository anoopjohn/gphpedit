/*
 *
 * This file was adapted from gedit
 *
 * Copyright (C) 2002 Paolo Maggi and James Willcox
 * Copyright (C) 2003-2006 Paolo Maggi
 * Copyright (C) 2010 - Jose Rostagno
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA. 
 */

#ifndef __PLUGIN_DIALOG_H__
#define __PLUGIN_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define OBJECT_TYPE_PLUGIN_DIALOG              (plugin_dialog_get_type())
#define PLUGIN_DIALOG(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), OBJECT_TYPE_PLUGIN_DIALOG, PluginDialog))
#define PLUGIN_DIALOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), OBJECT_TYPE_PLUGIN_DIALOG, PluginDialogClass))
#define GEDIT_IS_PLUGIN_MANAGER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), OBJECT_TYPE_PLUGIN_DIALOG))
#define GEDIT_IS_PLUGIN_MANAGER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), OBJECT_TYPE_PLUGIN_DIALOG))
#define PLUGIN_DIALOG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), OBJECT_TYPE_PLUGIN_DIALOG, PluginDialogClass))

/* Private structure type */
typedef struct _PluginDialogPrivate PluginDialogPrivate;

/*
 * Main object structure
 */
typedef struct _PluginDialog PluginDialog;

struct _PluginDialog 
{
	GtkVBox vbox;

	/*< private > */
	PluginDialogPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _PluginDialogClass PluginDialogClass;

struct _PluginDialogClass 
{
	GtkVBoxClass parent_class;
};

/*
 * Public methods
 */
GType		 plugin_dialog_get_type		(void) G_GNUC_CONST;

GtkWidget	*plugin_dialog_new		(void);
   
G_END_DECLS

#endif  /* __PLUGIN_DIALOG_H__  */
