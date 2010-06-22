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

#ifndef MAIN_WINDOW_MENU_H
#define MAIN_WINDOW_MENU_H
#include <gtk/gtk.h>
/*max plugins */
#define NUM_PLUGINS_MAX 30
/*max reopen files shown */
#define NUM_REOPEN_MAX 20
typedef struct
{
  GtkAccelGroup *accel_group;
  GtkWidget *menubar;
  GtkWidget *menunew;
  GtkWidget *menuedit;
  GtkWidget *menuview;
  GtkWidget *menucode;
  GtkWidget *file;
  GtkWidget *newi;
  GtkWidget *open;
  GtkWidget *opensel;
  GtkWidget *reload;
  GtkWidget *save;
  GtkWidget *saveas;
  GtkWidget *saveall;
  GtkWidget *rename;
  GtkWidget *close;
  GtkWidget *quit;

  GtkWidget *edit;
  GtkWidget *sep;
  GtkWidget *sep1;
  GtkWidget *sep2;
  GtkWidget *undo;
  GtkWidget *redo;
  GtkWidget *copy;
  GtkWidget *cut;
  GtkWidget *paste;
  GtkWidget *selectall;
  GtkWidget *sep3;
  GtkWidget *sep4;
  GtkWidget *find;
  GtkWidget *replace;
  GtkWidget *indent;
  GtkWidget *unindent;
  GtkWidget *sept;
  GtkWidget *upper;
  GtkWidget *lower;

  GtkWidget *view;
  GtkWidget *viewmaintoolbar;
  GtkWidget *viewfindtoolbar;
  GtkWidget *viewstatusbar;
  GtkWidget *viewfullscreen;
  GtkWidget *tog_class;
  GtkWidget *sep6;
  GtkWidget *sep7;
  GtkWidget *zoomin;
  GtkWidget *zoomout;
  GtkWidget *zoom100;
  GtkWidget *sep8;
  GtkWidget *seppr;
  GtkWidget *preview;
  GSList *iconsizegroup;
  GtkWidget *Ticonsizemenu;
  GtkWidget *Ticonsize;
  GtkWidget *sizebig;
  GtkWidget *sizesmall;
  GtkWidget *sepd;
  GtkWidget *preferences;

  GtkWidget *code;
  GtkWidget *syntax;
  GtkWidget *clearsyntax;
  GtkWidget *sep5;
  GtkWidget *record;
  GtkWidget *playback;
  GtkWidget *force;
  GtkWidget *menuforce;
  GtkWidget *forcephp;
  GtkWidget *forcecss;
  GtkWidget *forcecxx;
  GtkWidget *forcesql;
  GtkWidget *forceperl;
  GtkWidget *forcecobol;
  GtkWidget *forcepython;
  GtkWidget *menuplugin;
  GtkWidget *plugin;
  GtkWidget *plugins[NUM_PLUGINS_MAX];
  GtkWidget *reciente;
  GtkWidget *menureciente;
  GtkWidget *menuhelp;
  GtkWidget *help;
  GtkWidget *phphelp;
  #ifdef PACKAGE_BUGREPORT
  GtkWidget *bugreport;
  #endif
  #ifdef TRANSLATE_URL
  GtkWidget *translate;
  #endif
  GtkWidget *abouthelp;
}
Mainmenu;

void main_window_create_menu(void);
void install_menu_hint(GtkWidget *widget, gchar *message);
#endif /*MAIN_WINDOW_MENU_H*/
