/* This file is part of gPHPEdit, a GNOME2 PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009 José Rostagno(for vijona.com.ar)
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

#ifndef __GPHPEDIT_CLASSBROWSER_H__
#define __GPHPEDIT_CLASSBROWSER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GPHPEDIT_TYPE_CLASSBROWSER			(gphpedit_classbrowser_get_type ())
#define GPHPEDIT_CLASSBROWSER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GPHPEDIT_TYPE_CLASSBROWSER, gphpeditClassBrowser))
#define GPHPEDIT_CLASSBROWSER_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), gphpedit_TYPE_CLASSBROWSER, gphpeditClassBrowser const))
#define GPHPEDIT_CLASSBROWSER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), gphpedit_TYPE_CLASSBROWSER, gphpeditClassBrowserClass))
#define GPHPEDIT_IS_CLASSBROWSER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), gphpedit_TYPE_CLASSBROWSER))
#define GPHPEDIT_IS_CLASSBROWSER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), gphpedit_TYPE_CLASSBROWSER))
#define GPHPEDIT_CLASSBROWSER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), gphpedit_TYPE_CLASSBROWSER, gphpeditClassBrowserClass))

typedef struct _gphpeditClassBrowser	gphpeditClassBrowser;
typedef struct _gphpeditClassBrowserClass	gphpeditClassBrowserClass;
typedef struct _gphpeditClassBrowserPrivate	gphpeditClassBrowserPrivate;

struct _gphpeditClassBrowser {
	GtkVBox parent;
};

struct _gphpeditClassBrowserClass {
	GtkVBoxClass parent_class;

	gphpeditClassBrowserPrivate *priv;
};

GType		  gphpedit_classbrowser_get_type (void) G_GNUC_CONST;

GtkWidget	 *gphpedit_classbrowser_new (void);
void classbrowser_update(gphpeditClassBrowser *classbrowser);
GString *classbrowser_get_autocomplete_php_classes_string(gphpeditClassBrowser *classbrowser);
gchar *classbrowser_autocomplete_php_variables(gphpeditClassBrowser *classbrowser, gchar *prefix);
gchar *classbrowser_autocomplete_member_function(gphpeditClassBrowser *classbrowser, gchar *member);
gchar *classbrowser_custom_function_calltip(gphpeditClassBrowser *classbrowser, gchar *function_name, gint file_type);
gchar *classbrowser_add_custom_autocompletion(gphpeditClassBrowser *classbrowser, gchar *prefix, gint file_type, GSList *list);
G_END_DECLS

#endif /* __GPHPEDIT_CLASSBROWSER_H__ */
