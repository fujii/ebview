/*  Copyright (C) 2001-2004  Kenichi Suto
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/


#include "defs.h"
#include "global.h"


GtkWidget *entry_wave;
GtkWidget *entry_mpeg;
GtkWidget *entry_browser;
GtkWidget *entry_open;
static GtkWidget *check_sound;

gboolean pref_end_external(){

	const gchar *text;

	LOG(LOG_DEBUG, "IN : pref_end_external()");

	text = gtk_entry_get_text(GTK_ENTRY(entry_mpeg));
	if(mpeg_template)
		free(mpeg_template);
	mpeg_template = strdup(text);

	text = gtk_entry_get_text(GTK_ENTRY(entry_wave));
	if(wave_template)
		free(wave_template);
	wave_template = strdup(text);

	text = gtk_entry_get_text(GTK_ENTRY(entry_browser));
	if(browser_template)
		free(browser_template);
	browser_template = strdup(text);

	text = gtk_entry_get_text(GTK_ENTRY(entry_open));
	if(open_template)
		free(open_template);
	open_template = strdup(text);

	bplay_sound_internally = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_sound));


	LOG(LOG_DEBUG, "OUT : pref_end_external()");
	return(TRUE);
}


GtkWidget *pref_start_external()
{
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *label;
	GtkSizeGroup *label_group;
	GtkSizeGroup *entry_group;

	label_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	entry_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	LOG(LOG_DEBUG, "IN : pref_start_external()");

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
//	gtk_container_add(GTK_CONTAINER(frame), vbox);

	check_sound = gtk_check_button_new_with_label(_("Play sound internally"));
	gtk_box_pack_start (GTK_BOX(vbox)
			    , check_sound,FALSE, FALSE, 5);
	gtk_tooltips_set_tip(tooltip, check_sound, 
			     _("Use internal routine to play sound. Valid only on windows."),"Private");

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_sound), bplay_sound_internally);


	hbox = gtk_hbox_new(FALSE,10);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);
	gtk_box_pack_start (GTK_BOX(vbox)
			    , hbox,FALSE, FALSE, 5);

	label = gtk_label_new(_("Command to play sound "));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX(hbox), label,FALSE, FALSE, 0);
	gtk_size_group_add_widget (label_group, label);


	entry_wave = gtk_entry_new();
	gtk_box_pack_start (GTK_BOX(hbox)
			    , entry_wave,TRUE, TRUE, 0);
	gtk_size_group_add_widget (entry_group, entry_wave);

	gtk_tooltips_set_tip(tooltip, entry_wave, 
			     _("External command to play WAVE sound. %f will be replaced by data file name."),"Private");

	gtk_entry_set_text(GTK_ENTRY(entry_wave), wave_template);


	hbox = gtk_hbox_new(FALSE,10);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);
	gtk_box_pack_start (GTK_BOX(vbox)
			    , hbox,FALSE, FALSE, 5);

	label = gtk_label_new(_("Command to play movie "));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
//	gtk_widget_set_size_request( label, 120, 20 );
	gtk_box_pack_start (GTK_BOX(hbox), label,FALSE, FALSE, 0);
	gtk_size_group_add_widget (label_group, label);


	entry_mpeg = gtk_entry_new();
	gtk_box_pack_start (GTK_BOX(hbox)
			    , entry_mpeg, TRUE, TRUE, 0);
	gtk_size_group_add_widget (entry_group, entry_mpeg);
	gtk_tooltips_set_tip(tooltip, entry_mpeg, 
			     _("External command to play MPEG movie. %f will be replaced by data file name."),"Private");
	gtk_entry_set_text(GTK_ENTRY(entry_mpeg), mpeg_template);


	hbox = gtk_hbox_new(FALSE,10);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);
	gtk_box_pack_start (GTK_BOX(vbox)
			    , hbox,FALSE, FALSE, 5);


	label = gtk_label_new(_("Command to launch web browser "));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
//	gtk_widget_set_size_request( label, 120, 20 );
	gtk_box_pack_start (GTK_BOX(hbox), label,FALSE, FALSE, 0);
	gtk_size_group_add_widget (label_group, label);


	entry_browser = gtk_entry_new();
	gtk_box_pack_start (GTK_BOX(hbox)
			    , entry_browser, TRUE, TRUE, 0);
	gtk_size_group_add_widget (entry_group, entry_browser);
	gtk_tooltips_set_tip(tooltip, entry_browser, 
			     _("External command to launch Web browser. %f will be replaced by URL."),"Private");
	gtk_entry_set_text(GTK_ENTRY(entry_browser), browser_template);



	hbox = gtk_hbox_new(FALSE,10);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);
	gtk_box_pack_start (GTK_BOX(vbox)
			    , hbox,FALSE, FALSE, 5);


	label = gtk_label_new(_("Standard command to open file "));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
//	gtk_widget_set_size_request( label, 120, 20 );
	gtk_box_pack_start (GTK_BOX(hbox), label,FALSE, FALSE, 0);
	gtk_size_group_add_widget (label_group, label);


	entry_open = gtk_entry_new();
	gtk_box_pack_start (GTK_BOX(hbox)
			    , entry_open, TRUE, TRUE, 0);
	gtk_size_group_add_widget (entry_group, entry_open);
	gtk_tooltips_set_tip(tooltip, entry_open, 
			     _("Standard command to open file. %f will be replaced by filename, %l by line number."),"Private");
	gtk_entry_set_text(GTK_ENTRY(entry_open), open_template);


	LOG(LOG_DEBUG, "OUT : pref_start_external()");

	return(vbox);
}

