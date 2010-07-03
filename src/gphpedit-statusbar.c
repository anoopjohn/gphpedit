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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <glib/gi18n.h>
#include "gphpedit-statusbar.h"
#include "status-combo-box.h"
#include "tab.h"
#include "main_window.h"

#define GPHPEDIT_STATUSBAR_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    GPHPEDIT_TYPE_STATUSBAR,\
					    GphpeditStatusbarPrivate))

struct _GphpeditStatusbarPrivate
{
	GtkWidget     *zoom_level;
  GtkWidget     *filetype_menu;

	GtkWidget     *error_frame;
	GtkWidget     *error_event_box;

	/* tmp flash timeout data */
	guint          flash_timeout;
	guint          flash_context_id;
	guint          flash_message_id;
};

G_DEFINE_TYPE(GphpeditStatusbar, gphpedit_statusbar, GTK_TYPE_STATUSBAR)


static void
gphpedit_statusbar_notify (GObject    *object,
			GParamSpec *pspec)
{
	/* don't allow gtk_statusbar_set_has_resize_grip to mess with us.
	 * See _gphpedit_statusbar_set_has_resize_grip for an explanation.
	 */
	if (strcmp (g_param_spec_get_name (pspec), "has-resize-grip") == 0)
	{
		gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (object), FALSE);
		return;
	}

	if (G_OBJECT_CLASS (gphpedit_statusbar_parent_class)->notify)
		G_OBJECT_CLASS (gphpedit_statusbar_parent_class)->notify (object, pspec);
}

static void
gphpedit_statusbar_finalize (GObject *object)
{
	GphpeditStatusbar *statusbar = GPHPEDIT_STATUSBAR (object);

	if (statusbar->priv->flash_timeout > 0)
		g_source_remove (statusbar->priv->flash_timeout);

	G_OBJECT_CLASS (gphpedit_statusbar_parent_class)->finalize (object);
}

static void
gphpedit_statusbar_class_init (GphpeditStatusbarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->notify = gphpedit_statusbar_notify;
	object_class->finalize = gphpedit_statusbar_finalize;

	g_type_class_add_private (object_class, sizeof (GphpeditStatusbarPrivate));
}

#define RESIZE_GRIP_EXTRA_WIDTH 30

static void
set_statusbar_width_chars (GtkWidget *statusbar,
			   gint       n_chars,
			   gboolean   has_resize_grip)
{
	PangoContext *context;
	PangoFontMetrics *metrics;
	gint char_width, digit_width, width;
	GtkStyle *style;

	context = gtk_widget_get_pango_context (statusbar);
	style = gtk_widget_get_style (GTK_WIDGET (statusbar));
	metrics = pango_context_get_metrics (context,
					     style->font_desc,
					     pango_context_get_language (context));

	char_width = pango_font_metrics_get_approximate_digit_width (metrics);
	digit_width = pango_font_metrics_get_approximate_char_width (metrics);

	width = PANGO_PIXELS (MAX (char_width, digit_width) * n_chars);

	pango_font_metrics_unref (metrics);

	/* If there is a resize grip, allocate some extra width.
	 * It would be nice to calculate the exact size programmatically
	 * but I could not find out how to do it */
	if (has_resize_grip)
		width += RESIZE_GRIP_EXTRA_WIDTH;

	gtk_widget_set_size_request (statusbar, width, -1);
}

