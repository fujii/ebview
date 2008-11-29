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

#include "websearch.h"
#include "pref_io.h"
#include "dialog.h"
#include "misc.h"


GtkWidget *web_view;
static GtkWidget *entry_group_name;
static GtkWidget *entry_engine_name;
static GtkWidget *entry_engine_home;
static GtkWidget *entry_engine_pre;
static GtkWidget *entry_engine_post;
static GtkWidget *entry_engine_glue;
static GtkWidget *combo_charcode;
//static GtkCTreeNode *current_node=NULL;

static GtkTreeIter last_iter;
static gboolean edited = FALSE;
static gboolean rewinding=FALSE;

GList *web_list = NULL;

extern GtkWidget *web_pane;

void my_gtk_tree_store_swap (GtkTreeStore *tree_store, GtkTreeIter  *a, GtkTreeIter  *b);
static gboolean update_last_engine();

gboolean pref_end_weblist()
{

	LOG(LOG_DEBUG, "IN : pref_end_weblist()");

	if(update_last_engine() == FALSE)
		return(FALSE);

	save_weblist();

	LOG(LOG_DEBUG, "OUT : pref_end_weblist()");
	return(TRUE);
}

static void up_item(GtkWidget *widget, gpointer *data)
{
	GtkTreeIter iter;
	GtkTreeIter prev_iter;
	GtkTreeSelection *selection;
	GtkTreePath* path;

	LOG(LOG_DEBUG, "IN : up_item()");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(web_view));

	if (gtk_tree_selection_get_selected(selection, NULL, &iter) == FALSE)
	{
		popup_warning(_("Please select dictionary."));
		return;
	}

	path = gtk_tree_model_get_path(GTK_TREE_MODEL(web_store), &iter);

	gtk_tree_path_prev(path);
	if(gtk_tree_model_get_iter(GTK_TREE_MODEL(web_store),
				   &prev_iter,
				   path))
	{
//		gtk_tree_store_move_before(web_store, &iter, &prev_iter);
		my_gtk_tree_store_swap(web_store, &iter, &prev_iter);
	}
//
	gtk_tree_path_free(path);

	LOG(LOG_DEBUG, "OUT : up_item()");
	return;
}


static void down_item(GtkWidget *widget,gpointer *data)
{
	GtkTreeIter iter;
	GtkTreeIter next_iter;
	GtkTreeSelection *selection;
	GtkTreePath* path;
	GtkTreePath *next_path;

	LOG(LOG_DEBUG, "IN : down_item()");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(web_view));

	if (gtk_tree_selection_get_selected(selection, NULL, &iter) == FALSE)
	{
		popup_warning(_("Please select dictionary."));
		return;
	}

	path = gtk_tree_model_get_path(GTK_TREE_MODEL(web_store), &iter);
	next_path = gtk_tree_path_copy(path);

	gtk_tree_path_next(next_path);
	if(gtk_tree_model_get_iter(GTK_TREE_MODEL(web_store),
				   &next_iter,
				   next_path))
	{
		my_gtk_tree_store_swap(web_store, &iter, &next_iter);
	}

	gtk_tree_path_free(path);
	gtk_tree_path_free(next_path);

	LOG(LOG_DEBUG, "OUT : down_item()");
	return;
}


static void remove_item(GtkWidget *widget, gpointer *data)
{

	GtkTreeIter iter;
	GtkTreeSelection *selection;

	LOG(LOG_DEBUG, "IN : remove_item()");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(web_view));

	if (gtk_tree_selection_get_selected(selection, NULL, &iter))
	{
		edited = FALSE;
		gtk_tree_store_remove(GTK_TREE_STORE(web_store), &iter);
	}

	LOG(LOG_DEBUG, "OUT : remove_item()");
}


