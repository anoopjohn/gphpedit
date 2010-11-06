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

#ifndef __GPHPEDIT_FILEBROWSER_H__
#define __GPHPEDIT_FILEBROWSER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GPHPEDIT_TYPE_FILEBROWSER			(gphpedit_file_browser_get_type ())
#define GPHPEDIT_FILEBROWSER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GPHPEDIT_TYPE_FILEBROWSER, gphpeditFileBrowser))
#define GPHPEDIT_FILEBROWSER_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), gphpedit_TYPE_FILEBROWSER, gphpeditFileBrowser const))
#define GPHPEDIT_FILEBROWSER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), gphpedit_TYPE_FILEBROWSER, gphpeditFileBrowserClass))
#define GPHPEDIT_IS_FILEBROWSER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), gphpedit_TYPE_FILEBROWSER))
#define GPHPEDIT_IS_FILEBROWSER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), gphpedit_TYPE_FILEBROWSER))
#define GPHPEDIT_FILEBROWSER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), gphpedit_TYPE_FILEBROWSER, gphpeditFileBrowserClass))

typedef struct _gphpeditFileBrowser	gphpeditFileBrowser;
typedef struct _gphpeditFileBrowserClass	gphpeditFileBrowserClass;
typedef struct _gphpeditFileBrowserPrivate	gphpeditFileBrowserPrivate;

struct _gphpeditFileBrowser {
	GtkVBox parent;
};

struct _gphpeditFileBrowserClass {
	GtkVBoxClass parent_class;

	gphpeditFileBrowserPrivate *priv;
};

GType		  gphpedit_filebrowser_get_type (void) G_GNUC_CONST;

GtkWidget	 *gphpedit_filebrowser_new (void);
void  cancel_filebrowser_process (GtkWidget	 *widget);
G_END_DECLS

#endif /* __gphpedit_FILEBROWSER_H__ */
