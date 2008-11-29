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
#include "headword.h"

static GtkWidget *spin_max_search;
static GtkWidget *check_word_search;

gboolean pref_end_search(GtkWidget *widget,gpointer *data){

	LOG(LOG_DEBUG, "IN : pref_end_search()");

	max_search = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_max_search));

	bword_search_automatic = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_word_search));


	update_tree_view();

	LOG(LOG_DEBUG, "OUT : pref_end_search()");
	return(TRUE);
}

GtkWidget *pref_start_search()
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkObject *adj;
	GtkWidget *table;
	GtkAttachOptions xoption=0, yoption=0;

	LOG(LOG_DEBUG, "IN : pref_start_search()");

	vbox = gtk_vbox_new(FALSE,10);
	gtk_widget_set_size_request(vbox, 300, 200);

	xoption = GTK_SHRINK|GTK_FILL;
	yoption = GTK_SHRINK;

	table = gtk_table_new(3, 5, FALSE);
	gtk_box_pack_start (GTK_BOX(vbox)
			    , table,FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE,10);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);
	gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 0, 1,
			 xoption, yoption, 10, 10);	

	label = gtk_label_new(_("Maximum hits to search"));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX(hbox), 
			    label, FALSE, FALSE, 0);

	adj = gtk_adjustment_new( 100, //value
				  0, // lower
				  1000, //upper
                                  1, // step increment
                                  10,// page_increment,
                                  0.0);
	spin_max_search = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1.0, 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_max_search), max_search );
	gtk_widget_set_size_request(spin_max_search,60,20);
	gtk_table_attach(GTK_TABLE(table), spin_max_search, 1, 2, 0, 1,
			 xoption, yoption, 10, 10);	

	gtk_tooltips_set_tip(tooltip, spin_max_search, 
			     _("Maximum number of hits to be searched.\nIf you increase this number, it takes time to search."),"Private");


	check_word_search = gtk_check_button_new_with_label(_("Perform word search in automatic search"));
	gtk_tooltips_set_tip(tooltip, check_word_search, 
			     _("Perform word search in automatic search."),"Private");

	gtk_table_attach(GTK_TABLE(table), check_word_search, 0, 1, 2, 3,
			 xoption, yoption, 10, 10);	

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_word_search), bword_search_automatic);


	LOG(LOG_DEBUG, "OUT : pref_start_search()");

	return(vbox);

}


