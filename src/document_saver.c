/* This file is part of gPHPEdit, a GNOME2 PHP Editor.

   Copyright (C) 2009, 2010 Anoop John <anoop dot john at zyxware.com>
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

#include <glib/gi18n.h>
#include <gio/gio.h>
#include "debug.h"
#include "document_saver.h"

/*
* document_saver private struct
*/
struct DocumentSaverDetails
{
  Document *doc;
  gchar *write_buffer;
  GFile *file;
};

#define DOCUMENT_SAVER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    DOCUMENT_SAVER_TYPE,\
					    DocumentSaverDetails))

/*
 * register DocumentSaver type and returns a new GType
*/
G_DEFINE_TYPE(DocumentSaver, document_saver, G_TYPE_OBJECT);  

/* object signal enumeration */
enum {
	DONE_SAVING,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static void
document_saver_class_init (DocumentSaverClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);

/* if load is ok return TRUE. if load isn't complete return FALSE */
	signals[DONE_SAVING] =
		g_signal_new ("done_saving",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (DocumentSaverClass, done_saving),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__OBJECT,
		              G_TYPE_NONE, 1, DOCUMENT_TYPE);


	g_type_class_add_private (klass, sizeof (DocumentSaverDetails));
}

static void
document_saver_init (DocumentSaver * object)
{
}

DocumentSaver *document_saver_new (void)
{
	DocumentSaver *docsav;
  docsav = g_object_new (DOCUMENT_SAVER_TYPE, NULL);
	return docsav; /* return new object */
}

void _document_saver_update_modified_time(Document *doc, GFile *file)
{
  GTimeVal mtime;
  GFileInfo *info;
  GError *error=NULL;
  info= g_file_query_info (file,"time::modified,time::modified-usec", G_FILE_QUERY_INFO_NONE, NULL, &error);
  if (!info){
    g_warning (_("Could not get the file modification time. GIO error: %s\n"), error->message);
    g_error_free(error);
    g_get_current_time (&mtime); /*set current time*/
  } else {
  /* update modification time */
    g_file_info_get_modification_time (info, &mtime);
    g_object_unref(info);
  }
  document_set_mtime(doc, mtime);
}
void _document_saver_file_write (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  DocumentSaverDetails *docsavdet = DOCUMENT_SAVER_GET_PRIVATE(user_data);
  GError *error=NULL;
  GFile *file = (GFile *)source_object;
  g_free(docsavdet->write_buffer);
  if(!g_file_replace_contents_finish (file, res, NULL, &error)) {
    g_print("%s\n", error->message);
    g_error_free(error);
    return;
  }
  _document_saver_update_modified_time(docsavdet->doc, file);
  g_signal_emit (G_OBJECT (user_data), signals[DONE_SAVING], 0, docsavdet->doc);
}

gchar *_document_saver_convert_utf8_to_locale(gchar *text, gsize *size)
{
  if (!text) return NULL;
  gchar *result = NULL;
  gsize text_length = strlen(text);
  GError *error=NULL;

  /* If we converted to UTF-8 when loading, convert back to the locale to save */
  result = g_locale_from_utf8(text, text_length, NULL, size, &error);
  if (error != NULL) {
    g_error (_("UTF-8 Error: %s\n"), error->message);
    g_error_free(error);
    result = g_strdup(text);
    *size = strlen (result);
  }
  gphpedit_debug_message (DEBUG_DOCUMENT,"Converted size: %d\n", *size);
  return result;
}

void document_saver_save_document (DocumentSaver *docsav, Document *doc)
{
  gphpedit_debug (DEBUG_DOCUMENT);
  if (!docsav) return ;
  DocumentSaverDetails *docsavdet = DOCUMENT_SAVER_GET_PRIVATE(docsav);
  GFile *file;
  gsize text_length;
  gchar *text = document_get_text(doc);
  if (!text) return ;
  gboolean converted_to_utf8;
  g_object_get(doc, "converted_to_utf8", &converted_to_utf8, "GFile", &file, NULL);
  if (converted_to_utf8) {
    docsavdet->write_buffer = _document_saver_convert_utf8_to_locale(text, &text_length);
  } else {
    docsavdet->write_buffer = g_strdup(text);
    text_length= strlen(text);
  }
  g_free(text);
  docsavdet->doc = doc;
  g_file_replace_contents_async (file, docsavdet->write_buffer, text_length, NULL, 
    FALSE, G_FILE_CREATE_NONE, NULL, _document_saver_file_write, docsav);
}
