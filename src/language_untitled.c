/* This file is part of gPHPEdit, a GNOME PHP Editor.
 
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009, 2011 José Rostagno (for vijona.com.ar)
	  
   For more information or to find the latest release, visit our 
   website at http://www.guntitlededit.org/
 
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

#include <string.h>
#include "debug.h"
#include "language_untitled.h"
#include "preferences_manager.h"
#include "language_provider.h"
#include "symbol_manager.h"

/*
* language_untitled private struct
*/
struct Language_UntitledDetails
{
  Documentable *doc;
  GtkScintilla *sci;
};

#define LANGUAGE_UNTITLED_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    LANGUAGE_UNTITLED_TYPE,\
					    Language_UntitledDetails))
enum
{
  PROP_0,
  PROP_DOCUMENT_SCINTILLA
};

static void language_untitled_language_provider_init(Language_ProviderInterface *iface, gpointer user_data);
static void language_untitled_trigger_completion (Language_Provider *lguntitled, guint ch);
static void language_untitled_setup_lexer(Language_Provider *lguntitled);
static gchar *language_untitled_do_syntax_check(Language_Provider *lguntitled);

G_DEFINE_TYPE_WITH_CODE(Language_Untitled, language_untitled, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE (IFACE_TYPE_LANGUAGE_PROVIDER,
                                                 language_untitled_language_provider_init));

static void show_calltip (Language_Provider *lgcss)
{
 /* not implemented */
}

static void language_untitled_language_provider_init(Language_ProviderInterface *iface, gpointer user_data)
{
  iface->trigger_completion = language_untitled_trigger_completion;
  iface->show_calltip = show_calltip;
  iface->setup_lexer = language_untitled_setup_lexer;
  iface->do_syntax_check = language_untitled_do_syntax_check;
}

static void
language_untitled_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
  Language_UntitledDetails *lguntitleddet;
  lguntitleddet = LANGUAGE_UNTITLED_GET_PRIVATE(object);

  switch (prop_id)
  {
    case PROP_DOCUMENT_SCINTILLA:
      if (lguntitleddet->doc) g_object_unref(lguntitleddet->doc);
      lguntitleddet->doc = g_value_dup_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
language_untitled_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
  Language_UntitledDetails *lguntitleddet;
  lguntitleddet = LANGUAGE_UNTITLED_GET_PRIVATE(object);

  
  switch (prop_id)
  {
    case PROP_DOCUMENT_SCINTILLA:
      g_value_set_object (value, lguntitleddet->doc);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}


/*
* disposes the Gobject
*/
static void language_untitled_dispose (GObject *object)
{
  Language_UntitledDetails *lguntitleddet;
  lguntitleddet = LANGUAGE_UNTITLED_GET_PRIVATE(object);
  g_object_unref(lguntitleddet->doc);
  /* Chain up to the parent class */
  G_OBJECT_CLASS (language_untitled_parent_class)->dispose (object);
}

static void language_untitled_constructed (GObject *object)
{
  Language_UntitledDetails *lguntitleddet;
  lguntitleddet = LANGUAGE_UNTITLED_GET_PRIVATE(object);
  GtkScintilla *sci;
  g_object_get(lguntitleddet->doc, "scintilla", &sci, NULL);
  lguntitleddet->sci = sci;
}

static void
language_untitled_class_init (Language_UntitledClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = language_untitled_dispose;
  object_class->set_property = language_untitled_set_property;
  object_class->get_property = language_untitled_get_property;
  object_class->constructed = language_untitled_constructed;
  g_object_class_install_property (object_class,
                              PROP_DOCUMENT_SCINTILLA,
                              g_param_spec_object ("document_scintilla",
                              NULL, NULL,
                              DOCUMENT_SCINTILLA_TYPE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_type_class_add_private (klass, sizeof (Language_UntitledDetails));
}

static void
language_untitled_init (Language_Untitled * object)
{
//  Language_UntitledDetails *lguntitleddet;
//  lguntitleddet = LANGUAGE_UNTITLED_GET_PRIVATE(object);
}

Language_Untitled *language_untitled_new (Document_Scintilla *doc)
{
  Language_Untitled *lguntitled;
  lguntitled = g_object_new (LANGUAGE_UNTITLED_TYPE, "document_scintilla", doc, NULL);
  return lguntitled; /* return new object */
}

static void language_untitled_trigger_completion (Language_Provider *lguntitled, guint ch)
{
  g_return_if_fail(lguntitled);
  Language_UntitledDetails *lguntitleddet = LANGUAGE_UNTITLED_GET_PRIVATE(lguntitled);
  gint current_pos;
  gint wordStart;
  gint wordEnd;
  gchar *member_function_buffer = NULL;
  gint member_function_length;
  current_pos = gtk_scintilla_get_current_pos(lguntitleddet->sci);
  wordStart = gtk_scintilla_word_start_position(lguntitleddet->sci, current_pos-1, TRUE);
  wordEnd = gtk_scintilla_word_end_position(lguntitleddet->sci, current_pos-1, TRUE);

  member_function_buffer = gtk_scintilla_get_text_range (lguntitleddet->sci, wordStart-2, wordEnd, &member_function_length);
  /* if we type <?untitled then we are in a untitled file so force untitled syntax mode */
  if (g_strcmp0(member_function_buffer,"<?untitled")==0) {
    documentable_set_type(DOCUMENTABLE(lguntitleddet->doc), TAB_PHP);
  }
  g_free(member_function_buffer);
}

static gchar *language_untitled_do_syntax_check(Language_Provider *lguntitled)
{
  return NULL;
}

static void language_untitled_setup_lexer(Language_Provider *lguntitled)
{
  g_return_if_fail(lguntitled);
  Language_UntitledDetails *lguntitleddet = LANGUAGE_UNTITLED_GET_PRIVATE(lguntitled);

  gtk_scintilla_clear_document_style (lguntitleddet->sci);
  /* SCLEX_NULL to select no lexing action */
  gtk_scintilla_set_lexer(GTK_SCINTILLA (lguntitleddet->sci), SCLEX_NULL); 
  gtk_scintilla_colourise(lguntitleddet->sci, 0, -1);
}
