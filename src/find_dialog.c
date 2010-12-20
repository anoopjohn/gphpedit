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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "main_window.h"
#include "gphpedit-statusbar.h"
#include "history-entry.h"

#include "find_dialog.h"

#define SEARCH_DIALOG_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
						GOBJECT_TYPE_SEARCH_DIALOG,              \
						SearchDialogPrivate))

struct _SearchDialogPrivate 
{
  GtkWidget *diagbox;
  GtkWidget *findentry;
  GtkWidget *checkcase;
  GtkWidget *checkwholeword;
  // TODO - Rename this to wrap_around
  GtkWidget *checkwholedoc;
  GtkWidget *checkregex;

  GtkWidget *close_button;
  GtkWidget *find_button;
};

G_DEFINE_TYPE(SearchDialog, SEARCH_DIALOG, GTK_TYPE_DIALOG)

static void
SEARCH_DIALOG_class_init (SearchDialogClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (SearchDialogPrivate));
}


void find_action(SearchDialogPrivate *priv)
{
  const gchar *text;
  Documentable *doc = DOCUMENTABLE(document_manager_get_current_document(main_window.docmg));
  text = gtk_combo_box_get_active_text (GTK_COMBO_BOX(priv->findentry));
  gphpedit_history_entry_prepend_text	(GPHPEDIT_HISTORY_ENTRY(priv->findentry), text);
  gboolean checkwholedoc = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->checkwholedoc));
  gboolean checkcase = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->checkcase));
  gboolean checkwholeword = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->checkwholeword));
  gboolean checkregex = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->checkregex));
  gboolean found = documentable_search_text(doc, text, checkwholedoc, checkcase, checkwholeword, checkregex);

  if (!found) {
    if (checkwholedoc) {
      documentable_goto_pos(doc, 0);
      found = documentable_search_text(doc, text, checkwholedoc, checkcase, checkwholeword, checkregex);
      gphpedit_statusbar_flash_message (GPHPEDIT_STATUSBAR(main_window.appbar),0, "%s", _("Resuming search from top."));
    }
  }
  if (!found) {
    // Show message saying could not be found.
    gphpedit_statusbar_flash_message (GPHPEDIT_STATUSBAR(main_window.appbar),0, _("The text \"%s\" was not found."), text);
  }  
}

void search_dialog_process_response (GtkDialog *dialog, gint response_id, gpointer   user_data)
{
 if (response_id==GTK_RESPONSE_DELETE_EVENT){
    gtk_widget_destroy(GTK_WIDGET(dialog));
 } else {
    find_action(user_data);
 }
}

static void
SEARCH_DIALOG_init (SearchDialog *dialog)
{
  SearchDialogPrivate *priv = SEARCH_DIALOG_GET_PRIVATE(dialog);
  priv->diagbox = gtk_dialog_get_content_area (GTK_DIALOG(dialog));
  gtk_box_set_spacing (GTK_BOX(priv->diagbox), 5);
  GtkWidget *box = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(box);
  GtkWidget *findlabel = gtk_label_new(_("Find"));
  gtk_label_set_justify(GTK_LABEL(findlabel), GTK_JUSTIFY_LEFT);
  gtk_widget_show(findlabel);

  gtk_box_pack_start(GTK_BOX(box), findlabel, FALSE, FALSE, 0);

  priv->findentry = gphpedit_history_entry_new ("search-for-entry", TRUE);
	gtk_widget_set_size_request (priv->findentry, 300, -1);
  // Set enter key in the text box to activate find
  gtk_entry_set_activates_default (GTK_ENTRY (gtk_bin_get_child (GTK_BIN (priv->findentry))), TRUE);
  gtk_widget_show (priv->findentry);

  gtk_box_pack_start(GTK_BOX(box), priv->findentry, FALSE, FALSE, 6);

  gtk_box_pack_start(GTK_BOX(priv->diagbox), box, FALSE, FALSE, 4);

  /* Get selected text (Wendell) */
  gchar *buffer;

  buffer = documentable_get_current_selected_text(DOCUMENTABLE(document_manager_get_current_document(main_window.docmg)));
  if (buffer) {
      gphpedit_history_entry_prepend_text	(GPHPEDIT_HISTORY_ENTRY(priv->findentry),buffer);
      gtk_combo_box_set_active (GTK_COMBO_BOX(priv->findentry), 0);
  }
  /* End get selected text */

  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  gtk_box_pack_start(GTK_BOX(priv->diagbox), hbox, FALSE, FALSE, 0);

  priv->checkcase = gtk_check_button_new_with_mnemonic (_("_Match case"));
  gtk_widget_show (priv->checkcase);
  gtk_box_pack_start (GTK_BOX (hbox), priv->checkcase, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkcase), FALSE);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start(GTK_BOX(priv->diagbox), hbox, FALSE, FALSE, 0);

  priv->checkwholeword = gtk_check_button_new_with_mnemonic (_("Match _entire word only"));
  gtk_widget_show (priv->checkwholeword);
  gtk_box_pack_start (GTK_BOX (hbox), priv->checkwholeword, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkwholeword), FALSE);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start(GTK_BOX(priv->diagbox), hbox, FALSE, FALSE, 0);

  priv->checkwholedoc = gtk_check_button_new_with_mnemonic (_("Wrap around"));
  gtk_widget_show (priv->checkwholedoc);
  gtk_box_pack_start (GTK_BOX (hbox), priv->checkwholedoc, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkwholedoc), TRUE);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start(GTK_BOX(priv->diagbox), hbox, FALSE, FALSE, 0);

  priv->checkregex = gtk_check_button_new_with_mnemonic (_("RegExp"));
  gtk_widget_show (priv->checkregex);
  gtk_box_pack_start (GTK_BOX (hbox), priv->checkregex, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkregex), FALSE);

  priv->close_button = gtk_dialog_add_button (GTK_DIALOG(dialog), GTK_STOCK_CLOSE, GTK_RESPONSE_DELETE_EVENT);
  priv->find_button = gtk_dialog_add_button (GTK_DIALOG(dialog), GTK_STOCK_FIND, GTK_RESPONSE_OK);
  
  gtk_dialog_set_default_response (GTK_DIALOG(dialog), GTK_RESPONSE_OK);

  g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(search_dialog_process_response), priv);
}

GtkWidget *
search_dialog_new (GtkWindow *parent)
{
	SearchDialog *dialog;

	dialog = g_object_new (GOBJECT_TYPE_SEARCH_DIALOG, "has-separator", FALSE, NULL);

	if (parent != NULL)
	{
		gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);
	
		gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
	}

  gtk_window_set_position (GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_title (GTK_WINDOW (dialog), _("Find"));
  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (dialog), 10);

	return GTK_WIDGET (dialog);
}

