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

#include "document_manager.h"
#include "preferences_manager.h"
#include "search_infobar.h"
#include "gphpedit-close-button.h"

#define SEARCHINFOBAR_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
						INFOBAR_TYPE_SEARCH,              \
						SearchInfobarPrivate))

struct _SearchInfobarPrivate 
{
  GtkWidget *search_infobar_find;
  GtkWidget *search_label;
  GtkWidget *search_entry;
  GtkWidget *cleanimg;
  GtkEntryCompletion *completion;
  GtkTreeModel *completion_model;
};

G_DEFINE_TYPE(SearchInfobar, search_infobar, GTK_TYPE_INFO_BAR)

static void search_infobar_init (SearchInfobar *search_infobar);
static void search_infobar_grab_focus(GtkWidget *widget);

static void
search_infobar_class_init (SearchInfobarClass *klass)
{
  GtkWidgetClass *widget_class;

  widget_class = (GtkWidgetClass*) klass;  
  widget_class->grab_focus = search_infobar_grab_focus;

	g_type_class_add_private (klass, sizeof (SearchInfobarPrivate));
}

static void search_infobar_grab_focus(GtkWidget *widget)
{
  SearchInfobarPrivate *priv = SEARCHINFOBAR_GET_PRIVATE(widget);

  GTK_WIDGET_CLASS (search_infobar_parent_class)->grab_focus (widget);
  gtk_widget_grab_focus(GTK_WIDGET(priv->search_entry));

}
/**
* on_cleanicon_press
* Clear entry text 
*/
static void on_cleanicon_press (GtkEntry *entry, GtkEntryIconPosition icon_pos, GdkEvent *event, gpointer user_data)
{
    gtk_entry_set_text (entry,"");
}

/* Creates a tree model containing the completions */
static GtkTreeModel *create_completion_model (void)
{
  GtkListStore *store;
  GtkTreeIter iter;
  PreferencesManager *pref;
  pref = preferences_manager_new ();
  store = gtk_list_store_new (1, G_TYPE_STRING);
  GSList *walk;
  for (walk = get_preferences_manager_search_history(pref); walk!=NULL; walk = g_slist_next(walk)) {
    /* Append one word */
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter, 0, (gchar *) walk->data, -1);
    //g_print("completion added:%s\n",(gchar *) walk->data);
   }
 
  return GTK_TREE_MODEL (store);
}

static void on_close_button_activate(GtkWidget *widget, gpointer user_data)
{  
  gtk_widget_hide(user_data);
}

static void incremental_search_typed (GtkEntry *entry, const gchar *text, gint length,
                                      gint *position, gpointer data)
{
  gchar *current_text;
  current_text = (gchar *)gtk_entry_get_text(entry);
  DocumentManager *docmg = document_manager_new ();
  documentable_incremental_search(DOCUMENTABLE(document_manager_get_current_document(docmg)), current_text, FALSE);
  g_object_unref(docmg);
}

static void add_to_search_history(const gchar *current_text, GtkEntryCompletion *completion){
    /* add text to search history*/
    GSList *walk;
    gint i=0;
    PreferencesManager *pref = preferences_manager_new ();
    for (walk = get_preferences_manager_search_history(pref); walk!=NULL; walk = g_slist_next(walk)) {
      if (g_strcmp0((gchar *) walk->data, current_text)==0) {
       g_object_unref(pref);
       return;  /* already in the list */
      }
      i++;
    }
    set_preferences_manager_new_search_history_item(pref, i, current_text);
    gtk_entry_completion_insert_action_text (completion, 0, g_strdup(current_text));
    g_object_unref(pref);
}


static void incremental_search_activate(GtkEntry *entry, gpointer user_data)
{
  SearchInfobarPrivate *priv = SEARCHINFOBAR_GET_PRIVATE(user_data);
  gchar *current_text;

  //Inc search only if the current tab is not a help tab
  current_text = (gchar *)gtk_entry_get_text(entry);

  DocumentManager *docmg = document_manager_new ();
  documentable_incremental_search(DOCUMENTABLE(document_manager_get_current_document(docmg)), current_text, TRUE);
  g_object_unref(docmg);

  add_to_search_history(current_text, priv->completion);
}

