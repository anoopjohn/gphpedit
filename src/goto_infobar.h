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


#ifndef __GOTOINFOBAR_H__
#define __GOTOINFOBAR_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define INFOBAR_TYPE_GOTO              (goto_infobar_get_type())
#define GOTOINFOBAR(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), INFOBAR_TYPE_GOTO, GotoInfobar))
#define GOTOINFOBAR_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), INFOBAR_TYPE_GOTO, GotoInfobarClass))
#define INFOBAR_IS_GOTO(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), INFOBAR_TYPE_GOTO))
#define INFOBAR_IS_GOTO_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), INFOBAR_TYPE_GOTO))
#define GOTOINFOBAR_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), INFOBAR_TYPE_GOTO, GotoInfobarClass))

/* Private structure type */
typedef struct _GotoInfobarPrivate GotoInfobarPrivate;

/*
 * Main object structure
 */
typedef struct _GotoInfobar GotoInfobar;

struct _GotoInfobar 
{
	GtkInfoBar dialog;

	/*< private > */
	GotoInfobarPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GotoInfobarClass GotoInfobarClass;

struct _GotoInfobarClass 
{
	GtkInfoBarClass parent_class;
};

/*
 * Public methods
 */
GType 		 GOTOINFOBAR_get_type 		(void) G_GNUC_CONST;
GtkWidget *goto_infobar_new (void);

G_END_DECLS

#endif  /* __GOTOINFOBAR_H__  */
