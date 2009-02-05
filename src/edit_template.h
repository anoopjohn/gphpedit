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

#ifndef EDIT_TEMPLATE_H
#define EDIT_TEMPLATE_H

#include "main.h"
#include "tab_php.h"
#include "tab_css.h"
#include "tab_sql.h"

typedef struct
{

  GtkWidget *window1;
  GtkWidget *vbox1;
  GtkWidget *hbox2;
  GtkWidget *label2;
  GtkWidget *entry1;
  GtkWidget *hbox3;
  GtkWidget *label3;
  GtkWidget *scrolledwindow1;
  GtkWidget *textview1;
  GtkWidget *hbox4;
  GtkWidget *button2;
  GtkWidget *button1;
} EditTemplateDialog;

extern EditTemplateDialog edit_template_dialog; 
  


void create_edit_template_dialog (void);


#endif
