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

static GtkWidget *filter_view;
static GtkWidget *spin_additional_line;
static GtkWidget *spin_additional_char;
static GtkWidget *spin_cache_size;


gboolean pref_end_grep()
{

	LOG(LOG_DEBUG, "IN : pref_end_grep()");

	additional_lines = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_additional_line));
	additional_chars = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_additional_char));

	LOG(LOG_DEBUG, "OUT : pref_end_grep()");
	return(TRUE);
}

static void add_filter(GtkWidget *widget,gpointer *data)
{

	GtkTreeIter iter;

	LOG(LOG_DEBUG, "IN : add_filter()");

	gtk_list_store_append(GTK_LIST_STORE(filter_store), &iter);
	gtk_list_store_set(GTK_LIST_STORE(filter_store), &iter,
			   FILTER_EXT_COLUMN, ".ext",
			   FILTER_FILTER_COMMAND_COLUMN, "xxxtotext %f %o",
			   FILTER_OPEN_COMMAND_COLUMN, "openxxx %f",
			   FILTER_EDITABLE_COLUMN, TRUE,
			   -1);

	LOG(LOG_DEBUG, "OUT : add_filter()");
}

static void remove_filter(GtkWidget *widget, gpointer *data)
{

	GtkTreeIter iter;
	GtkTreeSelection *selection;

	LOG(LOG_DEBUG, "IN : remove_filter()");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(filter_view));

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		gtk_list_store_remove(GTK_LIST_STORE(filter_store), &iter);
	}

	LOG(LOG_DEBUG, "OUT : remove_filter()");
}

static void cell_edited(GtkCellRendererText *cell,
			const gchar         *path_string,
			const gchar         *new_text,
			gpointer             data)
{

	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
	GtkTreeIter iter;
	gint column;
	gchar *old_text;

	column = GPOINTER_TO_INT(g_object_get_data(G_OBJECT (cell), "column"));

	gtk_tree_model_get_iter(GTK_TREE_MODEL(filter_store), &iter, path);


	gtk_tree_model_get(GTK_TREE_MODEL(filter_store), &iter, column, &old_text, -1);
	g_free (old_text);
	gtk_list_store_set(GTK_LIST_STORE(filter_store), &iter,
			   column, new_text,
			   -1);

	gtk_tree_path_free (path);
}




GtkWidget *pref_start_grep()
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkObject *adj;
	GtkWidget *table;
	GtkAttachOptions xoption=0, yoption=0;

	LOG(LOG_DEBUG, "IN : pref_start_grep()");

	vbox = gtk_vbox_new(FALSE,10);
	gtk_widget_set_size_request(vbox, 300, 200);

	xoption = GTK_SHRINK|GTK_FILL;
	yoption = GTK_SHRINK;

	table = gtk_table_new(3, 4, FALSE);
	gtk_box_pack_start (GTK_BOX(vbox)
			    , table,FALSE, FALSE, 0);


	hbox = gtk_hbox_new(FALSE,10);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);
	gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 1, 2,
			 xoption, yoption, 10, 10);	

	label = gtk_label_new(_("Additional Lines To Display"));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX(hbox), 
			    label, FALSE, FALSE, 0);

	adj = gtk_adjustment_new( 5, //value
				  0, // lower
				  1000, //upper
                                  1, // step increment
                                  10,// page_increment,
                                  0.0);
	spin_additional_line = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1.0, 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_additional_line), additional_lines);
	gtk_widget_set_size_request(spin_additional_line,60,20);
	gtk_table_attach(GTK_TABLE(table), spin_additional_line, 1, 2, 1, 2,
			 xoption, yoption, 10, 10);	

	gtk_tooltips_set_tip(tooltip, spin_additional_line, 
			     _("In addition to matched line, additional lines will be shown in contents."),"Private");

	hbox = gtk_hbox_new(FALSE,10);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);
	gtk_table_attach(GTK_TABLE(table), hbox, 0, 1, 2, 3,
			 xoption, yoption, 10, 10);	

	label = gtk_label_new(_("Additional Chars To Display"));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX(hbox), 
			    label, FALSE, FALSE, 0);
	

	adj = gtk_adjustment_new( 16, //value
				  1, // lower
				  100, //upper
                                  1, // step increment
                                  10,// page_increment,
                                  0.0);
	spin_additional_char = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1.0, 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_additional_char), additional_chars);
	gtk_widget_set_size_request(spin_additional_char,60,20);
	gtk_table_attach(GTK_TABLE(table), spin_additional_char, 1, 2, 2, 3,
			 xoption, yoption, 10, 10);	

	gtk_tooltips_set_tip(tooltip, spin_additional_char, 
			     _("When matched line is too long, several characters around keyword will be shown in heading."), "Private");



	LOG(LOG_DEBUG, "OUT : pref_start_grep()");

	return(vbox);

}

