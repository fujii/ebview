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

static GtkWidget *spin_interval;
static GtkWidget *spin_minchar;
static GtkWidget *spin_maxchar;
static GtkWidget *spin_popup_width;
static GtkWidget *spin_popup_height;
static GtkWidget *check_beep;
static GtkWidget *check_popup_title;

gboolean pref_end_selection(GtkWidget *widget,gpointer *data){

	LOG(LOG_DEBUG, "IN : pref_end_selection()");

	auto_interval = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_interval));
	auto_minchar = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_minchar));
	auto_maxchar = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_maxchar));
	popup_width = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_popup_width));
	popup_height = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_popup_height));

	bbeep_on_nohit = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_beep));
	bshow_popup_title = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_popup_title));

	LOG(LOG_DEBUG, "OUT : pref_end_selection()");
	return(TRUE);
}



GtkWidget *pref_start_selection()
{

	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkObject *adj;
	GtkWidget *table;
	GtkAttachOptions xoption=GTK_SHRINK|GTK_FILL, yoption=0;

	LOG(LOG_DEBUG, "IN : pref_start_selection()");

	vbox = gtk_vbox_new(FALSE,10);
	gtk_widget_set_size_request(vbox, 400, 300);

	table = gtk_table_new(4, 8, FALSE);
	gtk_box_pack_start (GTK_BOX(vbox)
			    , table, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE,10);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);
	gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 0, 1,
			 xoption, yoption, 10, 10);	
	
	label = gtk_label_new(_("Lookup interval (ms)"));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
//	gtk_widget_set_size_request( label, 120, 20 );
	gtk_box_pack_start (GTK_BOX(hbox), label,FALSE, FALSE, 0);

	adj = gtk_adjustment_new( 1000, //value
				  10, // lower
				  10000, //upper
                                  100, // step increment
                                  1000,// page_increment,
                                  (gfloat)0.0);
	spin_interval = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1.0, 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_interval), auto_interval );
	gtk_widget_set_size_request(spin_interval,60,20);
//	gtk_box_pack_end (GTK_BOX(hbox), spin_interval, FALSE, FALSE, 0);
	gtk_table_attach(GTK_TABLE(table), spin_interval, 3, 4, 0, 1,
			 xoption, yoption, 10, 10);	

	gtk_tooltips_set_tip(tooltip, spin_interval,
			     _("Interval to check selection. \nIncreasing this number may eat up your CPU.\nIgnored on Windows."), "Private");

	hbox = gtk_hbox_new(FALSE,10);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);
	gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 1, 2,
			 xoption, yoption, 10, 10);	


	label = gtk_label_new(_("Minimum chars for selection lookup"));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
//	gtk_widget_set_size_request( label, 120, 20 );
	gtk_box_pack_start (GTK_BOX(hbox), label,FALSE, FALSE, 0);

	

	adj = gtk_adjustment_new( 3, //value
				  0, // lower
				  10, //upper
                                  1, // step increment
                                  5,// page_increment,
                                  (gfloat)0.0);
	spin_minchar = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1.0, 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_minchar), auto_minchar );
	gtk_widget_set_size_request(spin_minchar,60,20);
//	gtk_box_pack_end (GTK_BOX(hbox), spin_minchar, FALSE, FALSE, 0);
	gtk_table_attach(GTK_TABLE(table), spin_minchar, 3, 4, 1, 2,
			 xoption, yoption, 10, 10);	

	gtk_tooltips_set_tip(tooltip, spin_minchar,
			     _("When the number of characters in selection is less than this number, it will not be looked up."), "Private");


	hbox = gtk_hbox_new(FALSE,10);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);
	gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 2, 3,
			 xoption, yoption, 10, 10);	
//	gtk_box_pack_start (GTK_BOX(vbox)
//			    , hbox,FALSE, FALSE, 0);

	label = gtk_label_new(_("Maximum chars for automatic lookup"));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
//	gtk_widget_set_size_request( label, 120, 20 );
	gtk_box_pack_start (GTK_BOX(hbox), label,FALSE, FALSE, 0);

	

	adj = gtk_adjustment_new(100, //value
				  1, // lower
				  1000, //upper
                                  1, // step increment
                                  10,// page_increment,
                                  (gfloat)0.0);
	spin_maxchar = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1.0, 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_maxchar), auto_maxchar );
	gtk_widget_set_size_request(spin_maxchar,60,20);
