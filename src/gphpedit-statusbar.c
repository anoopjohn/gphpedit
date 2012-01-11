/*
 * gphpedit-statusbar.c
 * This file is part of gedit
 *
 * Copyright (C) 2005 - Paolo Borelli
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

/*
 * Modified by the gedit Team, 2005. See the AUTHORS file for a
 * list of people on the gedit Team.
 * See the ChangeLog files for a list of changes.
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gphpedit-statusbar.h"
#include "gedit-status-combo-box.h"
#include "tab.h"
#include "document.h"
#include "main_window.h"

#define GPHPEDIT_STATUSBAR_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    GPHPEDIT_TYPE_STATUSBAR,\
					    GphpeditStatusbarPrivate))

struct _GphpeditStatusbarPrivate
{
	MainWindow     *main_window;
	GtkWidget     *overwrite_mode_label;
	GtkWidget     *cursor_position_label;

	GtkWidget     *zoom_level;
	GtkWidget     *filetype_menu;

	/* tmp flash timeout data */
	guint          flash_timeout;
	guint          flash_context_id;
	guint          flash_message_id;
};

G_DEFINE_TYPE(GphpeditStatusbar, gphpedit_statusbar, GTK_TYPE_STATUSBAR)

enum
{
  PROP_0,
  PROP_MAIN_WINDOW
};

static void gphpedit_statusbar_constructed (GObject *object);

