/*
 * gphpedit-close-button.c
 *
 * This file was adapted from gedit
 *
 * Copyright (C) 2010 - Paolo Borelli
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

#ifndef __GPHPEDIT_CLOSE_BUTTON_H__
#define __GPHPEDIT_CLOSE_BUTTON_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GPHPEDIT_TYPE_CLOSE_BUTTON			(gphpedit_close_button_get_type ())
#define GPHPEDIT_CLOSE_BUTTON(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GPHPEDIT_TYPE_CLOSE_BUTTON, gphpeditCloseButton))
#define GPHPEDIT_CLOSE_BUTTON_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), gphpedit_TYPE_CLOSE_BUTTON, gphpeditCloseButton const))
#define GPHPEDIT_CLOSE_BUTTON_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), gphpedit_TYPE_CLOSE_BUTTON, gphpeditCloseButtonClass))
#define GPHPEDIT_IS_CLOSE_BUTTON(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), gphpedit_TYPE_CLOSE_BUTTON))
#define GPHPEDIT_IS_CLOSE_BUTTON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), gphpedit_TYPE_CLOSE_BUTTON))
#define GPHPEDIT_CLOSE_BUTTON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), gphpedit_TYPE_CLOSE_BUTTON, gphpeditCloseButtonClass))

typedef struct _gphpeditCloseButton	gphpeditCloseButton;
typedef struct _gphpeditCloseButtonClass	gphpeditCloseButtonClass;
typedef struct _gphpeditCloseButtonPrivate	gphpeditCloseButtonPrivate;

struct _gphpeditCloseButton {
	GtkButton parent;
};

struct _gphpeditCloseButtonClass {
	GtkButtonClass parent_class;
};

GType		  gphpedit_close_button_get_type (void) G_GNUC_CONST;

GtkWidget	 *gphpedit_close_button_new (void);

G_END_DECLS

#endif /* __gphpedit_CLOSE_BUTTON_H__ */
