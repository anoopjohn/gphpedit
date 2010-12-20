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

#ifndef _GPHPEDIT_SESSION_H_
#define _GPHPEDIT_SESSION_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define GPHPEDIT_TYPE_SESSION         (gphpedit_session_get_type ())
#define GPHPEDIT_SESSION(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GPHPEDIT_TYPE_SESSION, GphpeditSession))
#define GPHPEDIT_SESSION_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), GPHPEDIT_TYPE_SESSION, GphpeditSessionClass))
#define GPHPEDIT_IS_SESSION(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GPHPEDIT_TYPE_SESSION))
#define GPHPEDIT_IS_SESSION_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), GPHPEDIT_TYPE_SESSION))
#define GPHPEDIT_SESSION_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GPHPEDIT_TYPE_SESSION, GphpeditSessionClass))

typedef struct _GphpeditSessionPriv GphpeditSessionPriv;

typedef struct {
	GObject parent;
	GphpeditSessionPriv *priv;
} GphpeditSession;

typedef struct {
	GObjectClass parent_class;
	/* Add Signal Functions Here */
} GphpeditSessionClass;

GType gphpedit_session_get_type (void);
GphpeditSession *gphpedit_session_new (const gchar *session_directory);

gchar* gphpedit_session_get_session_filename (GphpeditSession *session);
const gchar* gphpedit_session_get_session_directory (GphpeditSession *session);

void gphpedit_session_sync (GphpeditSession *session);
void gphpedit_session_clear (GphpeditSession *session);
void gphpedit_session_clear_section (GphpeditSession *session,
								   const gchar *section);

void gphpedit_session_set_int (GphpeditSession *session, const gchar *section,
							 const gchar *key, gint value);
void gphpedit_session_set_boolean (GphpeditSession *session, const gchar *section,
							 const gchar *key, gboolean value);
void gphpedit_session_set_float (GphpeditSession *session, const gchar *section,
							   const gchar *key, gfloat value);
void gphpedit_session_set_string (GphpeditSession *session, const gchar *section,
								const gchar *key, const gchar *value);
void gphpedit_session_set_string_list (GphpeditSession *session,
									 const gchar *section,
									 const gchar *key, GSList *value);

gint gphpedit_session_get_int (GphpeditSession *session, const gchar *section,
							 const gchar *key);
gint gphpedit_session_get_int_with_default (GphpeditSession *session, const gchar *section,
						const gchar *key, gint default_value);
gboolean gphpedit_session_get_boolean (GphpeditSession *session, const gchar *section,
							 const gchar *key);
gboolean gphpedit_session_get_boolean_with_default (GphpeditSession *session, const gchar *section,
							 const gchar *key, gboolean default_value);
gfloat gphpedit_session_get_float (GphpeditSession *session, const gchar *section,
								 const gchar *key);
gchar* gphpedit_session_get_string (GphpeditSession *session, const gchar *section,
								  const gchar *key);
gchar* gphpedit_session_get_string_with_default (GphpeditSession *session, const gchar *section,
								  const gchar *key, const gchar *default_value);
GSList* gphpedit_session_get_string_list (GphpeditSession *session,
									   const gchar *section,
									   const gchar *key);
									   
G_END_DECLS

#endif /* _GPHPEDIT_SESSION_H_ */
