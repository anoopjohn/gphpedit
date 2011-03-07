/* This file is part of gPHPEdit, a GNOME PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009, 2010 José Rostagno (for vijona.com.ar) 

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

#include "document_manager.h"
#include "debug.h"
#include "main_window_callbacks.h"
#include "gvfs_utils.h"
#include "gphpedit-close-button.h"
#include "gphpedit-statusbar.h"
#include "document_loader.h"
#include "tab.h"

/*
* document_manager private struct
*/
struct DocumentManagerDetails
{
  Document *current_document;
  GSList *editors;
};

/* object signal enumeration */
enum {
	NEW_DOCUMENT,
	CHANGE_DOCUMENT,
	CLOSE_DOCUMENT,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


#define DOCUMENT_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    DOCUMENT_MANAGER_TYPE,\
					    DocumentManagerDetails))

static void document_manager_finalize (GObject *object);
void document_manager_create_new(void);
void document_loader_done_refresh_cb (DocumentLoader *doclod, gboolean result, gpointer user_data);

/*
 * register DocumentManager type and returns a new GType
*/
G_DEFINE_TYPE(DocumentManager, document_manager, G_TYPE_OBJECT);  

/*
* overide default contructor to make a singleton.
* see http://blogs.gnome.org/xclaesse/2010/02/11/how-to-make-a-gobject-singleton/
*/
static GObject*
document_manager_constructor (GType type,
                 guint n_construct_params,
                 GObjectConstructParam *construct_params)
{
  static GObject *self = NULL;

  if (self == NULL)
    {
      self = G_OBJECT_CLASS (document_manager_parent_class)->constructor (
          type, n_construct_params, construct_params);
      g_object_add_weak_pointer (self, (gpointer) &self);
      return self;
    }

  return g_object_ref (self);
}

static void
document_manager_dispose (GObject *object)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  DocumentManager *doc = DOCUMENT_MANAGER(object);
  DocumentManagerDetails *docdet;
  docdet = DOCUMENT_MANAGER_GET_PRIVATE(doc);
  /* save current session */
  document_manager_session_save(DOCUMENT_MANAGER(object));
  /* free class data */
  G_OBJECT_CLASS (document_manager_parent_class)->dispose (object);
}

static void
document_manager_class_init (DocumentManagerClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = document_manager_finalize;
  object_class->dispose = document_manager_dispose;
  object_class->constructor = document_manager_constructor;

	signals[NEW_DOCUMENT] =
		g_signal_new ("new_document",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (DocumentManagerClass, new_document),
		              NULL, NULL,
		               g_cclosure_marshal_VOID__OBJECT,
		               G_TYPE_NONE, 1, G_TYPE_OBJECT, NULL);

	signals[CHANGE_DOCUMENT] =
		g_signal_new ("change_document",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (DocumentManagerClass, new_document),
		              NULL, NULL,
		               g_cclosure_marshal_VOID__OBJECT,
		               G_TYPE_NONE, 1, G_TYPE_OBJECT, NULL);

	signals[CLOSE_DOCUMENT] =
		g_signal_new ("close_document",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (DocumentManagerClass, new_document),
		              NULL, NULL,
		               g_cclosure_marshal_VOID__OBJECT,
		               G_TYPE_NONE, 1, G_TYPE_OBJECT, NULL);

  g_type_class_add_private (klass, sizeof (DocumentManagerDetails));
}

static void
document_manager_init (DocumentManager * object)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  DocumentManagerDetails *docdet;
  docdet = DOCUMENT_MANAGER_GET_PRIVATE(object);
  docdet->current_document = NULL;
  docdet->editors = NULL;
}

static void
document_manager_finalize (GObject *object)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  DocumentManager *doc = DOCUMENT_MANAGER(object);
  DocumentManagerDetails *docdet;
  docdet = DOCUMENT_MANAGER_GET_PRIVATE(doc);
  /* save current session */
  document_manager_session_save(DOCUMENT_MANAGER(object));
  document_manager_close_all_tabs(doc);
  //free class data
  G_OBJECT_CLASS (document_manager_parent_class)->finalize (object);
}