gboolean pref_end_filter()
{

	LOG(LOG_DEBUG, "IN : pref_end_grep()");

	save_filter();

	LOG(LOG_DEBUG, "OUT : pref_end_grep()");
	return(TRUE);
}

GtkWidget *pref_start_filter()
{
	GtkWidget *button;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *scroll;

	GtkCellRenderer *renderer;


	LOG(LOG_DEBUG, "IN : pref_start_dictgroup()");


	// Left half

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 2);

	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_box_pack_start (GTK_BOX(vbox)
			    , frame,TRUE, TRUE, 0);
	

	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (frame), scroll);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	filter_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(filter_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(filter_view), TRUE);
	gtk_tree_view_expand_all(GTK_TREE_VIEW(filter_view));
	gtk_container_add (GTK_CONTAINER (scroll), filter_view);

	renderer = gtk_cell_renderer_text_new();
	g_signal_connect(G_OBJECT(renderer), "edited",
			 G_CALLBACK(cell_edited), NULL);
	g_object_set_data(G_OBJECT(renderer), "column", (gint *)FILTER_EXT_COLUMN);

	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(filter_view),
						     -1, _("Extension"), renderer,
						     "text", FILTER_EXT_COLUMN,
						     "editable", FILTER_EDITABLE_COLUMN,
						     NULL);

	renderer = gtk_cell_renderer_text_new();
	g_signal_connect(G_OBJECT(renderer), "edited",
			 G_CALLBACK(cell_edited), NULL);
	g_object_set_data(G_OBJECT(renderer), "column", (gint *)FILTER_FILTER_COMMAND_COLUMN);

	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(filter_view),
						     -1, _("Filter Command"), renderer,
						     "text", FILTER_FILTER_COMMAND_COLUMN,
						     "editable", FILTER_EDITABLE_COLUMN,
						     NULL);

	renderer = gtk_cell_renderer_text_new();
	g_signal_connect(G_OBJECT(renderer), "edited",
			 G_CALLBACK(cell_edited), NULL);
	g_object_set_data(G_OBJECT(renderer), "column", (gint *)FILTER_OPEN_COMMAND_COLUMN);

	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(filter_view),
						     -1, _("Open Command"), renderer,
						     "text", FILTER_OPEN_COMMAND_COLUMN,
						     "editable", FILTER_EDITABLE_COLUMN,
						     NULL);

	// Enable resizing all column
	{
		gint i;
		GtkTreeViewColumn *column;

		for(i=0;;i++){
			column = gtk_tree_view_get_column(GTK_TREE_VIEW(filter_view), i);
			if(column == NULL)
				break;
			gtk_tree_view_column_set_resizable(column, TRUE);
		}
	}



	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox),
			   hbox,FALSE, FALSE, 2);

	button = gtk_button_new_with_label(_("Add"));
	gtk_box_pack_start(GTK_BOX(hbox),
			   button,FALSE,FALSE, 2);
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(add_filter), (gpointer)button);


	button = gtk_button_new_with_label(_("Remove"));
	gtk_box_pack_start(GTK_BOX(hbox),
			   button,FALSE,FALSE, 2);
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(remove_filter), (gpointer)button);


	LOG(LOG_DEBUG, "OUT : pref_start_dictgroup()");

	return(vbox);

}

