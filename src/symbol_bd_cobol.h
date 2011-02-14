/* This file is part of gPHPEdit, a GNOME PHP Editor.

   Copyright (C) 2010 José Rostagno (for vijona.com.ar)

   For more information or to find the latest release, visit our 
   website at http://www.gcoboledit.org/

   gCOBOLEdit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   gCOBOLEdit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with gCOBOLEdit. If not, see <http://www.gnu.org/licenses/>.

   The GNU General Public License is contained in the file COPYING.
*/
#ifndef SYMBOL_BD_COBOL_H
#define SYMBOL_BD_COBOL_H

#include <glib-object.h>

#define SYMBOL_BD_COBOL_TYPE symbol_bd_cobol_get_type()
#define SYMBOL_BD_COBOL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), SYMBOL_BD_COBOL_TYPE, SymbolBdCOBOL))
#define SYMBOL_BD_COBOL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), SYMBOL_BD_COBOL_TYPE, SymbolBdCOBOLClass))
#define OBJECT_IS_SYMBOL_BD_COBOL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SYMBOL_BD_COBOL_TYPE))
#define OBJECT_IS_SYMBOL_BD_COBOL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), SYMBOL_BD_COBOL_TYPE))
#define SYMBOL_BD_COBOL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), SYMBOL_BD_COBOL_TYPE, SymbolBdCOBOLClass))

typedef struct SymbolBdCOBOLDetails SymbolBdCOBOLDetails;

typedef struct
{
	GObject object;
	SymbolBdCOBOLDetails *details;
} SymbolBdCOBOL;

typedef struct
{
	GObjectClass parent_class;

} SymbolBdCOBOLClass;

/* Basic GObject requirements. */
GType symbol_bd_cobol_get_type (void);
SymbolBdCOBOL *symbol_bd_cobol_new (void);
#endif /* SYMBOL_BD_COBOL_H */
