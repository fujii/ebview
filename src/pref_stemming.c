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

#include "pref_io.h"

GtkWidget *entry_pattern;
GtkWidget *entry_normal;
GtkWidget *check_nohit;
GtkWidget *option_lang;


GtkWidget *stemming_view;

static void check_changed(GtkWidget *widget,gpointer *data){

	LOG(LOG_DEBUG, "IN : check_changed()");

	if(GTK_TOGGLE_BUTTON(widget)->active){
		gtk_widget_set_sensitive(stemming_view, TRUE);
		gtk_widget_set_sensitive(entry_pattern, TRUE);
		gtk_widget_set_sensitive(entry_normal, TRUE);
		gtk_widget_set_sensitive(check_nohit, TRUE);
		bending_correction = TRUE;
	} else {
		gtk_widget_set_sensitive(stemming_view, FALSE);
		gtk_widget_set_sensitive(entry_pattern, FALSE);
		gtk_widget_set_sensitive(entry_normal, FALSE);
		gtk_widget_set_sensitive(check_nohit, FALSE);
		bending_correction = FALSE;
	}

	LOG(LOG_DEBUG, "OUT : check_changed()");
}

static void check_changed2(GtkWidget *widget,gpointer *data){

	LOG(LOG_DEBUG, "IN : check_changed2()");

	if(GTK_TOGGLE_BUTTON(widget)->active){
		bending_only_nohit = TRUE;
	} else {
		bending_only_nohit = FALSE;
	}

	LOG(LOG_DEBUG, "OUT : check_changed2()");
}


static void lang_changed(GtkWidget *widget,gpointer *data){
	gint index;
	
	LOG(LOG_DEBUG, "IN : check_changed()");

	index = gtk_option_menu_get_history(GTK_OPTION_MENU(option_lang));

	switch(index){
	case 0:
		gtk_tree_view_set_model(GTK_TREE_VIEW(stemming_view), 
					GTK_TREE_MODEL(stemming_en_store));
		break;
	case 1:
		gtk_tree_view_set_model(GTK_TREE_VIEW(stemming_view), 
					GTK_TREE_MODEL(stemming_ja_store));
		break;
	}

	LOG(LOG_DEBUG, "OUT : check_changed()");
}

static void remove_entry(GtkWidget *widget,gpointer *data){

	GtkTreeIter iter;
	GtkTreeSelection *selection;

	LOG(LOG_DEBUG, "IN : remove_entry()");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(stemming_view));

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		gtk_list_store_remove(GTK_LIST_STORE(stemming_en_store), &iter);
	}

	LOG(LOG_DEBUG, "OUT : remove_entry()");
}

static void add_entry(GtkWidget *widget,gpointer *data){
	const gchar *pattern;
	const gchar *normal;
	GtkTreeIter  iter;

	LOG(LOG_DEBUG, "IN : add_entry()");

	pattern = gtk_entry_get_text(GTK_ENTRY(entry_pattern));
	normal = gtk_entry_get_text(GTK_ENTRY(entry_normal));
	if((strlen(pattern) == 0) && (strlen(normal) == 0)){
		return;
	}

	gtk_list_store_append(stemming_en_store, &iter);
	gtk_list_store_set(stemming_en_store, &iter,
			   STEMMING_PATTERN_COLUMN, strdup(pattern),
			   STEMMING_NORMAL_COLUMN, strdup(normal),
			   -1);

	LOG(LOG_DEBUG, "OUT : add_entry()");
}


gboolean pref_end_stemming(){
	LOG(LOG_DEBUG, "IN : pref_end_stemming()");

	save_stemming_en();
	save_stemming_ja();

	LOG(LOG_DEBUG, "OUT : pref_end_stemming()");
	return(TRUE);
}