static void add_group(GtkWidget *widget,gpointer *data)
{

	GtkTreeIter iter;
	gchar *name;

	LOG(LOG_DEBUG, "IN : add_group()");

	name = (gchar *)gtk_entry_get_text(GTK_ENTRY(entry_group_name));
	name = g_strdup(name);
	remove_space(name);
	if(strlen(name) == 0){
	  return;
	}
	gtk_tree_store_append(GTK_TREE_STORE(web_store), &iter, NULL);
	gtk_tree_store_set(GTK_TREE_STORE(web_store), &iter,
			   WEB_TYPE_COLUMN, 0,
			   WEB_TITLE_COLUMN, name,
			   -1);

	gtk_entry_set_text(GTK_ENTRY(entry_group_name), "");
	g_free(name);	

	LOG(LOG_DEBUG, "OUT : add_group()");
}

static void add_engine(GtkWidget *widget,gpointer *data)
{

	GtkTreeIter iter;
	GtkTreeSelection *selection;
	GtkTreeIter child_iter;

	gint type;
	const gchar *title;
	const gchar *home;
	const gchar *pre;
	const gchar *post;
	const gchar *glue;
	const gchar *code;

	LOG(LOG_DEBUG, "IN : add_engine()");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(web_view));

	if (gtk_tree_selection_get_selected(selection, NULL, &iter) == FALSE)
	{
		popup_warning(_("Please select group."));
		LOG(LOG_DEBUG, "OUT : add_engine()");
		return;
	}

	title = gtk_entry_get_text(GTK_ENTRY(entry_engine_name));
	home = gtk_entry_get_text(GTK_ENTRY(entry_engine_home));
	pre = gtk_entry_get_text(GTK_ENTRY(entry_engine_pre));
	post = gtk_entry_get_text(GTK_ENTRY(entry_engine_post));
	glue = gtk_entry_get_text(GTK_ENTRY(entry_engine_glue));
	code = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo_charcode)->entry));


	gtk_tree_model_get(GTK_TREE_MODEL(web_store), 
			   &iter, 
			   DICT_TYPE_COLUMN, &type,
			   -1);

	if(type == 0){
		gtk_tree_store_append(web_store, &child_iter, &iter);
	} else {
		gtk_tree_store_insert_after(web_store, &child_iter, NULL, &iter);
	}

	gtk_tree_store_set(web_store, &child_iter,
			   WEB_TYPE_COLUMN, 1,
			   WEB_TITLE_COLUMN, title,
			   WEB_HOME_COLUMN, home,
			   WEB_PRE_COLUMN, pre,
			   WEB_POST_COLUMN, post,
			   WEB_GLUE_COLUMN, glue,
			   WEB_CODE_COLUMN, code,
			   -1);
	
	edited = FALSE;

	LOG(LOG_DEBUG, "OUT : add_engine()");
}


