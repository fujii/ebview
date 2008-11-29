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

#include "dialog.h"
#include "dirtree.h"
#include "grep.h"
#include "pref_io.h"

static GtkWidget *dirgroup_view;
static GtkWidget *entry_group_name;
GtkTreeSelection *selection;
static gchar last_dir[1024];

static void add_dirgroup(GtkWidget *widget,gpointer *data)
{

	GtkTreeIter iter;
//	GtkTreeSelection *selection;
	GtkTreeIter child_iter;

	const gchar *title;
	gchar *list;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;

	LOG(LOG_DEBUG, "IN : add_dirgroup()");

//	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(dirgroup_view));

	if (gtk_tree_selection_get_selected(selection, NULL, &iter) == FALSE)
	{
		gtk_list_store_append(dirgroup_store, &child_iter);
//		gtk_list_store_insert_after(dirgroup_store, &child_iter, &iter);
	} else {
		gtk_list_store_append(dirgroup_store, &child_iter);
	}

	title = gtk_entry_get_text(GTK_ENTRY(entry_group_name));

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(dirgroup_view));
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	list = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);


	gtk_list_store_set(dirgroup_store, &child_iter,
			   DIRGROUP_TITLE_COLUMN, title,
			   DIRGROUP_LIST_COLUMN, list,
			   DIRGROUP_ACTIVE_COLUMN, FALSE,
			   -1);

	g_free(list);

	LOG(LOG_DEBUG, "OUT : add_dirgroup()");
}


static void dirgroup_selection_changed(GtkTreeSelection *selection, gpointer data)
{
	GtkTreeIter iter;

	gchar *title;
	gchar *list;
	GtkTextBuffer *buffer;

	LOG(LOG_DEBUG, "IN : dirgroup_selection_changed()");

	if (gtk_tree_selection_get_selected(selection, NULL, &iter) == FALSE)
	{
		LOG(LOG_DEBUG, "OUT : dirgroup_selection_changed()");
		return;
	}

	gtk_tree_model_get(GTK_TREE_MODEL(dirgroup_store), 
			   &iter,
			   DIRGROUP_TITLE_COLUMN, &title,
			   DIRGROUP_LIST_COLUMN, &list,
			   -1);

	if(title != NULL)
		gtk_entry_set_text(GTK_ENTRY(entry_group_name), title);
	else
		gtk_entry_set_text(GTK_ENTRY(entry_group_name), "");

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(dirgroup_view));

	if(list != NULL)
		gtk_text_buffer_set_text(buffer, list, strlen(list));
	else
		gtk_text_buffer_set_text(buffer, "", 0);

	g_free(title);
	g_free(list);

	LOG(LOG_DEBUG, "OUT : dirgroup_selection_changed()");
}

static void change_dirgroup(GtkWidget *widget, gpointer *data)
{

	GtkTreeIter iter;
//	GtkTreeSelection *selection;

	const gchar *title;
	gchar *list;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;


	LOG(LOG_DEBUG, "IN : change_dirgroup()");

//	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(dirgroup_view));

	if (gtk_tree_selection_get_selected(selection, NULL, &iter) == FALSE)
	{
		popup_warning(_("Please select group."));
		LOG(LOG_DEBUG, "OUT : change_dirgroup()");
		return;
	}

	title = gtk_entry_get_text(GTK_ENTRY(entry_group_name));

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(dirgroup_view));
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	list = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

	gtk_list_store_set(dirgroup_store, &iter,
			   DIRGROUP_TITLE_COLUMN, title,
			   DIRGROUP_LIST_COLUMN, list,
			   -1);

	g_free(list);

	LOG(LOG_DEBUG, "OUT : change_dirgroup()");
}

static void remove_item(GtkWidget *widget, gpointer *data)
{

	GtkTreeIter iter;
//	GtkTreeSelection *selection;

	LOG(LOG_DEBUG, "IN : remove_item()");

//	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(dirgroup_view));
	if(selection == NULL)
		return;

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		gtk_list_store_remove(GTK_LIST_STORE(dirgroup_store), &iter);
	} else {
	}

	LOG(LOG_DEBUG, "OUT : remove_item()");
}

static void filesel_ok(GtkWidget *widget, GtkFileSelection *fs)
{
	gchar *dir;
	gchar *text;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;

	LOG(LOG_DEBUG, "IN : filesel_ok()");

	dir = (gchar *)gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));
	strcpy(last_dir, dir);
	dir = fs_to_unicode(dir);

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(dirgroup_view));
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
	if(strlen(text) != 0)
		gtk_text_buffer_insert(buffer, &end, "\n", 1);
	gtk_text_buffer_insert(buffer, &end, dir, strlen(dir));

	g_free(text);
	g_free(dir);

	gtk_grab_remove(GTK_WIDGET(fs));
	gtk_widget_destroy(GTK_WIDGET(fs));

	LOG(LOG_DEBUG, "OUT : filesel_ok()");
}

