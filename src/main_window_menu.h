/* This file is part of gPHPEdit, a GNOME2 PHP Editor.
 
   Copyright (C) 2003, 2004, 2005 Andy Jeffries
      andy@gphpedit.org
	  
   For more information or to find the latest release, visit our 
   website at http://www.gphpedit.org/
 
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
 
   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.
 
   The GNU General Public License is contained in the file COPYING.*/


#ifndef MAIN_WINDOW_MENU_H
#define MAIN_WINDOW_MENU_H

#include "main_window_callbacks.h"
#include <libgnome/gnome-i18n.h>


static GnomeUIInfo recent_menu[] =
	{
		GNOMEUIINFO_ITEM_DATA("New Window", "Create a new text viewer window", reopen_recent, (gpointer)0, NULL),
		GNOMEUIINFO_ITEM_DATA("New Window", "Create a new text viewer window", reopen_recent, (gpointer)1, NULL),
		GNOMEUIINFO_ITEM_DATA("New Window", "Create a new text viewer window", reopen_recent, (gpointer)2, NULL),
		GNOMEUIINFO_ITEM_DATA("New Window", "Create a new text viewer window", reopen_recent, (gpointer)3, NULL),
		GNOMEUIINFO_ITEM_DATA("New Window", "Create a new text viewer window", reopen_recent, (gpointer)4, NULL),
		GNOMEUIINFO_ITEM_DATA("New Window", "Create a new text viewer window", reopen_recent, (gpointer)5, NULL),
		GNOMEUIINFO_ITEM_DATA("New Window", "Create a new text viewer window", reopen_recent, (gpointer)6, NULL),
		GNOMEUIINFO_ITEM_DATA("New Window", "Create a new text viewer window", reopen_recent, (gpointer)7, NULL),
		GNOMEUIINFO_ITEM_DATA("New Window", "Create a new text viewer window", reopen_recent, (gpointer)8, NULL),
		GNOMEUIINFO_ITEM_DATA("New Window", "Create a new text viewer window", reopen_recent, (gpointer)9, NULL),
		GNOMEUIINFO_ITEM_DATA("New Window", "Create a new text viewer window", reopen_recent, (gpointer)10, NULL),
		GNOMEUIINFO_ITEM_DATA("New Window", "Create a new text viewer window", reopen_recent, (gpointer)11, NULL),
		GNOMEUIINFO_ITEM_DATA("New Window", "Create a new text viewer window", reopen_recent, (gpointer)12, NULL),
		GNOMEUIINFO_ITEM_DATA("New Window", "Create a new text viewer window", reopen_recent, (gpointer)13, NULL),
		GNOMEUIINFO_ITEM_DATA("New Window", "Create a new text viewer window", reopen_recent, (gpointer)14, NULL),
		GNOMEUIINFO_ITEM_DATA("New Window", "Create a new text viewer window", reopen_recent, (gpointer)15, NULL),
		GNOMEUIINFO_ITEM_DATA("New Window", "Create a new text viewer window", reopen_recent, (gpointer)16, NULL),
		GNOMEUIINFO_ITEM_DATA("New Window", "Create a new text viewer window", reopen_recent, (gpointer)17, NULL),
		GNOMEUIINFO_ITEM_DATA("New Window", "Create a new text viewer window", reopen_recent, (gpointer)18, NULL),
		GNOMEUIINFO_ITEM_DATA("New Window", "Create a new text viewer window", reopen_recent, (gpointer)19, NULL),
        GNOMEUIINFO_END		
	};		
#define NUM_REOPEN_MAX 20

static GnomeUIInfo plugin_menu[] =
	{
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)0, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)1, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)2, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)3, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)4, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)5, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)6, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)7, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)8, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)9, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)10, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)11, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)12, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)13, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)14, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)15, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)16, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)17, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)18, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)19, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)20, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)21, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)22, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)23, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)24, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)25, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)26, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)27, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)28, NULL),
		GNOMEUIINFO_ITEM_DATA("Run Plugin", "Run this plugin", run_plugin, (gpointer)29, NULL),
		GNOMEUIINFO_END		
	};	
#define NUM_PLUGINS_MAX 30

