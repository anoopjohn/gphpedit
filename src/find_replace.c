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

#include "config.h"
#include "find_replace.h"
#include "main_window.h"


FindDialog find_dialog;
ReplaceDialog replace_dialog;
GtkWidget *replace_prompt_dialog;

GdkPixbuf *get_window_icon (void){
GdkPixbuf *pixbuf = NULL;
GError *error = NULL;
pixbuf = gdk_pixbuf_new_from_file (PIXMAP_DIR "/" GPHPEDIT_PIXMAP_ICON, &error);
if (error) {
g_warning (G_STRLOC ": cannot open icon: %s", error->message);
g_error_free (error);
return NULL;
}
return pixbuf;
}

void find_destroy(GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy(find_dialog.window1);
}

void find_destroyed(GtkWidget *widget, gpointer data)
{
	find_dialog.window1=NULL;
}

void find_clicked(GtkButton *button, gpointer data)
{
	gboolean whole_document;
	gint search_flags = 0;
	gchar *text;
	glong length_of_document;
	glong current_pos;
	glong last_found = 0;
	glong start_found;
	glong end_found;
	glong result;

	length_of_document = gtk_scintilla_get_length(GTK_SCINTILLA(main_window.current_editor->scintilla));
	current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla));

	whole_document = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(find_dialog.radiobutton1));
	text = gtk_editable_get_chars (GTK_EDITABLE(find_dialog.entry1), 0, -1);

	if (whole_document) {
		current_pos = 0;
		gtk_scintilla_goto_pos(GTK_SCINTILLA(main_window.current_editor->scintilla), current_pos);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(find_dialog.radiobutton4), TRUE);
	}

	if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(find_dialog.checkbutton2))) {
		search_flags += SCFIND_MATCHCASE;
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(find_dialog.checkbutton1))) {
		search_flags += SCFIND_WHOLEWORD;
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(find_dialog.radiobutton5))) {
		search_flags += SCFIND_REGEXP;
	}

	result = gtk_scintilla_find_text (GTK_SCINTILLA(main_window.current_editor->scintilla),
	                                  search_flags, text, current_pos, length_of_document, &start_found, &end_found);

	if (result == -1) {
		// Show message saying could not be found.
                  GtkWidget *dialog;
                   dialog = gtk_message_dialog_new(GTK_WINDOW(main_window.window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,_("The text \"%s\" was not found."), text);
                  gtk_window_set_title(GTK_WINDOW(dialog), "gphpedit");
		  gtk_window_set_icon(GTK_WINDOW(dialog), get_window_icon());
                    gtk_dialog_run(GTK_DIALOG(dialog));
                    gtk_widget_destroy(dialog);
	}
	else {
		if (start_found == last_found) {
			return;
		}

		last_found = start_found;

		gtk_scintilla_goto_pos(GTK_SCINTILLA(main_window.current_editor->scintilla), start_found);	
		gtk_scintilla_set_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla), start_found);
		gtk_scintilla_set_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla), end_found);
	}
}

gint find_key_press_event(GtkWidget *widget, GdkEventKey *event,gpointer user_data)
{
	if (event->keyval == GDK_Return) {
		find_clicked(NULL,NULL);
		return TRUE;
	}
	else if (event->keyval == GDK_Escape) {
		find_destroy(NULL, NULL);
		return TRUE;
	}
	return FALSE;
}

