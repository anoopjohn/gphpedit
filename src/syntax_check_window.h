/* This file is part of gPHPEdit, a GNOME2 PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009 José Rostagno (for vijona.com.ar) 

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
#ifndef __GTK_SYNTAX_CHECK_WINDOW_H__
#define __GTK_SYNTAX_CHECK_WINDOW_H__

#include <gtk/gtk.h>
#include "tab.h"

G_BEGIN_DECLS

#define GTK_TYPE_SYNTAX_CHECK_WINDOW		(gtk_syntax_check_window_get_type ())
#define GTK_SYNTAX_CHECK_WINDOW(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_SYNTAX_CHECK_WINDOW, GtkSyntaxCheckWindow))
#define GTK_IS_SYNTAX_CHECK_WINDOW(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_SYNTAX_CHECK_WINDOW))
#define GTK_SYNTAX_CHECK_WINDOW_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_SYNTAX_CHECK_WINDOW, GtkSyntaxCheckWindowClass))
#define GTK_IS_SYNTAX_CHECK_WINDOW_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_SYNTAX_CHECK_WINDOW))
#define GTK_SYNTAX_CHECK_WINDOW_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_SYNTAX_CHECK_WINDOW, GtkSyntaxCheckWindowClass))

typedef struct _GtkSyntaxCheckWindow		GtkSyntaxCheckWindow;
typedef struct _GtkSyntaxCheckWindowClass	GtkSyntaxCheckWindowClass;
typedef struct _GtkSyntaxCheckWindowPrivate	GtkSyntaxCheckWindowPrivate;

struct _GtkSyntaxCheckWindow
{
  /*< private >*/
  GtkBox parent_instance;

  GtkSyntaxCheckWindowPrivate *priv;
};

struct _GtkSyntaxCheckWindowClass
{
  GtkBoxClass parent_class;
};

GType      gtk_syntax_check_window_get_type         (void) G_GNUC_CONST;
GtkWidget *gtk_syntax_check_window_new              (void);
void syntax_window(GtkSyntaxCheckWindow *win, Documentable *document, gchar *data);
void gtk_syntax_check_window_run_check(GtkSyntaxCheckWindow *win, Documentable *document);
G_END_DECLS

#endif /* ! __GTK_SYNTAX_CHECK_WINDOW_H__ */
