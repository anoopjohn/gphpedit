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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include "debug.h"

#ifdef DEBUG
static DebugSection debug = GPHPEDIT_NO_DEBUG;
#endif

void
gphpedit_debug_init (void)
{
#ifdef DEBUG
	if (g_getenv ("GPHPEDIT_DEBUG") != NULL)
	{
		/* enable all debugging */
		debug = ~GPHPEDIT_NO_DEBUG;
		return;
	}

	if (g_getenv ("GPHPEDIT_DEBUG_CALLTIP") != NULL)
		debug = debug | GPHPEDIT_DEBUG_CALLTIP;
	if (g_getenv ("GPHPEDIT_DEBUG_CLASSBROWSER") != NULL)
		debug = debug | GPHPEDIT_DEBUG_CLASSBROWSER;
	if (g_getenv ("GPHPEDIT_DEBUG_DOCUMENT") != NULL)
		debug = debug | GPHPEDIT_DEBUG_DOCUMENT;
	if (g_getenv ("GPHPEDIT_DEBUG_CLASSBROWSER_PARSE") != NULL)
		debug = debug | GPHPEDIT_DEBUG_CLASSBROWSER_PARSE;
	if (g_getenv ("GPHPEDIT_DEBUG_PLUGINS") != NULL)
		debug = debug | GPHPEDIT_DEBUG_PLUGINS;
	if (g_getenv ("GPHPEDIT_DEBUG_FILEBROWSER") != NULL)
		debug = debug | GPHPEDIT_DEBUG_FILEBROWSER;
	if (g_getenv ("GPHPEDIT_DEBUG_IPC") != NULL)
		debug = debug | GPHPEDIT_DEBUG_IPC;
	if (g_getenv ("GPHPEDIT_DEBUG_MAIN_WINDOW") != NULL)
		debug = debug | GPHPEDIT_DEBUG_MAIN_WINDOW;
	if (g_getenv ("GPHPEDIT_DEBUG_APP") != NULL)
		debug = debug | GPHPEDIT_DEBUG_APP;
	if (g_getenv ("GPHPEDIT_DEBUG_DOC_MANAGER") != NULL)
		debug = debug | GPHPEDIT_DEBUG_DOC_MANAGER;
	if (g_getenv ("GPHPEDIT_DEBUG_PREFS") != NULL)
		debug = debug | GPHPEDIT_DEBUG_PREFS;
	if (g_getenv ("GPHPEDIT_DEBUG_SYNTAX") != NULL)
		debug = debug | GPHPEDIT_DEBUG_SYNTAX;
#endif
}

void
gphpedit_debug_message (DebugSection  section,
		     const gchar       *file,
		     gint               line,
		     const gchar       *function,
		     const gchar       *format, ...)
{
#ifdef DEBUG
	if (G_UNLIKELY (debug & section))
	{	

		va_list args;
		gchar *msg;

		g_return_if_fail (format != NULL);

		va_start (args, format);
		msg = g_strdup_vprintf (format, args);
		va_end (args);

		g_print ("[%s] %s:%d (%s) %s\n", g_get_prgname(), file, line, function, msg);	

		fflush (stdout);

		g_free (msg);
	}
#endif
}

void gphpedit_debug (DebugSection  section,
		  const gchar       *file,
		  gint               line,
		  const gchar       *function)
{
#ifdef DEBUG
	if (G_UNLIKELY (debug & section))
	{
		g_print ("[%s] %s:%d (%s)\n", g_get_prgname(), file, line, function);

		fflush (stdout);
	}
#endif
}
