/* This file is part of gPHPEdit, a GNOME PHP Editor.

   Copyright (C) 2010 José Rostagno

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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>
#include <stdlib.h>

#include "document_manager.h"
#include "preferences_manager.h"
#include "goto_infobar.h"
#include "gphpedit-close-button.h"

#define GOTOINFOBAR_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
						INFOBAR_TYPE_GOTO,              \
						GotoInfobarPrivate))

struct _GotoInfobarPrivate 
{
  GtkWidget *goto_infobar_find;
  GtkWidget *goto_label;
  GtkWidget *goto_entry;
  GtkWidget *cleanimg;
};

G_DEFINE_TYPE(GotoInfobar, goto_infobar, GTK_TYPE_INFO_BAR)

static void goto_infobar_init (GotoInfobar *goto_infobar);
static void goto_infobar_grab_focus(GtkWidget *widget);

static void
goto_infobar_class_init (GotoInfobarClass *klass)
{
  GtkWidgetClass *widget_class;

  widget_class = (GtkWidgetClass*) klass;  
  widget_class->grab_focus = goto_infobar_grab_focus;

	g_type_class_add_private (klass, sizeof (GotoInfobarPrivate));
}

static void goto_infobar_grab_focus(GtkWidget *widget)
{
  GotoInfobarPrivate *priv = GOTOINFOBAR_GET_PRIVATE(widget);

  GTK_WIDGET_CLASS (goto_infobar_parent_class)->grab_focus (widget);
  gtk_widget_grab_focus(GTK_WIDGET(priv->goto_entry));

}
/**
* on_cleanicon_press
* Clear entry text 
*/
static void on_cleanicon_press (GtkEntry *entry, GtkEntryIconPosition icon_pos, GdkEvent *event, gpointer user_data)
{
    gtk_entry_set_text (entry,"");
}

static void on_close_button_activate(GtkWidget *widget, gpointer user_data)
{  
  gtk_widget_hide(user_data);
}

static gboolean goto_key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    if (event->keyval == GDK_KEY_Escape) {
        GtkWidget *document_widget;
        DocumentManager *docmg = document_manager_new ();
        g_object_get(document_manager_get_current_documentable(docmg), "editor_widget", &document_widget, NULL);
        gtk_widget_hide(user_data);
        gtk_widget_grab_focus(document_widget);
        g_object_unref(docmg);
      return TRUE;
    }
  return FALSE;
}

static void goto_line_activate(GtkEntry *entry, gpointer user_data)
{
  gchar *current_text;
  DocumentManager *docmg = document_manager_new ();
  Documentable *doc = document_manager_get_current_documentable(docmg);
  if (doc) {
    current_text = (gchar *)gtk_entry_get_text(entry);
    documentable_goto_line(doc, atoi(current_text));
  }
  g_object_unref(docmg);
}

static gboolean is_valid_digits_only(gchar *text)
{
  guint i;

  for (i=0;i<strlen(text);i++){
    if (!g_ascii_isdigit (*(text + i))) {
      return FALSE;
    }
  }
  return TRUE;
}

/* ensure that only valid text can be inserted into entry */
static void
goto_typed (GtkEditable *editable,
                     const gchar *text,
                     gint         length,
                     gint        *position,
                     gpointer user_data)
{
	if (is_valid_digits_only ((gchar *)text))
	{
		g_signal_handlers_block_by_func (editable, (gpointer) goto_typed, user_data);
		gtk_editable_insert_text (editable, text, length, position);
		g_signal_handlers_unblock_by_func (editable, (gpointer) goto_typed, user_data);
		goto_line_activate(GTK_ENTRY(editable), user_data);
		gtk_widget_grab_focus(GTK_WIDGET(editable));
	}
	else
	{
		gdk_display_beep (gtk_widget_get_display (GTK_WIDGET (editable)));
	}

	g_signal_stop_emission_by_name (editable, "insert-text");
}

static void
goto_infobar_init (GotoInfobar *goto_infobar)
{
  GotoInfobarPrivate *priv = GOTOINFOBAR_GET_PRIVATE(goto_infobar);
	GtkWidget *hbox;
	GtkWidget *label;

  /* set up info bar */
  GtkWidget *content_area = gtk_info_bar_get_content_area(GTK_INFO_BAR (goto_infobar));
  gtk_box_set_spacing(GTK_BOX (content_area), 0);

  hbox = gtk_hbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (content_area), hbox, TRUE, TRUE, 0);

  GtkWidget *close = gphpedit_close_button_new ();
  g_signal_connect(G_OBJECT(close), "clicked", G_CALLBACK(on_close_button_activate), goto_infobar);
  gtk_box_pack_start (GTK_BOX (hbox), close, FALSE, FALSE, 0);
  
  label = gtk_label_new (_("Go to line: "));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  priv->goto_entry = gtk_entry_new();
  gtk_entry_set_max_length (GTK_ENTRY(priv->goto_entry), 8);
  gtk_entry_set_width_chars(GTK_ENTRY(priv->goto_entry), 9);
  gtk_widget_set_tooltip_text (priv->goto_entry, _("Go to line"));
  gtk_entry_set_icon_from_stock (GTK_ENTRY(priv->goto_entry), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);
  g_signal_connect (G_OBJECT (priv->goto_entry), "icon-press", G_CALLBACK (on_cleanicon_press), NULL);
  gtk_widget_show(priv->goto_entry);
  gtk_box_pack_start (GTK_BOX (hbox), priv->goto_entry, FALSE, FALSE, 0);

  g_signal_connect(G_OBJECT(priv->goto_entry), "insert_text", G_CALLBACK(goto_typed), NULL);
  g_signal_connect_after(G_OBJECT(priv->goto_entry), "activate", G_CALLBACK(goto_line_activate), goto_infobar);
  g_signal_connect_after(G_OBJECT(priv->goto_entry), "key_release_event", G_CALLBACK(goto_key_release_event), goto_infobar);

  gtk_widget_show_all (content_area);
}

GtkWidget *
goto_infobar_new (void)
{
  GotoInfobar *goto_infobar = g_object_new (INFOBAR_TYPE_GOTO, "message-type", GTK_MESSAGE_OTHER, NULL);
	return GTK_WIDGET(goto_infobar);
}