static void
gphpedit_statusbar_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
  GphpeditStatusbarPrivate *priv = GPHPEDIT_STATUSBAR_GET_PRIVATE(object);

  switch (prop_id)
  {
    case PROP_MAIN_WINDOW:
        priv->main_window = g_value_get_pointer(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gphpedit_statusbar_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
  GphpeditStatusbarPrivate *priv = GPHPEDIT_STATUSBAR_GET_PRIVATE(object);
  
  switch (prop_id)
  {
    case PROP_MAIN_WINDOW:
      g_value_set_pointer (value, priv->main_window);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static gchar *
get_overwrite_mode_string (gboolean overwrite)
{
	/* Use spaces to leave padding proportional to the font size */
	return g_strdup_printf ("  %s  ", overwrite ? _("OVR") : _("INS"));
}

static gint
get_overwrite_mode_length (void)
{
	return 4 + MAX (g_utf8_strlen (_("OVR"), -1), g_utf8_strlen (_("INS"), -1));
}

static void
gphpedit_statusbar_dispose (GObject *object)
{
	GphpeditStatusbar *statusbar = GPHPEDIT_STATUSBAR (object);

	if (statusbar->priv->flash_timeout > 0)
	{
		g_source_remove (statusbar->priv->flash_timeout);
		statusbar->priv->flash_timeout = 0;
	}

	G_OBJECT_CLASS (gphpedit_statusbar_parent_class)->dispose (object);
}

static void
gphpedit_statusbar_class_init (GphpeditStatusbarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = gphpedit_statusbar_dispose;
	object_class->set_property = gphpedit_statusbar_set_property;
	object_class->get_property = gphpedit_statusbar_get_property;
	object_class->constructed = gphpedit_statusbar_constructed;

	g_object_class_install_property (object_class,
                              PROP_MAIN_WINDOW,
                              g_param_spec_pointer ("main_window",
                              NULL, NULL,
                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_type_class_add_private (object_class, sizeof (GphpeditStatusbarPrivate));
}

#define CURSOR_POSITION_LABEL_WIDTH_CHARS 18

void set_higthlight (GeditStatusComboBox *combo, GtkMenuItem *item, gpointer user_data){
  GphpeditStatusbar *statusbar = GPHPEDIT_STATUSBAR (user_data);
  const gchar *label = gtk_menu_item_get_label (item);
  gint type;
  if (g_strcmp0(label,_("Cobol"))==0){
    type = TAB_COBOL;
  } else if (g_strcmp0(label,_("C/C++"))==0){
    type = TAB_CXX;
  } else if (g_strcmp0(label,_("CSS"))==0){
    type = TAB_CSS;
  } else if (g_strcmp0(label,_("PHP/HTML/XML"))==0){
    type = TAB_PHP;
  } else if (g_strcmp0(label,_("Perl"))==0){
    type = TAB_PERL;
  } else if (g_strcmp0(label,_("SQL"))==0){
    type = TAB_SQL;
  } else if (g_strcmp0(label,_("Python"))==0){
    type = TAB_PYTHON;
  } else {
    type = TAB_FILE;
  }
  Documentable *doc = document_manager_get_current_documentable(statusbar->priv->main_window->docmg);
  if (doc) documentable_set_type(doc, type);
}

void set_status_combo_item (GphpeditStatusbar *statusbar,const gchar *label)
{
  GList *items = gedit_status_combo_box_get_items (GEDIT_STATUS_COMBO_BOX(statusbar->priv->filetype_menu));
  GList *walk;
  for (walk = items; walk != NULL; walk = g_list_next (walk)) {
    const gchar *lbl = gedit_status_combo_box_get_item_text(GEDIT_STATUS_COMBO_BOX(statusbar->priv->filetype_menu), (GtkMenuItem *)walk->data);
    if (g_strcmp0(label,lbl)==0) {
        gedit_status_combo_box_set_item (GEDIT_STATUS_COMBO_BOX(statusbar->priv->filetype_menu),(GtkMenuItem *)walk->data);
        break;
    }
  }
  g_list_free (items);
}
static void fill_combo_box(GeditStatusComboBox 	*combo, gpointer user_data)
{
  GtkMenuItem *item;
  item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("Cobol")));
  gtk_widget_show (GTK_WIDGET(item));
  gedit_status_combo_box_add_item (combo, item, _("Cobol"));
  item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("C/C++")));
  gtk_widget_show (GTK_WIDGET(item));
  gedit_status_combo_box_add_item (combo, item, _("C/C++"));
  item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("CSS")));
  gtk_widget_show (GTK_WIDGET(item));
  gedit_status_combo_box_add_item (combo, item, _("CSS"));
  item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("PHP/HTML/XML")));
  gtk_widget_show (GTK_WIDGET(item));
  gedit_status_combo_box_add_item (combo, item, _("PHP/HTML/XML"));
  item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("Perl")));
  gtk_widget_show (GTK_WIDGET(item));
  gedit_status_combo_box_add_item (combo, item, _("Perl"));
  item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("Python")));
  gtk_widget_show (GTK_WIDGET(item));
  gedit_status_combo_box_add_item (combo, item, _("Python"));
  item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("SQL")));
  gtk_widget_show (GTK_WIDGET(item));
  gedit_status_combo_box_add_item (combo, item, _("SQL"));
  item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(_("Text-Plain")));
  gtk_widget_show (GTK_WIDGET(item));
  gedit_status_combo_box_add_item (combo, item, _("Text-Plain"));
  
  g_signal_connect (combo, "changed", G_CALLBACK (set_higthlight), user_data);
}

