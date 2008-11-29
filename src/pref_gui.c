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

static GtkWidget *spin_dict_label;
static GtkWidget *spin_words;
static GtkWidget *check_splash;
static GtkWidget *check_heading_auto;
static GtkWidget *spin_max_heading;
static GtkWidget *check_button_color;

gboolean pref_end_gui()
{
	LOG(LOG_DEBUG, "IN : pref_end_gui()");

	max_remember_words = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_words));
	dict_button_length = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_dict_label));

	bshow_splash = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_splash));

	max_heading = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_max_heading));

	bheading_auto_calc = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_heading_auto));

	benable_button_color = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_color));

	LOG(LOG_DEBUG, "OUT : pref_end_gui()");
	return(TRUE);
}


GtkWidget *pref_start_gui(){
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *table;
	GtkWidget *label;
	GtkObject *adj;
	GtkAttachOptions xoption, yoption;

	LOG(LOG_DEBUG, "IN : pref_start_gui()");

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_set_size_request(vbox, 300, 200);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);

	xoption = GTK_SHRINK|GTK_FILL;
	yoption = GTK_SHRINK;

	table = gtk_table_new(2, 12, FALSE);
	gtk_box_pack_start (GTK_BOX(vbox)
			    , table,FALSE, FALSE, 0);


	hbox = gtk_hbox_new(FALSE,10);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);
	gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 0, 1,
			 xoption, yoption, 10, 10);	

	label = gtk_label_new(_("Maximum words in history"));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX(hbox), 
			    label, FALSE, FALSE, 0);


	adj = gtk_adjustment_new( 10, //value
				  0, // lower
				  20, //upper
                                  1, // step increment
                                  10,// page_increment,
                                  (gfloat)0.0);
	spin_words = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1.0, 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_words), max_remember_words );
	gtk_widget_set_size_request(spin_words,60,20);
	gtk_table_attach(GTK_TABLE(table), spin_words, 1, 2, 0, 1,
			 xoption, yoption, 10, 10);	

	gtk_tooltips_set_tip(tooltip, spin_words, 
			     _("Maximum number of words to remember in word history"), "Private");


	hbox = gtk_hbox_new(FALSE,10);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);
	gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 1, 2,
			 xoption, yoption, 10, 10);	


	label = gtk_label_new(_("Chars in dictionary bar"));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX(hbox), 
			    label, FALSE, FALSE, 0);

	adj = gtk_adjustment_new( 10, //value
				  1, // lower
				  32, //upper
                                  1, // step increment
                                  1,// page_increment,
                                  0.0);
	spin_dict_label = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1.0, 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_dict_label), dict_button_length );
	gtk_spin_button_set_snap_to_ticks(GTK_SPIN_BUTTON(spin_dict_label), TRUE);
	gtk_widget_set_size_request(spin_dict_label,60,20);
	gtk_table_attach(GTK_TABLE(table), spin_dict_label, 1, 2, 1, 2,
			 xoption, yoption, 10, 10);	

	gtk_tooltips_set_tip(tooltip, spin_dict_label, 
			     _("Specify the number of characters to display on top of each toggle buttons in dictionary bar."),"Private");



	check_splash = gtk_check_button_new_with_label(_("Show splash screen"));
	gtk_tooltips_set_tip(tooltip, check_splash, 
			     _("Show splash screen on loading."),"Private");

	gtk_table_attach(GTK_TABLE(table), check_splash, 0, 1, 5, 6,
			 xoption, yoption, 10, 10);	

	// 
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_splash), bshow_splash);

	check_heading_auto = gtk_check_button_new_with_label(_("Calculate heading automatically"));
	gtk_tooltips_set_tip(tooltip, check_heading_auto,
			     _("Calculate the number of cells in heading list to suit the window size."),"Private");

	gtk_table_attach(GTK_TABLE(table), check_heading_auto, 0, 1, 6, 7,
			 xoption, yoption, 10, 10);	

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_heading_auto), bheading_auto_calc);


	// 
	hbox = gtk_hbox_new(FALSE,10);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);
	gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 7, 8,
			 xoption, yoption, 10, 10);	

	label = gtk_label_new(_("Maximum hits to display"));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX(hbox), 
			    label, FALSE, FALSE, 0);
	

	adj = gtk_adjustment_new( 100, //value
				  1, // lower
				  1000, //upper
                                  1, // step increment
                                  10,// page_increment,
                                  0.0);
	spin_max_heading = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1.0, 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_max_heading), max_heading );
	gtk_widget_set_size_request(spin_max_heading,60,20);
	gtk_table_attach(GTK_TABLE(table), spin_max_heading, 1, 2, 7, 8,
			 xoption, yoption, 10, 10);	

	gtk_tooltips_set_tip(tooltip, spin_max_heading, 
			     _("Maximum number of hits to be displayed at once.\nYou can go forward and backward using buttons. Valid only if automatic calculation is disabled."), "Private");

	//
	check_button_color = gtk_check_button_new_with_label(_("Enable dictionary button color"));
	gtk_tooltips_set_tip(tooltip, check_button_color,
			     _("Enable background color of dictionary button."),"Private");

	gtk_table_attach(GTK_TABLE(table), check_button_color, 0, 1, 8, 9,
			 xoption, yoption, 10, 10);	

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_color), benable_button_color);


	LOG(LOG_DEBUG, "OUT : pref_start_gui()");

	return(vbox);
}