void find_create(void)
{
	find_dialog.radiobutton1_group = NULL;
	find_dialog.radiobutton2_group = NULL;
	find_dialog.radiobutton5_group = NULL;

	find_dialog.window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position (GTK_WINDOW(find_dialog.window1),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(find_dialog.window1), GTK_WINDOW(main_window.window));
	gtk_container_set_border_width (GTK_CONTAINER (find_dialog.window1), 6);
	gtk_window_set_title (GTK_WINDOW (find_dialog.window1), _("Find"));
	gtk_window_set_resizable (GTK_WINDOW (find_dialog.window1), FALSE);
	gtk_window_set_icon (GTK_WINDOW (find_dialog.window1),get_window_icon());

	find_dialog.vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (find_dialog.vbox1);
	gtk_container_add (GTK_CONTAINER (find_dialog.window1), find_dialog.vbox1);

	find_dialog.frame1 = gtk_frame_new (NULL);
	gtk_widget_show (find_dialog.frame1);
	gtk_box_pack_start (GTK_BOX (find_dialog.vbox1), find_dialog.frame1, TRUE, TRUE, 3);

	find_dialog.alignment1 = gtk_alignment_new(0,0,1,1);
	gtk_container_add (GTK_CONTAINER (find_dialog.frame1), find_dialog.alignment1);
	gtk_alignment_set_padding(GTK_ALIGNMENT(find_dialog.alignment1), 6, 6, 6, 6);
	gtk_widget_show(find_dialog.alignment1);
	
	find_dialog.entry1 = gtk_entry_new ();
	gtk_widget_show (find_dialog.entry1);
	gtk_container_add (GTK_CONTAINER (find_dialog.alignment1), find_dialog.entry1);
	/* Get selected text (Wendell) */
	gint wordStart;
	gint wordEnd;
	gint length;
	gchar *buffer;

	if (main_window.current_editor->type != TAB_HELP) {
		wordStart = gtk_scintilla_get_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla));
		wordEnd = gtk_scintilla_get_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla));
		if (wordStart != wordEnd) {
			buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(main_window.current_editor->scintilla), wordStart, wordEnd, &length);
			gtk_entry_set_text(GTK_ENTRY(find_dialog.entry1), buffer);
		}
	}
	/* End get selected text */

	find_dialog.label1 = gtk_label_new (_("RegExp/String to search"));
	gtk_widget_show (find_dialog.label1);
	gtk_frame_set_label_widget (GTK_FRAME (find_dialog.frame1), find_dialog.label1);
	gtk_label_set_justify (GTK_LABEL (find_dialog.label1), GTK_JUSTIFY_LEFT);

	find_dialog.frame2 = gtk_frame_new (NULL);
	gtk_widget_show (find_dialog.frame2);
	gtk_box_pack_start (GTK_BOX (find_dialog.vbox1), find_dialog.frame2, FALSE, FALSE, 3);

	find_dialog.vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (find_dialog.vbox2);
	gtk_container_add (GTK_CONTAINER (find_dialog.frame2), find_dialog.vbox2);

	find_dialog.hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (find_dialog.hbox1);
	gtk_box_pack_start (GTK_BOX (find_dialog.vbox2), find_dialog.hbox1, TRUE, TRUE, 0);

	find_dialog.frame3 = gtk_frame_new (NULL);
	gtk_widget_show (find_dialog.frame3);
	gtk_box_pack_start (GTK_BOX (find_dialog.hbox1), find_dialog.frame3, TRUE, TRUE, 4);

	find_dialog.vbox3 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (find_dialog.vbox3);
	gtk_container_add (GTK_CONTAINER (find_dialog.frame3), find_dialog.vbox3);

	find_dialog.radiobutton1 = gtk_radio_button_new_with_mnemonic (NULL, _("Whole document"));
	gtk_widget_show (find_dialog.radiobutton1);
	gtk_box_pack_start (GTK_BOX (find_dialog.vbox3), find_dialog.radiobutton1, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (find_dialog.radiobutton1), 2);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (find_dialog.radiobutton1), find_dialog.radiobutton1_group);
	find_dialog.radiobutton1_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (find_dialog.radiobutton1));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (find_dialog.radiobutton1), TRUE);

	find_dialog.radiobutton4 = gtk_radio_button_new_with_mnemonic (NULL, _("From cursor"));
	gtk_widget_show (find_dialog.radiobutton4);
	gtk_box_pack_start (GTK_BOX (find_dialog.vbox3), find_dialog.radiobutton4, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (find_dialog.radiobutton4), 2);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (find_dialog.radiobutton4), find_dialog.radiobutton1_group);
	find_dialog.radiobutton1_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (find_dialog.radiobutton4));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (find_dialog.radiobutton4), FALSE);

	find_dialog.label3 = gtk_label_new (_("Search"));
	gtk_widget_show (find_dialog.label3);
	gtk_frame_set_label_widget (GTK_FRAME (find_dialog.frame3), find_dialog.label3);
	gtk_label_set_justify (GTK_LABEL (find_dialog.label3), GTK_JUSTIFY_LEFT);

	/*find_dialog.frame4 = gtk_frame_new (NULL);
	gtk_widget_show (find_dialog.frame4);
	gtk_box_pack_start (GTK_BOX (find_dialog.hbox1), find_dialog.frame4, FALSE, FALSE, 4);

	find_dialog.vbox4 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (find_dialog.vbox4);
	gtk_container_add (GTK_CONTAINER (find_dialog.frame4), find_dialog.vbox4);

	find_dialog.radiobutton2 = gtk_radio_button_new_with_mnemonic (NULL, _("Forwards"));
	gtk_widget_show (find_dialog.radiobutton2);
	gtk_box_pack_start (GTK_BOX (find_dialog.vbox4), find_dialog.radiobutton2, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (find_dialog.radiobutton2), 2);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (find_dialog.radiobutton2), find_dialog.radiobutton2_group);
	find_dialog.radiobutton2_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (find_dialog.radiobutton2));

	find_dialog.radiobutton3 = gtk_radio_button_new_with_mnemonic (NULL, _("Backwards"));
	gtk_widget_show (find_dialog.radiobutton3);
	gtk_box_pack_start (GTK_BOX (find_dialog.vbox4), find_dialog.radiobutton3, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (find_dialog.radiobutton3), 2);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (find_dialog.radiobutton3), find_dialog.radiobutton2_group);
	find_dialog.radiobutton2_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (find_dialog.radiobutton3));

	find_dialog.label4 = gtk_label_new (_("Direction"));
	gtk_widget_show (find_dialog.label4);
	gtk_frame_set_label_widget (GTK_FRAME (find_dialog.frame4), find_dialog.label4);
	gtk_label_set_justify (GTK_LABEL (find_dialog.label4), GTK_JUSTIFY_LEFT);*/

	find_dialog.frame5 = gtk_frame_new (NULL);
	gtk_widget_show (find_dialog.frame5);
	gtk_box_pack_start (GTK_BOX (find_dialog.hbox1), find_dialog.frame5, TRUE, TRUE, 4);

	find_dialog.vbox5 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (find_dialog.vbox5);
	gtk_container_add (GTK_CONTAINER (find_dialog.frame5), find_dialog.vbox5);

	find_dialog.radiobutton5 = gtk_radio_button_new_with_mnemonic (NULL, _("RegExp"));
	gtk_widget_show (find_dialog.radiobutton5);
	gtk_box_pack_start (GTK_BOX (find_dialog.vbox5), find_dialog.radiobutton5, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (find_dialog.radiobutton5), 2);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (find_dialog.radiobutton5), find_dialog.radiobutton5_group);
	find_dialog.radiobutton5_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (find_dialog.radiobutton5));

	find_dialog.radiobutton6 = gtk_radio_button_new_with_mnemonic (NULL, _("String"));
	gtk_widget_show (find_dialog.radiobutton6);
	gtk_box_pack_start (GTK_BOX (find_dialog.vbox5), find_dialog.radiobutton6, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (find_dialog.radiobutton6), 2);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (find_dialog.radiobutton6), find_dialog.radiobutton5_group);
	find_dialog.radiobutton5_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (find_dialog.radiobutton6));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (find_dialog.radiobutton6), TRUE);

	find_dialog.label5 = gtk_label_new (_("Type"));
	gtk_widget_show (find_dialog.label5);
	gtk_frame_set_label_widget (GTK_FRAME (find_dialog.frame5), find_dialog.label5);
	gtk_label_set_justify (GTK_LABEL (find_dialog.label5), GTK_JUSTIFY_LEFT);

	find_dialog.hbox3 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (find_dialog.hbox3);
	gtk_box_pack_start (GTK_BOX (find_dialog.vbox2), find_dialog.hbox3, TRUE, TRUE, 3);

	find_dialog.checkbutton1 = gtk_check_button_new_with_mnemonic (_("Whole word"));
	gtk_widget_show (find_dialog.checkbutton1);
	gtk_box_pack_start (GTK_BOX (find_dialog.hbox3), find_dialog.checkbutton1, FALSE, FALSE, 4);

	find_dialog.checkbutton2 = gtk_check_button_new_with_mnemonic (_("Ignore case while searching"));
	gtk_widget_show (find_dialog.checkbutton2);
	gtk_box_pack_start (GTK_BOX (find_dialog.hbox3), find_dialog.checkbutton2, TRUE, FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (find_dialog.checkbutton2), TRUE);

	find_dialog.label2 = gtk_label_new (_("Options"));
	gtk_widget_show (find_dialog.label2);
	gtk_frame_set_label_widget (GTK_FRAME (find_dialog.frame2), find_dialog.label2);
	gtk_label_set_justify (GTK_LABEL (find_dialog.label2), GTK_JUSTIFY_LEFT);

	find_dialog.hseparator1 = gtk_hseparator_new ();
	gtk_widget_show (find_dialog.hseparator1);
	gtk_box_pack_start (GTK_BOX (find_dialog.vbox1), find_dialog.hseparator1, TRUE, TRUE, 0);

	find_dialog.hbox2 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (find_dialog.hbox2);
	gtk_box_pack_start (GTK_BOX (find_dialog.vbox1), find_dialog.hbox2, TRUE, TRUE, 6);

	find_dialog.button4 = gtk_button_new_from_stock ("gtk-close");
	gtk_widget_show (find_dialog.button4);
	gtk_box_pack_start (GTK_BOX (find_dialog.hbox2), find_dialog.button4, TRUE, FALSE, 0);

	find_dialog.button5 = gtk_button_new_from_stock ("gtk-find");
	gtk_widget_show (find_dialog.button5);
	gtk_box_pack_start (GTK_BOX (find_dialog.hbox2), find_dialog.button5, TRUE, FALSE, 0);

	g_signal_connect (G_OBJECT (find_dialog.button5),
	                    "clicked", G_CALLBACK (find_clicked), NULL);

	g_signal_connect (G_OBJECT (find_dialog.window1), "key_press_event", G_CALLBACK (find_key_press_event), NULL);

	// Hide the dialog box when the user clicks the cancel_button
	//g_signal_connect_object (G_OBJECT (find_dialog.button4),
	//                           "clicked", G_CALLBACK (gtk_widget_hide), (gpointer) find_dialog.window1);
	// Don't hide it, destroy it, the same as closing it (for consistency) - AJ 2005-10-14
	g_signal_connect (G_OBJECT (find_dialog.button4),
	                           "clicked", G_CALLBACK (find_destroy), NULL);
							   
	g_signal_connect(G_OBJECT(find_dialog.window1), "destroy", G_CALLBACK(find_destroyed), NULL);
}

