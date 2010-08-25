/*
 * gphpedit-status-combo-box.c
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

#include "status-combo-box.h"

#define COMBO_BOX_TEXT_DATA "GphpeditStatusComboBoxTextData"

#define GPHPEDIT_STATUS_COMBO_BOX_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GPHPEDIT_TYPE_STATUS_COMBO_BOX, GphpeditStatusComboBoxPrivate))

struct _GphpeditStatusComboBoxPrivate
{
	GtkWidget *frame;
	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *item;
	GtkWidget *arrow;
	
	GtkWidget *menu;
	GtkWidget *current_item;
};

/* Signals */
enum
{
	CHANGED,
	NUM_SIGNALS
};

/* Properties */
enum 
{
	PROP_0,
	
	PROP_LABEL
};

static guint signals[NUM_SIGNALS] = { 0 };

G_DEFINE_TYPE(GphpeditStatusComboBox, gphpedit_status_combo_box, GTK_TYPE_EVENT_BOX)

static void
gphpedit_status_combo_box_finalize (GObject *object)
{
	G_OBJECT_CLASS (gphpedit_status_combo_box_parent_class)->finalize (object);
}

static void
gphpedit_status_combo_box_get_property (GObject    *object,
			             guint       prop_id,
			             GValue     *value,
			             GParamSpec *pspec)
{
	GphpeditStatusComboBox *obj = GPHPEDIT_STATUS_COMBO_BOX (object);

	switch (prop_id)
	{
		case PROP_LABEL:
			g_value_set_string (value, gphpedit_status_combo_box_get_label (obj));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gphpedit_status_combo_box_set_property (GObject      *object,
			             guint         prop_id,
			             const GValue *value,
			             GParamSpec   *pspec)
{
	GphpeditStatusComboBox *obj = GPHPEDIT_STATUS_COMBO_BOX (object);

	switch (prop_id)
	{
		case PROP_LABEL:
			gphpedit_status_combo_box_set_label (obj, g_value_get_string (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gphpedit_status_combo_box_changed (GphpeditStatusComboBox *combo,
				GtkMenuItem         *item)
{
	const gchar *text;
	
	text = g_object_get_data (G_OBJECT (item), COMBO_BOX_TEXT_DATA);

	if (text != NULL)
	{
		gtk_label_set_markup (GTK_LABEL (combo->priv->item), text);
		combo->priv->current_item = GTK_WIDGET (item);
	}
}

static void
gphpedit_status_combo_box_class_init (GphpeditStatusComboBoxClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
	object_class->finalize = gphpedit_status_combo_box_finalize;
	object_class->get_property = gphpedit_status_combo_box_get_property;
	object_class->set_property = gphpedit_status_combo_box_set_property;
	
	klass->changed = gphpedit_status_combo_box_changed;

	signals[CHANGED] =
	    g_signal_new ("changed",
			  G_OBJECT_CLASS_TYPE (object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (GphpeditStatusComboBoxClass,
					   changed), NULL, NULL,
			  g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1,
			  GTK_TYPE_MENU_ITEM);
			  
	g_object_class_install_property (object_class, PROP_LABEL,
					 g_param_spec_string ("label",
					 		      "LABEL",
					 		      "The label",
					 		      NULL,
					 		      G_PARAM_READWRITE));

	/* Set up a style for the button to decrease spacing. */
	gtk_rc_parse_string (
		"style \"gphpedit-status-combo-button-style\"\n"
		"{\n"
		"  GtkWidget::focus-padding = 0\n"
		"  GtkWidget::focus-line-width = 0\n"
		"  xthickness = 0\n"
		"  ythickness = 0\n"
		"}\n"
		"widget \"*.gphpedit-status-combo-button\" style \"gphpedit-status-combo-button-style\"");

	g_type_class_add_private (object_class, sizeof(GphpeditStatusComboBoxPrivate));
}

static void
menu_deactivate (GtkMenu             *menu,
		 GphpeditStatusComboBox *combo)
{
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (combo->priv->button), FALSE);
}

static void
menu_position_func (GtkMenu		*menu,
	            gint		*x,
		    gint		*y,
		    gboolean		*push_in,
		    GphpeditStatusComboBox *combo)
{
	GtkRequisition request;
	
	*push_in = FALSE;
	
	gtk_widget_size_request (gtk_widget_get_toplevel (GTK_WIDGET (menu)), &request);
	
	/* get the origin... */
	gdk_window_get_origin (GTK_WIDGET (combo)->window, x, y);
	
	/* make the menu as wide as the widget */
	if (request.width < GTK_WIDGET (combo)->allocation.width)
	{
		gtk_widget_set_size_request (GTK_WIDGET (menu), GTK_WIDGET (combo)->allocation.width, -1);
	}
	
	/* position it above the widget */
	*y -= request.height;
}

static void
button_press_event (GtkWidget           *widget,
		    GdkEventButton      *event,
		    GphpeditStatusComboBox *combo)
{
	GtkRequisition request;
	gint max_height;
	
	gtk_widget_size_request (combo->priv->menu, &request);

	/* do something relative to our own height here, maybe we can do better */
	max_height = GTK_WIDGET (combo)->allocation.height * 20;
	
	if (request.height > max_height)
	{
		gtk_widget_set_size_request (combo->priv->menu, -1, max_height);
		gtk_widget_set_size_request (gtk_widget_get_toplevel (combo->priv->menu), -1, max_height);
	}
	
	gtk_menu_popup (GTK_MENU (combo->priv->menu), 
			NULL, 
			NULL, 
			(GtkMenuPositionFunc)menu_position_func, 
			combo, 
			event->button, 
			event->time);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (combo->priv->button), TRUE);

	if (combo->priv->current_item)
	{
		gtk_menu_shell_select_item (GTK_MENU_SHELL (combo->priv->menu), 
					    combo->priv->current_item);
	}
}

static void
set_shadow_type (GphpeditStatusComboBox *combo)
{
	GtkShadowType shadow_type;
	GtkWidget *statusbar;

	/* This is a hack needed to use the shadow type of a statusbar */
	statusbar = gtk_statusbar_new ();
	gtk_widget_ensure_style (statusbar);

	gtk_widget_style_get (statusbar, "shadow-type", &shadow_type, NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (combo->priv->frame), shadow_type);

	gtk_widget_destroy (statusbar);
}

static void
gphpedit_status_combo_box_init (GphpeditStatusComboBox *self)
{
	self->priv = GPHPEDIT_STATUS_COMBO_BOX_GET_PRIVATE (self);
	
	gtk_event_box_set_visible_window (GTK_EVENT_BOX (self), TRUE);

	self->priv->frame = gtk_frame_new (NULL);
	gtk_widget_show (self->priv->frame);
	
	self->priv->button = gtk_toggle_button_new ();
	gtk_widget_set_name (self->priv->button, "gphpedit-status-combo-button");
	gtk_button_set_relief (GTK_BUTTON (self->priv->button), GTK_RELIEF_NONE);
	gtk_widget_show (self->priv->button);

	set_shadow_type (self);

	self->priv->hbox = gtk_hbox_new (FALSE, 3);
	gtk_widget_show (self->priv->hbox);
	
	gtk_container_add (GTK_CONTAINER (self), self->priv->frame);
	gtk_container_add (GTK_CONTAINER (self->priv->frame), self->priv->button);
	gtk_container_add (GTK_CONTAINER (self->priv->button), self->priv->hbox);
	
	self->priv->label = gtk_label_new ("");
	gtk_widget_show (self->priv->label);
	
	gtk_label_set_single_line_mode (GTK_LABEL (self->priv->label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (self->priv->label), 0.0, 0.5);
	
	gtk_box_pack_start (GTK_BOX (self->priv->hbox), self->priv->label, FALSE, TRUE, 0);
	
	self->priv->item = gtk_label_new ("");
	gtk_widget_show (self->priv->item);
	
	gtk_label_set_single_line_mode (GTK_LABEL (self->priv->item), TRUE);
	gtk_misc_set_alignment (GTK_MISC (self->priv->item), 0, 0.5);
	
	gtk_box_pack_start (GTK_BOX (self->priv->hbox), self->priv->item, TRUE, TRUE, 0);
	
	self->priv->arrow = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_NONE);
	gtk_widget_show (self->priv->arrow);
	gtk_misc_set_alignment (GTK_MISC (self->priv->arrow), 0.5, 0.5);
	
	gtk_box_pack_start (GTK_BOX (self->priv->hbox), self->priv->arrow, FALSE, TRUE, 0);
	
	self->priv->menu = gtk_menu_new ();
	g_object_ref_sink (self->priv->menu);

	g_signal_connect (self->priv->button, 
			  "button-press-event", 
			  G_CALLBACK (button_press_event), 
			  self);
	g_signal_connect (self->priv->menu,
			  "deactivate",
			  G_CALLBACK (menu_deactivate),
			  self);
}

/* public functions */
GtkWidget *
gphpedit_status_combo_box_new (const gchar *label)
{
	return g_object_new (GPHPEDIT_TYPE_STATUS_COMBO_BOX, "label", label, NULL);
}

void
gphpedit_status_combo_box_set_label (GphpeditStatusComboBox *combo, 
				  const gchar         *label)
{
	gchar *text;

	g_return_if_fail (GPHPEDIT_IS_STATUS_COMBO_BOX (combo));
	
	text = g_strconcat ("  ", label, "", NULL);
	gtk_label_set_markup (GTK_LABEL (combo->priv->label), text);
	g_free (text);
}

const gchar *
gphpedit_status_combo_box_get_label (GphpeditStatusComboBox *combo)
{
	g_return_val_if_fail (GPHPEDIT_IS_STATUS_COMBO_BOX (combo), NULL);

	return gtk_label_get_label (GTK_LABEL (combo->priv->label));
}

static void
item_activated (GtkMenuItem         *item,
		GphpeditStatusComboBox *combo)
{
	gphpedit_status_combo_box_set_item (combo, item);

}

void
gphpedit_status_combo_box_add_item (GphpeditStatusComboBox *combo,
				 GtkMenuItem         *item,
				 const gchar         *text)
{
	g_return_if_fail (GPHPEDIT_IS_STATUS_COMBO_BOX (combo));
	g_return_if_fail (GTK_IS_MENU_ITEM (item));

	gtk_menu_shell_append (GTK_MENU_SHELL (combo->priv->menu), GTK_WIDGET (item));
	
	gphpedit_status_combo_box_set_item_text (combo, item, text);
	g_signal_connect (item, "activate", G_CALLBACK (item_activated), combo);
}

void
gphpedit_status_combo_box_remove_item (GphpeditStatusComboBox *combo,
				    GtkMenuItem         *item)
{
	g_return_if_fail (GPHPEDIT_IS_STATUS_COMBO_BOX (combo));
	g_return_if_fail (GTK_IS_MENU_ITEM (item));

	gtk_container_remove (GTK_CONTAINER (combo->priv->menu),
			      GTK_WIDGET (item));
}

GList *
gphpedit_status_combo_box_get_items (GphpeditStatusComboBox *combo)
{
	g_return_val_if_fail (GPHPEDIT_IS_STATUS_COMBO_BOX (combo), NULL);

	return gtk_container_get_children (GTK_CONTAINER (combo->priv->menu));
}

const gchar *
gphpedit_status_combo_box_get_item_text (GphpeditStatusComboBox *combo,
				      GtkMenuItem	  *item)
{
	const gchar *ret = NULL;
	
	g_return_val_if_fail (GPHPEDIT_IS_STATUS_COMBO_BOX (combo), NULL);
	g_return_val_if_fail (GTK_IS_MENU_ITEM (item), NULL);
	
	ret = g_object_get_data (G_OBJECT (item), COMBO_BOX_TEXT_DATA);
	
	return ret;
}

void 
gphpedit_status_combo_box_set_item_text (GphpeditStatusComboBox *combo,
				      GtkMenuItem	  *item,
				      const gchar         *text)
{
	g_return_if_fail (GPHPEDIT_IS_STATUS_COMBO_BOX (combo));
	g_return_if_fail (GTK_IS_MENU_ITEM (item));

	g_object_set_data_full (G_OBJECT (item), 
				COMBO_BOX_TEXT_DATA,
				g_strdup (text),
				(GDestroyNotify)g_free);
}

void
gphpedit_status_combo_box_set_item (GphpeditStatusComboBox *combo,
				 GtkMenuItem         *item)
{
	g_return_if_fail (GPHPEDIT_IS_STATUS_COMBO_BOX (combo));
	g_return_if_fail (GTK_IS_MENU_ITEM (item));

	g_signal_emit (combo, signals[CHANGED], 0, item, NULL);
}

GtkLabel *
gphpedit_status_combo_box_get_item_label (GphpeditStatusComboBox *combo)
{
	g_return_val_if_fail (GPHPEDIT_IS_STATUS_COMBO_BOX (combo), NULL);
	
	return GTK_LABEL (combo->priv->item);
}