static gboolean incremental_search_key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    if (event->keyval == GDK_Escape) {
        GtkWidget *document_widget;
        DocumentManager *docmg = document_manager_new ();
        g_object_get(document_manager_get_current_document(docmg), "editor_widget", &document_widget, NULL);
        gtk_widget_hide(user_data);
        documentable_grab_focus(DOCUMENTABLE(document_manager_get_current_document(docmg)));
        g_object_unref(docmg);
      return TRUE;
    }
  return FALSE;
}

static void
search_infobar_init (SearchInfobar *search_infobar)
{
  SearchInfobarPrivate *priv = SEARCHINFOBAR_GET_PRIVATE(search_infobar);
	GtkWidget *hbox;
	GtkWidget *label;

  /* set up info bar */
  GtkWidget *content_area = gtk_info_bar_get_content_area(GTK_INFO_BAR (search_infobar));
  gtk_box_set_spacing(GTK_BOX (content_area), 0);

  hbox = gtk_hbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (content_area), hbox, TRUE, TRUE, 0);

  GtkWidget *close = gphpedit_close_button_new ();
  g_signal_connect(G_OBJECT(close), "clicked", G_CALLBACK(on_close_button_activate), search_infobar);
  gtk_box_pack_start (GTK_BOX (hbox), close, FALSE, FALSE, 0);
  
  label = gtk_label_new (_("Search for: "));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  priv->search_entry = gtk_entry_new();
  gtk_entry_set_max_length (GTK_ENTRY(priv->search_entry), 20);
  gtk_entry_set_width_chars(GTK_ENTRY(priv->search_entry), 21);
  gtk_widget_set_tooltip_text (priv->search_entry, _("Incremental search"));
  gtk_entry_set_icon_from_stock (GTK_ENTRY(priv->search_entry), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);
  g_signal_connect (G_OBJECT (priv->search_entry), "icon-press", G_CALLBACK (on_cleanicon_press), NULL);
  gtk_widget_show(priv->search_entry);
  gtk_box_pack_start (GTK_BOX (hbox), priv->search_entry, FALSE, FALSE, 0);

  /* search completion code */
  priv->completion= gtk_entry_completion_new();

  /* Create a tree model and use it as the completion model */
  priv->completion_model = create_completion_model ();
  gtk_entry_completion_set_model (priv->completion, priv->completion_model);
  g_object_unref (priv->completion_model);
    
   /* Use model column 0 as the text column */
  gtk_entry_completion_set_text_column (priv->completion, 0);
  /* set autocompletion settings: complete inline and show pop-up */
  gtk_entry_completion_set_popup_completion (priv->completion,TRUE);
  gtk_entry_completion_set_inline_completion (priv->completion,TRUE);
  /* set min match as 2 */
  gtk_entry_completion_set_minimum_key_length (priv->completion, 2);
  /* Assign the completion to the entry */
  gtk_entry_set_completion (GTK_ENTRY(priv->search_entry), priv->completion);
  g_object_unref (priv->completion);
  g_signal_connect_after(G_OBJECT(priv->search_entry), "insert_text", G_CALLBACK(incremental_search_typed), NULL);
  g_signal_connect_after(G_OBJECT(priv->search_entry), "activate", G_CALLBACK(incremental_search_activate), search_infobar);
  g_signal_connect_after(G_OBJECT(priv->search_entry), "key_release_event", G_CALLBACK(incremental_search_key_release_event), search_infobar);
  gtk_widget_show_all (content_area);
}

GtkWidget *
search_infobar_new (void)
{
  SearchInfobar *search_infobar = g_object_new (INFOBAR_TYPE_SEARCH, "message-type", GTK_MESSAGE_OTHER, NULL);
	return GTK_WIDGET(search_infobar);
}