// -----------------------------------------------------------------------------

void replace_destroyed(GtkWidget *widget, gpointer data)
{
	replace_dialog.window2=NULL;
}

void replace_destroy(GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy(replace_dialog.window2);
}


void replace_clicked(GtkButton *button, gpointer data)
{
	static gint last_found = 0;
	gboolean whole_document;
	gint search_flags = 0;
	gchar *text;
	gchar *replace;
	glong length_of_document;
	glong current_pos;
	glong start_found;
	glong end_found;
	glong result;

	length_of_document = gtk_scintilla_get_length(GTK_SCINTILLA(main_window.current_editor->scintilla));
	current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla));

	whole_document = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(replace_dialog.radiobutton17));
	text = gtk_editable_get_chars (GTK_EDITABLE(replace_dialog.entry1), 0, -1);
	replace = gtk_editable_get_chars (GTK_EDITABLE(replace_dialog.entry2), 0, -1);

	if (whole_document) {
		current_pos = 0;
		gtk_scintilla_goto_pos(GTK_SCINTILLA(main_window.current_editor->scintilla), current_pos);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(replace_dialog.radiobutton18), TRUE);
	}

	if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(replace_dialog.checkbutton7))) {
		search_flags += SCFIND_MATCHCASE;
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(replace_dialog.checkbutton6))) {
		search_flags += SCFIND_WHOLEWORD;
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(replace_dialog.radiobutton21))) {
		search_flags += SCFIND_REGEXP;
	}

	result = gtk_scintilla_find_text (GTK_SCINTILLA(main_window.current_editor->scintilla),
	                                  search_flags, text, current_pos, length_of_document, &start_found, &end_found);

	if (result == -1) {
		// Show message saying could not be found.
                 GtkWidget *dialog;
                 dialog = gtk_message_dialog_new(GTK_WINDOW(main_window.window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,_("The text \"%s\" was not found."), text);
                 gtk_window_set_title(GTK_WINDOW(dialog), "gphpedit");
		 gtk_window_set_icon(GTK_WINDOW(dialog), get_window_icon());
                 gtk_dialog_run(GTK_DIALOG(dialog));
                 gtk_widget_destroy(dialog);
	}
	else {
		if (start_found == last_found) {
			return;
		}

		last_found = start_found;
		gtk_scintilla_goto_pos(GTK_SCINTILLA(main_window.current_editor->scintilla), start_found);	
		gtk_scintilla_set_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla), start_found);
		gtk_scintilla_set_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla), end_found);
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(replace_dialog.checkbutton8))) {
			// Prompt for replace?
                 replace_prompt_dialog = gtk_message_dialog_new(GTK_WINDOW(main_window.window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,
            _("Do you want to replace this occurence?"));
            gtk_window_set_title(GTK_WINDOW(replace_prompt_dialog), "Question");
	    gtk_window_set_icon(GTK_WINDOW(replace_prompt_dialog), get_window_icon());
            gint result = gtk_dialog_run (GTK_DIALOG (replace_prompt_dialog));
            gint selection_start;
            gchar *replace;
           if (result==GTK_RESPONSE_YES) {
            	replace = gtk_editable_get_chars (GTK_EDITABLE(replace_dialog.entry2), 0, -1);
		selection_start = gtk_scintilla_get_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla));
		gtk_scintilla_replace_sel(GTK_SCINTILLA(main_window.current_editor->scintilla), replace);
		gtk_scintilla_set_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla), selection_start);
		gtk_scintilla_set_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla), selection_start + strlen(replace));
                }
            replace_clicked(NULL, NULL);
            gtk_widget_destroy(replace_prompt_dialog);
		}
		else {
			gtk_scintilla_replace_sel(GTK_SCINTILLA(main_window.current_editor->scintilla), replace);
			gtk_scintilla_goto_pos(GTK_SCINTILLA(main_window.current_editor->scintilla), start_found);	
			gtk_scintilla_set_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla), start_found);
			gtk_scintilla_set_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla), start_found + strlen(replace));
		}
	}
}