static void open_filesel(GtkWidget *widget, gpointer *data)
{
	GtkWidget *filesel;

	LOG(LOG_DEBUG, "IN : open_filesel()");

	filesel = gtk_file_selection_new (_("Select directory"));
	gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(filesel));
	g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (filesel)->ok_button),
			  "clicked", G_CALLBACK (filesel_ok), (gpointer) filesel);

	g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (filesel)->cancel_button),
				  "clicked", G_CALLBACK (gtk_widget_destroy),
				  G_OBJECT (filesel));

	if(strcmp(&last_dir[strlen(last_dir) -1], DIR_DELIMITER) != 0)
		strcat(last_dir, DIR_DELIMITER);
	gtk_file_selection_set_filename(GTK_FILE_SELECTION(filesel), last_dir);
	gtk_widget_show(filesel);
	gtk_grab_add(filesel);

	LOG(LOG_DEBUG, "OUT : open_filesel()");
}


gboolean pref_end_dirgroup()
{

	LOG(LOG_DEBUG, "IN : pref_end_dirgroup()");

	update_grep_bar();

	LOG(LOG_DEBUG, "OUT : pref_end_dirgroup()");
	return(TRUE);
}


GtkWidget *pref_start_dirgroup()
{

	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *hbox2;
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *frame;
	GtkWidget *scroll;

	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;


	LOG(LOG_DEBUG, "IN : pref_start_dirgroup()");

	hbox = gtk_hbox_new(TRUE,0);

	gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);

	frame = gtk_frame_new(_("Directory group list"));
	gtk_box_pack_start (GTK_BOX(hbox), frame,TRUE, TRUE, 5);

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox);


	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_box_pack_start (GTK_BOX(vbox)
			    , frame,TRUE, TRUE, 0);
	
	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (frame), scroll);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	dirgroup_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(dirgroup_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(dirgroup_view), FALSE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(dirgroup_view), TRUE);
	gtk_container_add (GTK_CONTAINER (scroll), dirgroup_view);

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(dirgroup_view));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
	g_signal_connect(G_OBJECT(selection), "changed",
			 G_CALLBACK (dirgroup_selection_changed),
			 NULL);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Title", renderer,
							  "text", DIRGROUP_TITLE_COLUMN,
							  NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, 200);
	gtk_tree_view_append_column (GTK_TREE_VIEW (dirgroup_view), column);


//	gtk_tree_view_append_column (GTK_TREE_VIEW (dirgroup_view), column);


	hbox2 = gtk_hbox_new(FALSE,5);
	gtk_container_set_border_width(GTK_CONTAINER(hbox2), 2);
	gtk_box_pack_start(GTK_BOX(vbox),
			    hbox2,FALSE, FALSE, 0);

	frame = gtk_frame_new(_("Detail"));
	gtk_box_pack_start(GTK_BOX(hbox), frame,TRUE, TRUE, 5);

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	label = gtk_label_new(_("Name"));
	gtk_box_pack_start (GTK_BOX (vbox), label,
			    FALSE, FALSE, 0);

	entry_group_name = gtk_entry_new();
	gtk_box_pack_start (GTK_BOX (vbox), entry_group_name,
			    FALSE, FALSE, 0);
	gtk_tooltips_set_tip(tooltip, entry_group_name,
			     _("Enter the name of directory group."),"Private");

	label = gtk_label_new(_("Directory list"));
	gtk_box_pack_start (GTK_BOX (vbox), label,
			    FALSE, FALSE, 0);
	

	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_box_pack_start (GTK_BOX(vbox)
			    , frame,TRUE, TRUE, 0);
	
	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (frame), scroll);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	dirgroup_view = gtk_text_view_new();
//	gtk_widget_set_size_request(dirgroup_view, 200, 200);
	gtk_container_add(GTK_CONTAINER(scroll), dirgroup_view);
	gtk_tooltips_set_tip(tooltip, dirgroup_view,
			     _("Specify directory names one per line. You can specify extension of files that will be searched. For example, \"/some/dir/name,.txt\" searches all files under /some/dir/name which have the extension .txt."),"Private");



	hbox2 = gtk_hbox_new(FALSE,5);
	gtk_container_set_border_width(GTK_CONTAINER(hbox2), 5);
	gtk_box_pack_start(GTK_BOX(vbox),
			   hbox2,FALSE, FALSE, 0);


	button = gtk_button_new_with_label(_("Add"));
	gtk_box_pack_start(GTK_BOX(hbox2),
			   button,FALSE,FALSE, 0);
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(add_dirgroup), (gpointer)button);

	button = gtk_button_new_with_label(_("Change"));
	gtk_box_pack_start(GTK_BOX(hbox2),
			   button,FALSE,FALSE, 0);
	g_signal_connect(G_OBJECT(button), "clicked",
			 G_CALLBACK(change_dirgroup), (gpointer)button);


	button = gtk_button_new_with_label(_("Remove"));
	gtk_box_pack_start(GTK_BOX(hbox2),
			   button,FALSE,FALSE, 0);
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(remove_item), (gpointer)button);


	button = gtk_button_new_with_label(_("Choose.."));
	gtk_box_pack_end(GTK_BOX(hbox2),
			   button,FALSE,FALSE, 0);
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(open_filesel), (gpointer)button);

/*
	label = gtk_label_new(_("Enter name and directories, then push Add button. You can specify multiple directories devided by newline (one directory per line). Drag & Drop to change order."));
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_box_pack_start (GTK_BOX (vbox), label,
			    FALSE, FALSE, 0);
*/

#ifdef __WIN32__	
	strcpy(last_dir, "C:\\");
#else
	strcpy(last_dir, getenv("HOME"));
#endif

	LOG(LOG_DEBUG, "OUT : pref_start_dirgroup()");
	return(hbox);
}




