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

#include "document.h"

/*
* document_webkit private struct
*/
struct DocumentDetails
{
  GFile *file;
  GIcon *ico;
  gchar *short_filename;
  gint64 mtime;
  gboolean isreadonly;
  gboolean is_untitled;
  gboolean is_readonly;
  gchar *contenttype;
};


/*
 * register Document type and returns a new GType
*/
G_DEFINE_ABSTRACT_TYPE(Document, document, G_TYPE_OBJECT);

#define DOCUMENT_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    DOCUMENT_TYPE,\
					    DocumentDetails))

enum
{
  PROP_0,
  PROP_UNTITLED,
  PROP_READ_ONLY,
  PROP_CONTENT_TYPE,
  PROP_SHORT_FILENAME,
  PROP_GFILE,
  PROP_ICON,
  PROP_MTIME
};

static void
document_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
  DocumentDetails *docdet = DOCUMENT_GET_PRIVATE(object);

  switch (prop_id)
  {
    case PROP_MTIME:
      docdet->mtime = g_value_get_int64 (value);
      break;
    case PROP_READ_ONLY:
      docdet->is_readonly = g_value_get_boolean (value);
      break;
    case PROP_UNTITLED:
      docdet->is_untitled = g_value_get_boolean (value);
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
document_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
  DocumentDetails *docdet = DOCUMENT_GET_PRIVATE(object);
  
  switch (prop_id)
  {
    case PROP_READ_ONLY:
      g_value_set_boolean (value, docdet->is_readonly);
      break;
    case PROP_MTIME:
      g_value_set_int64 (value, docdet->mtime);
      break;
    case PROP_UNTITLED:
      g_value_set_boolean (value, docdet->is_untitled);
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
    case PROP_ICON:
      g_value_set_object (value, docdet->ico);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

/*
* disposes the Gobject
*/
static void document_dispose (GObject *object)
{
  Document *doc = DOCUMENT(object);
  DocumentDetails *docdet;
  docdet = DOCUMENT_GET_PRIVATE(doc);
  /* free object resources*/
  if (docdet->short_filename) g_free(docdet->short_filename);
  if (docdet->file) g_object_unref(docdet->file);
  if (docdet->ico) g_object_unref(docdet->ico);
  if (docdet->contenttype) g_free(docdet->contenttype);
  /* Chain up to the parent class */
  G_OBJECT_CLASS (document_parent_class)->dispose (object);
}

static void
document_class_init (DocumentClass *klass)
{
  GObjectClass *object_class;
  object_class = G_OBJECT_CLASS (klass);
  object_class->set_property = document_set_property;
  object_class->get_property = document_get_property;
  object_class->dispose = document_dispose;

  /*DOCUMENT_PROPERTIES*/
  g_object_class_install_property (object_class,
                              PROP_UNTITLED,
                              g_param_spec_boolean ("untitled",
                              NULL, NULL,
                              TRUE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
                              PROP_READ_ONLY,
                              g_param_spec_boolean ("read_only",
                              NULL, NULL,
                              FALSE, G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_MTIME,
                              g_param_spec_int64 ("mtime",
                              NULL, NULL,
                              G_MININT64,
                              G_MAXINT64,
                              0,
                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

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
                              PROP_GFILE,
                              g_param_spec_object ("GFile",
                              NULL, NULL,
                              G_TYPE_FILE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class,
                              PROP_ICON,
                              g_param_spec_object ("icon",
                              NULL, NULL,
                              G_TYPE_ICON, G_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (DocumentDetails));
}

static void
document_init (Document * object)
{
}