void replace_all_clicked(GtkButton *button, gpointer data)
{
	static gint last_found = 0;
	gint search_flags = 0;
	gchar *text;
	gchar *replace;
	glong length_of_document;
	glong current_pos;
	glong start_found;
	glong end_found;
	glong result;
	gint numfound;
	GtkWidget *replace_all_dialog;
	GString *message;
	gint start_pos;
	
	gtk_widget_hide(replace_dialog.window2);

	length_of_document = gtk_scintilla_get_length(GTK_SCINTILLA(main_window.current_editor->scintilla));

	text = gtk_editable_get_chars (GTK_EDITABLE(replace_dialog.entry1), 0, -1);
	replace = gtk_editable_get_chars (GTK_EDITABLE(replace_dialog.entry2), 0, -1);

	start_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla));

	current_pos = 0;
	gtk_scintilla_goto_pos(GTK_SCINTILLA(main_window.current_editor->scintilla), current_pos);

	if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(replace_dialog.checkbutton7))) {
		search_flags += SCFIND_MATCHCASE;
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(replace_dialog.checkbutton6))) {
		search_flags += SCFIND_WHOLEWORD;
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(replace_dialog.radiobutton21))) {
		search_flags += SCFIND_REGEXP;
	}

	numfound=0;

	result = gtk_scintilla_find_text (GTK_SCINTILLA(main_window.current_editor->scintilla),
	                                  search_flags, text, current_pos, length_of_document, &start_found, &end_found);

	while (result != -1) {
		if (start_found == last_found) {
			break;
		}

		last_found = start_found;
		gtk_scintilla_set_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla), start_found);
		gtk_scintilla_set_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla), end_found);
		gtk_scintilla_replace_sel(GTK_SCINTILLA(main_window.current_editor->scintilla), replace);
		gtk_scintilla_set_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla), start_found);
		gtk_scintilla_set_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla), start_found + strlen(replace));
		numfound++;
		result = gtk_scintilla_find_text (GTK_SCINTILLA(main_window.current_editor->scintilla),
		                                  search_flags, text, current_pos, length_of_document, &start_found, &end_found);
	}

	message = g_string_new("");
	if (numfound==0) {
		g_string_sprintf(message, _("%s not found, no replacements made."), text);
	}
	else if (numfound==1) {
		g_string_sprintf(message, _("1 occurence of %s found, replaced."), text);
	}
	else {
		g_string_sprintf(message, _("%d occurences of %s found, all replaced."), numfound, text);
	}

         replace_all_dialog = gtk_message_dialog_new(GTK_WINDOW(main_window.window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"%s",message->str);
         gtk_window_set_title(GTK_WINDOW(replace_all_dialog), "gphpedit");
	 gtk_window_set_icon(GTK_WINDOW(replace_all_dialog), get_window_icon());
         gtk_dialog_run(GTK_DIALOG(replace_all_dialog));
         gtk_widget_destroy(replace_all_dialog);

	gtk_scintilla_goto_pos(GTK_SCINTILLA(main_window.current_editor->scintilla), start_pos);
}