void set_higthlight (GphpeditStatusComboBox *combo, GtkMenuItem *item){
  const gchar *label =gtk_menu_item_get_label (item);
  if (g_strcmp0(label,_("Cobol"))==0){
    if (main_window.current_editor && GTK_IS_SCINTILLA(main_window.current_editor->scintilla)) {
      set_editor_to_cobol(main_window.current_editor);
    }
  } else if (g_strcmp0(label,_("C/C++"))==0){
    if (main_window.current_editor && GTK_IS_SCINTILLA(main_window.current_editor->scintilla)) {
      set_editor_to_cxx(main_window.current_editor);
    }
  } else if (g_strcmp0(label,_("CSS"))==0){
    if (main_window.current_editor && GTK_IS_SCINTILLA(main_window.current_editor->scintilla)) {
      set_editor_to_css(main_window.current_editor);
    }
  } else if (g_strcmp0(label,_("PHP/HTML/XML"))==0){
      if (main_window.current_editor && GTK_IS_SCINTILLA(main_window.current_editor->scintilla)) {
      set_editor_to_php(main_window.current_editor);
      }
  } else if (g_strcmp0(label,_("Perl"))==0){
    if (main_window.current_editor && GTK_IS_SCINTILLA(main_window.current_editor->scintilla)) {
      set_editor_to_perl(main_window.current_editor);
    }
  } else if (g_strcmp0(label,_("SQL"))==0){
    if (main_window.current_editor && GTK_IS_SCINTILLA(main_window.current_editor->scintilla)) {
      set_editor_to_sql(main_window.current_editor);
    }
  } else if (g_strcmp0(label,_("Python"))==0){
    if (main_window.current_editor && GTK_IS_SCINTILLA(main_window.current_editor->scintilla)) {
      set_editor_to_python(main_window.current_editor);
    }
  } else {
    if (GTK_IS_SCINTILLA(main_window.current_editor->scintilla)) {
    set_editor_to_text_plain (main_window.current_editor);
    }
  }
/* seleccionar el tipo de resaltado de acuerdo a la etiqueta del item */
}

void set_status_combo_item (GphpeditStatusbar *statusbar,const gchar *label)
{
  GList *items = gphpedit_status_combo_box_get_items (GPHPEDIT_STATUS_COMBO_BOX(statusbar->priv->filetype_menu));
  GList *walk;
  for (walk = items; walk != NULL; walk = g_list_next (walk)) {
    const gchar *lbl = gphpedit_status_combo_box_get_item_text 	(GPHPEDIT_STATUS_COMBO_BOX(statusbar->priv->filetype_menu), (GtkMenuItem *)walk->data);
    if (g_strcmp0(label,lbl)==0) {
        gphpedit_status_combo_box_set_item (GPHPEDIT_STATUS_COMBO_BOX(statusbar->priv->filetype_menu),(GtkMenuItem *)walk->data);
        break;
    }
  }
  g_list_free (items);
}
static void fill_combo_box(GphpeditStatusComboBox 	*combo)
{
  GtkMenuItem *item;
  item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("Cobol")));
  gtk_widget_show (GTK_WIDGET(item));
  gphpedit_status_combo_box_add_item (combo, item, _("Cobol"));
  item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("C/C++")));
  gtk_widget_show (GTK_WIDGET(item));
  gphpedit_status_combo_box_add_item (combo, item, _("C/C++"));
  item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("CSS")));
  gtk_widget_show (GTK_WIDGET(item));
  gphpedit_status_combo_box_add_item (combo, item, _("CSS"));
  item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("PHP/HTML/XML")));
  gtk_widget_show (GTK_WIDGET(item));
  gphpedit_status_combo_box_add_item (combo, item, _("PHP/HTML/XML"));
  item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("Perl")));
  gtk_widget_show (GTK_WIDGET(item));
  gphpedit_status_combo_box_add_item (combo, item, _("Perl"));
  item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("Python")));
  gtk_widget_show (GTK_WIDGET(item));
  gphpedit_status_combo_box_add_item (combo, item, _("Python"));
  item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("SQL")));
  gtk_widget_show (GTK_WIDGET(item));
  gphpedit_status_combo_box_add_item (combo, item, _("SQL"));
  item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("Text-Plain")));
  gtk_widget_show (GTK_WIDGET(item));
  gphpedit_status_combo_box_add_item (combo, item, _("Text-Plain"));
  
  g_signal_connect (combo, "changed", G_CALLBACK (set_higthlight), NULL);
}
static void
gphpedit_statusbar_init (GphpeditStatusbar *statusbar)
{
	GtkWidget *error_image;

	statusbar->priv = GPHPEDIT_STATUSBAR_GET_PRIVATE (statusbar);

	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (statusbar), FALSE);

	statusbar->priv->zoom_level = gtk_statusbar_new ();
	gtk_widget_show (statusbar->priv->zoom_level);
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (statusbar->priv->zoom_level),
					   FALSE);
	set_statusbar_width_chars (statusbar->priv->zoom_level, 18, FALSE);
	gtk_box_pack_end (GTK_BOX (statusbar),
			  statusbar->priv->zoom_level,
			  FALSE, TRUE, 0);

  statusbar->priv->filetype_menu= gphpedit_status_combo_box_new ("");
  gtk_widget_show (statusbar->priv->filetype_menu);
  gtk_box_pack_end (GTK_BOX (statusbar), statusbar->priv->filetype_menu, FALSE, TRUE, 0);
  fill_combo_box(GPHPEDIT_STATUS_COMBO_BOX(statusbar->priv->filetype_menu));

	statusbar->priv->error_frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (statusbar->priv->error_frame), GTK_SHADOW_IN);

	error_image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_ERROR, GTK_ICON_SIZE_MENU);
	gtk_misc_set_padding (GTK_MISC (error_image), 4, 0);
	gtk_widget_show (error_image);

	statusbar->priv->error_event_box = gtk_event_box_new ();
	gtk_event_box_set_visible_window  (GTK_EVENT_BOX (statusbar->priv->error_event_box),
					   FALSE);
	gtk_widget_show (statusbar->priv->error_event_box);

	gtk_container_add (GTK_CONTAINER (statusbar->priv->error_frame),
			   statusbar->priv->error_event_box);
	gtk_container_add (GTK_CONTAINER (statusbar->priv->error_event_box),
			   error_image);

	gtk_box_pack_start (GTK_BOX (statusbar),
			    statusbar->priv->error_frame,
			    FALSE, TRUE, 0);

	gtk_box_reorder_child (GTK_BOX (statusbar),
			       statusbar->priv->error_frame,
			       0);
}

