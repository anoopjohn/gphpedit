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

#include "replace_dialog.h"

#define REPLACE_DIALOG_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
						GOBJECT_TYPE_REPLACE_DIALOG,              \
						ReplaceDialogPrivate))

struct _ReplaceDialogPrivate 
{
  GtkWidget *diagbox;
  GtkWidget *findentry;
  GtkWidget *replace_entry;
  GtkWidget *checkcase;
  GtkWidget *checkwholeword;
  GtkWidget *checkwholedoc;
  GtkWidget *checkregex;

  GtkWidget *close_button;
  GtkWidget *find_button;
  GtkWidget *replaceall_button;
};

G_DEFINE_TYPE(ReplaceDialog, REPLACE_DIALOG, GTK_TYPE_DIALOG)

static void
REPLACE_DIALOG_class_init (ReplaceDialogClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (ReplaceDialogPrivate));
}

void replace_all_clicked(ReplaceDialogPrivate *priv)
{
  GString *message;
  const gchar *text;
  const gchar *replace;
  Documentable *doc = document_manager_get_current_documentable(main_window.docmg);

  text = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(priv->findentry));
  gedit_history_entry_prepend_text	(GEDIT_HISTORY_ENTRY(priv->findentry), text);
  replace = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(priv->replace_entry));
  gedit_history_entry_prepend_text	(GEDIT_HISTORY_ENTRY(priv->replace_entry), text);

  gboolean checkwholedoc = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->checkwholedoc));
  if (checkwholedoc) {
    documentable_goto_pos(doc, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(priv->checkwholedoc), FALSE);
  }
  gboolean checkcase = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->checkcase));
  gboolean checkwholeword = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->checkwholeword));
  gboolean checkregex = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->checkregex));
  gint numfound=0;
  while (documentable_search_replace_text(doc, text, replace, checkwholedoc, checkcase, checkwholeword, checkregex, FALSE)){
    numfound++;
  }
  message = g_string_new("");
  if (numfound==0) {
    g_string_printf(message, _("\"%s\" not found, no replacements made."), text);
  }
  else if (numfound==1) {
    g_string_printf(message, _("1 occurence of \"%s\" found, replaced."), text);
  }
  else {
    g_string_printf(message, _("\"%d\" occurences of \"%s\" found, all replaced."), numfound, text);
  }

  gphpedit_statusbar_flash_message (GPHPEDIT_STATUSBAR(main_window.appbar), 0,"%s",message->str);
  documentable_goto_pos(doc, documentable_get_current_position(doc));
}

void replace_clicked(GtkDialog *dialog, ReplaceDialogPrivate *priv)
{
  GString *message;
  const gchar *text;
  const gchar *replace;
  Documentable *doc = document_manager_get_current_documentable(main_window.docmg);

  text = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(priv->findentry));
  gedit_history_entry_prepend_text	(GEDIT_HISTORY_ENTRY(priv->findentry), text);
  replace = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(priv->replace_entry));
  gedit_history_entry_prepend_text	(GEDIT_HISTORY_ENTRY(priv->replace_entry), text);

  gboolean checkwholedoc = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->checkwholedoc));
  if (checkwholedoc) {
    documentable_goto_pos(DOCUMENTABLE(doc), 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(priv->checkwholedoc), FALSE);
  }
  gboolean checkcase = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->checkcase));
  gboolean checkwholeword = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->checkwholeword));
  gboolean checkregex = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->checkregex));

  gint numfound = 0;
  while (documentable_search_replace_text(doc, text, replace, checkwholedoc, checkcase, checkwholeword, checkregex, TRUE)) {
    numfound++;
  }
  if (numfound==0) {
    message = g_string_new("");
    g_string_printf(message, _("\"%s\" not found, no replacements made."), text);
    gphpedit_statusbar_flash_message (GPHPEDIT_STATUSBAR(main_window.appbar), 0, "%s",message->str);
  }
  documentable_goto_pos(doc, documentable_get_current_position(doc));
}


