/* This file is part of gPHPEdit, a GNOME2 PHP Editor.

   This file was adapted from gedit

   Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
   Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi
   Copyright (C) 2002 - 2005 Paolo Maggi  
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
 
//TODO: save debug output to file option

#ifndef __GPHPEDIT_DEBUG_H__
#define __GPHPEDIT_DEBUG_H__

#include <glib.h>

/*
 * Set an environmental var of the same name to turn on
 * debugging output. Setting GPHPEDIT_DEBUG will turn on all
 * sections.
 */
typedef enum {
	GPHPEDIT_NO_DEBUG                 = 0,
	GPHPEDIT_DEBUG_SYMBOLIZABLE       = 1 << 0,
	GPHPEDIT_DEBUG_CLASSBROWSER       = 1 << 1,
	GPHPEDIT_DEBUG_CLASSBROWSER_PARSE = 1 << 2,
	GPHPEDIT_DEBUG_DOCUMENT           = 1 << 3,
	GPHPEDIT_DEBUG_PLUGINS            = 1 << 4,
	GPHPEDIT_DEBUG_FILEBROWSER        = 1 << 5,
	GPHPEDIT_DEBUG_IPC                = 1 << 6,
	GPHPEDIT_DEBUG_MAIN_WINDOW        = 1 << 7,
	GPHPEDIT_DEBUG_APP                = 1 << 8,
	GPHPEDIT_DEBUG_DOC_MANAGER        = 1 << 9,
	GPHPEDIT_DEBUG_PREFS              = 1 << 10,
	GPHPEDIT_DEBUG_SYNTAX             = 1 << 11,
} DebugSection;


#define	DEBUG_SYMBOLIZABLE	GPHPEDIT_DEBUG_SYMBOLIZABLE,    __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_CLASSBROWSER	GPHPEDIT_DEBUG_CLASSBROWSER,  __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_CLASSBROWSER_PARSE	GPHPEDIT_DEBUG_CLASSBROWSER_PARSE,   __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_DOCUMENT	GPHPEDIT_DEBUG_DOCUMENT,   __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_PLUGINS	GPHPEDIT_DEBUG_PLUGINS, __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_FILEBROWSER	GPHPEDIT_DEBUG_FILEBROWSER,     __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_IPC	GPHPEDIT_DEBUG_IPC,__FILE__, __LINE__, G_STRFUNC
#define	DEBUG_MAIN_WINDOW	GPHPEDIT_DEBUG_MAIN_WINDOW,__FILE__, __LINE__, G_STRFUNC
#define	DEBUG_APP	GPHPEDIT_DEBUG_APP,     __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_DOC_MANAGER	GPHPEDIT_DEBUG_DOC_MANAGER, __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_PREFS	GPHPEDIT_DEBUG_PREFS,   __FILE__, __LINE__, G_STRFUNC
#define	DEBUG_SYNTAX	GPHPEDIT_DEBUG_SYNTAX,__FILE__, __LINE__, G_STRFUNC

void gphpedit_debug_init (void);

void gphpedit_debug (DebugSection  section,
		  const gchar       *file,
		  gint               line,
		  const gchar       *function);

void gphpedit_debug_message (DebugSection  section,
			  const gchar       *file,
			  gint               line,
			  const gchar       *function,
			  const gchar       *format, ...) G_GNUC_PRINTF(5, 6);


#endif /* __GPHPEDIT_DEBUG_H__ */