static GnomeUIInfo file1_menu_uiinfo[] =
    {
        GNOMEUIINFO_MENU_NEW_ITEM (N_("_New"), NULL, on_new1_activate, NULL),
        GNOMEUIINFO_MENU_OPEN_ITEM (on_open1_activate, NULL),
        { GNOME_APP_UI_ITEM, N_("Open selected file"), N_("Open a file with the name currently selected in the editor"), on_openselected1_activate, NULL,NULL, 0, 0, GDK_Return, GDK_CONTROL_MASK },
		GNOMEUIINFO_SUBTREE(N_("R_eopen recent"), recent_menu),
        { GNOME_APP_UI_ITEM, N_("Reload current file"), N_("Reload the file currently selected in the editor"), on_reload1_activate, NULL,NULL, 0, 0, 'o', GDK_SHIFT_MASK | GDK_CONTROL_MASK },
        GNOMEUIINFO_SEPARATOR,
		GNOMEUIINFO_MENU_SAVE_ITEM (on_save1_activate, NULL),
        GNOMEUIINFO_MENU_SAVE_AS_ITEM (on_save_as1_activate, NULL),
        { GNOME_APP_UI_ITEM, N_("Save _All"), N_("Save all open unsaved files"), on_saveall1_activate, NULL,NULL, 0, 0, 'a', GDK_SHIFT_MASK | GDK_MOD1_MASK },
        { GNOME_APP_UI_ITEM, N_("_Rename"), N_("Rename the current file 'on-the-fly'"), on_rename1_activate, NULL,NULL, 0, 0, 'r', GDK_SHIFT_MASK | GDK_MOD1_MASK },
        GNOMEUIINFO_MENU_CLOSE_ITEM (on_close1_activate, NULL),
        GNOMEUIINFO_SEPARATOR,
        //  GNOMEUIINFO_MENU_PROPERTIES_ITEM (on_properties1_activate, NULL),
        //  GNOMEUIINFO_SEPARATOR,
        GNOMEUIINFO_MENU_EXIT_ITEM (on_quit1_activate, NULL),
        GNOMEUIINFO_END
    };

static GnomeUIInfo edit1_menu_uiinfo[] =
    {
        GNOMEUIINFO_MENU_UNDO_ITEM (on_undo1_activate, NULL),
        GNOMEUIINFO_MENU_REDO_ITEM (on_redo1_activate, NULL),
        GNOMEUIINFO_SEPARATOR,
        GNOMEUIINFO_MENU_CUT_ITEM (on_cut1_activate, NULL),
        GNOMEUIINFO_MENU_COPY_ITEM (on_copy1_activate, NULL),
        GNOMEUIINFO_MENU_PASTE_ITEM (on_paste1_activate, NULL),
        GNOMEUIINFO_MENU_SELECT_ALL_ITEM (on_selectall1_activate, NULL),
        GNOMEUIINFO_SEPARATOR,
        GNOMEUIINFO_MENU_FIND_ITEM (on_find1_activate, NULL),
        GNOMEUIINFO_MENU_REPLACE_ITEM (on_replace1_activate, NULL),
        GNOMEUIINFO_SEPARATOR,
        { GNOME_APP_UI_ITEM, N_("_Indent block"), N_("Indent the currently selected block"), block_indent, NULL,NULL, 0, 0, 'i', GDK_SHIFT_MASK | GDK_MOD1_MASK },
        { GNOME_APP_UI_ITEM, N_("_Unindent block"), N_("Unindent the currently selected block"), block_unindent, NULL,NULL, 0, 0, 'i', GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK},
        //  GNOMEUIINFO_SEPARATOR,
        GNOMEUIINFO_MENU_PREFERENCES_ITEM (on_preferences1_activate, NULL),
        GNOMEUIINFO_END
    };

/*static GnomeUIInfo project1_menu_uiinfo[] =
{
  { GNOME_APP_UI_ITEM, "_Not used yet", "Not used yet", NULL, NULL,NULL, 0, 0, 'u', GDK_MOD1_MASK },
  GNOMEUIINFO_END
};*/

