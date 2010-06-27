/*
 * gphpedit-statusbar.h
 * This file was adapted from gedit
 *
 * Copyright (C) 2005 - Paolo Borelli
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

#ifndef GPHPEDIT_STATUSBAR_H
#define GPHPEDIT_STATUSBAR_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GPHPEDIT_TYPE_STATUSBAR		(gphpedit_statusbar_get_type ())
#define GPHPEDIT_STATUSBAR(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), GPHPEDIT_TYPE_STATUSBAR, GphpeditStatusbar))
#define GPHPEDIT_STATUSBAR_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), GPHPEDIT_TYPE_STATUSBAR, GphpeditStatusbarClass))
#define GPHPEDIT_IS_STATUSBAR(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), GPHPEDIT_TYPE_STATUSBAR))
#define GPHPEDIT_IS_STATUSBAR_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), GPHPEDIT_TYPE_STATUSBAR))
#define GPHPEDIT_STATUSBAR_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), GPHPEDIT_TYPE_STATUSBAR, GphpeditStatusbarClass))

typedef struct _GphpeditStatusbar		GphpeditStatusbar;
typedef struct _GphpeditStatusbarPrivate	GphpeditStatusbarPrivate;
typedef struct _GphpeditStatusbarClass	GphpeditStatusbarClass;

struct _GphpeditStatusbar
{
        GtkStatusbar parent;

	/* <private/> */
        GphpeditStatusbarPrivate *priv;
};

struct _GphpeditStatusbarClass
{
        GtkStatusbarClass parent_class;
};

GType		 gphpedit_statusbar_get_type		(void) G_GNUC_CONST;

GtkWidget	*gphpedit_statusbar_new			(void);

void    gphpedit_statusbar_set_zoom_level (GphpeditStatusbar *statusbar,
				     gint            level);

void set_status_combo_item (GphpeditStatusbar *statusbar,const gchar *label);

void		 gphpedit_statusbar_flash_message		(GphpeditStatusbar   *statusbar,
							 guint             context_id,
							 const gchar      *format,
							 ...) G_GNUC_PRINTF(3, 4);

G_END_DECLS

#endif