DocumentManager *document_manager_new (void)
{
  return g_object_new (DOCUMENT_MANAGER_TYPE, NULL);
}

DocumentManager *document_manager_new_full (char **argv, gint argc)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  DocumentManager *docmg;
  docmg = g_object_new (DOCUMENT_MANAGER_TYPE, NULL);
  
  /* load command line files */
  guint i;
  if (argc>1) {
    i = 1;
    while (argv[i] != NULL) {
      document_manager_switch_to_file_or_open(docmg, argv[i],0);
      ++i;
    }
  } else {
  document_manager_session_reopen(docmg);
  }
  return docmg; /* return new object */
}

void _document_manager_set_current_document(DocumentManager *docmg, Document *document)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return ;
  DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  docmgdet->current_document = document;
  g_signal_emit (G_OBJECT (docmg), signals[CHANGE_DOCUMENT], 0, docmgdet->current_document);
}

static void on_tab_close_activate(GtkWidget *widget, Document *document)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  document_manager_try_close_document(main_window.docmg, document);
  g_signal_emit (G_OBJECT (main_window.docmg), signals[CLOSE_DOCUMENT], 0, document);
}

/* internal */
GtkWidget *get_close_tab_widget(Document *document)
{
  GtkWidget *hbox;
  GtkWidget *close_button;
  hbox = gtk_hbox_new(FALSE, 0);
  close_button = gphpedit_close_button_new ();
  gtk_widget_set_tooltip_text(close_button, _("Close Tab"));

  g_signal_connect(G_OBJECT(close_button), "clicked", G_CALLBACK(on_tab_close_activate), document);
  /* load file icon */
  GtkWidget *label;
  GIcon *doc_icon;
  g_object_get(document, "editor_label", &label, "icon", &doc_icon, NULL);
  GtkWidget *icon= gtk_image_new_from_gicon (doc_icon, GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_widget_show (icon);
  gtk_box_pack_start(GTK_BOX(hbox), icon, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);
  gtk_box_pack_end(GTK_BOX(hbox), close_button, FALSE, FALSE, 0);
  gtk_widget_show(close_button);
  gtk_widget_show(hbox);
  return hbox;
}

/* internal */
void document_save_update_cb (Document *doc, gpointer user_data)
{
  DocumentManager *docmg = DOCUMENT_MANAGER(user_data);
  DocumentManagerDetails *docmgdet;
  docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  if (doc==docmgdet->current_document){
    g_signal_emit (G_OBJECT (docmg), signals[CHANGE_DOCUMENT], 0, docmgdet->current_document);
  }
  gint ftype;
  g_object_get(doc, "type", &ftype, NULL);
  gchar *filename = documentable_get_filename(DOCUMENTABLE(doc));
  symbol_manager_rescan_file (main_window.symbolmg, filename, ftype);
  g_free(filename);
}

void document_save_start_cb (Document *doc, gpointer user_data)
{
  /* show status in statusbar */
  const gchar *short_filename;
  g_object_get(doc, "short_filename", &short_filename, NULL);
  gphpedit_statusbar_flash_message (GPHPEDIT_STATUSBAR(main_window.appbar),0,_("Saving %s"), short_filename);
}

void document_type_changed_cb (Document *doc, gint type, gpointer user_data)
{
	DocumentManager *docmg = DOCUMENT_MANAGER(user_data);
	DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
	/* only process if document is current_document */
	if (doc==docmgdet->current_document) g_signal_emit (G_OBJECT (docmg), signals[CHANGE_DOCUMENT], 0, docmgdet->current_document);
}

void document_col_changed_cb (Document *doc, gint col, gpointer user_data)
{
  gphpedit_statusbar_set_cursor_position (GPHPEDIT_STATUSBAR(main_window.appbar), -1, col);
}

void document_pos_changed_cb (Document *doc, gint pos, gpointer user_data)
{
  gphpedit_statusbar_set_cursor_position (GPHPEDIT_STATUSBAR(main_window.appbar), pos, -1);
}

void document_need_reload_cb (Document *doc, gpointer user_data)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  DocumentLoader *loader = document_loader_new (GTK_WINDOW(main_window.window));
  g_signal_connect(G_OBJECT(loader), "done_refresh", G_CALLBACK(document_loader_done_refresh_cb), user_data);
  document_loader_reload_file(loader, doc);
}

