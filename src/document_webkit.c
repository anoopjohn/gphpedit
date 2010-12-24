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

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <webkit/webkit.h>
#include "debug.h"
#include "document.h"
#include "document_webkit.h"
#include "gvfs_utils.h"
#include "search_infobar.h"
/* object signal enumeration */
enum {
  LOAD_COMPLETE,
  SAVE_UPDATE,
	NEED_RELOAD,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

/*
* document_webkit private struct
*/
struct Document_WebkitDetails
{
	gint type;
  GtkWidget *container;
	GtkWidget *help_scrolled_window;
	WebKitWebView *help_view;
	GtkWidget *label;
  GFile *file;
  GIcon *ico;
  gchar *short_filename;
  gint64 mtime;
	gboolean isreadonly;
	gboolean is_untitled;
  gchar *contenttype;

  /* incremental search widget */
  GtkWidget *searchbar;
};

#define DOCUMENT_WEBKIT_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    DOCUMENT_WEBKIT_TYPE,\
					    Document_WebkitDetails))

static void document_webkit_dispose (GObject *gobject);
static void notify_title_cb (WebKitWebView* web_view, GParamSpec* pspec, Document_Webkit *document_webkit);
static gboolean webkit_link_clicked (WebKitWebView *web_view, WebKitWebFrame *frame, WebKitNetworkRequest *request,
                                                        WebKitWebNavigationAction *navigation_action,
                                                        WebKitWebPolicyDecision   *policy_decision,
                                                        Document_Webkit *document_webkit);
static void document_webkit_constructed (GObject *object);
/*
 * register Document_Webkit type and returns a new GType
*/

static void document_webkit_documentable_init(DocumentableIface *iface, gpointer user_data);

G_DEFINE_TYPE_WITH_CODE(Document_Webkit, document_webkit, DOCUMENT_TYPE,
                        G_IMPLEMENT_INTERFACE (IFACE_TYPE_DOCUMENTABLE,
                                                 document_webkit_documentable_init));

void document_webkit_zoom_in(Documentable *doc)
{
  g_return_if_fail(doc);
  Document_WebkitDetails *docdet = DOCUMENT_WEBKIT_GET_PRIVATE(DOCUMENT_WEBKIT(doc));
  webkit_web_view_zoom_in (docdet->help_view);
}

void document_webkit_zoom_out(Documentable *doc){
  g_return_if_fail(doc);
  Document_WebkitDetails *docdet = DOCUMENT_WEBKIT_GET_PRIVATE(doc);
  webkit_web_view_zoom_out (docdet->help_view);
}

void document_webkit_zoom_restore(Documentable *doc){
  g_return_if_fail(doc);
  Document_WebkitDetails *docdet = DOCUMENT_WEBKIT_GET_PRIVATE(doc);
  webkit_web_view_set_zoom_level (docdet->help_view, 1);
}

void document_webkit_undo(Documentable *doc){
  g_return_if_fail(doc);
  Document_WebkitDetails *docdet = DOCUMENT_WEBKIT_GET_PRIVATE(doc);
  webkit_web_view_undo (WEBKIT_WEB_VIEW(docdet->help_view));
}

void document_webkit_redo(Documentable *doc){
  g_return_if_fail(doc);
  Document_WebkitDetails *docdet = DOCUMENT_WEBKIT_GET_PRIVATE(doc);
  webkit_web_view_redo (WEBKIT_WEB_VIEW(docdet->help_view));
}

void document_webkit_select_all(Documentable *doc){
  g_return_if_fail(doc);
  Document_WebkitDetails *docdet = DOCUMENT_WEBKIT_GET_PRIVATE(doc);
  webkit_web_view_select_all (WEBKIT_WEB_VIEW(docdet->help_view));
}

void document_webkit_selection_to_lower(Documentable *doc)
{
}

void document_webkit_selection_to_upper(Documentable *doc)
{
}

void document_webkit_copy(Documentable *doc)
{
  g_return_if_fail(doc);
  Document_WebkitDetails *docdet = DOCUMENT_WEBKIT_GET_PRIVATE(doc);
  webkit_web_view_copy_clipboard (WEBKIT_WEB_VIEW(docdet->help_view));
}