static void web_selection_changed(GtkTreeSelection *selection, gpointer data)
{
	GtkTreeIter iter;

	gint type;
	gchar *title;
	gchar *home;
	gchar *pre;
	gchar *post;
	gchar *glue;
	gchar *code;

	LOG(LOG_DEBUG, "IN : web_selection_changed()");

	if(rewinding == TRUE){
		rewinding = FALSE;
		goto END;
	} else {
		if(update_last_engine() == FALSE)
			goto END;
	}


	if (gtk_tree_selection_get_selected(selection, NULL, &iter) == FALSE)
	{
		//popup_warning(_("Please select group."));
		LOG(LOG_DEBUG, "OUT : web_selection_changed()");
		return;
	}

	gtk_tree_model_get(GTK_TREE_MODEL(web_store), 
			   &iter,
			   WEB_TYPE_COLUMN, &type,
			   WEB_TITLE_COLUMN, &title,
			   WEB_HOME_COLUMN, &home,
			   WEB_PRE_COLUMN, &pre,
			   WEB_POST_COLUMN, &post,
			   WEB_GLUE_COLUMN, &glue,
			   WEB_CODE_COLUMN, &code,
			   -1);

	if(type == 0)
		goto END;

	if(title != NULL)
		gtk_entry_set_text(GTK_ENTRY(entry_engine_name), title);
	else
		gtk_entry_set_text(GTK_ENTRY(entry_engine_name), "");

	if(home != NULL)
		gtk_entry_set_text(GTK_ENTRY(entry_engine_home), home);
	else
		gtk_entry_set_text(GTK_ENTRY(entry_engine_home), "");

	if(pre != NULL)
		gtk_entry_set_text(GTK_ENTRY(entry_engine_pre), pre);
	else
		gtk_entry_set_text(GTK_ENTRY(entry_engine_pre), "");

	if(post != NULL)
		gtk_entry_set_text(GTK_ENTRY(entry_engine_post), post);
	else
		gtk_entry_set_text(GTK_ENTRY(entry_engine_post), "");

	if(glue != NULL)
		gtk_entry_set_text(GTK_ENTRY(entry_engine_glue), glue);
	else
		gtk_entry_set_text(GTK_ENTRY(entry_engine_glue), "");

	if(code != NULL)
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_charcode)->entry), code);
	else
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo_charcode)->entry), "iso-2022-jp");

	gtk_tree_selection_get_selected(selection, NULL, &last_iter);
	edited = TRUE;


 END:
	g_free(title);
	g_free(home);
	g_free(pre);
	g_free(post);
	g_free(glue);
	g_free(code);

	LOG(LOG_DEBUG, "OUT : web_selection_changed()");
}

static gboolean update_last_engine()
{

	GtkTreeSelection *select;

	const gchar *title;
	const gchar *home;
	const gchar *pre;
	const gchar *post;
	const gchar *glue;
	const gchar *code;

	LOG(LOG_DEBUG, "IN : update_last_engine()");

	if(edited != TRUE)
		goto END;
	
	if(popup_active())
		goto END;

	title = gtk_entry_get_text(GTK_ENTRY(entry_engine_name));
	if(!title  || (strlen(title) == 0)) {
		popup_warning(_("Please specify name"));
		goto FAILED;
	}

	home = gtk_entry_get_text(GTK_ENTRY(entry_engine_home));
	pre = gtk_entry_get_text(GTK_ENTRY(entry_engine_pre));
	if(!pre  || (strlen(pre) == 0)) {
		popup_warning(_("Please specify pre string"));
		goto FAILED;
	}

	post = gtk_entry_get_text(GTK_ENTRY(entry_engine_post));
	glue = gtk_entry_get_text(GTK_ENTRY(entry_engine_glue));
	code = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo_charcode)->entry));


	gtk_tree_store_set(web_store, &last_iter,
			   WEB_TITLE_COLUMN, title,
			   WEB_HOME_COLUMN, home,
			   WEB_PRE_COLUMN, pre,
			   WEB_POST_COLUMN, post,
			   WEB_GLUE_COLUMN, glue,
			   WEB_CODE_COLUMN, code,
			   -1);

 END:
	LOG(LOG_DEBUG, "OUT : updaet_last_engine() = TRUE");
	return(TRUE);

 FAILED:
	rewinding = TRUE;
	select = gtk_tree_view_get_selection(GTK_TREE_VIEW(web_view));
	gtk_tree_selection_select_iter(select, &last_iter);

	LOG(LOG_DEBUG, "OUT : update_last_engine() = FALSE");
	return(FALSE);

}

static gboolean drag_data_received(GtkTreeDragDest   *drag_dest,
				   GtkTreePath       *dest,
				   GtkSelectionData  *selection_data)
{
	LOG(LOG_DEBUG, "IN : drag_data_received()");
	edited = FALSE;
	LOG(LOG_DEBUG, "OUT : drag_data_received()");
	return(FALSE);
	
}