static void document_ovr_changed_cb (Document *doc, gboolean status, gpointer user_data)
{
  gphpedit_statusbar_set_overwrite (GPHPEDIT_STATUSBAR(main_window.appbar), status);
}

static void document_marker_not_found_cb (Document *doc, gpointer user_data)
{
  gphpedit_statusbar_flash_message (GPHPEDIT_STATUSBAR(main_window.appbar),0 , "%s",_("No marker found"));
}

static void document_open_request_cb (Document *doc, gchar *uri, gpointer user_data)
{
  DocumentManager *docmg = DOCUMENT_MANAGER(user_data);
  document_manager_switch_to_file_or_open(docmg, uri, 0);
}

void document_loader_done_loading_cb (DocumentLoader *doclod, gboolean result, Document *doc, gpointer user_data)
{
  DocumentManager *docmg = DOCUMENT_MANAGER(user_data);
  DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  if (result) {
    gboolean untitled;
    docmgdet->editors = g_slist_append(docmgdet->editors, doc);
    GtkWidget *document_tab;
    document_tab = get_close_tab_widget(doc);
    GtkWidget *document_widget;
    g_object_get(doc, "untitled", &untitled, "editor_widget", &document_widget, NULL);
    gtk_widget_show(document_widget);
    gtk_notebook_append_page (GTK_NOTEBOOK (main_window.notebook_editor), document_widget, document_tab);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (main_window.notebook_editor), -1);
    _document_manager_set_current_document(docmg, doc);
    g_signal_connect(G_OBJECT(doc), "save_update", G_CALLBACK(document_save_update_cb), docmg);
    if (OBJECT_IS_DOCUMENT_SCINTILLA(doc)) {
      g_signal_connect(G_OBJECT(doc), "save_start", G_CALLBACK(document_save_start_cb), NULL);
      g_signal_connect(G_OBJECT(doc), "type_changed", G_CALLBACK(document_type_changed_cb), docmg);
      g_signal_connect(G_OBJECT(doc), "pos_changed", G_CALLBACK(document_pos_changed_cb), docmg);
      g_signal_connect(G_OBJECT(doc), "col_changed", G_CALLBACK(document_col_changed_cb), docmg);
      g_signal_connect(G_OBJECT(doc), "need_reload", G_CALLBACK(document_need_reload_cb), docmg);
      g_signal_connect(G_OBJECT(doc), "ovr_changed", G_CALLBACK(document_ovr_changed_cb), NULL);
      g_signal_connect(G_OBJECT(doc), "marker_not_found", G_CALLBACK(document_marker_not_found_cb), NULL);
      g_signal_connect(G_OBJECT(doc), "open_request", G_CALLBACK(document_open_request_cb), docmg);
    }
    g_signal_emit (G_OBJECT (docmg), signals[NEW_DOCUMENT], 0, doc);
    gtk_widget_grab_focus(document_widget);
    if (!untitled) document_manager_session_save(docmg);
  }
  g_object_unref(doclod);
}

void document_loader_need_mounting_cb (DocumentLoader *doclod, gpointer user_data)
{
  gphpedit_statusbar_flash_message (GPHPEDIT_STATUSBAR(main_window.appbar),0,"%s", 
    _("Error filesystem not mounted. Mounting filesystem, this will take a few seconds..."));
}