void document_webkit_cut(Documentable *doc) {
  g_return_if_fail(doc);
  Document_WebkitDetails *docdet = DOCUMENT_WEBKIT_GET_PRIVATE(doc);
  webkit_web_view_cut_clipboard (WEBKIT_WEB_VIEW(docdet->help_view));
}

void document_webkit_paste(Documentable *doc) {
  g_return_if_fail(doc);
  Document_WebkitDetails *docdet = DOCUMENT_WEBKIT_GET_PRIVATE(doc);
  webkit_web_view_paste_clipboard (WEBKIT_WEB_VIEW(docdet->help_view));
}

void document_webkit_block_indent(Documentable *doc)
{
}

void document_webkit_block_unindent(Documentable *doc)
{
}

gchar *document_webkit_get_filename (Documentable  *doc) {
  if (!doc) return NULL;
  Document_WebkitDetails *docdet = DOCUMENT_WEBKIT_GET_PRIVATE(doc);
  gchar *uri = g_strdup(webkit_web_view_get_uri (WEBKIT_WEB_VIEW(docdet->help_view)));
  if (!uri) uri = g_file_get_uri(docdet->file);
  return uri;
}

void document_webkit_set_type (Documentable  *doc, gint type)
{
}

void document_webkit_goto_pos(Documentable *doc, glong pos)
{
}

void document_webkit_goto_line(Documentable *doc, gint line)
{
}

void document_webkit_scroll_to_current_pos (Documentable  *document_webkit)
{
}

gchar *document_webkit_get_current_selected_text (Documentable  *doc) {
  return NULL;
}

gchar *document_webkit_get_session_entry (Documentable  *doc)
{
  gphpedit_debug(DEBUG_DOCUMENT);
  gchar *result;
  Document_WebkitDetails *docdet;
  gboolean untitled;
  gchar *docfilename;

  if (!doc) return NULL;
  docdet = DOCUMENT_WEBKIT_GET_PRIVATE(doc);
  g_object_get(doc, "untitled", &untitled, NULL);
  if (untitled) return NULL;
  docfilename = documentable_get_filename(DOCUMENTABLE(doc));
  switch (docdet->type)
  {
    case TAB_HELP:
      result = g_strdup_printf ("phphelp:%s\n", docfilename);
      break;
    case TAB_PREVIEW:
      result = g_strdup_printf ("preview:%s\n",docfilename);
      break;
    default:
      result = g_strdup_printf ("%s\n",docfilename);
  }
  g_free(docfilename);
  return result;
}

void document_webkit_reload (Documentable *document_webkit)
{
  gphpedit_debug (DEBUG_DOCUMENT);
  g_return_if_fail(document_webkit);
  Document_WebkitDetails *docdet = DOCUMENT_WEBKIT_GET_PRIVATE(document_webkit);
  webkit_web_view_reload (docdet->help_view);
}

gchar *document_webkit_get_title(Document_Webkit *doc)
{
  gphpedit_debug (DEBUG_DOCUMENT);
  if (!doc) return NULL;
  Document_WebkitDetails *docdet = DOCUMENT_WEBKIT_GET_PRIVATE(doc);
  GString *title= NULL;
  if( WEBKIT_IS_WEB_VIEW(docdet->help_view)){
      title = g_string_new(docdet->short_filename);
      g_string_append(title, _(" - gPHPEdit"));
  }
  if (title) return g_string_free(title, FALSE);
  return NULL;
}

gint document_webkit_get_current_position(Documentable *doc)
{
  return -1;
}

gboolean document_webkit_search_text (Documentable  *doc, const gchar *text, gboolean checkwholedoc, gboolean checkcase, gboolean checkwholeword, gboolean checkregex)
{
  g_return_val_if_fail(doc, FALSE);
  Document_WebkitDetails *docdet = DOCUMENT_WEBKIT_GET_PRIVATE(doc);
  return webkit_web_view_search_text (WEBKIT_WEB_VIEW(docdet->help_view), text, checkcase, TRUE, checkwholeword);
}

