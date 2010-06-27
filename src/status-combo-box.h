/*
 * gphpedit-status-combo-box.h
 * This file is part of gphpedit
 *
 * Copyright (C) 2008 - Jesse van den Kieboom
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

#ifndef __GPHPEDIT_STATUS_COMBO_BOX_H__
#define __GPHPEDIT_STATUS_COMBO_BOX_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GPHPEDIT_TYPE_STATUS_COMBO_BOX		(gphpedit_status_combo_box_get_type ())
#define GPHPEDIT_STATUS_COMBO_BOX(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GPHPEDIT_TYPE_STATUS_COMBO_BOX, GphpeditStatusComboBox))
#define GPHPEDIT_STATUS_COMBO_BOX_CONST(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GPHPEDIT_TYPE_STATUS_COMBO_BOX, GphpeditStatusComboBox const))
#define GPHPEDIT_STATUS_COMBO_BOX_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GPHPEDIT_TYPE_STATUS_COMBO_BOX, GphpeditStatusComboBoxClass))
#define GPHPEDIT_IS_STATUS_COMBO_BOX(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPHPEDIT_TYPE_STATUS_COMBO_BOX))
#define GPHPEDIT_IS_STATUS_COMBO_BOX_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GPHPEDIT_TYPE_STATUS_COMBO_BOX))
#define GPHPEDIT_STATUS_COMBO_BOX_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GPHPEDIT_TYPE_STATUS_COMBO_BOX, GphpeditStatusComboBoxClass))

typedef struct _GphpeditStatusComboBox		GphpeditStatusComboBox;
typedef struct _GphpeditStatusComboBoxClass	GphpeditStatusComboBoxClass;
typedef struct _GphpeditStatusComboBoxPrivate	GphpeditStatusComboBoxPrivate;

struct _GphpeditStatusComboBox {
	GtkEventBox parent;
	
	GphpeditStatusComboBoxPrivate *priv;
};

struct _GphpeditStatusComboBoxClass {
	GtkEventBoxClass parent_class;
	
	void (*changed) (GphpeditStatusComboBox *combo,
			 GtkMenuItem         *item);
};

GType gphpedit_status_combo_box_get_type 			(void) G_GNUC_CONST;
GtkWidget *gphpedit_status_combo_box_new			(const gchar 		*label);

const gchar *gphpedit_status_combo_box_get_label 		(GphpeditStatusComboBox 	*combo);
void gphpedit_status_combo_box_set_label 			(GphpeditStatusComboBox 	*combo,
							 const gchar         	*label);

void gphpedit_status_combo_box_add_item 			(GphpeditStatusComboBox 	*combo,
							 GtkMenuItem         	*item,
							 const gchar         	*text);
void gphpedit_status_combo_box_remove_item			(GphpeditStatusComboBox    *combo,
							 GtkMenuItem            *item);

GList *gphpedit_status_combo_box_get_items			(GphpeditStatusComboBox    *combo);
const gchar *gphpedit_status_combo_box_get_item_text 	(GphpeditStatusComboBox	*combo,
							 GtkMenuItem		*item);
void gphpedit_status_combo_box_set_item_text 		(GphpeditStatusComboBox	*combo,
							 GtkMenuItem		*item,
							 const gchar            *text);

void gphpedit_status_combo_box_set_item			(GphpeditStatusComboBox	*combo,
							 GtkMenuItem		*item);

GtkLabel *gphpedit_status_combo_box_get_item_label		(GphpeditStatusComboBox	*combo);

G_END_DECLS

#endif /* __GPHPEDIT_STATUS_COMBO_BOX_H__ */