static GnomeUIInfo force1_menu_uiinfo[] =
    {
        { GNOME_APP_UI_ITEM, N_("PHP/HTML/XML"), N_("Force syntax highlighting to PHP/HTML/XML mode"), force_php, NULL,NULL, 0, 0, 0, 0 },
        { GNOME_APP_UI_ITEM, N_("CSS"), N_("Force syntax highlighting to CSS mode"), force_css, NULL,NULL, 0, 0, 0, 0 },
        { GNOME_APP_UI_ITEM, N_("SQL"), N_("Force syntax highlighting to SQL mode"), force_sql, NULL,NULL, 0, 0, 0, 0 },
        { GNOME_APP_UI_ITEM, N_("C/C++"), N_("Force syntax highlighting to C/C++ mode"), force_cxx, NULL,NULL, 0, 0, 0, 0 },
        { GNOME_APP_UI_ITEM, N_("Perl"), N_("Force syntax highlighting to Perl mode"), force_perl, NULL,NULL, 0, 0, 0, 0 },
        { GNOME_APP_UI_ITEM, N_("Python"), N_("Force syntax highlighting to Python mode"), force_python, NULL,NULL, 0, 0, 0, 0 },
        GNOMEUIINFO_END
    };
	
static GnomeUIInfo code1_menu_uiinfo[] =
    {
        { GNOME_APP_UI_ITEM, N_("_Syntax check"), N_("Check the syntax using the PHP command line binary"), syntax_check, NULL,NULL, 0, 0, GDK_F9, 0 },
        { GNOME_APP_UI_ITEM, N_("_Clear syntax check"), N_("Remove the syntax check window"), syntax_check_clear, NULL,NULL, 0, 0, GDK_F9, GDK_SHIFT_MASK },
        GNOMEUIINFO_SEPARATOR,
        { GNOME_APP_UI_ITEM, N_("_Record keyboard macro start/stop"), N_("Record keyboard actions"), keyboard_macro_startstop, NULL,NULL, 0, 0, 'k', GDK_MOD1_MASK },
        { GNOME_APP_UI_ITEM, N_("_Playback keyboard macro"), N_("Playback the stored keyboard macro"), keyboard_macro_playback, NULL,NULL, 0, 0, 'k', GDK_CONTROL_MASK },
        GNOMEUIINFO_SEPARATOR,
        { GNOME_APP_UI_ITEM, N_("S_how/Hide class browser"), N_("Show/Hide class browser"), classbrowser_show_hide, NULL,NULL, 0, 0, GDK_F8, 0 },
        //{ GNOME_APP_UI_ITEM, N_("H_ide class browser"), N_("Hide class browser"), classbrowser_hide, NULL,NULL, 0, 0, GDK_F8, GDK_SHIFT_MASK },
        GNOMEUIINFO_SEPARATOR,
		GNOMEUIINFO_SUBTREE(N_("_Force highlighting mode"), force1_menu_uiinfo),
        GNOMEUIINFO_END
    };


static GnomeUIInfo help1_menu_uiinfo[] =
    {
        { GNOME_APP_UI_ITEM, N_("_PHP Help"), N_("Look for help on the currently selected function"), context_help, NULL,NULL, 0, 0, GDK_F1, 0 },
        GNOMEUIINFO_MENU_ABOUT_ITEM (on_about1_activate, NULL),
        GNOMEUIINFO_END
    };


static GnomeUIInfo menubar1_uiinfo[] =
    {
        GNOMEUIINFO_MENU_FILE_TREE (file1_menu_uiinfo),
        GNOMEUIINFO_MENU_EDIT_TREE (edit1_menu_uiinfo),
        //  { GNOME_APP_UI_SUBTREE_STOCK, "_Project", NULL, project1_menu_uiinfo, NULL, NULL,
        //		(GnomeUIPixmapType) 0, NULL, 0,	(GdkModifierType) 0, NULL },
		GNOMEUIINFO_SUBTREE (N_("_Code"), code1_menu_uiinfo),
		GNOMEUIINFO_SUBTREE(N_("_Plugins"), plugin_menu),
        GNOMEUIINFO_MENU_HELP_TREE (help1_menu_uiinfo),
        GNOMEUIINFO_END
    };

#endif