gboolean document_webkit_search_replace_text(Documentable  *doc, const gchar *text, const gchar *replace, 
                gboolean checkwholedoc, gboolean checkcase, gboolean checkwholeword, gboolean checkregex, gboolean ask_replace)
{
  return FALSE;
}

void document_webkit_incremental_search (Documentable  *doc, gchar *current_text, gboolean advancing)
{
  g_return_if_fail(doc);
  Document_WebkitDetails *docdet = DOCUMENT_WEBKIT_GET_PRIVATE(doc);
  webkit_web_view_search_text (WEBKIT_WEB_VIEW(docdet->help_view), current_text, FALSE, advancing, TRUE);
}

void document_webkit_activate_incremental_search(Documentable  *doc)
{
  Document_WebkitDetails *docdet = DOCUMENT_WEBKIT_GET_PRIVATE(doc);
  gtk_widget_show(docdet->searchbar);
  gtk_widget_grab_focus(docdet->searchbar);
}

gchar *document_webkit_get_text (Documentable  *doc)
{
  return NULL;
}

void document_webkit_check_externally_modified (Documentable  *doc)
{
}

void document_webkit_save(Documentable *doc)
{
}

void document_webkit_save_as(Documentable *doc, GFile *file)
{
}

void document_webkit_replace_text (Documentable  *document_webkit, gchar *new_text)
{
}

void document_webkit_insert_text (Documentable  *doc, gchar *data)
{
}

gchar *document_webkit_get_current_word (Documentable  *doc)
{
  return NULL;
}

void document_webkit_replace_current_selection (Documentable *doc, gchar *data)
{
}

static void document_webkit_apply_preferences (Documentable *doc)
{
}

static void document_webkit_grab_focus (Documentable *doc)
{
  Document_WebkitDetails *docdet = DOCUMENT_WEBKIT_GET_PRIVATE(doc);
  gtk_widget_grab_focus(GTK_WIDGET(docdet->help_view));
}

static void document_webkit_documentable_init(DocumentableIface *iface, gpointer user_data)
{
	iface->zoom_in = document_webkit_zoom_in;
	iface->zoom_out = document_webkit_zoom_out;
	iface->zoom_restore = document_webkit_zoom_restore;
	iface->undo = document_webkit_undo;
	iface->redo = document_webkit_redo;
	iface->select_all = document_webkit_select_all;
  iface->selection_to_upper = document_webkit_selection_to_upper;
  iface->selection_to_lower = document_webkit_selection_to_lower;
  iface->copy = document_webkit_copy;
  iface->cut = document_webkit_cut;
  iface->paste = document_webkit_paste;
  iface->block_indent = document_webkit_block_indent;
  iface->block_unindent = document_webkit_block_unindent;
  iface->get_filename = document_webkit_get_filename;
  iface->set_type = document_webkit_set_type;
  iface->goto_pos = document_webkit_goto_pos;
  iface->goto_line = document_webkit_goto_line;
  iface->scroll_to_current_pos = document_webkit_scroll_to_current_pos;
  iface->get_current_selected_text = document_webkit_get_current_selected_text;
  iface->get_session_entry = document_webkit_get_session_entry;
  iface->reload = document_webkit_reload;
  iface->get_current_position = document_webkit_get_current_position;
  iface->search_text = document_webkit_search_text;
  iface->search_replace_text = document_webkit_search_replace_text;
  iface->incremental_search = document_webkit_incremental_search;
  iface->activate_incremental_search = document_webkit_activate_incremental_search;
  iface->get_text = document_webkit_get_text;
  iface->check_externally_modified = document_webkit_check_externally_modified;
  iface->save = document_webkit_save;
  iface->save_as = document_webkit_save_as;
  iface->replace_text = document_webkit_replace_text;
  iface->get_current_word = document_webkit_get_current_word;
  iface->insert_text = document_webkit_insert_text;
  iface->replace_current_selection = document_webkit_replace_current_selection;
  iface->apply_preferences = document_webkit_apply_preferences;
  iface->grab_focus = document_webkit_grab_focus;
}