gint replace_key_press_event(GtkWidget *widget, GdkEventKey *event,gpointer user_data)
{
	if (event->keyval == GDK_Return) {
		replace_clicked(NULL,NULL);
		return TRUE;
	}
	else if (event->keyval == GDK_Escape) {
		replace_destroy(NULL, NULL);
		return TRUE;
	}
	return FALSE;
}

void replace_create(void)
{
	replace_dialog.radiobutton17_group = NULL;
	replace_dialog.radiobutton18_group = NULL;
	replace_dialog.radiobutton19_group = NULL;
	replace_dialog.radiobutton20_group = NULL;
	replace_dialog.radiobutton21_group = NULL;
	replace_dialog.radiobutton22_group = NULL;

	replace_dialog.window2 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position (GTK_WINDOW(replace_dialog.window2),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (replace_dialog.window2), 6);
	gtk_window_set_title (GTK_WINDOW (replace_dialog.window2), _("Find and Replace"));
	gtk_window_set_icon(GTK_WINDOW(replace_dialog.window2), get_window_icon());
        

	replace_dialog.vbox10 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (replace_dialog.vbox10);
	gtk_container_add (GTK_CONTAINER (replace_dialog.window2), replace_dialog.vbox10);

	replace_dialog.frame10 = gtk_frame_new (NULL);
	gtk_widget_show (replace_dialog.frame10);
	gtk_box_pack_start (GTK_BOX (replace_dialog.vbox10), replace_dialog.frame10, TRUE, TRUE, 0);

	replace_dialog.alignment1 = gtk_alignment_new(0,0,1,1);
	gtk_container_add (GTK_CONTAINER (replace_dialog.frame10), replace_dialog.alignment1);
	gtk_alignment_set_padding(GTK_ALIGNMENT(replace_dialog.alignment1), 6, 6, 6, 6);
	gtk_widget_show(replace_dialog.alignment1);
	
	replace_dialog.entry1 = gtk_entry_new ();
	gtk_widget_show (replace_dialog.entry1);
	gtk_container_add (GTK_CONTAINER (replace_dialog.alignment1), replace_dialog.entry1);
	/* Get selected text (Wendell) */
	gint wordStart;
	gint wordEnd;
	gint length;
	gchar *buffer;

	if (main_window.current_editor->type != TAB_HELP) {
		wordStart = gtk_scintilla_get_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla));
		wordEnd = gtk_scintilla_get_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla));
		if (wordStart != wordEnd) {
			buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(main_window.current_editor->scintilla), wordStart, wordEnd, &length);
			gtk_entry_set_text(GTK_ENTRY(replace_dialog.entry1), buffer);
		}
	}
	/* End get selected text */
	
	replace_dialog.label10 = gtk_label_new (_("RegExp/String to search"));
	gtk_widget_show (replace_dialog.label10);
	gtk_frame_set_label_widget (GTK_FRAME (replace_dialog.frame10), replace_dialog.label10);
	gtk_label_set_justify (GTK_LABEL (replace_dialog.label10), GTK_JUSTIFY_LEFT);

	replace_dialog.frame15 = gtk_frame_new (NULL);
	gtk_widget_show (replace_dialog.frame15);
	gtk_box_pack_start (GTK_BOX (replace_dialog.vbox10), replace_dialog.frame15, TRUE, TRUE, 0);

	replace_dialog.alignment2 = gtk_alignment_new(0,0,1,1);
	gtk_container_add (GTK_CONTAINER (replace_dialog.frame15), replace_dialog.alignment2);
	gtk_alignment_set_padding(GTK_ALIGNMENT(replace_dialog.alignment2), 6, 6, 6, 6);
	gtk_widget_show(replace_dialog.alignment2);
	
	replace_dialog.entry2 = gtk_entry_new ();
	gtk_widget_show (replace_dialog.entry2);
	gtk_container_add (GTK_CONTAINER (replace_dialog.alignment2), replace_dialog.entry2);

	replace_dialog.label15 = gtk_label_new (_("String to Replace"));
	gtk_widget_show (replace_dialog.label15);
	gtk_frame_set_label_widget (GTK_FRAME (replace_dialog.frame15), replace_dialog.label15);
	gtk_label_set_justify (GTK_LABEL (replace_dialog.label15), GTK_JUSTIFY_LEFT);

	replace_dialog.frame11 = gtk_frame_new (NULL);
	gtk_widget_show (replace_dialog.frame11);
	gtk_box_pack_start (GTK_BOX (replace_dialog.vbox10), replace_dialog.frame11, TRUE, TRUE, 0);

	replace_dialog.vbox14 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (replace_dialog.vbox14);
	gtk_container_add (GTK_CONTAINER (replace_dialog.frame11), replace_dialog.vbox14);

	replace_dialog.hbox9 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (replace_dialog.hbox9);
	gtk_box_pack_start (GTK_BOX (replace_dialog.vbox14), replace_dialog.hbox9, TRUE, TRUE, 0);

	replace_dialog.frame16 = gtk_frame_new (NULL);
	gtk_widget_show (replace_dialog.frame16);
	gtk_box_pack_start (GTK_BOX (replace_dialog.hbox9), replace_dialog.frame16, TRUE, TRUE, 4);

	replace_dialog.vbox15 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (replace_dialog.vbox15);
	gtk_container_add (GTK_CONTAINER (replace_dialog.frame16), replace_dialog.vbox15);

	replace_dialog.radiobutton17 = gtk_radio_button_new_with_mnemonic (NULL, _("Whole document"));
	gtk_widget_show (replace_dialog.radiobutton17);
	gtk_box_pack_start (GTK_BOX (replace_dialog.vbox15), replace_dialog.radiobutton17, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (replace_dialog.radiobutton17), 2);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (replace_dialog.radiobutton17), replace_dialog.radiobutton17_group);
	replace_dialog.radiobutton17_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (replace_dialog.radiobutton17));

	replace_dialog.radiobutton18 = gtk_radio_button_new_with_mnemonic (NULL, _("From cursor"));
	gtk_widget_show (replace_dialog.radiobutton18);
	gtk_box_pack_start (GTK_BOX (replace_dialog.vbox15), replace_dialog.radiobutton18, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (replace_dialog.radiobutton18), 2);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (replace_dialog.radiobutton18), replace_dialog.radiobutton17_group);
	replace_dialog.radiobutton17_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (replace_dialog.radiobutton18));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (replace_dialog.radiobutton17), TRUE);

	replace_dialog.label16 = gtk_label_new (_("Search"));
	gtk_widget_show (replace_dialog.label16);
	gtk_frame_set_label_widget (GTK_FRAME (replace_dialog.frame16), replace_dialog.label16);
	gtk_label_set_justify (GTK_LABEL (replace_dialog.label16), GTK_JUSTIFY_LEFT);

	/*replace_dialog.frame17 = gtk_frame_new (NULL);
	gtk_widget_show (replace_dialog.frame17);
	gtk_box_pack_start (GTK_BOX (replace_dialog.hbox9), replace_dialog.frame17, FALSE, FALSE, 4);

	replace_dialog.vbox16 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (replace_dialog.vbox16);
	gtk_container_add (GTK_CONTAINER (replace_dialog.frame17), replace_dialog.vbox16);

	replace_dialog.radiobutton19 = gtk_radio_button_new_with_mnemonic (NULL, _("Forwards"));
	gtk_widget_show (replace_dialog.radiobutton19);
	gtk_box_pack_start (GTK_BOX (replace_dialog.vbox16), replace_dialog.radiobutton19, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (replace_dialog.radiobutton19), 2);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (replace_dialog.radiobutton19), replace_dialog.radiobutton19_group);
	replace_dialog.radiobutton19_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (replace_dialog.radiobutton19));

	replace_dialog.radiobutton20 = gtk_radio_button_new_with_mnemonic (NULL, _("Backwards"));
	gtk_widget_show (replace_dialog.radiobutton20);
	gtk_box_pack_start (GTK_BOX (replace_dialog.vbox16), replace_dialog.radiobutton20, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (replace_dialog.radiobutton20), 2);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (replace_dialog.radiobutton20), replace_dialog.radiobutton20_group);
	replace_dialog.radiobutton20_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (replace_dialog.radiobutton20));

	replace_dialog.label17 = gtk_label_new (_("Direction"));
	gtk_widget_show (replace_dialog.label17);
	gtk_frame_set_label_widget (GTK_FRAME (replace_dialog.frame17), replace_dialog.label17);
	gtk_label_set_justify (GTK_LABEL (replace_dialog.label17), GTK_JUSTIFY_LEFT);*/

	replace_dialog.frame18 = gtk_frame_new (NULL);
	gtk_widget_show (replace_dialog.frame18);
	gtk_box_pack_start (GTK_BOX (replace_dialog.hbox9), replace_dialog.frame18, TRUE, TRUE, 4);

	replace_dialog.vbox17 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (replace_dialog.vbox17);
	gtk_container_add (GTK_CONTAINER (replace_dialog.frame18), replace_dialog.vbox17);

	replace_dialog.radiobutton21 = gtk_radio_button_new_with_mnemonic (NULL, _("RegExp"));
	gtk_widget_show (replace_dialog.radiobutton21);
	gtk_box_pack_start (GTK_BOX (replace_dialog.vbox17), replace_dialog.radiobutton21, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (replace_dialog.radiobutton21), 2);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (replace_dialog.radiobutton21), replace_dialog.radiobutton21_group);
	replace_dialog.radiobutton21_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (replace_dialog.radiobutton21));

	replace_dialog.radiobutton22 = gtk_radio_button_new_with_mnemonic (NULL, _("String"));
	gtk_widget_show (replace_dialog.radiobutton22);
	gtk_box_pack_start (GTK_BOX (replace_dialog.vbox17), replace_dialog.radiobutton22, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (replace_dialog.radiobutton22), 2);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (replace_dialog.radiobutton22), replace_dialog.radiobutton21_group);
	replace_dialog.radiobutton21_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (replace_dialog.radiobutton22));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (replace_dialog.radiobutton22), TRUE);

	replace_dialog.label18 = gtk_label_new (_("Type"));
	gtk_widget_show (replace_dialog.label18);
	gtk_frame_set_label_widget (GTK_FRAME (replace_dialog.frame18), replace_dialog.label18);
	gtk_label_set_justify (GTK_LABEL (replace_dialog.label18), GTK_JUSTIFY_LEFT);

	replace_dialog.hbox10 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (replace_dialog.hbox10);
	gtk_box_pack_start (GTK_BOX (replace_dialog.vbox14), replace_dialog.hbox10, TRUE, TRUE, 0);

	replace_dialog.checkbutton6 = gtk_check_button_new_with_mnemonic (_("Whole word"));
	gtk_widget_show (replace_dialog.checkbutton6);
	gtk_box_pack_start (GTK_BOX (replace_dialog.hbox10), replace_dialog.checkbutton6, FALSE, FALSE, 4);

	replace_dialog.checkbutton7 = gtk_check_button_new_with_mnemonic (_("Ignore case while searching"));
	gtk_widget_show (replace_dialog.checkbutton7);
	gtk_box_pack_start (GTK_BOX (replace_dialog.hbox10), replace_dialog.checkbutton7, TRUE, FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (replace_dialog.checkbutton7), TRUE);

	replace_dialog.checkbutton8 = gtk_check_button_new_with_mnemonic (_("Prompt before Replace"));
	gtk_widget_show (replace_dialog.checkbutton8);
	gtk_box_pack_start (GTK_BOX (replace_dialog.vbox14), replace_dialog.checkbutton8, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (replace_dialog.checkbutton8), 4);

	replace_dialog.label14 = gtk_label_new (_("Options"));
	gtk_widget_show (replace_dialog.label14);
	gtk_frame_set_label_widget (GTK_FRAME (replace_dialog.frame11), replace_dialog.label14);
	gtk_label_set_justify (GTK_LABEL (replace_dialog.label14), GTK_JUSTIFY_LEFT);

	replace_dialog.hseparator2 = gtk_hseparator_new ();
	gtk_widget_show (replace_dialog.hseparator2);
	gtk_box_pack_start (GTK_BOX (replace_dialog.vbox10), replace_dialog.hseparator2, TRUE, TRUE, 0);

	replace_dialog.hbox8 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (replace_dialog.hbox8);
	gtk_box_pack_start (GTK_BOX (replace_dialog.vbox10), replace_dialog.hbox8, TRUE, TRUE, 0);

	replace_dialog.button7 = gtk_button_new_from_stock ("gtk-close");
	gtk_widget_show (replace_dialog.button7);
	gtk_box_pack_start (GTK_BOX (replace_dialog.hbox8), replace_dialog.button7, TRUE, FALSE, 0);

	replace_dialog.button8 = gtk_button_new_from_stock ("gtk-find-and-replace");
	gtk_widget_show (replace_dialog.button8);
	gtk_box_pack_start (GTK_BOX (replace_dialog.hbox8), replace_dialog.button8, TRUE, FALSE, 0);

	replace_dialog.button9 = gtk_button_new_with_mnemonic(_("Replace _all"));
	gtk_widget_show (replace_dialog.button9);
	gtk_box_pack_start (GTK_BOX (replace_dialog.hbox8), replace_dialog.button9, TRUE, FALSE, 0);

	g_signal_connect (G_OBJECT (replace_dialog.button8),
	                    "clicked", G_CALLBACK (replace_clicked), NULL);

	g_signal_connect (G_OBJECT (replace_dialog.button9),
	                    "clicked", G_CALLBACK (replace_all_clicked), NULL);

	g_signal_connect (G_OBJECT (replace_dialog.window2), "key_press_event", G_CALLBACK (replace_key_press_event), NULL);

	// Hide the dialog box when the user clicks the cancel_button
	//g_signal_connect (G_OBJECT (replace_dialog.button7),
	//                           "clicked", G_CALLBACK (gtk_widget_hide), (gpointer) replace_dialog.window2);
	// Don't hide it, destroy it, the same as closing it (for consistency) - AJ 2005-10-14
	g_signal_connect (G_OBJECT (replace_dialog.button7),
	                           "clicked", G_CALLBACK (replace_destroy), NULL);
							   
	g_signal_connect(G_OBJECT(replace_dialog.window2), "destroy",
	                   G_CALLBACK(replace_destroyed), NULL);
}