void document_loader_help_file_not_found_cb (DocumentLoader *doclod, gpointer user_data)
{
  gphpedit_statusbar_flash_message (GPHPEDIT_STATUSBAR(main_window.appbar),0,"%s",_("Could not find the required command in the online help"));
  g_object_unref(doclod);
}

void document_manager_add_new_document(DocumentManager *docmg, gint type, const gchar *filename, gint goto_line)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return ;

  DocumentLoader *loader = document_loader_new (GTK_WINDOW(main_window.window));
  g_signal_connect(G_OBJECT(loader), "done_loading", G_CALLBACK(document_loader_done_loading_cb), docmg);
  g_signal_connect(G_OBJECT(loader), "need_mounting", G_CALLBACK(document_loader_need_mounting_cb), docmg);
  g_signal_connect(G_OBJECT(loader), "help_file_not_found", G_CALLBACK(document_loader_help_file_not_found_cb), docmg);
  document_loader_load (loader, type, (gchar *) filename, goto_line);
}

Documentable *document_manager_find_documentable_from_widget (DocumentManager *docmg, void *widget)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return NULL;
  DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  GSList *walk;
  Documentable *document;
  GtkWidget *document_widget;
  for (walk = docmgdet->editors; walk != NULL; walk = g_slist_next (walk)) {
    document = DOCUMENTABLE(walk->data);
    g_object_get(document, "editor_widget", &document_widget, NULL);
    if (document_widget == GTK_WIDGET(widget)) return walk->data;
  }
  return NULL;
}

Documentable *document_manager_find_documentable_from_filename (DocumentManager *docmg, gchar *filename)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return NULL;
  DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  GSList *walk;
  Documentable *document;

  for (walk = docmgdet->editors; walk != NULL; walk = g_slist_next (walk)) {
    document = DOCUMENTABLE(walk->data);
    gchar *doc_filename = documentable_get_filename(document);
    if (g_strcmp0(doc_filename,filename)==0) {
      g_free(doc_filename);
      return document;
    }
    g_free(doc_filename);
  }
  return NULL;
}

Documentable *document_manager_get_current_documentable (DocumentManager *docmg)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return NULL;
  DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  return DOCUMENTABLE(docmgdet->current_document);
}

gboolean document_manager_set_current_document_from_widget (DocumentManager *docmg, GtkWidget *child)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return FALSE;
  Document *data = DOCUMENT(document_manager_find_documentable_from_widget (docmg, (void *) child));
  if (data){
    _document_manager_set_current_document(docmg, data);
    gtk_widget_grab_focus(child);
  } else {
    return FALSE;
  }
  return TRUE;
}

void document_manager_get_context_help(DocumentManager *docmg)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return ;
  DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  gchar *buffer = documentable_get_current_selected_text(DOCUMENTABLE(docmgdet->current_document));
  if (buffer){
    document_manager_add_new_document(docmg, TAB_HELP, buffer, 0);
  }
}

void document_manager_get_document_preview(DocumentManager *docmg)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return ;
  DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  gchar *filename = documentable_get_filename(DOCUMENTABLE(docmgdet->current_document));
  document_manager_add_new_document(docmg, TAB_PREVIEW, filename, 0);
  g_free(filename);
}
/*
* session_save relies on the fact that all tabs can be closed without 
* prompting, they should already be saved.  Also, the title won't be set
* afterwards.
*/
void document_manager_session_save(DocumentManager *docmg)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return ;
  DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  GSList *walk;
  Document *document;
  gboolean save_session;
  GSList *files=NULL;
  gchar *fileentry = NULL;

  PreferencesManager *prefmg = preferences_manager_new();
  g_object_get (prefmg, "save_session", &save_session, NULL);
  if (save_session) {
    for(walk = docmgdet->editors; walk!= NULL; walk = g_slist_next(walk)) {
      document = walk->data;
      gchar *entry = documentable_get_session_entry(DOCUMENTABLE(document));
      if (entry) {
        if (document == docmgdet->current_document) {
          fileentry = g_strdup_printf("*%s", entry);
        } else {
          fileentry = g_strdup (entry);
        }
        g_free(entry);
        files = g_slist_prepend (files, fileentry);
      }
    }
    files = g_slist_reverse(files);
  }
  set_preferences_manager_session_files(prefmg, files);
  g_object_unref(prefmg);
}