GtkWidget *pref_start_stemming()
{
	GtkWidget *button;
	GtkWidget *hbox1;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *vbox2;
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *scroll;
	GtkWidget *check_ending;

	GtkWidget *menu;
	GtkWidget *menu_item;


	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	LOG(LOG_DEBUG, "IN : pref_start_stemming()");

	hbox1 = gtk_hbox_new(FALSE, 3);
	gtk_widget_set_size_request(hbox1, 240, 300);

	vbox = gtk_vbox_new(FALSE, 3);
	gtk_box_pack_start (GTK_BOX(hbox1)
			    , vbox,TRUE,FALSE, 0);

	check_ending = gtk_check_button_new_with_label(_("Perform stemming"));
	gtk_tooltips_set_tip(tooltip, check_ending, 
			     _("When ending of each words matches the pattern in the list, normal form of the word will also be tried. It takes longer."),"Private");
	gtk_box_pack_start (GTK_BOX(vbox)
			    , check_ending,FALSE,FALSE, 0);
	g_signal_connect(G_OBJECT (check_ending), "clicked",
			 G_CALLBACK(check_changed), NULL);


	check_nohit = gtk_check_button_new_with_label(_("Stemming only when no hit"));
	gtk_tooltips_set_tip(tooltip, check_nohit, 
			     _("Do not perform stemming when original words hit."),"Private");
	gtk_box_pack_start (GTK_BOX(vbox)
			    , check_nohit,FALSE,FALSE, 0);
	g_signal_connect(G_OBJECT (check_nohit), "clicked",
			 G_CALLBACK(check_changed2), NULL);


        // Option menu for language selection

	menu = gtk_menu_new ();
	menu_item = gtk_menu_item_new_with_label (_("English"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

	menu_item = gtk_menu_item_new_with_label (_("Japanese"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

	option_lang = gtk_option_menu_new ();
	gtk_option_menu_set_menu (GTK_OPTION_MENU (option_lang), menu);
	gtk_box_pack_start (GTK_BOX(vbox)
			    , option_lang, FALSE, FALSE, 0);

	g_signal_connect(G_OBJECT(option_lang), "changed",
			 G_CALLBACK(lang_changed), NULL);


	hbox = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start (GTK_BOX(vbox)
			    , hbox,FALSE, FALSE, 0);


	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start (GTK_BOX(hbox)
			    , vbox2, TRUE, TRUE, 0);

	label = gtk_label_new(_("Pattern"));
	gtk_box_pack_start (GTK_BOX(vbox2)
			    , label,FALSE, FALSE, 0);
	entry_pattern = gtk_entry_new();
	gtk_widget_set_size_request(entry_pattern,90,20);
	gtk_box_pack_start (GTK_BOX(vbox2)
			    , entry_pattern, FALSE, FALSE, 0);


	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start (GTK_BOX(hbox)
			    , vbox2, TRUE, TRUE, 0);

	label = gtk_label_new(_("Correction"));
	gtk_box_pack_start (GTK_BOX(vbox2)
			    , label,FALSE, FALSE, 0);
	entry_normal = gtk_entry_new();
	gtk_widget_set_size_request(entry_normal,90, 20);
	gtk_box_pack_start (GTK_BOX(vbox2)
			    , entry_normal, FALSE, FALSE, 0);
	
	button = gtk_button_new_with_label(_("Add"));
	GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX (hbox), button,
			    FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(add_entry), (gpointer)NULL);

	button = gtk_button_new_with_label(_("Remove"));
	GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX (hbox), button,
			    FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(remove_entry), (gpointer)NULL);

	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_box_pack_start (GTK_BOX(vbox)
			    , frame,TRUE, TRUE, 0);
	
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add (GTK_CONTAINER (frame), scroll);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
					GTK_POLICY_NEVER, 
					GTK_POLICY_AUTOMATIC);


	stemming_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(stemming_en_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(stemming_view), TRUE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(stemming_view), TRUE);

	gtk_container_add(GTK_CONTAINER (scroll), stemming_view);

/*
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(NULL,
					  renderer,
					  _("Pattern"), STEMMING_PATTERN_COLUMN,
					  _("Normal"), STEMMING_NORMAL_COLUMN,
					  NULL);
*/
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Pattern"),
					  renderer,
					  "text", STEMMING_PATTERN_COLUMN,
					  NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, 200);
	gtk_tree_view_append_column (GTK_TREE_VIEW (stemming_view), column);

	column = gtk_tree_view_column_new_with_attributes(_("Normal"),
					  renderer,
					  "text", STEMMING_NORMAL_COLUMN,
					  NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, 200);
	gtk_tree_view_append_column (GTK_TREE_VIEW (stemming_view), column);

	gtk_widget_set_sensitive(stemming_view, bending_correction);
	gtk_widget_set_sensitive(entry_pattern, bending_correction);
	gtk_widget_set_sensitive(entry_normal, bending_correction);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_ending), bending_correction);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_nohit), bending_only_nohit);

	LOG(LOG_DEBUG, "OUT : pref_start_stemming()");

	return(hbox1);

}