//	gtk_box_pack_end (GTK_BOX(hbox), spin_maxchar, FALSE, FALSE, 0);
	gtk_table_attach(GTK_TABLE(table), spin_maxchar, 3, 4, 2, 3,
			 xoption, yoption, 10, 10);	

	gtk_tooltips_set_tip(tooltip, spin_maxchar,
			     _("When the number of characters in selection is larger than this number, it will not be looked up."), "Private");


	hbox = gtk_hbox_new(FALSE,10);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);
	gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 3, 4,
			 xoption, yoption, 10, 10);	
//	gtk_box_pack_start (GTK_BOX(vbox)
//			    , hbox,FALSE, FALSE, 0);

	label = gtk_label_new(_("Popup window size"));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
//	gtk_widget_set_size_request( label, 120, 20 );
	gtk_box_pack_start (GTK_BOX(hbox), label,FALSE, FALSE, 0);




	adj = gtk_adjustment_new(100, //value
				  1, // lower
				  1024, //upper
                                  10, // step increment
                                  10,// page_increment,
                                  (gfloat)0.0);
	spin_popup_height = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1.0, 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_popup_height), popup_height );
	gtk_widget_set_size_request(spin_popup_height,60,20);
//	gtk_box_pack_end (GTK_BOX(hbox), spin_popup_height, FALSE, FALSE, 0);
	gtk_table_attach(GTK_TABLE(table), spin_popup_height, 1, 2, 3, 4,
			 xoption, yoption, 10, 10);	


	label = gtk_label_new(" x ");
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
//	gtk_widget_set_size_request( label, 120, 20 );
//	gtk_box_pack_end(GTK_BOX(hbox), label,FALSE, FALSE, 0);
	gtk_table_attach(GTK_TABLE(table), label, 2, 3, 3, 4,
			 xoption, yoption, 10, 10);	


	adj = gtk_adjustment_new(100, //value
				  1, // lower
				  1024, //upper
                                  10, // step increment
                                  10,// page_increment,
                                  (gfloat)0.0);
	spin_popup_width = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1.0, 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_popup_width), popup_width );
	gtk_widget_set_size_request(spin_popup_width,60,20);
//	gtk_box_pack_end(GTK_BOX(hbox), spin_popup_width, FALSE, FALSE, 0);
	gtk_table_attach(GTK_TABLE(table), spin_popup_width, 3, 4, 3, 4,
			 xoption, yoption, 10, 10);	



/*
	hbox = gtk_hbox_new(FALSE,10);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);
	gtk_box_pack_start (GTK_BOX(vbox)
			    , hbox,FALSE, FALSE, 0);
*/

	check_popup_title = gtk_check_button_new_with_label(_("Show popup title"));
	gtk_tooltips_set_tip(tooltip, check_popup_title, 
			     _("Show title of popup window."),"Private");
//	gtk_box_pack_start (GTK_BOX(hbox)
//			    , check_popup_title,FALSE,FALSE, 0);
	gtk_table_attach(GTK_TABLE(table), check_popup_title, 0, 1, 4, 5,
			 xoption, yoption, 10, 10);	

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_popup_title), bshow_popup_title);

/*
	hbox = gtk_hbox_new(FALSE,10);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);
	gtk_box_pack_start (GTK_BOX(vbox)
			    , hbox,FALSE, FALSE, 0);
*/

	check_beep = gtk_check_button_new_with_label(_("Beep on no hit"));
	gtk_tooltips_set_tip(tooltip, check_beep, 
			     _("Beep when no hit."),"Private");
//	gtk_box_pack_start (GTK_BOX(hbox)
//			    , check_beep,FALSE,FALSE, 0);
	gtk_table_attach(GTK_TABLE(table), check_beep, 0, 1, 5, 6,
			 xoption, yoption, 10, 10);	

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_beep), bbeep_on_nohit);

	LOG(LOG_DEBUG, "OUT : pref_start_selection()");

	return(vbox);

}