void document_manager_session_reopen(DocumentManager *docmg)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return ;
  DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  int focus_tab=-1;
  gboolean focus_this_one = FALSE;
  PreferencesManager *prefmg = preferences_manager_new();
  GSList *files = get_preferences_manager_session_files(prefmg);
  if (g_slist_length (files)==0) {
    //session file exists but is empty
    //add a new untitled
    document_manager_add_new_document(docmg, TAB_FILE, NULL, 0);
  } else {
    GSList *walk;
    gchar *filename;
    for(walk = files; walk!= NULL; walk = g_slist_next(walk)) {
        filename = (gchar *)walk->data;
        /* filename contains possibly:
          file:///blah\n
          *file:///blah\n
          phphelp:function\n
          *phphelp:function\n
          preview:function\n
          *preview:function\n

        */
	filename = g_strdelimit (filename, "\n", 0);
        if (filename[0]=='*') {
          filename++;
          focus_this_one = TRUE;
        }
        if (g_str_has_prefix(filename, "phphelp:")){
          filename += 8;
          document_manager_add_new_document(docmg, TAB_HELP, filename, 0);
        } else if (g_str_has_prefix(filename, "preview:")){
          filename += 8;
          document_manager_add_new_document(docmg, TAB_PREVIEW, filename, 0);
        } else {
          if (filename){
          document_manager_switch_to_file_or_open(docmg, filename, 0);
          }
        }
        if (focus_this_one && (docmgdet->current_document)) {
            GtkWidget *document_widget;
            g_object_get(docmgdet->current_document, "editor_widget", &document_widget, NULL);
            focus_tab = gtk_notebook_page_num(GTK_NOTEBOOK(main_window.notebook_editor), document_widget);
        }
        focus_this_one=FALSE;
    }
  }
  gtk_notebook_set_current_page(GTK_NOTEBOOK(main_window.notebook_editor), focus_tab);
  g_object_unref(prefmg);
}

void document_manager_switch_to_file_or_open(DocumentManager *docmg, gchar *filename, gint line_number)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return ;
  DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  Document *document;
  GSList *walk;
  /* need to check if filename is local before adding to the list */
  filename = g_strdup(filename);
  for (walk = docmgdet->editors; walk!=NULL; walk = g_slist_next(walk)) {
    document = walk->data;
    gchar *docfilename;
    GFile *file;
    g_object_get(document, "GFile", &file, NULL);
    docfilename = g_file_get_uri(file);
    gchar *filename_uri = filename_get_uri(filename);
    if (g_strcmp0(docfilename, filename_uri)==0) {
      GtkWidget *document_widget;
      g_object_get(document, "editor_widget", &document_widget, NULL);
      gtk_notebook_set_current_page( GTK_NOTEBOOK(main_window.notebook_editor), gtk_notebook_page_num(GTK_NOTEBOOK(main_window.notebook_editor), document_widget));
      documentable_goto_line(DOCUMENTABLE(docmgdet->current_document), line_number);
      g_free(docfilename);
      return ;
    }
    g_free(filename_uri);
    g_free(docfilename);
  }
  document_manager_add_new_document(docmg, TAB_FILE, filename, line_number);
  register_file_opened(filename);
  g_free(filename);
  return ;
}

void document_loader_done_refresh_cb (DocumentLoader *doclod, gboolean result, gpointer user_data)
{
  g_object_unref(doclod);
}