static void
gphpedit_statusbar_init (GphpeditStatusbar *statusbar)
{
	statusbar->priv = GPHPEDIT_STATUSBAR_GET_PRIVATE (statusbar);

	statusbar->priv->overwrite_mode_label = gtk_label_new (NULL);
	gtk_label_set_width_chars (GTK_LABEL (statusbar->priv->overwrite_mode_label),
				   get_overwrite_mode_length ());
	gtk_widget_show (statusbar->priv->overwrite_mode_label);
	gtk_box_pack_end (GTK_BOX (statusbar),
			  statusbar->priv->overwrite_mode_label,
			  FALSE, TRUE, 0);

	statusbar->priv->cursor_position_label = gtk_label_new (NULL);
	gtk_label_set_width_chars (GTK_LABEL (statusbar->priv->cursor_position_label),
				  CURSOR_POSITION_LABEL_WIDTH_CHARS);
	gtk_widget_show (statusbar->priv->cursor_position_label);
	gtk_box_pack_end (GTK_BOX (statusbar),
			  statusbar->priv->cursor_position_label,
			  FALSE, TRUE, 0);

	statusbar->priv->zoom_level = gtk_label_new (NULL);
	gtk_label_set_width_chars (GTK_LABEL (statusbar->priv->cursor_position_label),
				  12);
	gtk_widget_show (statusbar->priv->zoom_level);
	gtk_box_pack_end (GTK_BOX (statusbar),
			  statusbar->priv->zoom_level,
			  FALSE, TRUE, 0);

	statusbar->priv->filetype_menu= gedit_status_combo_box_new ("");
	gtk_widget_show (statusbar->priv->filetype_menu);
	gtk_box_pack_end (GTK_BOX (statusbar), statusbar->priv->filetype_menu, FALSE, TRUE, 0);
}

static void gphpedit_statusbar_constructed (GObject *object)
{
	GphpeditStatusbar *statusbar = GPHPEDIT_STATUSBAR (object);
	fill_combo_box(GEDIT_STATUS_COMBO_BOX(statusbar->priv->filetype_menu), object);
}
/**
 * gphpedit_statusbar_new:
 *
 * Creates a new #GphpeditStatusbar.
 *
 * Return value: the new #GphpeditStatusbar object
 **/
GtkWidget *
gphpedit_statusbar_new (gpointer main_window)
{
	return GTK_WIDGET (g_object_new (GPHPEDIT_TYPE_STATUSBAR, "main_window", main_window, NULL));
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

	if (level==-1)
		return;

	msg = g_strdup_printf (_("Zoom: %d%s"), level,"%");
	gtk_label_set_text (GTK_LABEL (statusbar->priv->zoom_level), msg);
	g_free (msg);
}

/**
 * gphpedit_statusbar_set_overwrite:
 * @statusbar: a #GphpeditStatusbar
 * @overwrite: if the overwrite mode is set
 *
 * Sets the overwrite mode on the statusbar.
 **/
void
gphpedit_statusbar_set_overwrite (GphpeditStatusbar *statusbar,
                               gboolean        overwrite)
{
	gchar *msg;

	g_return_if_fail (GPHPEDIT_IS_STATUSBAR (statusbar));

	msg = get_overwrite_mode_string (overwrite);

	gtk_label_set_text (GTK_LABEL (statusbar->priv->overwrite_mode_label), msg);

	g_free (msg);
}

void
gphpedit_statusbar_clear_overwrite (GphpeditStatusbar *statusbar)
{
	g_return_if_fail (GPHPEDIT_IS_STATUSBAR (statusbar));

	gtk_label_set_text (GTK_LABEL (statusbar->priv->overwrite_mode_label), NULL);
}

/**
 * gphpedit_statusbar_cursor_position:
 * @statusbar: an #GphpeditStatusbar
 * @line: line position
 * @col: column position
 *
 * Sets the cursor position on the statusbar.
 **/
void
gphpedit_statusbar_set_cursor_position (GphpeditStatusbar *statusbar,
				     gint            line,
				     gint            col)
{
	gchar *msg = NULL;

	g_return_if_fail (GPHPEDIT_IS_STATUSBAR (statusbar));

	if ((line >= 0) || (col >= 0))
	{
		/* Translators: "Ln" is an abbreviation for "Line", Col is an abbreviation for "Column". Please,
		use abbreviations if possible to avoid space problems. */
		msg = g_strdup_printf (_("  Ln %d, Col %d"), line, col);
	}

	gtk_label_set_text (GTK_LABEL (statusbar->priv->cursor_position_label), msg);

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

/* FIXME this is an issue for introspection */
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
							(GSourceFunc) remove_message_timeout,
							statusbar);

	g_free (msg);
}

/* ex:set ts=8 noet: */
