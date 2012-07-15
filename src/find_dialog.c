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

#include <glib/gi18n.h>

#include "main_window.h"
#include "gphpedit-statusbar.h"
#include "gedit-history-entry.h"

#include "find_dialog.h"

#define SEARCH_DIALOG_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
						GOBJECT_TYPE_SEARCH_DIALOG,              \
						SearchDialogPrivate))

struct _SearchDialogPrivate 
{
  MainWindow *main_window;
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

static void search_dialog_constructed (GObject *object);

G_DEFINE_TYPE(SearchDialog, SEARCH_DIALOG, GTK_TYPE_DIALOG)

enum
{
  PROP_0,
  PROP_MAIN_WINDOW
};

static void
search_dialog_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
  SearchDialogPrivate *priv = SEARCH_DIALOG_GET_PRIVATE(object);

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
search_dialog_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
  SearchDialogPrivate *priv = SEARCH_DIALOG_GET_PRIVATE(object);
  
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

static void
SEARCH_DIALOG_class_init (SearchDialogClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
    object_class->set_property = search_dialog_set_property;
    object_class->get_property = search_dialog_get_property;
    object_class->constructed = search_dialog_constructed;

    g_object_class_install_property (object_class,
                              PROP_MAIN_WINDOW,
                              g_param_spec_pointer ("main_window",
                              NULL, NULL,
                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_type_class_add_private (klass, sizeof (SearchDialogPrivate));
}


void find_action(SearchDialogPrivate *priv)
{
  const gchar *text;
  Documentable *doc = document_manager_get_current_documentable(priv->main_window->docmg);
  text = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(priv->findentry));
  gedit_history_entry_prepend_text	(GEDIT_HISTORY_ENTRY(priv->findentry), text);
  gboolean checkwholedoc = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->checkwholedoc));
  gboolean checkcase = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->checkcase));
  gboolean checkwholeword = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->checkwholeword));
  gboolean checkregex = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->checkregex));
  gboolean found = documentable_search_text(doc, text, checkwholedoc, checkcase, checkwholeword, checkregex);

  if (!found) {
    if (checkwholedoc) {
      documentable_goto_pos(doc, 0);
      found = documentable_search_text(doc, text, checkwholedoc, checkcase, checkwholeword, checkregex);
      gphpedit_statusbar_flash_message (GPHPEDIT_STATUSBAR(priv->main_window->appbar),0, "%s", _("Resuming search from top."));
    }
  }
  if (!found) {
    // Show message saying could not be found.
    gphpedit_statusbar_flash_message (GPHPEDIT_STATUSBAR(priv->main_window->appbar),0, _("The text \"%s\" was not found."), text);
  }  
}

void search_dialog_process_response (GtkDialog *dialog, gint response_id, gpointer user_data)
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

  GtkBuilder *builder = gtk_builder_new ();
  GError *error = NULL;
  guint res = gtk_builder_add_from_file (builder, GPHPEDIT_UI_DIR "/find_dialog.ui", &error);
  if (!res) {
    g_critical ("Unable to load the UI file!");
    g_error_free(error);
    return ;
  }

  GtkWidget *box = GTK_WIDGET(gtk_builder_get_object (builder, "find_dialog_content"));
  gtk_widget_show (box);
  gtk_widget_reparent (box, priv->diagbox);

  GtkWidget *findbox = GTK_WIDGET(gtk_builder_get_object (builder, "findbox"));
  priv->findentry = gedit_history_entry_new ("search-for-entry", TRUE);
  gtk_widget_set_size_request (priv->findentry, 300, -1);
  /* Set enter key in the text box to activate find */
  gtk_entry_set_activates_default (GTK_ENTRY (gtk_bin_get_child (GTK_BIN (priv->findentry))), TRUE);
  gtk_widget_show (priv->findentry);

  gtk_box_pack_start(GTK_BOX(findbox), priv->findentry, FALSE, FALSE, 6);

  priv->checkcase = GTK_WIDGET(gtk_builder_get_object (builder, "match_case_checkbutton"));
  priv->checkwholeword = GTK_WIDGET(gtk_builder_get_object (builder, "entire_word_checkbutton"));
  priv->checkwholedoc = GTK_WIDGET(gtk_builder_get_object (builder, "wrapdoc"));
  priv->checkregex = GTK_WIDGET(gtk_builder_get_object (builder, "regexp"));

  priv->close_button = gtk_dialog_add_button (GTK_DIALOG(dialog), GTK_STOCK_CLOSE, GTK_RESPONSE_DELETE_EVENT);
  priv->find_button = gtk_dialog_add_button (GTK_DIALOG(dialog), GTK_STOCK_FIND, GTK_RESPONSE_OK);
  
  gtk_dialog_set_default_response (GTK_DIALOG(dialog), GTK_RESPONSE_OK);
}

static void search_dialog_constructed (GObject *object)
{
    SearchDialogPrivate *priv = SEARCH_DIALOG_GET_PRIVATE(object);

    /* Get selected text */
    gchar *buffer;
    Documentable *doc = document_manager_get_current_documentable(priv->main_window->docmg);
    buffer = documentable_get_current_selected_text(doc);
    if (buffer) {
        gedit_history_entry_prepend_text (GEDIT_HISTORY_ENTRY(priv->findentry), buffer);
        gtk_combo_box_set_active (GTK_COMBO_BOX(priv->findentry), 0);
    }
    /* End get selected text */

    gtk_window_set_position (GTK_WINDOW(object), GTK_WIN_POS_CENTER);
    gtk_window_set_title (GTK_WINDOW (object), _("Find"));
    gtk_window_set_resizable (GTK_WINDOW (object), FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (object), 10);

    g_signal_connect(object, "response", G_CALLBACK(search_dialog_process_response), priv);
}


GtkWidget *
search_dialog_new (GtkWindow *parent, gpointer main_window)
{
	SearchDialog *dialog;
	dialog = g_object_new (GOBJECT_TYPE_SEARCH_DIALOG, "main_window", main_window, NULL);

	if (parent != NULL)
	{
		gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);
		gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
	}

	return GTK_WIDGET (dialog);
}