static void remove_recursive(gchar *dirname)
{
	GDir *dir;
	const gchar *name;
	gchar fullpath[512];
	gint r;

	LOG(LOG_DEBUG, "IN : remove_recursive(%s)", dirname);
	
	if((dir = g_dir_open(dirname, 0, NULL)) == NULL){
		LOG(LOG_ERROR, "Failed to open directory %s", dirname);
		LOG(LOG_DEBUG, "OUT : list_file_recursive()");
		return;
	}

	while((name = g_dir_read_name(dir)) != NULL){
		if(strcmp(dirname,"/")==0){
			sprintf(fullpath,"/%s",name);
		} else {
			sprintf(fullpath,"%s%s%s",dirname, DIR_DELIMITER, name);
		}

		if(g_file_test(fullpath, G_FILE_TEST_IS_REGULAR) == TRUE){
			r = unlink(fullpath);
			if(r != 0){
				LOG(LOG_ERROR, "unlink : %s", strerror(errno));
			}
		} else if(g_file_test(fullpath, G_FILE_TEST_IS_DIR) == TRUE){
			remove_recursive(fullpath);
			r = rmdir(fullpath);
			if(r != 0){
				LOG(LOG_ERROR, "rmdir: %s", strerror(errno));
			}
		}
	}
	g_dir_close(dir);

	LOG(LOG_DEBUG, "OUT : remove_recursive()");
}

static gint clear_cache(GtkWidget *widget,gpointer *data)
{

	LOG(LOG_DEBUG, "IN : clear_cache()");

	remove_recursive(cache_dir);

	LOG(LOG_DEBUG, "OUT : clear_cache()");
	return(TRUE);
}

gboolean pref_end_cache()
{

	LOG(LOG_DEBUG, "IN : pref_end_cache()");


	cache_size = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_cache_size));

	LOG(LOG_DEBUG, "OUT : pref_end_cache()");
	return(TRUE);
}


GtkWidget *pref_start_cache()
{
	GtkWidget *vbox;
	GtkWidget *label;
	GtkObject *adj;
	GtkWidget *table;
	GtkWidget *button;
	GtkAttachOptions xoption=0, yoption=0;

	LOG(LOG_DEBUG, "IN : pref_start_cache()");

	vbox = gtk_vbox_new(FALSE,10);
	gtk_widget_set_size_request(vbox, 300, 200);

	table = gtk_table_new(3, 1, FALSE);
	gtk_box_pack_start (GTK_BOX(vbox)
			    , table,FALSE, FALSE, 0);

	label = gtk_label_new(_("Maximum Cache Size (MB)"));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
			 xoption, yoption, 10, 10);	
	

	adj = gtk_adjustment_new( 50, //value
				  1, // lower
				  1000, //upper
                                  1, // step increment
                                  10,// page_increment,
                                  0.0);
	spin_cache_size = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1.0, 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_cache_size), cache_size);
	gtk_widget_set_size_request(spin_cache_size,60,20);
	gtk_table_attach(GTK_TABLE(table), spin_cache_size, 1, 2, 0, 1,
			 xoption, yoption, 10, 10);	

	gtk_tooltips_set_tip(tooltip, spin_cache_size, 
			     _("Specify maximum cache size in MB."), "Private");

	button = gtk_button_new_with_label(_("Clear Cache"));
	gtk_table_attach(GTK_TABLE(table), button, 2, 3, 0, 1,
			 xoption, yoption, 10, 10);	
	g_signal_connect(G_OBJECT(button), "clicked",
			 G_CALLBACK(clear_cache), NULL);

	LOG(LOG_DEBUG, "OUT : pref_start_grep()");

	return(vbox);

}