void document_manager_document_reload(DocumentManager *docmg)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return ;
  DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  gboolean saved;
  g_object_get(docmgdet->current_document, "saved", &saved, NULL);
  if (!saved) {
    gint result = yes_no_dialog (_("Question"), _("Are you sure you wish to reload the current file, losing your changes?"));
    if (result==GTK_RESPONSE_NO) return ;
  }
  const gchar *short_filename;
  g_object_get(docmgdet->current_document, "short_filename", &short_filename, NULL);
  gphpedit_statusbar_flash_message (GPHPEDIT_STATUSBAR(main_window.appbar),0,_("Reloading %s"), short_filename);
  documentable_reload(DOCUMENTABLE(docmgdet->current_document));
}

void document_manager_open_selected(DocumentManager *docmg)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return ;
  DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  GSList *li;
  Document *document;
  gchar *ac_buffer;
  gchar *file;

  ac_buffer = documentable_get_current_selected_text(DOCUMENTABLE(docmgdet->current_document));
  if (!ac_buffer){
    for(li = docmgdet->editors; li!= NULL; li = g_slist_next(li)) {
      document = li->data;
      if (document) {
         gchar *filename = documentable_get_filename(DOCUMENTABLE(document));
         file = filename_parent_uri(filename);
         g_free(filename);
         gphpedit_debug_message(DEBUG_MAIN_WINDOW,"file: %s", filename);
         if (!strstr(ac_buffer, "://") && file) {
          gchar *filetemp= g_strdup_printf("%s/%s",file, ac_buffer);
          g_free(file);
          file=g_strdup(filetemp);
          g_free(filetemp);
        }
        else if (strstr(ac_buffer, "://")) {
            if (file) g_free(file);
            file = g_strdup(ac_buffer);
        }
        if(filename_file_exist(file)) document_manager_switch_to_file_or_open(docmg, file,0);
        if (file) g_free(file);
      }
    }
  }
  g_free(ac_buffer);
}

GSList *document_manager_get_document_list (DocumentManager *docmg)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return NULL;
  DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  return docmgdet->editors;
}

gint document_manager_get_document_count (DocumentManager *docmg)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return 0;
  DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  if (!docmgdet->editors) return 0;
  return g_slist_length(docmgdet->editors);
}

static void close_tab (gpointer data, gpointer user_data)
{
  document_manager_close_page(DOCUMENT_MANAGER(user_data), data);
}

// This procedure relies on the fact that all tabs will be closed without prompting
// for whether they need saving beforehand.  If in doubt, call can_all_tabs_be_saved
// and pay attention to the return value.
void document_manager_close_all_tabs(DocumentManager *docmg)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return ;
  DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  if (!docmgdet->editors) return;
  g_slist_foreach (docmgdet->editors, close_tab, docmg);
  docmgdet->editors = NULL;
  docmgdet->current_document = NULL;
}

// Returns true if all tabs are either saved or closed
gboolean document_manager_can_all_tabs_be_saved(DocumentManager *docmg)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return TRUE;
  DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  GSList *walk;
  Document *document;
  gboolean saved;

  for(walk = docmgdet->editors; walk!= NULL; walk = g_slist_next(walk)) {
    document = walk->data;
    GtkWidget *document_widget;
    g_object_get(document, "editor_widget", &document_widget, NULL);
    if (document_widget) {
      gboolean read_only, saved_status;
      g_object_get(document, "read_only", &read_only, "saved", &saved_status, NULL);
      if (!saved_status && !read_only) {
        saved = document_manager_try_save_page(docmg, document, FALSE);
        if (saved==FALSE) return FALSE;
      }
    }
  }
  return TRUE;
}

void document_manager_close_page(DocumentManager *docmg, Document *document)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return ;
  DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  close_page(document);
  gchar *filename = documentable_get_filename (DOCUMENTABLE(document));
  gint ftype;
  g_object_get(document, "type", &ftype, NULL);
  symbol_manager_purge_file (main_window.symbolmg, filename, ftype);
  g_free(filename);
  g_object_unref(document);
  docmgdet->editors = g_slist_remove(docmgdet->editors, document);
  if (!docmgdet->editors) {
    docmgdet->current_document = NULL;
  }
  document_manager_session_save(docmg);
}

