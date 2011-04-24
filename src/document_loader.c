/* This file is part of gPHPEdit, a GNOME2 PHP Editor.

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

#include "debug.h"
#include "document_loader.h"
#include "tab.h"
#include "gvfs_utils.h"

#define IS_MIME(stringa,stringb) (g_content_type_equals (stringa, stringb))
#define IS_TEXT(stringa) (g_content_type_is_a (stringa, "text/*"))
#define IS_APPLICATION(stringa) (g_content_type_is_a (stringa, "application/*") && !IS_MIME(stringa,"application/x-php") && !IS_MIME(stringa,"application/javascript") && !IS_MIME(stringa,"application/x-perl"))


/*
* document_loader private struct
*/
struct DocumentLoaderDetails
{
  GtkWindow *dialog_parent_window;
  GMountOperation *gmo;
  gint type;
  guint goto_line;

  /**/
  Document *doc;
  gint current_action;
};

#define DOCUMENT_LOADER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    DOCUMENT_LOADER_TYPE,\
					    DocumentLoaderDetails))

static void document_loader_dispose (GObject *gobject);
static void _document_loader_create_file(DocumentLoader *doclod, gchar *filename, gint goto_line);
static void _document_loader_load_file(DocumentLoader *doclod, GFile *file);
/*
 * register DocumentLoader type and returns a new GType
*/
G_DEFINE_TYPE(DocumentLoader, document_loader, G_TYPE_OBJECT);  

/* object signal enumeration */
enum {
  DONE_LOADING,
  DONE_NAVIGATE,
  DONE_REFRESH,
  NEED_MOUNTING,
  HELP_FILE_NOT_FOUND,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

enum
{
  PROP_0,
  PROP_PARENT_WINDOW
};

/* actions */
enum {
  LOAD,
  NAVIGATE,
  REFRESH
};

static void
document_loader_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(object);

