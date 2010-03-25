/* This file is part of gPHPEdit, a GNOME2 PHP Editor.
 
   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
	  
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
   
#include "plugin.h"
#include <gdk/gdkkeysyms.h>
/*
* transform a number into it's corresponding keysym
*/
gint parse_shortcut(gint accel_number){ 
     switch (accel_number) {
 	case 0: return GDK_0;
		break;
	case 1: return GDK_1;
		break;
	case 2: return GDK_2;
		break;
	case 3: return GDK_3;
		break;
	case 4: return GDK_4;
		break;
	case 5: return GDK_5;
		break;
	case 6: return GDK_6;
		break;
	case 7: return GDK_7;
		break;
	case 8: return GDK_8;
		break;
	case 9: return GDK_9;
		break;
}
 return GDK_0;
}