gboolean document_manager_try_save_page(DocumentManager *docmg, Document *document, gboolean close_if_can)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return TRUE;
  gint ret;
  const gchar *short_filename;
  g_object_get(document, "short_filename", &short_filename, NULL);

  GtkWidget *confirm_dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW(main_window.window),
    GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE,
      /*TRANSLATORS: this is a pango markup string you must keep the format tags. */
    _("<b>The file '%s' has not been saved since your last changes.</b>\n<small>Are you sure you want to close it and lose these changes?</small>"),
     short_filename);

  gtk_dialog_add_button (GTK_DIALOG(confirm_dialog),_("Close and _lose changes"), 0);
  gboolean untitled;
  g_object_get(document, "untitled", &untitled, NULL);
  if (untitled){
    gtk_dialog_add_button (GTK_DIALOG(confirm_dialog), GTK_STOCK_SAVE_AS, 1);
  } else {
    gtk_dialog_add_button (GTK_DIALOG(confirm_dialog), GTK_STOCK_SAVE, 1);
  }
  gtk_dialog_add_button (GTK_DIALOG(confirm_dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
  gtk_dialog_set_default_response (GTK_DIALOG(confirm_dialog), 1); /* set save as default */

  ret = gtk_dialog_run (GTK_DIALOG (confirm_dialog));
  gtk_widget_destroy(confirm_dialog);
  switch (ret) {
    case 0:
      if (close_if_can) {
          document_manager_close_page(docmg, document);
      }
      return TRUE;
    case 1:
      on_save1_activate(NULL);
      // If chose neither of these, dialog either cancelled or closed. Do nothing.
  }
  return FALSE;
}


void document_manager_save_all(DocumentManager *docmg)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return ;
  DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  GSList *li;
  Document *doc;
  for(li = docmgdet->editors; li!= NULL; li = g_slist_next(li)) {
    doc = li->data;
    gboolean untitled;
    g_object_get(doc, "untitled", &untitled, NULL);
    if (untitled){
      gphpedit_debug_message(DEBUG_MAIN_WINDOW,"%s","Untitled found. Save not implemented"); //FIXME:
    } else {
      documentable_save(DOCUMENTABLE(doc));
    }
  }
}

gboolean document_manager_set_current_document_from_position(DocumentManager *docmg, gint page_num)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return FALSE;
  DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  Document *new_current_editor;
  new_current_editor = DOCUMENT(g_slist_nth_data(docmgdet->editors, page_num));
  if (new_current_editor){
   _document_manager_set_current_document(docmg, new_current_editor);
   return TRUE;
  }
  return FALSE;
}

gboolean document_manager_try_close_document(DocumentManager *docmg, Document *document)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  gboolean read_only, is_empty, saved_status;
  g_object_get(document, "read_only", &read_only, "is_empty", &is_empty, "saved", &saved_status, NULL);

  if (!read_only && !saved_status && !is_empty) {
    return document_manager_try_save_page(main_window.docmg, document, TRUE);
  }
  document_manager_close_page(main_window.docmg, document);
  return TRUE;
}

gboolean document_manager_try_close_current_document(DocumentManager *docmg)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return FALSE;
  DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  return document_manager_try_close_document(docmg, docmgdet->current_document);
}

void document_manager_refresh_properties_all(DocumentManager *docmg)
{
  gphpedit_debug(DEBUG_DOC_MANAGER);
  if (!docmg) return ;
  DocumentManagerDetails *docmgdet = DOCUMENT_MANAGER_GET_PRIVATE(docmg);
  g_slist_foreach (docmgdet->editors, (GFunc) documentable_apply_preferences, NULL);
}