  switch (prop_id)
  {
    case PROP_PARENT_WINDOW:
      docloddet->dialog_parent_window = g_value_dup_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
document_loader_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
  switch (prop_id)
  {
    case PROP_PARENT_WINDOW:
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}
static void
document_loader_class_init (DocumentLoaderClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = document_loader_dispose;
  object_class->set_property = document_loader_set_property;
  object_class->get_property = document_loader_get_property;

/* if load is ok return TRUE. if load isn't complete return FALSE */
	signals[DONE_LOADING] =
		g_signal_new ("done_loading",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (DocumentLoaderClass, done_loading),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__UINT_POINTER,
	                G_TYPE_NONE, 2, G_TYPE_BOOLEAN, DOCUMENT_TYPE, NULL);

	signals[DONE_REFRESH] =
		g_signal_new ("done_refresh",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (DocumentLoaderClass, done_navigate),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__BOOLEAN,
		              G_TYPE_NONE, 1, G_TYPE_BOOLEAN, NULL);

	signals[NEED_MOUNTING] =
		g_signal_new ("need_mounting",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (DocumentLoaderClass, need_mounting),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);

	signals[HELP_FILE_NOT_FOUND] =
		g_signal_new ("help_file_not_found",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (DocumentLoaderClass, need_mounting),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);

  g_object_class_install_property (object_class,
                              PROP_PARENT_WINDOW,
                              g_param_spec_object ("parent_window",
                              "The dialog parent window", NULL,
                              GTK_TYPE_WIDGET, G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));


  g_type_class_add_private (klass, sizeof (DocumentLoaderDetails));
}

static void
document_loader_init (DocumentLoader * object)
{
  DocumentLoaderDetails *docloddet;
  docloddet = DOCUMENT_LOADER_GET_PRIVATE(object);
  docloddet->gmo= gtk_mount_operation_new(docloddet->dialog_parent_window);
}

/*
* disposes the Gobject
*/
static void document_loader_dispose (GObject *object)
{
  DocumentLoader *doclod = DOCUMENT_LOADER(object);
  DocumentLoaderDetails *docloddet;
  docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  /* free object resources*/
  g_object_unref(docloddet->dialog_parent_window);
  if (docloddet->gmo) g_object_unref(docloddet->gmo);

  /* Chain up to the parent class */
  G_OBJECT_CLASS (document_loader_parent_class)->dispose (object);
}

DocumentLoader *document_loader_new (GtkWindow *dialog_parent_window)
{
  DocumentLoader *doclod;
  doclod = g_object_new (DOCUMENT_LOADER_TYPE, "parent_window", dialog_parent_window, NULL);
  return doclod; /* return new object */
}

static void _document_loader_create_untitled(DocumentLoader *doclod)
{
  Document_Scintilla *doc = document_scintilla_new (TAB_FILE, NULL, 0, NULL);
  g_object_set(doc, "icon", g_themed_icon_new ("text-plain"), NULL);
  g_signal_emit (G_OBJECT (doclod), signals[DONE_LOADING], 0, TRUE, DOCUMENT(doc));
}

#ifdef PHP_DOC_DIR
static GString *_document_loader_help_file_try_filename(gchar *prefix, gchar *command, gchar *suffix)
{
  GString *long_filename;
        
  long_filename = g_string_new(command);
  long_filename = g_string_prepend(long_filename, prefix);
  if (suffix) {
    long_filename = g_string_append(long_filename, suffix);
  }
  gphpedit_debug_message (DEBUG_DOCUMENT,"filename: %s", long_filename->str);

  if (filename_file_exist(long_filename->str)){
    return long_filename;
  }
  else {
    g_string_free(long_filename, TRUE);
  }
  long_filename = g_string_new(command);
  str_replace(long_filename->str, '_', '-');
  long_filename = g_string_prepend(long_filename, prefix);
  if (suffix) {
    long_filename = g_string_append(long_filename, suffix);
  }
  gphpedit_debug_message (DEBUG_DOCUMENT,"filename: %s", long_filename->str);

  if (filename_file_exist(long_filename->str)){
    return long_filename;
  }
  g_string_free(long_filename, TRUE);
  return NULL;
}
#endif

static GString *tab_help_find_helpfile(gchar *command)
{
  GString *long_filename = NULL;
  if (!command) return long_filename;
#ifdef PHP_DOC_DIR
 //FIX: avoid duplicated call
 if (strstr(command,"/usr/")!=NULL) return long_filename;
 gchar *temp= NULL;
 temp=g_strdup_printf ("%s/%s",PHP_DOC_DIR,"function.");
 long_filename = _document_loader_help_file_try_filename(temp, command, ".html");
 g_free(temp);
 if (long_filename) return long_filename;
 temp=g_strdup_printf ("%s/%s",PHP_DOC_DIR,"ref.");
 long_filename = _document_loader_help_file_try_filename(temp, command, ".html");
 g_free(temp);
 if (long_filename) return long_filename;
 temp=g_strdup_printf ("%s/",PHP_DOC_DIR);
 long_filename = _document_loader_help_file_try_filename(temp, command, NULL);
 g_free(temp);
 if (long_filename) return long_filename;
  gphpedit_debug_message (DEBUG_DOCUMENT,_("Help for function not found: %s"), command);
 return long_filename;
#else
  long_filename = g_string_new("http://www.php.net/manual/en/function.");
  long_filename = g_string_append(long_filename, command);
  long_filename = g_string_append(long_filename, ".php");
  GFile *temp= g_file_new_for_commandline_arg(long_filename->str);
  if(g_file_query_exists(temp,NULL)){
  g_object_unref(temp);
  return long_filename;
  }else{
  g_object_unref(temp);
  gphpedit_debug_message (DEBUG_DOCUMENT,_("Help for function not found: %s"), command);
  return NULL;
  }
#endif
}

static gboolean _document_loader_validate_and_convert_utf8_buffer (gchar **buffer) //FIXME: multiple enconding support
{
  gboolean result;
  if (g_utf8_validate(*buffer, -1, NULL)) {
    gphpedit_debug_message (DEBUG_DOCUMENT,"%s", "Valid UTF8");
    result = FALSE;
  } else {
    gchar *converted_text;
    gsize utf8_size;
    GError *error = NULL;      
    converted_text = g_locale_to_utf8(*buffer, -1, NULL, &utf8_size, &error); 
    if (error != NULL) {
      /* if locale isn't set */
      g_error_free(error);
      error = NULL;
      converted_text = g_convert(*buffer, -1, "UTF-8", "ISO-8859-15", NULL, &utf8_size, &error);
      if (error!= NULL) {
        gphpedit_debug_message (DEBUG_DOCUMENT,_("gPHPEdit UTF-8 Error: %s"), error->message);
        g_error_free(error);
        return FALSE;
      }
    }
    gphpedit_debug_message (DEBUG_DOCUMENT,_("Converted to UTF-8 size: %u"), utf8_size);
    g_free(*buffer);
    *buffer = converted_text;
    result = TRUE;
  }
  return result;
}

/*
* _document_loader_help_file_load
* loads a new help file async
*/
static void _document_loader_help_file_load(DocumentLoader *doclod, GFile *file)
{
  gphpedit_debug (DEBUG_DOCUMENT);
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  GFileInfo *info;
  GError *error=NULL;
  info=g_file_query_info (file,"standard::icon", 0, NULL, &error);
  if (!info){
    g_warning ("%s\n", error->message);
    g_signal_emit (G_OBJECT (doclod), signals[DONE_LOADING], 0, FALSE, NULL);
    return ;
  }
  GIcon *icon= g_file_info_get_icon (info); /* get Gicon for mimetype*/

  /* Open file*/
  /*it's all ok, read file*/
  Document_Webkit *document = document_webkit_new (docloddet->type, file);
  g_object_set(document, "icon", icon, NULL);
  g_object_unref(info);
  g_signal_emit (G_OBJECT (doclod), signals[DONE_LOADING], 0, TRUE, DOCUMENT(document));
}

static void _document_loader_create_help(DocumentLoader *doclod, gchar *help_function)
{
  GString *long_filename = NULL;
  if (!g_str_has_prefix(help_function, "http") && !g_str_has_prefix(help_function, "file:")){
  long_filename = tab_help_find_helpfile(help_function);
  } else {
  long_filename = g_string_new(help_function);
  }
  if (!long_filename) {
    g_signal_emit (G_OBJECT (doclod), signals[HELP_FILE_NOT_FOUND], 0);
    return ;
  } else {
    gphpedit_debug_message (DEBUG_DOCUMENT,"filename:%s", long_filename->str);
    GFile *file = g_file_new_for_commandline_arg(long_filename->str);
    _document_loader_help_file_load(doclod, file);
  }
}

static void _document_loader_create_preview(DocumentLoader *doclod, gchar *filename)
{
  gphpedit_debug_message (DEBUG_DOCUMENT,"filename:%s", filename);
  GFile *file=g_file_new_for_commandline_arg(filename);
  _document_loader_help_file_load(doclod, file);
}

static gboolean _document_loader_prompt_create_file(const gchar *filename)
{
  if (!filename) return TRUE;
  gint result;
  gchar *message = g_strdup_printf(_("The file %s was not found.\n\nWould you like to create it as an empty document?"), filename);
  result = yes_no_dialog(_("File not found"), message);
  g_free(message);
  if (result != GTK_RESPONSE_YES){
    return FALSE;
  }
  return TRUE;
}

static gboolean _document_loader_check_supported_type(GFile *file)
{
  GFileInfo *info;
  GError *error=NULL;
  gboolean result = TRUE;

  info= g_file_query_info (file,"standard::content-type",G_FILE_QUERY_INFO_NONE, NULL,&error);
  if (!info){
    g_warning (_("Could not get the file info. GIO error: %s \n"), error->message);
    g_error_free(error);
    return FALSE;
  }
  const char *contenttype= g_file_info_get_content_type (info); 
  /*we could open text based types so if it not a text based content don't open and displays error*/
  if (!IS_TEXT(contenttype) && IS_APPLICATION(contenttype)){
    info_dialog (_("gPHPEdit"), _("Sorry, I can open this kind of file.\n"));
    result = FALSE;
  }
  g_object_unref(info);
  return result;
}

static void openfile_mount(GObject *source_object, GAsyncResult *res, gpointer user_data) {
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(user_data);

  GError *error=NULL;
  if (!g_file_mount_enclosing_volume_finish((GFile *)source_object,res,&error)) {
    g_error("%s\n",error->message);
    g_error_free(error);
    g_signal_emit (G_OBJECT (user_data), signals[DONE_LOADING], 0, FALSE);
  }
    /* open again */
  docloddet->current_action = LOAD;
  if (!_document_loader_check_supported_type((GFile *)source_object)){
    g_signal_emit (G_OBJECT (user_data), signals[DONE_LOADING], 0, FALSE, NULL); 
    return ;
  }
  _document_loader_load_file(user_data, (GFile *)source_object);
}

static gboolean mount_mountable_file(DocumentLoader *doclod, GFile *file)
{
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  gphpedit_debug (DEBUG_DOCUMENT);
  GError *error=NULL;
  GMount *ex= g_file_find_enclosing_mount (file,NULL,&error);
  if (!ex){
    if (error->code == G_IO_ERROR_NOT_MOUNTED){
      g_signal_emit (G_OBJECT (doclod), signals[NEED_MOUNTING], 0);
      g_file_mount_enclosing_volume (file, G_MOUNT_MOUNT_NONE, docloddet->gmo, NULL, openfile_mount, doclod);
      g_error_free(error);
      return TRUE;
    }
    g_error("%s\n", error->message);
    g_error_free(error);
    return FALSE;
  }
  return TRUE;
}

#define INFO_FLAGS "standard::display-name,standard::content-type,standard::edit-name,standard::size,access::can-write,standard::icon,time::modified,time::modified-usec"

static void emit_signal (DocumentLoader *doclod, gboolean result, Document *doc) {
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  if (docloddet->current_action == LOAD) {
   g_signal_emit (G_OBJECT (doclod), signals[DONE_LOADING], 0, result, doc);
  } else {
   g_signal_emit (G_OBJECT (doclod), signals[DONE_REFRESH], 0, result);
  }
}

static void _document_loader_load_file_finish (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  DocumentLoader *doclod = (DocumentLoader *) user_data;
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  GError *error=NULL;
  gsize size;
  gchar *buffer;
  GFileInfo *info;
  gboolean converted_to_utf8 = FALSE;
  GFile *file = (GFile *) source_object;
  if (!g_file_load_contents_finish (file, res, &buffer, &size, NULL, &error)) {
    g_print("Error reading file. Gio error:%s",error->message);
    g_error_free(error);
    emit_signal (doclod, FALSE, NULL);
    return ;
  }
  gphpedit_debug_message (DEBUG_DOCUMENT,"Loaded %u bytes", size);
  gphpedit_debug_message (DEBUG_DOCUMENT,"BUFFER=\n%s\n-------------------------------------------", buffer);

  if (size) converted_to_utf8 = _document_loader_validate_and_convert_utf8_buffer(&buffer);

  info = g_file_query_info (file, INFO_FLAGS,G_FILE_QUERY_INFO_NONE, NULL, &error);
  if (!info){
    g_warning ("%s\n", error->message);
    emit_signal (doclod, FALSE, NULL);
    return;
  }
  documentable_replace_text (DOCUMENTABLE(docloddet->doc), buffer);
  g_object_set(docloddet->doc, "converted_to_utf8", converted_to_utf8, NULL);
  g_object_set(docloddet->doc, "read_only", !g_file_info_get_attribute_boolean (info,"access::can-write"), "can_modify", TRUE, NULL);
  g_object_set(docloddet->doc,"content_type", g_file_info_get_content_type (info),NULL);
  GIcon *icon = g_file_info_get_icon (info); /* get Gicon for mimetype*/
  g_object_set(docloddet->doc, "icon", icon, NULL);
  GTimeVal file_mtime;
  g_file_info_get_modification_time (info,&file_mtime);
  gint64 time = (((gint64) file_mtime.tv_sec) * G_USEC_PER_SEC) + file_mtime.tv_usec;
  g_object_set(docloddet->doc, "mtime", time, NULL);
  g_object_unref(info);
  emit_signal (doclod, TRUE, DOCUMENT(docloddet->doc));
}

static void _document_loader_load_file(DocumentLoader *doclod, GFile *file)
{
  /* Open file*/
  g_file_load_contents_async (file, NULL, _document_loader_load_file_finish, doclod); //FIXME:: cancellable???
}

static void _document_loader_create_file(DocumentLoader *doclod, gchar *filename, gint goto_line)
{
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  gphpedit_debug_message (DEBUG_DOCUMENT,"filename:%s", filename);
  docloddet->goto_line = goto_line;
  GFile *file = g_file_new_for_commandline_arg(filename);

  if (!GFile_is_local_or_http(file)){
    mount_mountable_file(doclod, file);
    return;
  }
  if(!g_file_query_exists (file,NULL)) {
    if (!_document_loader_prompt_create_file(filename)){
      g_signal_emit (G_OBJECT (doclod), signals[DONE_LOADING], 0, FALSE, NULL);
    } else {
      _document_loader_create_untitled(doclod);
    }
    return;
  }
  if (!_document_loader_check_supported_type(file)){
    g_signal_emit (G_OBJECT (doclod), signals[DONE_LOADING], 0, FALSE, NULL); 
    return ;
  }
  docloddet->current_action = LOAD;
  docloddet->doc = DOCUMENT(document_scintilla_new (docloddet->type, file, 0, NULL));
  _document_loader_load_file(doclod, file);
}

void document_loader_load (DocumentLoader *doclod, gint type, gchar *filename, gint goto_line)
{
  if (!doclod) return;
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  docloddet->type = type;
  if (!filename){
     /* create a new untitled and exit */
    _document_loader_create_untitled(doclod);
    return ;
  }
  switch (type) {
    case TAB_HELP:
      _document_loader_create_help(doclod, filename);
      break;
    case TAB_PREVIEW:
      _document_loader_create_preview(doclod, filename);
      break;
    default:
      _document_loader_create_file(doclod, filename, goto_line);
      break;
  }
}

void document_loader_reload_file(DocumentLoader *doclod, Document *doc)
{
  gphpedit_debug (DEBUG_DOCUMENT);
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);

  GFile *file;
  g_object_get(doc, "GFile", &file,NULL);

  docloddet->current_action = REFRESH;
  docloddet->doc = doc;
  //FIXME: get current_line, store it and jump after reload
  _document_loader_load_file(doclod, file);
}
