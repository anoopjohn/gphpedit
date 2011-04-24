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

#include <stdlib.h>
#include <string.h>

#include "gphpedit-session.h"
 
struct _GphpeditSessionPriv {
	gchar *dir_path;
	GKeyFile *key_file;
};

G_DEFINE_TYPE(GphpeditSession, gphpedit_session, G_TYPE_OBJECT)

static void gphpedit_session_constructed (GObject *object);

#define GPHPEDIT_SESSION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    GPHPEDIT_TYPE_SESSION,\
					    GphpeditSessionPriv))

enum
{
  PROP_0,
  PROP_SESSION_DIRECTORY
};

static void
gphpedit_session_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(object);

	switch (prop_id)
	{
	case PROP_SESSION_DIRECTORY:
		g_free(priv->dir_path);
		priv->dir_path = g_value_dup_string (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gphpedit_session_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(object);


	switch (prop_id)
	{
	case PROP_SESSION_DIRECTORY:
		g_value_set_string (value, priv->dir_path);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gphpedit_session_finalize (GObject *object)
{
	GphpeditSession *cobj;
	cobj = GPHPEDIT_SESSION (object);

	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(object);
	
	g_free (priv->dir_path);
	g_key_file_free (priv->key_file);

	G_OBJECT_CLASS(gphpedit_session_parent_class)->finalize(object);
}

static void
gphpedit_session_class_init (GphpeditSessionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gphpedit_session_finalize;
	object_class->set_property = gphpedit_session_set_property;
	object_class->get_property = gphpedit_session_get_property;
	object_class->constructed = gphpedit_session_constructed;

	g_object_class_install_property (object_class,
								PROP_SESSION_DIRECTORY,
								g_param_spec_string ("session_directory",
								NULL, NULL,
								"", G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));


	g_type_class_add_private (klass, sizeof (GphpeditSessionPriv));
}

static void gphpedit_session_constructed (GObject *object)
{
	gchar *filename;
	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(object);

	priv->key_file = g_key_file_new ();
	
	filename = gphpedit_session_get_session_filename (GPHPEDIT_SESSION(object));
	g_key_file_load_from_file (priv->key_file, filename,
							   G_KEY_FILE_NONE, NULL);
	g_free (filename);

}

static void
gphpedit_session_init (GphpeditSession *obj)
{
	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(obj);
	priv->dir_path = NULL;
}

/**
 * gphpedit_session_new:
 * @session_directory: Directory where session is loaded from/saved to.
 * 
 * Created a new session object. @session_directory is the directory
 * where session information will be stored or loaded in case of existing
 * session.
 * 
 * Returns: an #GphpeditSession Object
 */
GphpeditSession*
gphpedit_session_new (const gchar *session_directory)
{
	GphpeditSession *obj;
	
	g_return_val_if_fail (session_directory != NULL, NULL);
	g_return_val_if_fail (g_path_is_absolute (session_directory), NULL);

	obj = GPHPEDIT_SESSION (g_object_new (GPHPEDIT_TYPE_SESSION, "session_directory", session_directory, NULL));
	 
	return obj;
}

/**
 * gphpedit_session_get_session_directory:
 * @session: an #GphpeditSession object
 * 
 * Returns the directory corresponding to this session object.
 * 
 * Returns: session directory
 */
const gchar*
gphpedit_session_get_session_directory (GphpeditSession *session)
{
	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(session);
	return priv->dir_path;
}

/**
 * gphpedit_session_get_session_filename:
 * @session: an #GphpeditSession object
 * 
 * Gets the session filename corresponding to this session object.
 * 
 * Returns: session (absolute) filename
 */
gchar*
gphpedit_session_get_session_filename (GphpeditSession *session)
{
	g_return_val_if_fail (GPHPEDIT_IS_SESSION (session), NULL);

	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(session);

	return g_build_filename (priv->dir_path,
							 "gphpedit.session", NULL);
}

/**
 * gphpedit_session_sync:
 * @session: an #GphpeditSession object
 * 
 * Synchronizes session object with session file
 */
void
gphpedit_session_sync (GphpeditSession *session)
{
	gchar *filename, *data;
	
	g_return_if_fail (GPHPEDIT_IS_SESSION (session));

	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(session);

	filename = gphpedit_session_get_session_filename (session);
	data = g_key_file_to_data (priv->key_file, NULL, NULL);
	g_file_set_contents (filename, data, -1, NULL);
	
	g_free (filename);
	g_free (data);
}

/**
 * gphpedit_session_clear:
 * @session: an #GphpeditSession object
 * 
 * Clears the session.
 */
void
gphpedit_session_clear (GphpeditSession *session)
{
	gchar *cmd;
	gchar *quoted;
	gint ret;
	
	g_return_if_fail (GPHPEDIT_IS_SESSION (session));

	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(session);
	
	g_key_file_free (priv->key_file);
	priv->key_file = g_key_file_new ();
	
	gphpedit_session_sync (session);

	quoted = g_shell_quote (priv->dir_path);	
	cmd = g_strconcat ("rm -fr ", quoted, NULL);
	ret = system (cmd);
	g_free (cmd);

	cmd = g_strconcat ("mkdir -p ", quoted, NULL);
	ret = system (cmd);
	g_free (cmd);
	g_free (quoted);
}

/**
 * gphpedit_session_clear_section:
 * @session: an #GphpeditSession object.
 * @section: Section to clear.
 * 
 * Clears the given section in session object.
 */
void
gphpedit_session_clear_section (GphpeditSession *session,
							  const gchar *section)
{	
	g_return_if_fail (GPHPEDIT_IS_SESSION (session));
	g_return_if_fail (section != NULL);

	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(session);

	g_key_file_remove_group (priv->key_file, section, NULL);
}
 
/**
 * gphpedit_session_set_int:
 * @session: an #GphpeditSession object
 * @section: Section.
 * @key: Key name.
 * @value: Key value
 * 
 * Set an integer @value to @key in given @section.
 */
void
gphpedit_session_set_int (GphpeditSession *session, const gchar *section,
						const gchar *key, gint value)
{
	g_return_if_fail (GPHPEDIT_IS_SESSION (session));
	g_return_if_fail (section != NULL);
	g_return_if_fail (key != NULL);

	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(session);
	
	if (!value)
	{
		g_key_file_remove_key (priv->key_file, section, key, NULL);
		return;
	}
	
	g_key_file_set_integer (priv->key_file, section, key, value);
}

/**
 * gphpedit_session_set_boolean:
 * @session: an #GphpeditSession object
 * @section: Section.
 * @key: Key name.
 * @value: Key value
 * 
 * Set an integer @value to @key in given @section.
 */
void
gphpedit_session_set_boolean (GphpeditSession *session, const gchar *section,
						const gchar *key, gboolean value)
{
	g_return_if_fail (GPHPEDIT_IS_SESSION (session));
	g_return_if_fail (section != NULL);
	g_return_if_fail (key != NULL);

	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(session);

	g_key_file_set_boolean (priv->key_file, section, key, value);
}

/**
 * gphpedit_session_set_float:
 * @session: an #GphpeditSession object
 * @section: Section.
 * @key: Key name.
 * @value: Key value
 * 
 * Set a float @value to @key in given @section.
 */
void
gphpedit_session_set_float (GphpeditSession *session, const gchar *section,
						  const gchar *key, gfloat value)
{
	g_return_if_fail (GPHPEDIT_IS_SESSION (session));
	g_return_if_fail (section != NULL);
	g_return_if_fail (key != NULL);

	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(session);
	
	if (!value)
	{
		g_key_file_remove_key (priv->key_file, section, key, NULL);
		return;
	}
	
	g_key_file_set_double (priv->key_file, section, key, value);
}

/**
 * gphpedit_session_set_string:
 * @session: an #GphpeditSession object
 * @section: Section.
 * @key: Key name.
 * @value: Key value
 * 
 * Set a string @value to @key in given @section.
 */
void
gphpedit_session_set_string (GphpeditSession *session, const gchar *section,
						   const gchar *key, const gchar *value)
{
	g_return_if_fail (GPHPEDIT_IS_SESSION (session));
	g_return_if_fail (section != NULL);
	g_return_if_fail (key != NULL);

	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(session);
	
	if (!value)
	{
		g_key_file_remove_key (priv->key_file, section, key, NULL);
		return;
	}
	
	g_key_file_set_string (priv->key_file, section, key, value);
}

/**
 * gphpedit_session_set_string_list:
 * @session: an #GphpeditSession object
 * @section: Section.
 * @key: Key name.
 * @value: Key value
 * 
 * Set a list of strings @value to @key in given @section.
 */
void
gphpedit_session_set_string_list (GphpeditSession *session,
								const gchar *section,
								const gchar *key, GSList *value)
{
	gchar *value_str;
	GString *str;
	GSList *node;
	gboolean first_item = TRUE;
	
	g_return_if_fail (GPHPEDIT_IS_SESSION (session));
	g_return_if_fail (section != NULL);
	g_return_if_fail (key != NULL);

	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(session);
	
	if (!value)
	{
		g_key_file_remove_key (priv->key_file, section, key, NULL);
		return;
	}
	
	str = g_string_new ("");
	node = value;
	while (node)
	{
		if (node->data && strlen (node->data) > 0)
		{
			if (first_item)
				first_item = FALSE;
			else
				g_string_append (str, "%%%");
			g_string_append (str, node->data);
		}
		node = g_slist_next (node);
	}
	
	value_str = g_string_free (str, FALSE);
	g_key_file_set_string (priv->key_file, section, key, value_str);
	
	g_free (value_str);
}

/**
 * gphpedit_session_get_int:
 * @session: an #GphpeditSession object
 * @section: Section.
 * @key: Key name.
 * 
 * Get an integer @value of @key in given @section.
 * 
 * Returns: Key value
 */
gint
gphpedit_session_get_int (GphpeditSession *session, const gchar *section,
						const gchar *key)
{
	gint value;
	
	g_return_val_if_fail (GPHPEDIT_IS_SESSION (session), 0);
	g_return_val_if_fail (section != NULL, 0);
	g_return_val_if_fail (key != NULL, 0);

	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(session);
	
	value = g_key_file_get_integer (priv->key_file, section, key, NULL);
	
	return value;
}

/**
 * gphpedit_session_get_int_with_default:
 * @session: an #GphpeditSession object
 * @section: Section.
 * @key: Key name.
 * 
 * Get an integer @value of @key in given @section.
 * If key isn't found return default_value
 * 
 * Returns: Key value
 */
gint
gphpedit_session_get_int_with_default (GphpeditSession *session, const gchar *section,
						const gchar *key, gint default_value)
{
	gint value;
	GError *error = NULL;

	g_return_val_if_fail (GPHPEDIT_IS_SESSION (session), 0);
	g_return_val_if_fail (section != NULL, 0);
	g_return_val_if_fail (key != NULL, 0);

	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(session);
	
	value = g_key_file_get_integer (priv->key_file, section, key, &error);
	if (error)
	{
		value = default_value;
		g_error_free(error);
	}
	
	return value;
}

/**
 * gphpedit_session_get_boolean:
 * @session: an #GphpeditSession object
 * @section: Section.
 * @key: Key name.
 * 
 * Get a boolean @value of @key in given @section.
 * 
 * Returns: Key value
 */
gboolean
gphpedit_session_get_boolean (GphpeditSession *session, const gchar *section,
						const gchar *key)
{
	gint value;
	
	g_return_val_if_fail (GPHPEDIT_IS_SESSION (session), 0);
	g_return_val_if_fail (section != NULL, 0);
	g_return_val_if_fail (key != NULL, 0);

	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(session);
	
	value = g_key_file_get_boolean (priv->key_file, section, key, NULL);
	
	return value;
}

/**
 * gphpedit_session_get_boolean_with_default:
 * @session: an #GphpeditSession object
 * @section: Section.
 * @key: Key name.
 * 
 * Get a boolean @value of @key in given @section.
 * If key isn't found return default_value
 *
 * Returns: Key value
 */
gboolean
gphpedit_session_get_boolean_with_default (GphpeditSession *session, const gchar *section,
						const gchar *key, gboolean default_value)
{
	gint value;
	GError *error = NULL;

	g_return_val_if_fail (GPHPEDIT_IS_SESSION (session), 0);
	g_return_val_if_fail (section != NULL, 0);
	g_return_val_if_fail (key != NULL, 0);

	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(session);
	
	value = g_key_file_get_boolean (priv->key_file, section, key, &error);
	if (error)
	{
		value = default_value;
		g_error_free(error);
	}
	
	return value;
}

/**
 * gphpedit_session_get_float:
 * @session: an #GphpeditSession object
 * @section: Section.
 * @key: Key name.
 * 
 * Get a float @value of @key in given @section.
 * 
 * Returns: Key value
 */
gfloat
gphpedit_session_get_float (GphpeditSession *session, const gchar *section,
						  const gchar *key)
{
	gfloat value;
	
	g_return_val_if_fail (GPHPEDIT_IS_SESSION (session), 0);
	g_return_val_if_fail (section != NULL, 0);
	g_return_val_if_fail (key != NULL, 0);

	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(session);
	
	value = (float)g_key_file_get_double (priv->key_file, section, key, NULL);
	
	return value;
}

/**
 * gphpedit_session_get_string:
 * @session: an #GphpeditSession object
 * @section: Section.
 * @key: Key name.
 * 
 * Get a string @value of @key in given @section.
 * 
 * Returns: Key value
 */
gchar*
gphpedit_session_get_string (GphpeditSession *session, const gchar *section,
						   const gchar *key)
{
	gchar *value;
	
	g_return_val_if_fail (GPHPEDIT_IS_SESSION (session), NULL);
	g_return_val_if_fail (section != NULL, NULL);
	g_return_val_if_fail (key != NULL, NULL);

	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(session);
	
	value = g_key_file_get_string (priv->key_file, section, key, NULL);
	
	return value;
}

/**
 * gphpedit_session_get_string_with_default:
 * @session: an #GphpeditSession object
 * @section: Section.
 * @key: Key name.
 * 
 * Get a string @value of @key in given @section.
 * 
 * Returns: Key value
 */
gchar*
gphpedit_session_get_string_with_default (GphpeditSession *session, const gchar *section,
						   const gchar *key, const gchar *default_value)
{
	gchar *value;
	GError *error = NULL;
	
	g_return_val_if_fail (GPHPEDIT_IS_SESSION (session), NULL);
	g_return_val_if_fail (section != NULL, NULL);
	g_return_val_if_fail (key != NULL, NULL);

	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(session);
	
	value = g_key_file_get_string (priv->key_file, section, key, &error);
	if (error)
	{
		value = g_strdup(default_value);
		g_error_free(error);
	}
	
	return value;
}

/**
 * gphpedit_session_get_string_list:
 * @session: an #GphpeditSession object
 * @section: Section.
 * @key: Key name.
 * 
 * Get a list of strings @value of @key in given @section.
 * 
 * Returns: Key value
 */
GSList*
gphpedit_session_get_string_list (GphpeditSession *session,
								const gchar *section,
								const gchar *key)
{
	gchar *val, **str, **ptr;
	GSList *value;
	
	g_return_val_if_fail (GPHPEDIT_IS_SESSION (session), NULL);
	g_return_val_if_fail (section != NULL, NULL);
	g_return_val_if_fail (key != NULL, NULL);

	GphpeditSessionPriv *priv = GPHPEDIT_SESSION_GET_PRIVATE(session);
	
	val = g_key_file_get_string (priv->key_file, section, key, NULL);
	
	
	value = NULL;
	if (val)
	{
		str = g_strsplit (val, "%%%", -1);
		if (str)
		{
			ptr = str;
			while (*ptr)
			{
				if (strlen (*ptr) > 0)
					value = g_slist_prepend (value, g_strdup (*ptr));
				ptr++;
			}
			g_strfreev (str);
		}
		g_free (val);
	}
	
	return g_slist_reverse (value);
}