void replace_dialog_process_response (GtkDialog *dialog, gint response_id, gpointer   user_data)
{
 if (response_id==GTK_RESPONSE_DELETE_EVENT){
    gtk_widget_destroy(GTK_WIDGET(dialog));
 } else if (response_id==GTK_RESPONSE_APPLY){
    replace_all_clicked(user_data);
 } else {
    replace_clicked(dialog, user_data);
  }
}

static void
REPLACE_DIALOG_init (ReplaceDialog *dialog)
{
  ReplaceDialogPrivate *priv = REPLACE_DIALOG_GET_PRIVATE(dialog);
  priv->diagbox = gtk_dialog_get_content_area (GTK_DIALOG(dialog));

  GtkBuilder *builder = gtk_builder_new ();
  GError *error = NULL;
  guint res = gtk_builder_add_from_file (builder, GPHPEDIT_UI_DIR "/replace_dialog.ui", &error);
  if (!res) {
    g_critical ("Unable to load the UI file!");
    g_error_free(error);
    return ;
  }

  GtkWidget *box = GTK_WIDGET(gtk_builder_get_object (builder, "replace_dialog_content"));
  gtk_widget_show (box);
  gtk_widget_reparent (box, priv->diagbox);

  GtkWidget *table = GTK_WIDGET(gtk_builder_get_object (builder, "table"));
  gtk_table_set_row_spacings (GTK_TABLE (table), 12);

  priv->findentry = gedit_history_entry_new ("search-for-entry", TRUE);
	gtk_widget_set_size_request (priv->findentry, 200, -1);
  gtk_widget_show (priv->findentry);
  gtk_table_attach_defaults (GTK_TABLE (table), priv->findentry, 1, 2, 0, 1);
  
  /* Get selected text */
  gchar *buffer;
  buffer = documentable_get_current_selected_text(document_manager_get_current_documentable(main_window.docmg));
  if (buffer) {
      gedit_history_entry_prepend_text	(GEDIT_HISTORY_ENTRY(priv->findentry), buffer);
      gtk_combo_box_set_active (GTK_COMBO_BOX(priv->findentry), 0);
  }
  /* End get selected text */

  priv->replace_entry = gedit_history_entry_new ("replace-with-entry", TRUE);
	gtk_widget_set_size_request (priv->replace_entry, 200, -1);
  gtk_widget_show (priv->replace_entry);
  gtk_table_attach_defaults (GTK_TABLE (table), priv->replace_entry, 1, 2, 1, 2);

  priv->checkcase = GTK_WIDGET(gtk_builder_get_object (builder, "match_case_checkbutton"));
  priv->checkwholeword = GTK_WIDGET(gtk_builder_get_object (builder, "entire_word_checkbutton"));
  priv->checkwholedoc = GTK_WIDGET(gtk_builder_get_object (builder, "checkwholedoc"));
  priv->checkregex = GTK_WIDGET(gtk_builder_get_object (builder, "regexp"));

  priv->close_button = gtk_dialog_add_button (GTK_DIALOG(dialog), GTK_STOCK_CLOSE, GTK_RESPONSE_DELETE_EVENT);
  priv->replaceall_button = gtk_dialog_add_button (GTK_DIALOG(dialog), _("Replace _all"), GTK_RESPONSE_APPLY);
  priv->find_button = gtk_dialog_add_button (GTK_DIALOG(dialog),  GTK_STOCK_FIND_AND_REPLACE, GTK_RESPONSE_OK);
  
  gtk_dialog_set_default_response (GTK_DIALOG(dialog), GTK_RESPONSE_OK);

  g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(replace_dialog_process_response), priv);
}

GtkWidget *
replace_dialog_new (GtkWindow *parent)
{
	ReplaceDialog *dialog;

	dialog = g_object_new (GOBJECT_TYPE_REPLACE_DIALOG, "has-separator", FALSE, NULL);

	if (parent != NULL)
	{
		gtk_window_set_transient_for (GTK_WINDOW (dialog),
					      parent);
	
		gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog),
						    TRUE);
	}

  gtk_window_set_position (GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_title (GTK_WINDOW (dialog), _("Replace"));
  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (dialog), 10);

	return GTK_WIDGET (dialog);
}