GtkWidget *pref_start_weblist()
{

	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *hbox2;
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *frame;
	GtkWidget *scroll;
	GtkWidget *table;
	GtkAttachOptions xoption, yoption;
	GList *charcode_list=NULL;

	GtkSizeGroup *entry_group;

	GtkTreeSelection *select;
	GtkCellRenderer *renderer;


	LOG(LOG_DEBUG, "IN : pref_start_weblist()");

	hbox = gtk_hbox_new(FALSE,0);

	gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);

	frame = gtk_frame_new(_("Search engines"));
	gtk_box_pack_start (GTK_BOX(hbox), frame,TRUE, TRUE, 5);

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox);


	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX(vbox)
			    , scroll,TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);


	web_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(web_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(web_view), FALSE);
	gtk_tree_view_expand_all(GTK_TREE_VIEW(web_view));
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(web_view), TRUE);
	gtk_container_add (GTK_CONTAINER (scroll), web_view);

	select = gtk_tree_view_get_selection(GTK_TREE_VIEW(web_view));
	gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
	g_signal_connect(G_OBJECT(select), "changed",
			 G_CALLBACK (web_selection_changed),
			 NULL);

	g_signal_connect(G_OBJECT(web_view), "drag_data_received",
			 G_CALLBACK(drag_data_received), NULL);


	renderer = gtk_cell_renderer_text_new();
/*
	column = gtk_tree_view_column_new_with_attributes(NULL,
							  renderer,
							  "text", DICT_TITLE_COLUMN,
							  "editable", DICT_EDITABLE_COLUMN,
							  NULL);
*/
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(web_view),
						    -1, "Title", renderer,
						    "text", WEB_TITLE_COLUMN,
					       NULL);