enum
{
  PROP_0,
  PROP_UNTITLED,
  PROP_READ_ONLY,
  PROP_CAN_MODIFY,
  PROP_CONVERTED_TO_UTF8,
  PROP_IS_EMPTY,
  PROP_SAVED,
  PROP_CAN_PREVIEW,
  PROP_ZOOM_LEVEL,
  PROP_CONTENT_TYPE,
  PROP_SHORT_FILENAME,
  PROP_GFILE,
  PROP_TYPE,
  PROP_LABEL,
  PROP_ICON,
  PROP_WIDGET,
  PROP_MTIME,
  PROP_TITLE
};

static void
document_webkit_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
  Document_WebkitDetails *docdet = DOCUMENT_WEBKIT_GET_PRIVATE(object);

	switch (prop_id)
	{
		case PROP_CAN_MODIFY:
      webkit_web_view_set_editable (WEBKIT_WEB_VIEW(docdet->help_view), g_value_get_boolean (value));
			break;
    case PROP_CONVERTED_TO_UTF8:
			break;
    case PROP_MTIME:
			docdet->mtime = g_value_get_int64 (value);
			break;
		case PROP_READ_ONLY:
			docdet->isreadonly = g_value_get_boolean (value);
			break;
		case PROP_UNTITLED:
			docdet->is_untitled = g_value_get_boolean (value);
			break;
		case PROP_TYPE:
      docdet->type = g_value_get_int (value);
      documentable_set_type(DOCUMENTABLE(object), docdet->type);
			break;
    case PROP_CONTENT_TYPE:
			g_free(docdet->contenttype);
			docdet->contenttype = g_value_dup_string (value);
      break;
    case PROP_SHORT_FILENAME:
			g_free(docdet->short_filename);
			docdet->short_filename = g_value_dup_string (value);
      break;
    case PROP_GFILE:
      if(docdet->file) g_object_unref(docdet->file);
      docdet->file = g_value_dup_object (value);
      break;
    case PROP_ICON:
      if (docdet->ico) g_object_unref(docdet->ico);
      docdet->ico = g_value_dup_object (value);
      break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
document_webkit_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
  Document_WebkitDetails *docdet = DOCUMENT_WEBKIT_GET_PRIVATE(object);
  
  gint p;
	switch (prop_id)
	{
		case PROP_READ_ONLY:
			g_value_set_boolean (value, TRUE);
			break;
    case PROP_CAN_MODIFY:
			  g_value_set_boolean (value, webkit_web_view_get_editable (WEBKIT_WEB_VIEW(docdet->help_view)));
 			break;
    case PROP_CONVERTED_TO_UTF8:
      break;
    case PROP_MTIME:
			  g_value_set_int64 (value, docdet->mtime);
      break;
		case PROP_UNTITLED:
			g_value_set_boolean (value, docdet->is_untitled);
			break;
		case PROP_TYPE:
			g_value_set_int (value, docdet->type);
			break;
		case PROP_IS_EMPTY:
			g_value_set_boolean (value, FALSE);
			break;
    case PROP_SAVED:
 			g_value_set_boolean (value, TRUE);
      break;
    case PROP_CAN_PREVIEW:
 			g_value_set_boolean (value, FALSE);
      break;
    case PROP_ZOOM_LEVEL:
      p = webkit_web_view_get_zoom_level (docdet->help_view) * 100;
      g_value_set_uint (value, p);
      break;
    case PROP_TITLE:
      g_value_set_string (value, document_webkit_get_title(DOCUMENT_WEBKIT(object)));
      break;
    case PROP_CONTENT_TYPE:
			g_value_set_string (value, docdet->contenttype);
      break;
    case PROP_SHORT_FILENAME:
			g_value_set_string (value, docdet->short_filename);
      break;
    case PROP_GFILE:
      g_value_set_object (value, docdet->file);
      break;
    case PROP_LABEL:
      g_value_set_object (value, docdet->label);
      break;
    case PROP_ICON:
      g_value_set_object (value, docdet->ico);
      break;
    case PROP_WIDGET:
      g_value_set_object (value, docdet->container);
      break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
document_webkit_class_init (Document_WebkitClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = document_webkit_dispose;
  object_class->set_property = document_webkit_set_property;
  object_class->get_property = document_webkit_get_property;
  object_class->constructed = document_webkit_constructed;

	signals[LOAD_COMPLETE] =
		g_signal_new ("load_complete",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (Document_WebkitClass, load_complete),
		              NULL, NULL,
		               g_cclosure_marshal_VOID__BOOLEAN,
		               G_TYPE_NONE, 1, G_TYPE_BOOLEAN, NULL);

	signals[SAVE_UPDATE] =
		g_signal_new ("save_update",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (Document_WebkitClass, save_update),
		              NULL, NULL,
		               g_cclosure_marshal_VOID__VOID,
		               G_TYPE_NONE, 0);

  /*DOCUMENT_WEBKIT PROPERTIES*/
  g_object_class_install_property (object_class,
                              PROP_UNTITLED,
                              g_param_spec_boolean ("untitled",
                              NULL, NULL,
                              FALSE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  /* READ_ONLY PROPERTY:
  * When a document_webkit can't be saved. default value FALSE.
  */
  g_object_class_install_property (object_class,
                              PROP_READ_ONLY,
                              g_param_spec_boolean ("read_only",
                              NULL, NULL,
                              FALSE, G_PARAM_READWRITE));

  /* CAN_MODIFY PROPERTY: When a document_webkit can be modified */
  g_object_class_install_property (object_class,
                              PROP_CAN_MODIFY,
                              g_param_spec_boolean ("can_modify",
                              NULL, NULL,
                              TRUE, G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_CONVERTED_TO_UTF8,
                              g_param_spec_boolean ("converted_to_utf8",
                              NULL, NULL,
                              FALSE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
                              PROP_MTIME,
                              g_param_spec_int64 ("mtime",
                              NULL, NULL,
                               G_MININT64,
                               G_MAXINT64,
                               0,
                               G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
                              PROP_IS_EMPTY,
                              g_param_spec_boolean ("is_empty",
                              NULL, NULL,
                              FALSE, G_PARAM_READABLE));

  g_object_class_install_property (object_class,
                              PROP_SAVED,
                              g_param_spec_boolean ("saved",
                              NULL, NULL,
                              FALSE, G_PARAM_READABLE));

  g_object_class_install_property (object_class,
                              PROP_CAN_PREVIEW,
                              g_param_spec_boolean ("can_preview",
                              NULL, NULL,
                              FALSE, G_PARAM_READABLE));

  g_object_class_install_property (object_class,
                              PROP_ZOOM_LEVEL,
                              g_param_spec_uint ("zoom_level",
                              NULL, NULL, 0, G_MAXUINT, 
                              100, G_PARAM_READABLE));

  g_object_class_install_property (object_class,
                              PROP_TYPE,
                              g_param_spec_int ("type",
                              NULL, NULL, 0, G_MAXINT, 
                              TAB_FILE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
                              PROP_CONTENT_TYPE,
                              g_param_spec_string ("content_type",
                              NULL, NULL,
                              "text/plain", G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_SHORT_FILENAME,
                              g_param_spec_string ("short_filename",
                              NULL, NULL,
                              "", G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_TITLE,
                              g_param_spec_string ("title",
                              NULL, NULL,
                              "", G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_GFILE,
                              g_param_spec_object ("GFile",
                              NULL, NULL,
                              G_TYPE_FILE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
                              PROP_LABEL,
                              g_param_spec_object ("editor_label",
                              NULL, NULL,
                              GTK_TYPE_WIDGET, G_PARAM_READABLE));

  g_object_class_install_property (object_class,
                              PROP_ICON,
                              g_param_spec_object ("icon",
                              NULL, NULL,
                              G_TYPE_ICON, G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_WIDGET,
                              g_param_spec_object ("editor_widget",
                              NULL, NULL,
                              GTK_TYPE_WIDGET, G_PARAM_READABLE));

	g_type_class_add_private (klass, sizeof (Document_WebkitDetails));
}

static void
document_webkit_init (Document_Webkit * object)
{
	Document_WebkitDetails *docdet;
	docdet = DOCUMENT_WEBKIT_GET_PRIVATE(object);
  docdet->help_view= WEBKIT_WEB_VIEW(webkit_web_view_new ());
  docdet->help_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  docdet->label = gtk_label_new ("");

  gtk_widget_show (docdet->label);
  gtk_container_add(GTK_CONTAINER(docdet->help_scrolled_window), GTK_WIDGET(docdet->help_view));

  docdet->container = gtk_vbox_new (FALSE, 0);

  /* create incremental search widget */
  docdet->searchbar = search_infobar_new();
  gtk_box_pack_start(GTK_BOX(docdet->container), docdet->searchbar, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(docdet->container), docdet->help_scrolled_window, TRUE, TRUE, 0);

  g_signal_connect(G_OBJECT(docdet->help_view), "navigation-policy-decision-requested",
       G_CALLBACK(webkit_link_clicked), object);
  g_signal_connect (G_OBJECT(docdet->help_view), "notify::title", G_CALLBACK (notify_title_cb), object);

  gtk_widget_show_all(docdet->help_scrolled_window);
}

static void document_webkit_constructed (GObject *object)
{
  Document_WebkitDetails *docdet = DOCUMENT_WEBKIT_GET_PRIVATE(object);
  GString *caption = NULL;
  gchar *filename = documentable_get_filename(DOCUMENTABLE(object));
  if (docdet->type==TAB_HELP){
  caption = g_string_new(_("Help: "));
  } else {
  gchar *disp = filename_get_display_name(filename);
  caption = g_string_new(disp);
  g_free(disp);
  caption = g_string_prepend(caption, _("Preview: "));
  }

  docdet->short_filename = g_strdup(caption->str);
  gtk_label_set_text(GTK_LABEL(docdet->label), caption->str);

  webkit_web_view_load_uri (WEBKIT_WEB_VIEW(docdet->help_view), filename);

  gphpedit_debug_message (DEBUG_DOCUMENT, "WEBKIT FILE: %s\n", caption->str);

  g_string_free(caption,TRUE);

}
/*
* disposes the Gobject
*/
static void document_webkit_dispose (GObject *object)
{
  Document_Webkit *doc = DOCUMENT_WEBKIT(object);
  Document_WebkitDetails *docdet;
	docdet = DOCUMENT_WEBKIT_GET_PRIVATE(doc);
  /* free object resources*/
	if (docdet->short_filename) g_free(docdet->short_filename);
	if (docdet->file) g_object_unref(docdet->file);
	if (docdet->ico) g_object_unref(docdet->ico);
  if (docdet->contenttype) g_free(docdet->contenttype);
  /* Chain up to the parent class */
  G_OBJECT_CLASS (document_webkit_parent_class)->dispose (object);
}

Document_Webkit *document_webkit_new (gint type, GFile *file)
{
	Document_Webkit *doc;
  doc = g_object_new (DOCUMENT_WEBKIT_TYPE, "type", type, "GFile", file, NULL);

	return doc; /* return new object */
}

static void notify_title_cb (WebKitWebView* web_view, GParamSpec* pspec, Document_Webkit *document_webkit)
{
  Document_WebkitDetails *docdet = DOCUMENT_WEBKIT_GET_PRIVATE(document_webkit);
  gchar *main_title = g_strdup (webkit_web_view_get_title(web_view));
  if (main_title){
    if (docdet->short_filename) g_free(docdet->short_filename);
    if (docdet->type==TAB_HELP){
      docdet->short_filename = g_strconcat(_("Help: "), main_title, NULL);
    } else {
      docdet->short_filename = g_strconcat(_("Preview: "), main_title, NULL);
    }
    gtk_label_set_text(GTK_LABEL(docdet->label), docdet->short_filename);
    g_signal_emit (G_OBJECT (document_webkit), signals[SAVE_UPDATE], 0);
    g_free(main_title);
  }
}

/*
*/
static gboolean webkit_link_clicked (WebKitWebView *web_view, WebKitWebFrame *frame, WebKitNetworkRequest *request,
                                                        WebKitWebNavigationAction *navigation_action,
                                                        WebKitWebPolicyDecision   *policy_decision,
                                                        Document_Webkit *document_webkit)
{
  if (webkit_web_navigation_action_get_reason (navigation_action) != WEBKIT_WEB_NAVIGATION_REASON_LINK_CLICKED) return FALSE;
  webkit_web_view_load_request (web_view, request);
  return TRUE;
}