/**
 * gphpedit_statusbar_new:
 *
 * Creates a new #GphpeditStatusbar.
 *
 * Return value: the new #GphpeditStatusbar object
 **/
GtkWidget *
gphpedit_statusbar_new (void)
{
	return gtk_widget_new (GPHPEDIT_TYPE_STATUSBAR, NULL);
}

/**
 * gphpedit_statusbar_set_zoom_level:
 * @statusbar: an #GphpeditStatusbar
 * @zoom: current zoom level
 *
 * Sets the zoom level on the statusbar.
 **/
void
gphpedit_statusbar_set_zoom_level (GphpeditStatusbar *statusbar,
				     gint            level)
{
	gchar *msg;

	g_return_if_fail (GPHPEDIT_IS_STATUSBAR (statusbar));

	gtk_statusbar_pop (GTK_STATUSBAR (statusbar->priv->zoom_level), 0);

	if (level==-1)
		return;

	msg = g_strdup_printf (_("Zoom: %d%s"), level,"%");

	gtk_statusbar_push (GTK_STATUSBAR (statusbar->priv->zoom_level), 0, msg);

  g_free (msg);
}

static gboolean
remove_message_timeout (GphpeditStatusbar *statusbar)
{
	gtk_statusbar_remove (GTK_STATUSBAR (statusbar),
			      statusbar->priv->flash_context_id,
			      statusbar->priv->flash_message_id);

	/* remove the timeout */
	statusbar->priv->flash_timeout = 0;
  	return FALSE;
}

/**
 * gphpedit_statusbar_flash_message:
 * @statusbar: a #GphpeditStatusbar
 * @context_id: message context_id
 * @format: message to flash on the statusbar
 *
 * Flash a temporary message on the statusbar.
 */
void
gphpedit_statusbar_flash_message (GphpeditStatusbar *statusbar,
			       guint           context_id,
			       const gchar    *format, ...)
{
	const guint32 flash_length = 3000; /* three seconds */
	va_list args;
	gchar *msg;

	g_return_if_fail (GPHPEDIT_IS_STATUSBAR (statusbar));
	g_return_if_fail (format != NULL);

	va_start (args, format);
	msg = g_strdup_vprintf (format, args);
	va_end (args);

	/* remove a currently ongoing flash message */
	if (statusbar->priv->flash_timeout > 0)
	{
		g_source_remove (statusbar->priv->flash_timeout);
		statusbar->priv->flash_timeout = 0;

		gtk_statusbar_remove (GTK_STATUSBAR (statusbar),
				      statusbar->priv->flash_context_id,
				      statusbar->priv->flash_message_id);
	}

	statusbar->priv->flash_context_id = context_id;
	statusbar->priv->flash_message_id = gtk_statusbar_push (GTK_STATUSBAR (statusbar),
								context_id,
								msg);

	statusbar->priv->flash_timeout = g_timeout_add (flash_length,
							(GtkFunction) remove_message_timeout,
							statusbar);

	g_free (msg);
}