//	gtk_tree_view_append_column (GTK_TREE_VIEW (web_view), column);


	hbox2 = gtk_hbox_new(FALSE,5);
	gtk_container_set_border_width(GTK_CONTAINER(hbox2), 2);
	gtk_box_pack_start(GTK_BOX(vbox),
			    hbox2,FALSE, FALSE, 0);

	label = gtk_label_new(_("Group name"));
	gtk_box_pack_start(GTK_BOX(hbox2),
			   label,FALSE, FALSE, 2);

	entry_group_name = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox2),
			   entry_group_name,TRUE, TRUE, 0);

	button = gtk_button_new_with_label(_("Add"));
	gtk_box_pack_end(GTK_BOX(hbox2),
			   button,FALSE,FALSE, 0);
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(add_group), (gpointer)button);

	hbox2 = gtk_hbox_new(FALSE,5);
	gtk_container_set_border_width(GTK_CONTAINER(hbox2), 2);
	gtk_box_pack_start(GTK_BOX(vbox),
			   hbox2,FALSE, FALSE, 0);

	button = gtk_button_new_with_label(_("Remove"));
	gtk_box_pack_start(GTK_BOX(hbox2),
			   button,FALSE,FALSE, 0);
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(remove_item), (gpointer)button);


	button = gtk_button_new_with_label(_("Up"));
	gtk_box_pack_start(GTK_BOX(hbox2),
			   button,FALSE,FALSE, 0);
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(up_item), (gpointer)button);

	button = gtk_button_new_with_label(_("Down"));
	gtk_box_pack_start(GTK_BOX(hbox2),
			   button,FALSE,FALSE, 0);
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(down_item), (gpointer)button);


	frame = gtk_frame_new(_("Search engine"));
	gtk_box_pack_start(GTK_BOX(hbox), frame,TRUE, TRUE, 5);

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	entry_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	xoption = GTK_EXPAND | GTK_SHRINK;
	yoption = GTK_EXPAND | GTK_SHRINK;

	table = gtk_table_new(6, 2, FALSE);
	gtk_box_pack_start(GTK_BOX(vbox),
			   table,FALSE, FALSE, 0);

	label = gtk_label_new(_("Name"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, 
			 xoption, yoption, 5, 5);

	entry_engine_name = gtk_entry_new();
//	gtk_widget_set_usize(entry_engine_name,250,20);
	gtk_table_attach(GTK_TABLE(table), entry_engine_name, 1, 2, 0, 1,
			 xoption, yoption, 5, 5);	
	gtk_size_group_add_widget (entry_group, entry_engine_name);

	label = gtk_label_new(_("Homepage"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
			 xoption, yoption, 5, 5);

	entry_engine_home = gtk_entry_new();
//	gtk_widget_set_usize(entry_engine_home,250,20);
	gtk_table_attach(GTK_TABLE(table), entry_engine_home, 1, 2, 1, 2,
			 xoption, yoption, 5, 5);
	gtk_size_group_add_widget (entry_group, entry_engine_home);

	label = gtk_label_new(_("Pre string"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3,
			 xoption, yoption, 5, 5);

	entry_engine_pre = gtk_entry_new();
//	gtk_widget_set_usize(entry_engine_pre,250,20);
	gtk_table_attach(GTK_TABLE(table), entry_engine_pre, 1, 2, 2, 3,
			 xoption, yoption, 5, 5);
	gtk_size_group_add_widget (entry_group, entry_engine_pre);

	label = gtk_label_new(_("Post string"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 3, 4,
			 xoption, yoption, 5, 5);

	entry_engine_post = gtk_entry_new();
//	gtk_widget_set_usize(entry_engine_post,250,20);
	gtk_table_attach(GTK_TABLE(table), entry_engine_post, 1, 2, 3, 4,
			 xoption, yoption, 5, 5);
	gtk_size_group_add_widget (entry_group, entry_engine_post);

	label = gtk_label_new(_("Glue string"));
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 4, 5,
			 xoption, yoption, 5, 5);

	entry_engine_glue = gtk_entry_new();
//	gtk_widget_set_usize(entry_engine_glue,250,20);
	gtk_table_attach(GTK_TABLE(table), entry_engine_glue, 1, 2, 4, 5,
			 xoption, yoption, 5, 5);
	gtk_size_group_add_widget (entry_group, entry_engine_glue);

	label = gtk_label_new(_("Character Code"));
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 5, 6,
			 xoption, yoption, 5, 5);

	combo_charcode = gtk_combo_new();
//	gtk_widget_set_usize(GTK_WIDGET(combo_charcode), 250, 20);
	gtk_editable_set_editable(GTK_EDITABLE(GTK_COMBO(combo_charcode)->entry), FALSE);
	gtk_table_attach(GTK_TABLE(table), combo_charcode, 1, 2, 5, 6,
			 xoption, yoption, 5, 5);
	gtk_size_group_add_widget (entry_group, combo_charcode);

	charcode_list = g_list_append(charcode_list, "euc-jp");
	charcode_list = g_list_append(charcode_list, "shift_jis");
	charcode_list = g_list_append(charcode_list, "iso-2022-jp");
	charcode_list = g_list_append(charcode_list, "utf-8");
	charcode_list = g_list_append(charcode_list, "ascii");
	gtk_combo_set_popdown_strings(GTK_COMBO(combo_charcode), 
				       charcode_list) ;


	hbox2 = gtk_hbox_new(FALSE,5);
	gtk_container_set_border_width(GTK_CONTAINER(hbox2), 5);
	gtk_box_pack_start(GTK_BOX(vbox),
			   hbox2,FALSE, FALSE, 0);


	button = gtk_button_new_with_label(_("Add"));
	gtk_box_pack_start(GTK_BOX(hbox2),
			   button,FALSE,FALSE, 0);
	g_signal_connect(G_OBJECT (button), "clicked",
			 G_CALLBACK(add_engine), (gpointer)button);

/*
	button = gtk_button_new_with_label(_("Change"));
	gtk_box_pack_start(GTK_BOX(hbox2),
			   button,FALSE,FALSE, 0);
	g_signal_connect(G_OBJECT(button), "clicked",
			 G_CALLBACK(change_engine), (gpointer)button);
*/

	LOG(LOG_DEBUG, "OUT : pref_start_weblist()");
	return(hbox);

}

