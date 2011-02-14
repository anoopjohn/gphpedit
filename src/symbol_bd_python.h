/* This file is part of gPHPEdit, a GNOME PHP Editor.

   Copyright (C) 2010 José Rostagno (for vijona.com.ar)

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
#ifndef SYMBOL_BD_PYTHON_H
#define SYMBOL_BD_PYTHON_H

#include <glib-object.h>

#define SYMBOL_BD_PYTHON_TYPE symbol_bd_python_get_type()
#define SYMBOL_BD_PYTHON(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), SYMBOL_BD_PYTHON_TYPE, SymbolBdPYTHON))
#define SYMBOL_BD_PYTHON_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), SYMBOL_BD_PYTHON_TYPE, SymbolBdPYTHONClass))
#define OBJECT_IS_SYMBOL_BD_PYTHON(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SYMBOL_BD_PYTHON_TYPE))
#define OBJECT_IS_SYMBOL_BD_PYTHON_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), SYMBOL_BD_PYTHON_TYPE))
#define SYMBOL_BD_PYTHON_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), SYMBOL_BD_PYTHON_TYPE, SymbolBdPYTHONClass))

typedef struct SymbolBdPYTHONDetails SymbolBdPYTHONDetails;

typedef struct
{
	GObject object;
	SymbolBdPYTHONDetails *details;
} SymbolBdPYTHON;

typedef struct
{
	GObjectClass parent_class;

} SymbolBdPYTHONClass;

/* Basic GObject requirements. */
GType symbol_bd_python_get_type (void);
SymbolBdPYTHON *symbol_bd_python_new (void);
#endif /* SYMBOL_BD_PYTHON_H */
